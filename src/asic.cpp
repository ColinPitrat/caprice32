#include "asic.h"
#include "log.h"
#include "cap32.h"
#include "SDL.h"

byte *pbRegisterPage;
extern SDL_Color colours[32];
extern t_GateArray GateArray;
extern t_CRTC CRTC;
extern t_CPC CPC;
extern SDL_Surface *back_surface;
extern dword dwXScale;

bool asic_locked = true;
byte asic_sprites[16][16][16];
int16_t asic_sprites_x[16];
int16_t asic_sprites_y[16];
short int asic_sprites_mag_x[16];
short int asic_sprites_mag_y[16];
double asic_colours[32][3];

void asic_poke_lock_sequence(byte val) {
   static const byte lockSeq[] = { 0x00, 0x00, 0xff, 0x77, 0xb3, 0x51, 0xa8, 0xd4, 0x62, 0x39, 0x9c, 0x46, 0x2b, 0x15, 0x8a, 0xcd };
   static const int lockSeqLength = sizeof(lockSeq)/sizeof(lockSeq[0]);
   static int lockPos = 0;
   LOG("ASIC poked with val=" << std::hex << static_cast<int>(val) << std::dec << " lockPos = " << lockPos);
   // Lock sequence can only start after a non zero value
   if (lockPos == 0) {
      if (val > 0) {
         lockPos = 1;
      }
   } else {
      if(lockPos < lockSeqLength) {
         if (val == lockSeq[lockPos]) {
            lockPos++;
         } else {
            lockPos++;
            // If the lock sequence is matched except for the last byte, it means lock
            if (lockPos == lockSeqLength) {
               LOG("ASIC locked");
               asic_locked = true;
            }
            if (val == 0) {
               if (lockPos == 3) {
                  // We had two 0 in a row, we still need a full sequence
                  lockPos = 0;
               } else {
                  // We had a non 0 then a 0, we're now waiting for 0xFF
                  lockPos = 2;
               }
            } else {
               // We had a non 0, we're now waiting for 0
               lockPos = 1;
            }
         }
      } else {
         // Full sequence matched and an additional value was written, it means unlock
         if (lockPos == lockSeqLength) {
            LOG("ASIC unlocked");
            asic_locked = false;
            lockPos = (val == 0) ? 0 : 1;
         }
      }
   }
}

static inline unsigned short decode_magnification(byte val) {
   byte mag = (val & 0x3);
   if (mag == 3) mag = 4;
   return mag;
}

// Return true if byte should be written in memory
bool asic_register_page_write(word addr, byte val) {
   if (addr < 0x4000 || addr > 0x7FFF) {
      return true;
   }
   LOG("ASIC register page write: addr=" << std::hex << addr << ", val=" << static_cast<int>(val) << std::dec);
   // TODO:double check the writes (more cases with mirroring / write only ?)
   if (addr >= 0x4000 && addr < 0x5000) {
      int id = ((addr & 0xF00) >> 8);
      int y = ((addr & 0xF0) >> 4);
      int x = (addr & 0xF);
      byte color = (val & 0xF);
      if(color > 0) {
         color += 16;
      }
      asic_sprites[id][x][y] = color;
      //LOG("Received sprite data for sprite " << id << ": x=" << x << ", y=" << y << ", color=" << static_cast<int>(val));
   } else if (addr >= 0x6000 && addr < 0x607D) {
      int id = ((addr - 0x6000) >> 3);
      int type = (addr & 0x7);
      switch (type) {
         case 0:
            // X position
            asic_sprites_x[id] = (asic_sprites_x[id] & 0xFF00) | val;
            //LOG("Received sprite X for sprite " << id << " x=" << asic_sprites_x[id]);
            // Mirrored in RAM image 4 bytes after
            pbRegisterPage[(addr & 0x3FFF) + 4] = val;
            break;
         case 1:
            // X position
            asic_sprites_x[id] = (asic_sprites_x[id] & 0x00FF) | (val << 8);
            //LOG("Received sprite X for sprite " << id << " x=" << asic_sprites_x[id]);
            // Mirrored in RAM image 4 bytes after
            pbRegisterPage[(addr & 0x3FFF) + 4] = val;
            break;
         case 2:
            // Y position
            asic_sprites_y[id] = ((asic_sprites_y[id] & 0xFF00) | val);
            //LOG("Received sprite Y for sprite " << id << " y=" << asic_sprites_y[id]);
            // Mirrored in RAM image 4 bytes after
            pbRegisterPage[(addr & 0x3FFF) + 4] = val;
            break;
         case 3:
            // Y position
            asic_sprites_y[id] = ((asic_sprites_y[id] & 0x00FF) | (val << 8));
            //LOG("Received sprite Y for sprite " << id << " y=" << asic_sprites_y[id]);
            // Affect RAM image
            // Mirrored in RAM image 4 bytes after
            pbRegisterPage[(addr & 0x3FFF) + 4] = val;
            break;
         case 4:
            // Magnification
            asic_sprites_mag_x[id] = decode_magnification(val >> 2);
            asic_sprites_mag_y[id] = decode_magnification(val);
            // Write-only: does not affect pbRegisterPage
            return false;
         default:
            LOG("Received sprite operation of unsupported type: " << type << " addr=" << std::hex << addr << " - val=" << static_cast<int>(val) << std::dec);
            break;
      }
   } else if (addr >= 0x6400 && addr < 0x6440) {
      int colour = (addr & 0x3F) >> 1;
      if ((addr % 2) == 1) {
         double green = static_cast<double>(val & 0x0F)/16;
         //LOG("Received color operation: color " << colour << " has green = " << green);
         asic_colours[colour][1] = green;
         pbRegisterPage[(addr & 0x3FFF)] = (val & 0x0F);
         // TODO: find a cleaner way to do this - this is a copy paste from "Set ink value" in cap32.cpp
      } else {
         double red   = static_cast<double>((val & 0xF0) >> 4)/16;
         double blue  = static_cast<double>(val & 0x0F)/16;
         //LOG("Received color operation: color " << colour << " has red = " << red << " and blue = " << blue);
         asic_colours[colour][0] = red;
         asic_colours[colour][2] = blue;
         pbRegisterPage[(addr & 0x3FFF)] = val;
      }
      // TODO: deduplicate with code in video_set_palette + make it work in monochrome
               dword red = static_cast<dword>(asic_colours[colour][0] * (CPC.scr_intensity / 10.0) * 255);
               if (red > 255) { // limit to the maximum
                  red = 255;
               }
               dword green = static_cast<dword>(asic_colours[colour][1] * (CPC.scr_intensity / 10.0) * 255);
               if (green > 255) {
                  green = 255;
               }
               dword blue = static_cast<dword>(asic_colours[colour][2] * (CPC.scr_intensity / 10.0) * 255);
               if (blue > 255) {
                  blue = 255;
               }
      GateArray.palette[colour] = SDL_MapRGB(back_surface->format, red, green, blue);
/*
      if (colour < 2) {
         byte r = (static_cast<dword>(colours[GateArray.ink_values[0]].r) + static_cast<dword>(colours[GateArray.ink_values[1]].r)) >> 1;
         byte g = (static_cast<dword>(colours[GateArray.ink_values[0]].g) + static_cast<dword>(colours[GateArray.ink_values[1]].g)) >> 1;
         byte b = (static_cast<dword>(colours[GateArray.ink_values[0]].b) + static_cast<dword>(colours[GateArray.ink_values[1]].b)) >> 1;
         GateArray.palette[18] = SDL_MapRGB(back_surface->format, r, g, b); // update the mode 2 'anti-aliasing' colour
      }
*/
      return false;
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
      LOG("Received analog input stuff");
   } else if (addr >= 0x6C00 && addr < 0x6C10) {
      LOG("Received DMA stuff");
   } else {
      //LOG("Received unused write at " << std::hex << addr << " - val: " << static_cast<int>(val) << std::dec);
   }
   return true;
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
   int bpp = surface->format->BytesPerPixel;
   /* Here p is the address to the pixel we want to set */
   Uint8 *p = static_cast<Uint8 *>(surface->pixels) + y * surface->pitch + x * bpp;

   switch(bpp) {
      case 1:
         *p = pixel;
         break;

      case 2:
         *reinterpret_cast<Uint16 *>(p) = pixel;
         break;

      case 3:
         if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
         } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
         }
         break;

      case 4:
         *reinterpret_cast<Uint32 *>(p) = pixel;
         break;
   }
}

// TODO: all this could be replaced by pre-computed SDL sprites
void asic_draw_sprites() {
   const int borderWidth = 64;
   const int borderHeight = 40;
   const int screenWidth = 640 + borderWidth;
   const int screenHeight = 200 + borderHeight;
   // For each sprite
   for(int i = 15; i >= 0; i--) {
      int sx = asic_sprites_x[i];
      int mx = asic_sprites_mag_x[i];
      // If some part of the sprite is visible (horizontal check)
      if(mx > 0 && (sx + 64*mx) >= borderWidth && sx <= screenWidth) {
         int sy = asic_sprites_y[i];
         int my = asic_sprites_mag_y[i];
         // If some part of the sprite is visible (vertical check)
         if(my > 0 && (sy + 64*my) >= borderHeight && sy <= screenHeight) {
            sx += borderWidth;
            sy += borderHeight;
            // For each column of the sprite
            for(int x = 0; x < 16; x++) {
               if(sx + (x*mx) <= borderWidth) {
                  continue;
               }
               if(sx + (x*mx) >= screenWidth) {
                  break;
               }
               // For each line of the sprite
               for(int y = 0; y < 16; y++) {
                  if(sy + (y*my) <= borderHeight) {
                     continue;
                  }
                  if(sy + (y*my) >= screenHeight) {
                     break;
                  }
                  // Draw pixel
                  byte p = asic_sprites[i][x][y];
                  if(p) {
                     Uint32 pixel = GateArray.palette[p];
                     for(int dx = 0; dx < mx; dx++) {
                        for(int dy = 0; dy < my * static_cast<int>(dwXScale); dy++) {
                           putpixel(back_surface, (sx+(x*mx)+dx) * dwXScale / 2, (sy+(y*my)) * dwXScale + dy, pixel);
                        }
                     }
                  }
               }
            }
         }
      }
   }
}
