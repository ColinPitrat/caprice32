#include "asic.h"
#include "log.h"
#include "cap32.h"
#include "SDL.h"

byte *pbRegisterPage;
extern double colours_rgb[32][3];
extern SDL_Color colours[32];
extern t_GateArray GateArray;
extern SDL_Surface *back_surface;

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
      //LOG("Received sprite data");
   } else if (addr >= 0x6000 && addr < 0x607D) {
      //LOG("Received sprite operation");
   } else if (addr >= 0x6400 && addr < 0x6440) {
      int colour = (addr - 0x6400) / 2;
      if ((addr % 2) == 1) {
         double green = static_cast<double>(val & 0x0F)/16;
         //LOG("Received color operation: color " << colour << " has green = " << green);
         colours_rgb[colour][1] = green;
         // TODO: find a cleaner way to do this - this is a copy paste from "Set ink value" in cap32.cpp
      } else {
         double red   = static_cast<double>((val & 0xF0) >> 4)/16;
         double blue  = static_cast<double>(val & 0x0F)/16;
         //LOG("Received color operation: color " << colour << " has red = " << red << " and blue = " << blue);
         colours_rgb[colour][0] = red;
         colours_rgb[colour][2] = blue;
      }
      video_set_palette();
      if (colour < 17) {
         GateArray.ink_values[colour] = colour;
         GateArray.palette[colour] = SDL_MapRGB(back_surface->format, colours[colour].r, colours[colour].g, colours[colour].b);
         if (colour < 2) {
            byte r = (static_cast<dword>(colours[GateArray.ink_values[0]].r) + static_cast<dword>(colours[GateArray.ink_values[1]].r)) >> 1;
            byte g = (static_cast<dword>(colours[GateArray.ink_values[0]].g) + static_cast<dword>(colours[GateArray.ink_values[1]].g)) >> 1;
            byte b = (static_cast<dword>(colours[GateArray.ink_values[0]].b) + static_cast<dword>(colours[GateArray.ink_values[1]].b)) >> 1;
            GateArray.palette[18] = SDL_MapRGB(back_surface->format, r, g, b); // update the mode 2 'anti-aliasing' colour
         }
      }
   } else if (addr >= 0x6800 && addr < 0x6806) {
      //LOG("Received raster interrupt stuff: " << std::hex << addr << " - val: " << static_cast<int>(val) << std::dec);
   } else if (addr >= 0x6808 && addr < 0x6810) {
      //LOG("Received analog input stuff");
   } else if (addr >= 0x6C00 && addr < 0x6C10) {
      //LOG("Received DMA stuff");
   } else {
      //LOG("Received unused write at " << std::hex << addr << " - val: " << static_cast<int>(val) << std::dec);
   }
}
