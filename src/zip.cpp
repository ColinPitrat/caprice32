#include "zip.h"

#include <string.h>
#include <strings.h>
#include <zlib.h>
#include "errors.h"

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
    char *pchStrPtr;
    dword dwOffset;

    zi->iFiles = 0;
    if ((pfileObject = fopen(zi->filename.c_str(), "rb")) == nullptr) {
      return ERR_FILE_NOT_FOUND;
    }

    wCentralDirEntries = 0;
    wCentralDirSize = 0;
    dwCentralDirPosition = 0;
    do {
      lFilePosition -= 256; // move backwards through ZIP file
      fseek(pfileObject, lFilePosition, SEEK_END);
      if (fread(pbGPBuffer, 256, 1, pfileObject) == 0) {
        fclose(pfileObject);
        return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
      }
      pbPtr = pbGPBuffer + (256 - 22); // pointer to end of central directory (under ideal conditions)
      while (pbPtr != (byte *)pbGPBuffer) {
        if (*(dword *)pbPtr == 0x06054b50) { // check for end of central directory signature
          wCentralDirEntries = *(word *)(pbPtr + 10);
          wCentralDirSize = *(word *)(pbPtr + 12);
          dwCentralDirPosition = *(dword *)(pbPtr + 16);
          break;
        }
        pbPtr--; // move backwards through buffer
      }
    } while (wCentralDirEntries == 0);
    if (wCentralDirSize == 0) {
      fclose(pfileObject);
      return ERR_FILE_BAD_ZIP; // exit if no central directory was found
    }
    fseek(pfileObject, dwCentralDirPosition, SEEK_SET);
    if (fread(pbGPBuffer, wCentralDirSize, 1, pfileObject) == 0) {
      fclose(pfileObject);
      return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
    }

    pbPtr = pbGPBuffer;
    if (zi->pchFileNames) {
      free(zi->pchFileNames); // dealloc old string table
    }
    zi->pchFileNames = (char *)malloc(wCentralDirSize); // approximate space needed by using the central directory size
    pchStrPtr = zi->pchFileNames;

    for (n = wCentralDirEntries; n; n--) {
      wFilenameLength = *(word *)(pbPtr + 28);
      dwOffset = *(dword *)(pbPtr + 42);
      dwNextEntry = wFilenameLength + *(word *)(pbPtr + 30) + *(word *)(pbPtr + 32);
      pbPtr += 46;
      const char *pchThisExtension = zi->extensions.c_str();
      while (*pchThisExtension != '\0') { // loop for all extensions to be checked
        if (strncasecmp((char *)pbPtr + (wFilenameLength - 4), pchThisExtension, 4) == 0) {
          strncpy(pchStrPtr, (char *)pbPtr, wFilenameLength); // copy filename from zip directory
          pchStrPtr[wFilenameLength] = 0; // zero terminate string
          pchStrPtr += wFilenameLength+1;
          zi->dwOffset = dwOffset;
          *(dword *)pchStrPtr = dwOffset; // associate offset with string
          pchStrPtr += 4;
          zi->iFiles++;
          break;
        }
        pchThisExtension += 4; // advance to next extension
      }
      pbPtr += dwNextEntry;
    }
    fclose(pfileObject);

    if (zi->iFiles == 0) { // no files found?
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
      return ERR_FILE_UNZIP_FAILED; // couldn't create output file
    }
    pfileIn = fopen(zi.filename.c_str(), "rb"); // open ZIP file for reading
    fseek(pfileIn, dwOffset, SEEK_SET); // move file pointer to beginning of data block
    size_t rc;
    if((rc = fread(pbGPBuffer, 30, 1, pfileIn)) != 1) { // read local header
      fclose(pfileIn);
      fclose(*pfileOut);
      return ERR_FILE_UNZIP_FAILED;
    }
    dwSize = *(dword *)(pbGPBuffer + 18); // length of compressed data
    dwOffset += 30 + *(word *)(pbGPBuffer + 26) + *(word *)(pbGPBuffer + 28);
    fseek(pfileIn, dwOffset, SEEK_SET); // move file pointer to start of compressed data

    pbInputBuffer = pbGPBuffer; // space for compressed data chunck
    pbOutputBuffer = pbInputBuffer + 16384; // space for uncompressed data chunck
    z.zalloc = (alloc_func)nullptr;
    z.zfree = (free_func)nullptr;
    z.opaque = (voidpf)nullptr;
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
        if (iCount) { // save data to file if output buffer is full
          fwrite(pbOutputBuffer, iCount, 1, *pfileOut);
        }
      }
      dwSize -= 16384; // advance to next chunck
    } while ((dwSize > 0) && (iStatus == Z_OK)) ; // loop until done
    if (iStatus != Z_STREAM_END) {
      return ERR_FILE_UNZIP_FAILED; // abort on error
    }
    iStatus = inflateEnd(&z); // clean up
    fclose(pfileIn);
    fseek(*pfileOut, 0, SEEK_SET);

    return 0; // data was successfully decompressed
  }
}
