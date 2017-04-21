#include "cartridge.h"
#include "types.h"
#include "errors.h"
#include "log.h"
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <algorithm>
#include <memory>

const uint32_t CARTRIDGE_NB_PAGES = 32;
const uint32_t CARTRIDGE_PAGE_SIZE = 16*1024;
const uint32_t CARTRIDGE_MAX_SIZE = CARTRIDGE_NB_PAGES*CARTRIDGE_PAGE_SIZE;

byte *pbCartridgeImage = nullptr;
byte *pbCartridgePages[CARTRIDGE_NB_PAGES] = { nullptr };

extern byte* pbROMlo;

void cpr_eject ()
{
   delete[] pbCartridgeImage;
   pbCartridgeImage = nullptr;
   for(uint32_t i = 0; i < CARTRIDGE_NB_PAGES; ++i) {
      pbCartridgePages[i] = nullptr;
   }
}

int cartridge_init()
{
   pbCartridgeImage = new byte [CARTRIDGE_MAX_SIZE]; // attempt to allocate the general purpose buffer
   if (pbCartridgeImage == nullptr) {
      return ERR_OUT_OF_MEMORY;
   }
   for(uint32_t i = 0; i < CARTRIDGE_NB_PAGES; ++i) {
      pbCartridgePages[i] = &pbCartridgeImage[i*CARTRIDGE_PAGE_SIZE];
   }
   return 0;
}

int cpr_load (const std::string &filename)
{
   LOG_DEBUG("cpr_load " << filename);
   FILE *pfile;
   if ((pfile = fopen(filename.c_str(), "rb")) == nullptr) {
      LOG_DEBUG("File not found: " << filename);
      return ERR_FILE_NOT_FOUND;
   }

   int iRetCode = cpr_load(pfile);
   fclose(pfile);

   return iRetCode;
}

uint32_t extractChunkSize(byte *pChunk)
{
   return static_cast<uint32_t>(pChunk[4]) +
          (static_cast<uint32_t>(pChunk[5]) << 8) +
          (static_cast<uint32_t>(pChunk[6]) << 16) +
          (static_cast<uint32_t>(pChunk[7]) << 24);
}

int cpr_load (FILE *pfile)
{
   const int CPR_HEADER_SIZE = 12;
   const int CPR_CHUNK_ID_SIZE = 4;
   const int CPR_CHUNK_HEADER_SIZE = 8;

   cpr_eject();
   int rc = cartridge_init();
   if (rc != 0) {
      return rc;
   }

   std::unique_ptr<byte[]> tmpBuffer(new byte[CARTRIDGE_MAX_SIZE]);
   byte *pbTmpBuffer = tmpBuffer.get();

   // Check RIFF header
   if(fread(pbTmpBuffer, CPR_HEADER_SIZE, 1, pfile) != 1) { // read RIFF header
      LOG_DEBUG("Cartridge file less than " << CPR_HEADER_SIZE << " bytes long !");
      return ERR_CPR_INVALID;
   }
   if (memcmp(pbTmpBuffer, "RIFF", 4) != 0) { // RIFF file
      LOG_DEBUG("Cartridge file is not a RIFF file");
      return ERR_CPR_INVALID;
   }
   if (memcmp(pbTmpBuffer + 8, "AMS!", 4) != 0) { // CPR file
      LOG_DEBUG("Cartridge file is not a CPR file");
      return ERR_CPR_INVALID;
   }
   uint32_t totalSize = extractChunkSize(pbTmpBuffer);
   LOG_DEBUG("CPR size: " << totalSize)

   // Extract all chunks
   uint32_t offset = CPR_HEADER_SIZE;
   uint32_t cartridgeOffset = 0;
   while(offset < totalSize) {
      if(fread(pbTmpBuffer, CPR_CHUNK_HEADER_SIZE, 1, pfile) != 1) { // read chunk header
         LOG_DEBUG("Failed reading chunk header");
         return ERR_CPR_INVALID;
      }
      offset += CPR_CHUNK_HEADER_SIZE;

      byte chunkId[CPR_CHUNK_ID_SIZE+1];
      memcpy(chunkId, pbTmpBuffer, CPR_CHUNK_ID_SIZE);
      chunkId[CPR_CHUNK_ID_SIZE] = '\0';

      uint32_t chunkSize = extractChunkSize(pbTmpBuffer);
      LOG_DEBUG("Chunk '" << chunkId << "' at offset " << offset << " of size " << chunkSize);

      // Normal chunk size is 16kB
      // If smaller, it must be filled with 0 up to this limit
      // If bigger, what is after must be ignored
      uint32_t chunkKept = std::min(chunkSize, CARTRIDGE_PAGE_SIZE);
      // If chunk size is not even, there's a pad byte at the end of it
      if (chunkKept % 2 != 0) {
         chunkKept++;
      }
      // A chunk can be empty (observed on some CPR files)
      if(chunkKept > 0) {
         if(fread(&pbCartridgeImage[cartridgeOffset], chunkKept, 1, pfile) != 1) { // read chunk content
            LOG_DEBUG("Failed reading chunk content");
            return ERR_CPR_INVALID;
         }
         if(chunkKept < CARTRIDGE_PAGE_SIZE) {
            // TODO: use the chunkId to identify the cartridge page to set (cbXX with XX between 00 and 31)
            // This would require intializing the whole to 0 before instead of filling what remains at the end
            // Not sure if there are some CPR with unordered pages but this seems to be allowed in theory
            memset(&pbCartridgeImage[cartridgeOffset+chunkKept], 0, CARTRIDGE_PAGE_SIZE-chunkKept);
         } else if(chunkKept < chunkSize) {
            LOG_DEBUG("This chunk is bigger than the max allowed size !!!");
            if(fread(pbTmpBuffer, chunkSize-chunkKept, 1, pfile) != 1) { // read excessive chunk content
               LOG_DEBUG("Failed reading chunk content");
               return ERR_CPR_INVALID;
            }
         }
         cartridgeOffset += CARTRIDGE_PAGE_SIZE;
         offset += chunkSize;
      }
   }
   LOG_DEBUG("Final offset: " << offset);
   LOG_DEBUG("Final cartridge offset: " << cartridgeOffset);
   memset(&pbCartridgeImage[cartridgeOffset], 0, CARTRIDGE_MAX_SIZE-cartridgeOffset);
   pbROMlo = &pbCartridgeImage[0];
   return 0;
}
