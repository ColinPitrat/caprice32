#include "asic.h"
#include "log.h"
#include "cap32.h"
#include "SDL.h"

byte *pbRegisterPage;
extern double colours_rgb[32][3];
extern SDL_Color colours[32];
extern t_GateArray GateArray;
extern t_CRTC CRTC;
extern SDL_Surface *back_surface;

bool asic_locked = true;
byte asic_sprites[16][16][16];
int asic_sprites_x[16];
int asic_sprites_y[16];
short int asic_sprites_mag_x[16];
short int asic_sprites_mag_y[16];

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
            lockPos = (val == 0) ? -1 : 0;
         }
      } else {
         lockPos++;
         // If the lock sequence is matched except for the last byte, it means lock
         if (lockPos == sizeof(lockSeq)/sizeof(lockSeq[0])) {
            LOG("ASIC locked");
            asic_locked = true;
         }
         lockPos = (val == 0) ? -1 : 0;
      }
   }
}

static inline unsigned short decode_magnification(byte val) {
   byte mag = (val & 0x3);
   if (mag == 3) mag = 4;
   return mag;
}

void asic_register_page_write(word addr, byte val) {
   if (addr < 0x4000 || addr > 0x7FFF) {
      return;
   }
   if (addr >= 0x4000 && addr < 0x5000) {
      int id = ((addr - 0x4000) >> 8);
      int y = ((addr & 0xF0) >> 4);
      int x = (addr & 0xF);
      byte color = (val & 0xF);
      if(color > 0) {
         color += 16;
      }
      asic_sprites[id][x][y] = color;
      //LOG("Received sprite data for sprite " << id << ": x=" << x << ", y=" << y << ", color=" << static_cast<int>(color));
   } else if (addr >= 0x6000 && addr < 0x607D) {
      int id = ((addr - 0x6000) >> 3);
      int type = (addr & 0x7);
      switch (type) {
         case 0:
            // X position
            asic_sprites_x[id] = (asic_sprites_x[id] & 0xFF00) | val;
            //LOG("Received sprite X for sprite " << id << " x=" << asic_sprites_x[id]);
            break;
         case 1:
            // X position
            asic_sprites_x[id] = (asic_sprites_x[id] & 0x00FF) | (val << 8);
            //LOG("Received sprite X for sprite " << id << " x=" << asic_sprites_x[id]);
            break;
         case 2:
            // Y position
            asic_sprites_y[id] = ((asic_sprites_y[id] & 0xFF00) | val);
            //LOG("Received sprite Y for sprite " << id << " y=" << asic_sprites_y[id]);
            break;
         case 3:
            // Y position
            asic_sprites_y[id] = ((asic_sprites_y[id] & 0x00FF) | (val << 8));
            //LOG("Received sprite Y for sprite " << id << " y=" << asic_sprites_y[id]);
            break;
         case 4:
            // Magnification
            asic_sprites_mag_x[id] = decode_magnification(val >> 2);
            asic_sprites_mag_y[id] = decode_magnification(val);
            //LOG("Received sprite magnification for sprite " << id << " mx=" << asic_sprites_mag_x[id] << ", my=" << asic_sprites_mag_y[id]);
            break;
         default:
            //LOG("Received sprite operation of unsupported type: " << type << " addr=" << std::hex << addr << " - val=" << static_cast<int>(val) << std::dec);
            break;
      }
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
      GateArray.ink_values[colour] = colour;
      GateArray.palette[colour] = SDL_MapRGB(back_surface->format, colours[colour].r, colours[colour].g, colours[colour].b);
/*
      if (colour < 2) {
         byte r = (static_cast<dword>(colours[GateArray.ink_values[0]].r) + static_cast<dword>(colours[GateArray.ink_values[1]].r)) >> 1;
         byte g = (static_cast<dword>(colours[GateArray.ink_values[0]].g) + static_cast<dword>(colours[GateArray.ink_values[1]].g)) >> 1;
         byte b = (static_cast<dword>(colours[GateArray.ink_values[0]].b) + static_cast<dword>(colours[GateArray.ink_values[1]].b)) >> 1;
         GateArray.palette[18] = SDL_MapRGB(back_surface->format, r, g, b); // update the mode 2 'anti-aliasing' colour
      }
*/
   } else if (addr >= 0x6800 && addr < 0x6806) {
      if (addr == 0x6800) {
         //LOG("Received programmable raster interrupt scan line: " << static_cast<int>(val));
         CRTC.interrupt_sl = val;
      } else if (addr == 0x6801) {
         LOG("Received scan line for split: " << static_cast<int>(val));
         CRTC.split_sl = val;
      } else if (addr == 0x6802) {
         CRTC.split_addr &= 0x00FF;
         CRTC.split_addr |= (val << 8);
         LOG("Received address for split: " << std::hex << CRTC.split_addr << std::dec);
      } else if (addr == 0x6803) {
         CRTC.split_addr &= 0x3F00;
         CRTC.split_addr |= val;
         LOG("Received address for split: " << std::hex << CRTC.split_addr << std::dec);
      } else if (addr == 0x6804) {
         LOG("Received soft scroll control: " << static_cast<int>(val));
      } else if (addr == 0x6805) {
         LOG("Received interrupt vector: " << static_cast<int>(val));
      }
   } else if (addr >= 0x6808 && addr < 0x6810) {
      //LOG("Received analog input stuff");
   } else if (addr >= 0x6C00 && addr < 0x6C10) {
      //LOG("Received DMA stuff");
   } else {
      //LOG("Received unused write at " << std::hex << addr << " - val: " << static_cast<int>(val) << std::dec);
   }
   pbRegisterPage[addr - 0x4000] = val;
}
