#include "asic.h"
#include "log.h"

byte *pbRegisterPage;

bool asic_locked = true;

void asic_poke_lock_sequence(byte val) {
   static const byte lockSeq[] = { 0x00, 0xff, 0x77, 0xb3, 0x51, 0xa8, 0xd4, 0x62, 0x39, 0x9c, 0x46, 0x2b, 0x15, 0x8a, 0xcd };
   static int lockPos = -1;
   // Lock sequence can only start after a non zero value
   if (lockPos == -1) {
      if (val > 0) {
         lockPos = 0;
      }
   } else {
      if (val == lockSeq[lockPos]) {
         LOG("Received " << std::hex << static_cast<int>(val) << std::dec);
         lockPos++;
         if (lockPos == sizeof(lockSeq)/sizeof(lockSeq[0])) {
            LOG("ASIC unlocked");
            asic_locked = false;
            lockPos = 0;
         }
      } else {
         // If the lock sequence is matched except for the last byte, it means lock
         if (lockPos == sizeof(lockSeq)/sizeof(lockSeq[0])) {
            LOG("ASIC locked");
            asic_locked = true;
         }
         lockPos = 0;
      }
   }
}

void asic_register_page_write(word addr, byte val) {
   if (addr >= 0x4000 && addr < 0x5000) {
      LOG("Received sprite data");
   } else if (addr >= 0x6000 && addr < 0x607D) {
      LOG("Received sprite operation");
   } else if (addr >= 0x6400 && addr < 0x6440) {
      LOG("Received color operation");
   } else if (addr >= 0x6800 && addr < 0x6806) {
      LOG("Received raster interrupt stuff: " << std::hex << addr << " - val: " << static_cast<int>(val) << std::dec);
   } else if (addr >= 0x6808 && addr < 0x6810) {
      LOG("Received analog input stuff");
   } else if (addr >= 0x6C00 && addr < 0x6C10) {
      LOG("Received DMA stuff");
   } else {
      LOG("Received unused write at " << std::hex << addr << " - val: " << static_cast<int>(val) << std::dec);
   }
}
