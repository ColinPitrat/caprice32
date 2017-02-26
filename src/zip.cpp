#include "zip.h"

#include <string.h>
#include <strings.h>
#include <zlib.h>
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
      LOG_DEBUG("File not found or not readable: " << zi->filename);
      return ERR_FILE_NOT_FOUND;
    }

    wCentralDirEntries = 0;
    wCentralDirSize = 0;
    dwCentralDirPosition = 0;
    do {
      lFilePosition -= 256; // move backwards through ZIP file
      if (fseek(pfileObject, lFilePosition, SEEK_END) != 0) {
        fclose(pfileObject);
        LOG_DEBUG("Couldn't read zip file: " << zi->filename);
        return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
      };
      if (fread(pbGPBuffer, 256, 1, pfileObject) == 0) {
        fclose(pfileObject);
        LOG_DEBUG("Couldn't read zip file: " << zi->filename);
        return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
      }
      pbPtr = pbGPBuffer + (256 - 22); // pointer to end of central directory (under ideal conditions)
      while (pbPtr != static_cast<byte *>(pbGPBuffer)) {
        if (*reinterpret_cast<dword *>(pbPtr) == 0x06054b50) { // check for end of central directory signature
          wCentralDirEntries = *reinterpret_cast<word *>(pbPtr + 10);
          wCentralDirSize = *reinterpret_cast<word *>(pbPtr + 12);
          dwCentralDirPosition = *reinterpret_cast<dword *>(pbPtr + 16);
          break;
        }
        pbPtr--; // move backwards through buffer
      }
    } while (wCentralDirEntries == 0);
    if (wCentralDirSize == 0) {
      fclose(pfileObject);
      LOG_DEBUG("Couldn't read zip file (no central directory): " << zi->filename);
      return ERR_FILE_BAD_ZIP; // exit if no central directory was found
    }
    if (fseek(pfileObject, dwCentralDirPosition, SEEK_SET) != 0) {
      fclose(pfileObject);
      LOG_DEBUG("Couldn't read zip file: " << zi->filename);
      return ERR_FILE_BAD_ZIP; // exit if seeking to the central directory failed
    };
    if (fread(pbGPBuffer, wCentralDirSize, 1, pfileObject) == 0) {
      fclose(pfileObject);
      LOG_DEBUG("Couldn't read zip file: " << zi->filename);
      return ERR_FILE_BAD_ZIP; // exit if reading the central directory failed
    }

    pbPtr = pbGPBuffer;

    for (n = wCentralDirEntries; n; n--) {
      wFilenameLength = *reinterpret_cast<word *>(pbPtr + 28);
      dwOffset = *reinterpret_cast<dword *>(pbPtr + 42);
      dwNextEntry = wFilenameLength + *reinterpret_cast<word *>(pbPtr + 30) + *reinterpret_cast<word *>(pbPtr + 32);
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
      LOG_DEBUG("Empty zip file: " << zi->filename);
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

    *pfileOut = tmpfile();
    if (*pfileOut == nullptr) {
      LOG_DEBUG("Couldn't unzip file: Couldn't create temporary file.");
      return ERR_FILE_UNZIP_FAILED; // couldn't create output file
    }
    pfileIn = fopen(zi.filename.c_str(), "rb"); // open ZIP file for reading
    if (pfileIn == nullptr) {
      LOG_DEBUG("Couldn't open zip file for reading: " << zi.filename);
      return ERR_FILE_UNZIP_FAILED; // couldn't open input file
    }
    if (fseek(pfileIn, dwOffset, SEEK_SET) != 0) {  // move file pointer to beginning of data block
      LOG_DEBUG("Couldn't read zip file: " << zi.filename);
      fclose(pfileIn);
      fclose(*pfileOut);
      return ERR_FILE_UNZIP_FAILED;
    };
    size_t rc;
    if((rc = fread(pbGPBuffer, 30, 1, pfileIn)) != 1) { // read local header
      LOG_DEBUG("Couldn't read zip file: " << zi.filename);
      fclose(pfileIn);
      fclose(*pfileOut);
      return ERR_FILE_UNZIP_FAILED;
    }
    dwSize = *reinterpret_cast<dword *>(pbGPBuffer + 18); // length of compressed data
    dwOffset += 30 + *reinterpret_cast<word *>(pbGPBuffer + 26) + *reinterpret_cast<word *>(pbGPBuffer + 28);
    if (fseek(pfileIn, dwOffset, SEEK_SET) != 0) {  // move file pointer to start of compressed data
      LOG_DEBUG("Couldn't read zip file: " << zi.filename);
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
            LOG_DEBUG("Couldn't unzip file: Couldn't write to output file:");
            fclose(pfileIn);
            fclose(*pfileOut);
            return ERR_FILE_UNZIP_FAILED;
          }
        }
      }
      dwSize -= 16384; // advance to next chunck
    } while ((dwSize > 0) && (iStatus == Z_OK)) ; // loop until done
    if (iStatus != Z_STREAM_END) {
      LOG_DEBUG("Couldn't unzip file: " << zi.filename << " (" << iStatus << ")");
      return ERR_FILE_UNZIP_FAILED; // abort on error
    }
    iStatus = inflateEnd(&z); // clean up
    fclose(pfileIn);
    fseek(*pfileOut, 0, SEEK_SET);

    return 0; // data was successfully decompressed
  }
}
