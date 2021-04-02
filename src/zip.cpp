#include "zip.h"

#include <cstring>
#include <strings.h>
#include <zlib.h>
#include <errno.h>
#include <cstdio>
#include <string>
#include "errors.h"
#include "log.h"

// TODO(cpitrat): refactoring
namespace zip
{
  int dir (t_zip_info *zi)
  {
    FILE *pfileObject;
    byte pbGPBuffer[32768];
    int n;
    long lFilePosition = 0;
    dword dwCentralDirPosition, dwNextEntry;
    word wCentralDirEntries, wCentralDirSize, wFilenameLength;
    byte *pbPtr;
    dword dwOffset;

    if ((pfileObject = fopen(zi->filename.c_str(), "rb")) == nullptr) {
      LOG_ERROR("File not found or not readable: " << zi->filename);
      return ERR_FILE_NOT_FOUND;
    }

    wCentralDirEntries = 0;
    wCentralDirSize = 0;
    dwCentralDirPosition = 0;
    do {
      lFilePosition -= 256; // move backwards through ZIP file
      if (fseek(pfileObject, lFilePosition, SEEK_END) != 0) {
        fclose(pfileObject);
        LOG_ERROR("Couldn't read zip file: " << zi->filename);
        return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
      };
      if (fread(pbGPBuffer, 256, 1, pfileObject) == 0) {
        fclose(pfileObject);
        LOG_ERROR("Couldn't read zip file: " << zi->filename);
        return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
      }
      pbPtr = pbGPBuffer + (256 - 22); // pointer to end of central directory (under ideal conditions)
      while (pbPtr != static_cast<byte *>(pbGPBuffer)) {
        if (*pbPtr == 0x50 && *(pbPtr+1) == 0x4b && *(pbPtr+2) == 0x05 && *(pbPtr+3) == 0x06) { // check for end of central directory signature
          wCentralDirEntries = (*(pbPtr+11) << 8) + *(pbPtr+10);
          wCentralDirSize = (*(pbPtr+13) << 8) + *(pbPtr+12);
          dwCentralDirPosition = (*(pbPtr+19) << 24) + (*(pbPtr+18) << 16) + (*(pbPtr+17) << 8) + *(pbPtr+16);
          break;
        }
        pbPtr--; // move backwards through buffer
      }
    } while (wCentralDirEntries == 0);
    if (wCentralDirSize == 0) {
      fclose(pfileObject);
      LOG_ERROR("Couldn't read zip file (no central directory): " << zi->filename);
      return ERR_FILE_BAD_ZIP; // exit if no central directory was found
    }
    if (fseek(pfileObject, dwCentralDirPosition, SEEK_SET) != 0) {
      fclose(pfileObject);
      LOG_ERROR("Couldn't read zip file: " << zi->filename);
      return ERR_FILE_BAD_ZIP; // exit if seeking to the central directory failed
    };
    if (fread(pbGPBuffer, wCentralDirSize, 1, pfileObject) == 0) {
      fclose(pfileObject);
      LOG_ERROR("Couldn't read zip file: " << zi->filename);
      return ERR_FILE_BAD_ZIP; // exit if reading the central directory failed
    }

    pbPtr = pbGPBuffer;

    for (n = wCentralDirEntries; n; n--) {
      wFilenameLength = (*(pbPtr+29) << 8) + *(pbPtr+28);
      dwOffset = (*(pbPtr+45) << 24) + (*(pbPtr+44) << 16) + (*(pbPtr+43) << 8) + *(pbPtr+42);
      dwNextEntry = wFilenameLength + (*(pbPtr+31) << 8) + *(pbPtr+30) + (*(pbPtr + 33) << 8) + *(pbPtr + 32);
      pbPtr += 46;
      const char *pchThisExtension = zi->extensions.c_str();
      while (*pchThisExtension != '\0') { // loop for all extensions to be checked
        if (strncasecmp(reinterpret_cast<char*>(pbPtr) + (wFilenameLength - 4), pchThisExtension, 4) == 0) {
          std::string filename(reinterpret_cast<char*>(pbPtr), wFilenameLength);
          zi->filesOffsets.push_back({filename, dwOffset});
          zi->dwOffset = dwOffset;
          break;
        }
        pchThisExtension += 4; // advance to next extension
      }
      pbPtr += dwNextEntry;
    }
    fclose(pfileObject);

    if (zi->filesOffsets.empty()) { // no files found?
      LOG_ERROR("Empty zip file: " << zi->filename);
      return ERR_FILE_EMPTY_ZIP;
    }

    return 0; // operation completed successfully
  }

  int extract(const t_zip_info& zi, FILE **pfileOut)
  {
    int iStatus;
    unsigned int iCount;
    byte pbGPBuffer[32768];
    dword dwSize;
    byte *pbInputBuffer, *pbOutputBuffer;
    FILE *pfileIn;
    z_stream z;
    dword dwOffset = zi.dwOffset;

#ifdef WINDOWS
    // Windows version of tmpfile is broken by design as it tries to create the temporary file in the root directory.
    // The "official" recommendation is to use the yet borken tempnam/fopen combination.
    // https://msdn.microsoft.com/en-us/library/x8x7sakw.aspx
    char *tmpFilePath = tempnam(".", "cap32_tmp_");
    if (tmpFilePath == nullptr) {
      LOG_ERROR("Couldn't unzip file: Couldn't generate temporary file name: " << strerror(errno));
      return ERR_FILE_UNZIP_FAILED; // couldn't create output file
    }
    LOG_DEBUG("Using temporary file: " << tmpFilePath);
    *pfileOut = fopen(tmpFilePath, "w+b");
#else
    *pfileOut = tmpfile();
#endif
    if (*pfileOut == nullptr) {
      LOG_ERROR("Couldn't unzip file: Couldn't create temporary file: " << strerror(errno));
      return ERR_FILE_UNZIP_FAILED; // couldn't create output file
    }
    pfileIn = fopen(zi.filename.c_str(), "rb"); // open ZIP file for reading
    if (pfileIn == nullptr) {
      LOG_ERROR("Couldn't open zip file for reading: " << zi.filename);
      return ERR_FILE_UNZIP_FAILED; // couldn't open input file
    }
    if (fseek(pfileIn, dwOffset, SEEK_SET) != 0) {  // move file pointer to beginning of data block
      LOG_ERROR("Couldn't read zip file: " << zi.filename);
      fclose(pfileIn);
      fclose(*pfileOut);
      return ERR_FILE_UNZIP_FAILED;
    };
    size_t rc;
    if((rc = fread(pbGPBuffer, 30, 1, pfileIn)) != 1) { // read local header
      LOG_ERROR("Couldn't read zip file: " << zi.filename);
      fclose(pfileIn);
      fclose(*pfileOut);
      return ERR_FILE_UNZIP_FAILED;
    }
    dwSize = *reinterpret_cast<dword *>(pbGPBuffer + 18); // length of compressed data
    dwOffset += 30 + *reinterpret_cast<word *>(pbGPBuffer + 26) + *reinterpret_cast<word *>(pbGPBuffer + 28);
    if (fseek(pfileIn, dwOffset, SEEK_SET) != 0) {  // move file pointer to start of compressed data
      LOG_ERROR("Couldn't read zip file: " << zi.filename);
      fclose(pfileIn);
      fclose(*pfileOut);
      return ERR_FILE_UNZIP_FAILED;
    }

    pbInputBuffer = pbGPBuffer; // space for compressed data chunck
    pbOutputBuffer = pbInputBuffer + 16384; // space for uncompressed data chunck
    z.zalloc = nullptr;
    z.zfree = nullptr;
    z.opaque = nullptr;
    iStatus = inflateInit2(&z, -MAX_WBITS); // init zlib stream (no header)
    do {
      z.next_in = pbInputBuffer;
      if (dwSize > 16384) { // limit input size to max 16K or remaining bytes
        z.avail_in = 16384;
      } else {
        z.avail_in = dwSize;
      }
      z.avail_in = fread(pbInputBuffer, 1, z.avail_in, pfileIn); // load compressed data chunck from ZIP file
      while ((z.avail_in) && (iStatus == Z_OK)) { // loop until all data has been processed
        z.next_out = pbOutputBuffer;
        z.avail_out = 16384;
        iStatus = inflate(&z, Z_NO_FLUSH); // decompress data
        iCount = 16384 - z.avail_out;
        if (iCount) { // save data to file if some is available
          if (fwrite(pbOutputBuffer, iCount, 1, *pfileOut) != 1) {
            LOG_ERROR("Couldn't unzip file: Couldn't write to output file:");
            fclose(pfileIn);
            fclose(*pfileOut);
            return ERR_FILE_UNZIP_FAILED;
          }
        }
      }
      dwSize -= 16384; // advance to next chunck
    } while ((dwSize > 0) && (iStatus == Z_OK)) ; // loop until done
    if (iStatus != Z_STREAM_END && iStatus != Z_OK) {
      LOG_ERROR("Couldn't unzip file: " << zi.filename << " (" << iStatus << ")");
      return ERR_FILE_UNZIP_FAILED; // abort on error
    }
    iStatus = inflateEnd(&z); // clean up
    fclose(pfileIn);
    fseek(*pfileOut, 0, SEEK_SET);

    return 0; // data was successfully decompressed
  }
}
