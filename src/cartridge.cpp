#include "cartridge.h"
#include "types.h"
#include "errors.h"
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <algorithm>

// TODO: avoid double definition, find cleaner solution
#ifdef DEBUG
#define LOG(x) std::cout << __FILE__ << ":" << __LINE__ << " - " << x << std::endl;
#else
#define LOG(x)
#endif

byte *pbCartridgeImage = nullptr;
extern byte* pbGPBuffer;

void cpr_eject (void)
{
   delete[] pbCartridgeImage;
   pbCartridgeImage = nullptr;
}

int cpr_load (const std::string &filename)
{
   LOG("cpr_load " << filename);
   FILE *pfile;
   if ((pfile = fopen(filename.c_str(), "rb")) == nullptr) {
      LOG("File not found: " << filename);
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
   const uint32_t CPR_MAX_CHUNK_SIZE = 16*1024;
   const uint32_t CPR_MAX_NB_CHUNKS = 32;
   const uint32_t CPR_MAX_CARTRIDGE_SIZE = CPR_MAX_NB_CHUNKS*CPR_MAX_CHUNK_SIZE;

   cpr_eject();

   pbCartridgeImage = new byte [CPR_MAX_CARTRIDGE_SIZE]; // attempt to allocate the general purpose buffer
   if (pbCartridgeImage == nullptr) {
      return ERR_OUT_OF_MEMORY;
   }
   // Check RIFF header
   if(fread(pbGPBuffer, CPR_HEADER_SIZE, 1, pfile) != 1) { // read RIFF header
      LOG("Cartridge file less than " << CPR_HEADER_SIZE << " bytes long !");
      return ERR_CPR_INVALID;
   }
   byte *pbPtr = pbGPBuffer;
   if (memcmp(pbPtr, "RIFF", 4) != 0) { // RIFF file
      LOG("Cartridge file is not a RIFF file");
      return ERR_CPR_INVALID;
   }
   if (memcmp(pbPtr + 8, "AMS!", 4) != 0) { // CPR file
      LOG("Cartridge file is not a CPR file");
      return ERR_CPR_INVALID;
   }
   uint32_t totalSize = extractChunkSize(pbPtr);
   LOG("CPR size: " << totalSize)

   // Extract all chunks
   uint32_t offset = CPR_HEADER_SIZE;
   uint32_t cartridgeOffset = 0;
   while(offset < totalSize) {
      if(fread(pbGPBuffer, CPR_CHUNK_HEADER_SIZE, 1, pfile) != 1) { // read chunk header
         LOG("Failed reading chunk header");
         return ERR_CPR_INVALID;
      }
      offset += CPR_CHUNK_HEADER_SIZE;

      byte chunkId[CPR_CHUNK_ID_SIZE+1];
      memcpy(chunkId, pbGPBuffer, CPR_CHUNK_ID_SIZE);
      chunkId[CPR_CHUNK_ID_SIZE] = '\0';

      uint32_t chunkSize = extractChunkSize(pbGPBuffer);
      LOG("Chunk '" << chunkId << "' at offset " << offset << " of size " << chunkSize);

      // Normal chunk size is 16kB
      // If smaller, it must be filled with 0 up to this limit
      // If bigger, what is after must be ignored
      uint32_t chunkKept = std::min(chunkSize, CPR_MAX_CHUNK_SIZE);
      // If chunk size is not even, there's a pad byte at the end of it
      if (chunkKept % 2 != 0) {
         chunkKept++;
      }
      // A chunk can be empty (observed on some CPR files)
      if(chunkKept > 0) {
         if(fread(&pbCartridgeImage[cartridgeOffset], chunkKept, 1, pfile) != 1) { // read chunk content
            LOG("Failed reading chunk content");
            return ERR_CPR_INVALID;
         }
         if(chunkKept < CPR_MAX_CHUNK_SIZE) {
            // TODO: use the chunkId to identify the cartridge page to set (cbXX with XX between 00 and 31)
            // This would require intializing the whole to 0 before instead of filling what remains at the end
            // Not sure if there are some CPR with unordered pages but this seems to be allowed in theory
            memset(&pbCartridgeImage[cartridgeOffset+chunkKept], 0, CPR_MAX_CHUNK_SIZE-chunkKept);
         } else if(chunkKept < chunkSize) {
            LOG("This chunk is bigger than the max allowed size !!!");
            if(fread(pbGPBuffer, chunkSize-chunkKept, 1, pfile) != 1) { // read excessive chunk content
               LOG("Failed reading chunk content");
               return ERR_CPR_INVALID;
            }
         }
         cartridgeOffset += CPR_MAX_CHUNK_SIZE;
         offset += chunkSize;
      }
   }
   LOG("Final offset: " << offset);
   LOG("Final cartridge offset: " << cartridgeOffset);
   memset(&pbCartridgeImage[cartridgeOffset], 0, CPR_MAX_CARTRIDGE_SIZE-cartridgeOffset);
   return 0;
}
