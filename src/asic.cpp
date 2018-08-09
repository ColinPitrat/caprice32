#include "asic.h"
#include "log.h"
#include "cap32.h"
#include "SDL.h"
#include "crtc.h"

byte *pbRegisterPage;
extern SDL_Color colours[32];
extern t_GateArray GateArray;
extern t_CRTC CRTC;
extern t_CPC CPC;
extern t_PSG PSG;
extern SDL_Surface *back_surface;
extern dword dwXScale;
extern byte *membank_config[8][4];
extern byte *membank_write[4];

asic_t asic;
double asic_colours[32][3];

void asic_reset() {
  asic.locked = true;

  asic.extend_border = false;
  asic.hscroll = 0;
  asic.vscroll = 0;

  for(int i = 0; i < 16; i++) {
    asic.sprites_x[i] = asic.sprites_y[i] = asic.sprites_mag_x[i] = asic.sprites_mag_y[i] = 0;
    for(int j = 0; j < 16; j++) {
      for(int k = 0; k < 16; k++) {
        asic.sprites[i][j][k] = 0;
      }
    }
  }

  asic.raster_interrupt = false;
  asic.interrupt_vector = 1;

  for(auto &channel : asic.dma.ch) {
    channel.source_address = 0;
    channel.loop_address = 0;
    channel.prescaler = 0;
    channel.enabled = false;
    channel.interrupt = false;
    channel.pause_ticks = 0;
    channel.tick_cycles = 0;
    channel.loops = 0;
  }
}

void asic_poke_lock_sequence(byte val) {
   static const byte lockSeq[] = { 0x00, 0x00, 0xff, 0x77, 0xb3, 0x51, 0xa8, 0xd4, 0x62, 0x39, 0x9c, 0x46, 0x2b, 0x15, 0x8a, 0xcd };
   static const int lockSeqLength = sizeof(lockSeq)/sizeof(lockSeq[0]);
   LOG_DEBUG("ASIC poked with val=" << std::hex << static_cast<int>(val) << std::dec << " lockPos = " << asic.lockSeqPos);
   // Lock sequence can only start after a non zero value
   if (asic.lockSeqPos == 0) {
      if (val > 0) {
         asic.lockSeqPos = 1;
      }
   } else {
      if(asic.lockSeqPos < lockSeqLength) {
         if (val == lockSeq[asic.lockSeqPos]) {
            asic.lockSeqPos++;
         } else {
            asic.lockSeqPos++;
            // If the lock sequence is matched except for the last byte, it means lock
            if (asic.lockSeqPos == lockSeqLength) {
               LOG_DEBUG("ASIC locked");
               asic.locked = true;
            }
            if (val == 0) {
              // We're now waiting for 0xFF
              asic.lockSeqPos = 2;
            } else {
               // We had a non 0, we're now waiting for 0
               asic.lockSeqPos = 1;
            }
         }
      } else {
         // Full sequence matched and an additional value was written, it means unlock
         if (asic.lockSeqPos == lockSeqLength) {
            LOG_DEBUG("ASIC unlocked");
            asic.locked = false;
            asic.lockSeqPos = (val == 0) ? 0 : 1;
         }
      }
   }
}

static inline unsigned short decode_magnification(byte val) {
   byte mag = (val & 0x3);
   if (mag == 3) mag = 4;
   return mag;
}

void asic_dma_cycle() {
  // Use the DMA info to feed PSG from RAM:
  // Read one 16bits instruction for each enabled channel X at each scan line and execute it.
  // More precisely: after leading edge of HSYNC, one dead cycle followed by a fetch cycle for each active channel (enabled and not executing a pause) followed by an execution cycle for each active channel.
  // All instructions last 1 cycle except LOAD that lasts 8 (up to 10 if CPU is also accessing the PSG).
  //  - LOAD R,DD will write DD to PSG.RegisterAY.Index[R] - The ASIC should actually be blocking the CPU if it tries to access the PSG, to determine if it's important to emulate.
  //  - PAUSE N set a counter to wait N*(chX.prescaler+1) cycles
  //  - REPEAT NNN keep address of loop start (next instruction) and a counter of loops
  //  - NOP does nothing
  //  - LOOP jump to address of loop stat if counter of loops is >0 and decrement it (yes, code is actually executed NNN+1 times)
  //  - INT generates an interruption for chX by setting chX.interrupt to true (code for CPU to detect it must also be done !)
  //  - STOP set chX.enabled to false ? (still increment address for when processing will restart)
  //  The last 4 can be OR-ed to be combined

  // The two first bits of the address give the page to read from
  byte dcsr = 0;
  bool dcsr_changed = false;
  for(int c = 0; c < NB_DMA_CHANNELS; c++) {
    dma_channel &channel = asic.dma.ch[c];
    if(!channel.enabled) continue;
    if(channel.pause_ticks > 0) { // PAUSE on-going
      if(channel.tick_cycles < channel.prescaler) {
        channel.tick_cycles++;
        continue;
      }
      channel.tick_cycles = 0;
      channel.pause_ticks--;
      continue;
    }
    int bank = ((channel.source_address & 0xC000) >> 14);
    int addr = (channel.source_address & 0x3FFF);
    word instruction = 0;
    instruction |= membank_config[GateArray.RAM_config & 7][bank][addr];
    instruction |= membank_config[GateArray.RAM_config & 7][bank][addr+1] << 8;
    LOG_DEBUG("DMA [" << c << "] instruction " << std::hex << instruction << " from " << channel.source_address << std::dec);
    int opcode = ((instruction & 0x7000) >> 12);
    if (opcode == 0) { // LOAD
      int R = ((instruction & 0x0F00) >> 8);
      byte val = (instruction & 0x00FF);
      SetAYRegister(R, val);
      LOG_DEBUG("DMA [" << c << "] load " << std::hex << static_cast<int>(val) << " in register " << R << std::dec);
    }
    else {
      if (opcode & 0x01) { // PAUSE
        channel.pause_ticks = (instruction) & 0x0FFF;
        channel.tick_cycles = 0;
        LOG_DEBUG("DMA [" << c << "] pause " << channel.pause_ticks << "*" << static_cast<int>(channel.prescaler) << " cycles");
      }
      if (opcode & 0x02) { // REPEAT
        channel.loops = (instruction) & 0x0FFF;
        channel.loop_address = channel.source_address;
        LOG_DEBUG("DMA [" << c << "] repeat " << channel.loops);
      }
      if (opcode & 0x04) { // NOP, LOOP, INT, STOP
        if(instruction & 0x0001) { // LOOP
          if(channel.loops > 0) {
            channel.source_address = channel.loop_address;
            LOG_DEBUG("DMA [" << c << "] loop");
          }
        }
        if(instruction & 0x0010) { // INT
          channel.interrupt = true;
          LOG_DEBUG("DMA [" << c << "] interrupt");
        }
        if(instruction & 0x0020) { // STOP
          channel.enabled = false;
          LOG_DEBUG("DMA [" << c << "] stop");
        }
      }
    }
    channel.source_address += 2;
    // TODO: cleaner way to modify back the register value here ...
    {
      word addr = 0x6C00 + (c << 2);
      *(membank_write[addr >> 14] + (addr & 0x3fff)) = static_cast<byte>(channel.source_address & 0xFF);
      addr++;
      *(membank_write[addr >> 14] + (addr & 0x3fff)) = static_cast<byte>((channel.source_address & 0xFF00) >> 8);
      /* Useless ?
      addr++;
      *(membank_write[addr >> 14] + (addr & 0x3fff)) = channel.prescaler;
      */
      if (channel.enabled) {
        dcsr |= (0x1 << c);
        dcsr_changed = true;
      }
      if (channel.interrupt) {
        dcsr |= (0x40 >> c);
        dcsr_changed = true;
      }
    }
  }
  // TODO: ... and here
  // Run RAM test of testplus.cpr when touching this (this is not a guarantee that this is correct but at least it's a guarantee that it's less wrong ! cf issue #40)
  if (dcsr_changed)
  {
    word addr = 0x6C0F;
    *(membank_write[addr >> 14] + (addr & 0x3fff)) = dcsr;
  }
}

// Return true if byte should be written in memory
bool asic_register_page_write(word addr, byte val) {
   if (addr < 0x4000 || addr > 0x7FFF) {
      return true;
   }
   LOG_DEBUG("ASIC register page write: addr=" << std::hex << addr << ", val=" << static_cast<int>(val) << std::dec);
   // TODO:double check the writes (more cases with mirroring / write only ?)
   if (addr >= 0x4000 && addr < 0x5000) {
      int id = ((addr & 0xF00) >> 8);
      int y = ((addr & 0xF0) >> 4);
      int x = (addr & 0xF);
      byte color = (val & 0xF);
      if(color > 0) {
         color += 16;
      }
      asic.sprites[id][x][y] = color;
      //LOG_DEBUG("Received sprite data for sprite " << id << ": x=" << x << ", y=" << y << ", color=" << static_cast<int>(val));
   } else if (addr >= 0x6000 && addr < 0x607D) {
      int id = ((addr - 0x6000) >> 3);
      int type = (addr & 0x7);
      switch (type) {
         case 0:
            // X position
            asic.sprites_x[id] = (asic.sprites_x[id] & 0xFF00) | val;
            //LOG_DEBUG("Received sprite X for sprite " << id << " x=" << asic.sprites_x[id]);
            // Mirrored in RAM image 4 bytes after
            pbRegisterPage[(addr & 0x3FFF) + 4] = val;
            break;
         case 1:
            // X position
            asic.sprites_x[id] = (asic.sprites_x[id] & 0x00FF) | (val << 8);
            //LOG_DEBUG("Received sprite X for sprite " << id << " x=" << asic.sprites_x[id]);
            // Mirrored in RAM image 4 bytes after
            pbRegisterPage[(addr & 0x3FFF) + 4] = val;
            break;
         case 2:
            // Y position
            asic.sprites_y[id] = ((asic.sprites_y[id] & 0xFF00) | val);
            //LOG_DEBUG("Received sprite Y for sprite " << id << " y=" << asic.sprites_y[id]);
            // Mirrored in RAM image 4 bytes after
            pbRegisterPage[(addr & 0x3FFF) + 4] = val;
            break;
         case 3:
            // Y position
            asic.sprites_y[id] = ((asic.sprites_y[id] & 0x00FF) | (val << 8));
            //LOG_DEBUG("Received sprite Y for sprite " << id << " y=" << asic.sprites_y[id]);
            // Affect RAM image
            // Mirrored in RAM image 4 bytes after
            pbRegisterPage[(addr & 0x3FFF) + 4] = val;
            break;
         case 4:
            // Magnification
            asic.sprites_mag_x[id] = decode_magnification(val >> 2);
            asic.sprites_mag_y[id] = decode_magnification(val);
            // Write-only: does not affect pbRegisterPage
            return false;
         default:
            LOG_DEBUG("Received sprite operation of unsupported type: " << type << " addr=" << std::hex << addr << " - val=" << static_cast<int>(val) << std::dec);
            break;
      }
   } else if (addr >= 0x6400 && addr < 0x6440) {
      int colour = (addr & 0x3F) >> 1;
      if ((addr % 2) == 1) {
         double green = static_cast<double>(val & 0x0F)/16;
         //LOG_DEBUG("Received color operation: color " << colour << " has green = " << green);
         asic_colours[colour][1] = green;
         pbRegisterPage[(addr & 0x3FFF)] = (val & 0x0F);
         // TODO: find a cleaner way to do this - this is a copy paste from "Set ink value" in cap32.cpp
      } else {
         double red   = static_cast<double>((val & 0xF0) >> 4)/16;
         double blue  = static_cast<double>(val & 0x0F)/16;
         //LOG_DEBUG("Received color operation: color " << colour << " has red = " << red << " and blue = " << blue);
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
         GateArray.palette[33] = SDL_MapRGB(back_surface->format, r, g, b); // update the mode 2 'anti-aliasing' colour
      }
*/
      return false;
   } else if (addr >= 0x6800 && addr < 0x6806) {
      if (addr == 0x6800) {
         LOG_DEBUG("Received programmable raster interrupt scan line: " << static_cast<int>(val));
         CRTC.interrupt_sl = val;
      } else if (addr == 0x6801) {
         LOG_DEBUG("Received scan line for split: " << static_cast<int>(val));
         CRTC.split_sl = val;
      } else if (addr == 0x6802) {
         CRTC.split_addr &= 0x00FF;
         CRTC.split_addr |= (val << 8);
         LOG_DEBUG("Received address for split: " << std::hex << CRTC.split_addr << std::dec);
      } else if (addr == 0x6803) {
         CRTC.split_addr &= 0x3F00;
         CRTC.split_addr |= val;
         LOG_DEBUG("Received address for split: " << std::hex << CRTC.split_addr << std::dec);
      } else if (addr == 0x6804) {
         asic.hscroll = (val & 0xf);
         asic.vscroll = ((val >> 4) & 0x7);
         asic.extend_border = (val >> 7);
         LOG_DEBUG("Received soft scroll control: " << static_cast<int>(val) << ": dx=" << asic.hscroll << ", dy=" << asic.vscroll << ", border=" << asic.extend_border);
         update_skew();
      } else if (addr == 0x6805) {
         LOG_DEBUG("[UNSUPPORTED] Received interrupt vector: " << static_cast<int>(val));
         asic.interrupt_vector = val & 0xF8;
         // TODO: Write this part !!! (Interrupt service part from http://www.cpcwiki.eu/index.php/Arnold_V_Specs_Revised)
      }
   } else if (addr >= 0x6808 && addr < 0x6810) {
     LOG_DEBUG("[UNSUPPORTED] Received analog input stuff");
   } else if (addr >= 0x6C00 && addr < 0x6C0B) {
     int c = ((addr & 0xc) >> 2);
     LOG_DEBUG("Received DMA source address: " << std::hex << addr << " (channel " << c << ") " << static_cast<int>(val) << std::dec);
     dma_channel *channel = nullptr;
     channel = &asic.dma.ch[c];
     switch(addr & 0x3) {
       case 0:
         channel->source_address &= 0xFF00;
         // least significant bit is ignored (address are word-aligned)
         channel->source_address |= (val & 0xFE);
         break;
       case 1:
         channel->source_address &= 0x00FF;
         channel->source_address |= (val << 8);
         break;
       case 2:
         channel->prescaler = val;
         break;
       default:
         // unused
         break;
     }
   } else if (addr == 0x6C0F) {
      LOG_DEBUG("Received DMA control register: " << std::hex << static_cast<int>(val) << std::dec);
      for (int c = 0; c < NB_DMA_CHANNELS; c++) {
        asic.dma.ch[c].enabled = (val & (0x1 << c));
        LOG_DEBUG("DMA channel " << c << (asic.dma.ch[c].enabled ? " enabled" : " disabled"))
      }
   } else {
      LOG_DEBUG("Received unused write at " << std::hex << addr << " - val: " << static_cast<int>(val) << std::dec);
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
   // TODO: this should be affected by other CRTC.registers (reg2 for borderWidth ?, reg1 for screenWidth ? reg6 for screenHeight ?)
   const int borderWidth = 64 + (asic.extend_border ? 16 : 0);
   const int borderHeight = 40 + 8*(30 - CRTC.registers[7]);
   const int screenWidth = 640 + borderWidth;
   const int screenHeight = 200 + borderHeight;
   // For each sprite
   for(int i = 15; i >= 0; i--) {
      int sx = asic.sprites_x[i];
      int mx = asic.sprites_mag_x[i];
      // If some part of the sprite is visible (horizontal check)
      if(mx > 0 && (sx + 64*mx) >= borderWidth && sx <= screenWidth) {
         int sy = asic.sprites_y[i];
         int my = asic.sprites_mag_y[i];
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
                  byte p = asic.sprites[i][x][y];
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
