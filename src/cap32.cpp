/* Caprice32 - Amstrad CPC Emulator
   (c) Copyright 1997-2005 Ulrich Doewich

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <iostream>
#include <sstream>

#include "SDL.h"

#include "cap32.h"
#include "crtc.h"
#include "tape.h"
#include "video.h"
#include "z80.h"
#include "configuration.h"
#include "stringutils.h"
#include "zip.h"
#include "keyboard.h"

#include <errno.h>
#include <string.h>

#define VERSION_STRING "v4.3.0"

#include "CapriceGui.h"
#include "CapriceGuiView.h"
#include "CapriceVKeyboardView.h"

#include "errors.h"

#define MAX_LINE_LEN 256

#define MIN_SPEED_SETTING 2
#define MAX_SPEED_SETTING 32
#define DEF_SPEED_SETTING 4

#define MAX_NB_JOYSTICKS 2

#ifdef DEBUG
#define LOG(x) std::cout << __FILE__ << ":" << __LINE__ << " - " << x << std::endl;
#else
#define LOG(x)
#endif


extern byte bTapeLevel;
extern t_z80regs z80;

extern dword *ScanPos;
extern dword *ScanStart;
extern word MaxVSync;
extern t_flags1 flags1;
extern t_new_dt new_dt;

SDL_AudioSpec *audio_spec = nullptr;

SDL_Surface *back_surface = nullptr;
video_plugin* vid_plugin;
SDL_Joystick* joysticks[MAX_NB_JOYSTICKS];

dword dwTicks, dwTicksOffset, dwTicksTarget, dwTicksTargetFPS;
dword dwFPS, dwFrameCount;
dword dwXScale, dwYScale;
dword dwSndBufferCopied;

dword dwBreakPoint, dwTrace, dwMF2ExitAddr;
dword dwMF2Flags = 0;
byte *pbGPBuffer = nullptr;
byte *pbSndBuffer = nullptr;
byte *pbSndBufferEnd = nullptr;
byte *pbSndStream = nullptr;
byte *membank_read[4], *membank_write[4], *memmap_ROM[256];
byte *pbRAM = nullptr;
byte *pbROMlo = nullptr;
byte *pbROMhi = nullptr;
byte *pbExpansionROM = nullptr;
byte *pbMF2ROMbackup = nullptr;
byte *pbMF2ROM = nullptr;
byte *pbTapeImage = nullptr;
byte *pbTapeImageEnd = nullptr;
byte keyboard_matrix[16];

std::list<SDL_Event> virtualKeyboardEvents;
dword lastVirtualEventTicks;

static byte *membank_config[8][4];

FILE *pfileObject;
FILE *pfoPrinter;

#ifdef DEBUG
#define DEBUG_KEY SDLK_F12
dword dwDebugFlag = 0;
FILE *pfoDebug = nullptr;
#endif

#define MAX_FREQ_ENTRIES 5
dword freq_table[MAX_FREQ_ENTRIES] = {
   11025,
   22050,
   44100,
   48000,
   96000
};

#include "font.h"

static double colours_rgb[32][3] = {
   { 0.5, 0.5, 0.5 }, { 0.5, 0.5, 0.5 },{ 0.0, 1.0, 0.5 }, { 1.0, 1.0, 0.5 },
   { 0.0, 0.0, 0.5 }, { 1.0, 0.0, 0.5 },{ 0.0, 0.5, 0.5 }, { 1.0, 0.5, 0.5 },
   { 1.0, 0.0, 0.5 }, { 1.0, 1.0, 0.5 },{ 1.0, 1.0, 0.0 }, { 1.0, 1.0, 1.0 },
   { 1.0, 0.0, 0.0 }, { 1.0, 0.0, 1.0 },{ 1.0, 0.5, 0.0 }, { 1.0, 0.5, 1.0 },
   { 0.0, 0.0, 0.5 }, { 0.0, 1.0, 0.5 },{ 0.0, 1.0, 0.0 }, { 0.0, 1.0, 1.0 },
   { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 },{ 0.0, 0.5, 0.0 }, { 0.0, 0.5, 1.0 },
   { 0.5, 0.0, 0.5 }, { 0.5, 1.0, 0.5 },{ 0.5, 1.0, 0.0 }, { 0.5, 1.0, 1.0 },
   { 0.5, 0.0, 0.0 }, { 0.5, 0.0, 1.0 },{ 0.5, 0.5, 0.0 }, { 0.5, 0.5, 1.0 }
};

static double colours_green[32] = {
   0.5647, 0.5647, 0.7529, 0.9412,
   0.1882, 0.3765, 0.4706, 0.6588,
   0.3765, 0.9412, 0.9098, 0.9725,
   0.3451, 0.4078, 0.6275, 0.6902,
   0.1882, 0.7529, 0.7216, 0.7843,
   0.1569, 0.2196, 0.4392, 0.5020,
   0.2824, 0.8471, 0.8157, 0.8784,
   0.2510, 0.3137, 0.5333, 0.5961
};

SDL_Color colours[32];

static byte bit_values[8] = {
   0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

static dword keyboard_normal[SDLK_LAST];
static dword keyboard_shift[SDLK_LAST];
static dword keyboard_ctrl[SDLK_LAST];
static dword keyboard_mode[SDLK_LAST];

static int joy_layout[12][2] = {
   { CPC_J0_UP,      SDLK_UP },
   { CPC_J0_DOWN,    SDLK_DOWN },
   { CPC_J0_LEFT,    SDLK_LEFT },
   { CPC_J0_RIGHT,   SDLK_RIGHT },
   { CPC_J0_FIRE1,   SDLK_z },
   { CPC_J0_FIRE2,   SDLK_x },
   { CPC_J1_UP,      0 },
   { CPC_J1_DOWN,    0 },
   { CPC_J1_LEFT,    0 },
   { CPC_J1_RIGHT,   0 },
   { CPC_J1_FIRE1,   0 },
   { CPC_J1_FIRE2,   0 }
};

#define MAX_ROM_MODS 2
#include "rom_mods.h"

char chAppPath[_MAX_PATH + 1];
char chROMSelected[_MAX_PATH + 1];
std::string chROMFile[3] = {
   "cpc464.rom",
   "cpc664.rom",
   "cpc6128.rom"
};

t_CPC CPC;
t_CRTC CRTC;
t_FDC FDC;
t_GateArray GateArray;
t_PPI PPI;
t_PSG PSG;
t_VDU VDU;

t_drive driveA;
t_drive driveB;

zip::t_zip_info zip_info;

#define MAX_DISK_FORMAT 8
#define DEFAULT_DISK_FORMAT 0
#define FIRST_CUSTOM_DISK_FORMAT 2
t_disk_format disk_format[MAX_DISK_FORMAT] = {
   { "178K Data Format", 40, 1, 9, 2, 0x52, 0xe5, {{ 0xc1, 0xc6, 0xc2, 0xc7, 0xc3, 0xc8, 0xc4, 0xc9, 0xc5 }} },
   { "169K Vendor Format", 40, 1, 9, 2, 0x52, 0xe5, {{ 0x41, 0x46, 0x42, 0x47, 0x43, 0x48, 0x44, 0x49, 0x45 }} }
};



#define psg_write \
{ \
   byte control = PSG.control & 0xc0; /* isolate PSG control bits */ \
   if (control == 0xc0) { /* latch address? */ \
      PSG.reg_select = psg_data; /* select new PSG register */ \
   } else if (control == 0x80) { /* write? */ \
      if (PSG.reg_select < 16) { /* valid register? */ \
         SetAYRegister(PSG.reg_select, psg_data); \
      } \
   } \
}



void ga_init_banking (void)
{
   byte *romb0, *romb1, *romb2, *romb3, *romb4, *romb5, *romb6, *romb7;
   byte *pbRAMbank;

   romb0 = pbRAM;
   romb1 = pbRAM + 1*16384;
   romb2 = pbRAM + 2*16384;
   romb3 = pbRAM + 3*16384;

   pbRAMbank = pbRAM + ((GateArray.RAM_bank + 1) * 65536);
   romb4 = pbRAMbank;
   romb5 = pbRAMbank + 1*16384;
   romb6 = pbRAMbank + 2*16384;
   romb7 = pbRAMbank + 3*16384;

   membank_config[0][0] = romb0;
   membank_config[0][1] = romb1;
   membank_config[0][2] = romb2;
   membank_config[0][3] = romb3;

   membank_config[1][0] = romb0;
   membank_config[1][1] = romb1;
   membank_config[1][2] = romb2;
   membank_config[1][3] = romb7;

   membank_config[2][0] = romb4;
   membank_config[2][1] = romb5;
   membank_config[2][2] = romb6;
   membank_config[2][3] = romb7;

   membank_config[3][0] = romb0;
   membank_config[3][1] = romb3;
   membank_config[3][2] = romb2;
   membank_config[3][3] = romb7;

   membank_config[4][0] = romb0;
   membank_config[4][1] = romb4;
   membank_config[4][2] = romb2;
   membank_config[4][3] = romb3;

   membank_config[5][0] = romb0;
   membank_config[5][1] = romb5;
   membank_config[5][2] = romb2;
   membank_config[5][3] = romb3;

   membank_config[6][0] = romb0;
   membank_config[6][1] = romb6;
   membank_config[6][2] = romb2;
   membank_config[6][3] = romb3;

   membank_config[7][0] = romb0;
   membank_config[7][1] = romb7;
   membank_config[7][2] = romb2;
   membank_config[7][3] = romb3;
}



void ga_memory_manager (void)
{
   dword mem_bank;
   if (CPC.ram_size == 64) { // 64KB of RAM?
      mem_bank = 0; // no expansion memory
      GateArray.RAM_config = 0; // the only valid configuration is 0
   } else {
      mem_bank = (GateArray.RAM_config >> 3) & 7; // extract expansion memory bank
      if (((mem_bank+2)*64) > CPC.ram_size) { // selection is beyond available memory?
         mem_bank = 0; // force default mapping
      }
   }
   if (mem_bank != GateArray.RAM_bank) { // requested bank is different from the active one?
      GateArray.RAM_bank = mem_bank;
      ga_init_banking();
   }
   for (int n = 0; n < 4; n++) { // remap active memory banks
      membank_read[n] = membank_config[GateArray.RAM_config & 7][n];
      membank_write[n] = membank_config[GateArray.RAM_config & 7][n];
   }
   if (!(GateArray.ROM_config & 0x04)) { // lower ROM is enabled?
      if (dwMF2Flags & MF2_ACTIVE) { // is the Multiface 2 paged in?
         membank_read[0] = pbMF2ROM;
         membank_write[0] = pbMF2ROM;
      } else {
         membank_read[0] = pbROMlo; // 'page in' lower ROM
      }
   }
   if (!(GateArray.ROM_config & 0x08)) { // upper/expansion ROM is enabled?
      membank_read[3] = pbExpansionROM; // 'page in' upper/expansion ROM
   }
}



byte z80_IN_handler (reg_pair port)
{
   byte ret_val;

   ret_val = 0xff; // default return value
// CRTC -----------------------------------------------------------------------
   if (!(port.b.h & 0x40)) { // CRTC chip select?
      if ((port.b.h & 3) == 3) { // read CRTC register?
         if ((CRTC.reg_select > 11) && (CRTC.reg_select < 18)) { // valid range?
            ret_val = CRTC.registers[CRTC.reg_select];
         }
         else {
            ret_val = 0; // write only registers return 0
         }
      }
   }
// PPI ------------------------------------------------------------------------
   else if (!(port.b.h & 0x08)) { // PPI chip select?
      byte ppi_port = port.b.h & 3;
      switch (ppi_port) {
         case 0: // read from port A?
            if (PPI.control & 0x10) { // port A set to input?
               if ((PSG.control & 0xc0) == 0x40) { // PSG control set to read?
                  if (PSG.reg_select < 16) { // within valid range?
                     if (PSG.reg_select == 14) { // PSG port A?
                        if (!(PSG.RegisterAY.Index[7] & 0x40)) { // port A in input mode?
                           ret_val = keyboard_matrix[CPC.keyboard_line & 0x0f]; // read keyboard matrix node status
                        } else {
                           ret_val = PSG.RegisterAY.Index[14] & (keyboard_matrix[CPC.keyboard_line & 0x0f]); // return last value w/ logic AND of input
                        }
                     } else if (PSG.reg_select == 15) { // PSG port B?
                        if ((PSG.RegisterAY.Index[7] & 0x80)) { // port B in output mode?
                           ret_val = PSG.RegisterAY.Index[15]; // return stored value
                        }
                     } else {
                        ret_val = PSG.RegisterAY.Index[PSG.reg_select]; // read PSG register
                     }
                  }
               }
            } else {
               ret_val = PPI.portA; // return last programmed value
            }
            break;

         case 1: // read from port B?
            if (PPI.control & 2) { // port B set to input?
               ret_val = bTapeLevel | // tape level when reading
                         (CPC.printer ? 0 : 0x40) | // ready line of connected printer
                         (CPC.jumpers & 0x7f) | // manufacturer + 50Hz
                         (CRTC.flag_invsync ? 1 : 0); // VSYNC status
            } else {
               ret_val = PPI.portB; // return last programmed value
            }
            break;

         case 2: // read from port C?
            byte direction = PPI.control & 9; // isolate port C directions
            ret_val = PPI.portC; // default to last programmed value
            if (direction) { // either half set to input?
               if (direction & 8) { // upper half set to input?
                  ret_val &= 0x0f; // blank out upper half
                  byte val = PPI.portC & 0xc0; // isolate PSG control bits
                  if (val == 0xc0) { // PSG specify register?
                     val = 0x80; // change to PSG write register
                  }
                  ret_val |= val | 0x20; // casette write data is always set
                  if (CPC.tape_motor) {
                     ret_val |= 0x10; // set the bit if the tape motor is running
                  }
               }
               if (!(direction & 1)) { // lower half set to output?
                  ret_val |= 0x0f; // invalid - set all bits
               }
            }
            break;
      }
   }
// ----------------------------------------------------------------------------
   else if (!(port.b.h & 0x04)) { // external peripheral?
      if ((port.b.h == 0xfb) && (!(port.b.l & 0x80))) { // FDC?
         if (!(port.b.l & 0x01)) { // FDC status register?
            ret_val = fdc_read_status();
         } else { // FDC data register
            ret_val = fdc_read_data();
         }
      }
   }
   return ret_val;
}



void z80_OUT_handler (reg_pair port, byte val)
{
// Gate Array -----------------------------------------------------------------
   if ((port.b.h & 0xc0) == 0x40) { // GA chip select?
      switch (val >> 6) {
         case 0: // select pen
            #ifdef DEBUG_GA
            if (dwDebugFlag) {
               fprintf(pfoDebug, "pen 0x%02x\r\n", val);
            }
            #endif
            GateArray.pen = val & 0x10 ? 0x10 : val & 0x0f; // if bit 5 is set, pen indexes the border colour
            if (CPC.mf2) { // MF2 enabled?
               *(pbMF2ROM + 0x03fcf) = val;
            }
            break;
         case 1: // set colour
            #ifdef DEBUG_GA
            if (dwDebugFlag) {
               fprintf(pfoDebug, "clr 0x%02x\r\n", val);
            }
            #endif
            {
               byte colour = val & 0x1f; // isolate colour value
               GateArray.ink_values[GateArray.pen] = colour;
               GateArray.palette[GateArray.pen] = SDL_MapRGB(back_surface->format,
                colours[colour].r, colours[colour].g, colours[colour].b);
               if (GateArray.pen < 2) {
                  byte r = (static_cast<dword>(colours[GateArray.ink_values[0]].r) + static_cast<dword>(colours[GateArray.ink_values[1]].r)) >> 1;
                  byte g = (static_cast<dword>(colours[GateArray.ink_values[0]].g) + static_cast<dword>(colours[GateArray.ink_values[1]].g)) >> 1;
                  byte b = (static_cast<dword>(colours[GateArray.ink_values[0]].b) + static_cast<dword>(colours[GateArray.ink_values[1]].b)) >> 1;
                  GateArray.palette[18] = SDL_MapRGB(back_surface->format, r, g, b); // update the mode 2 'anti-aliasing' colour
               }
            }
            if (CPC.mf2) { // MF2 enabled?
               int iPen = *(pbMF2ROM + 0x03fcf);
               *(pbMF2ROM + (0x03f90 | ((iPen & 0x10) << 2) | (iPen & 0x0f))) = val;
            }
            break;
         case 2: // set mode
            #ifdef DEBUG_GA
            if (dwDebugFlag) {
               fprintf(pfoDebug, "rom 0x%02x\r\n", val);
            }
            #endif
            GateArray.ROM_config = val;
            GateArray.requested_scr_mode = val & 0x03; // request a new CPC screen mode
            ga_memory_manager();
            if (val & 0x10) { // delay Z80 interrupt?
               z80.int_pending = 0; // clear pending interrupts
               GateArray.sl_count = 0; // reset GA scanline counter
            }
            if (CPC.mf2) { // MF2 enabled?
               *(pbMF2ROM + 0x03fef) = val;
            }
            break;
         case 3: // set memory configuration
            #ifdef DEBUG_GA
            if (dwDebugFlag) {
               fprintf(pfoDebug, "mem 0x%02x\r\n", val);
            }
            #endif
            GateArray.RAM_config = val;
            ga_memory_manager();
            if (CPC.mf2) { // MF2 enabled?
               *(pbMF2ROM + 0x03fff) = val;
            }
            break;
      }
   }
// CRTC -----------------------------------------------------------------------
   if (!(port.b.h & 0x40)) { // CRTC chip select?
      byte crtc_port = port.b.h & 3;
      if (crtc_port == 0) { // CRTC register select?
         CRTC.reg_select = val;
         if (CPC.mf2) { // MF2 enabled?
            *(pbMF2ROM + 0x03cff) = val;
         }
      }
      else if (crtc_port == 1) { // CRTC write data?
         if (CRTC.reg_select < 16) { // only registers 0 - 15 can be written to
            switch (CRTC.reg_select) {
               case 0: // horizontal total
                  CRTC.registers[0] = val;
                  break;
               case 1: // horizontal displayed
                  CRTC.registers[1] = val;
                  update_skew();
                  break;
               case 2: // horizontal sync position
                  CRTC.registers[2] = val;
                  break;
               case 3: // sync width
                  CRTC.registers[3] = val;
                  CRTC.hsw = val & 0x0f; // isolate horizontal sync width
                  CRTC.vsw = val >> 4; // isolate vertical sync width
                  break;
               case 4: // vertical total
                  CRTC.registers[4] = val & 0x7f;
                  if (CRTC.CharInstMR == CharMR2) {
                     if (CRTC.line_count == CRTC.registers[4]) { // matches vertical total?
                        if (CRTC.raster_count == CRTC.registers[9]) { // matches maximum raster address?
                           CRTC.flag_startvta = 1;
                        }
                     }
                  }
                  break;
               case 5: // vertical total adjust
                  CRTC.registers[5] = val & 0x1f;
                  break;
               case 6: // vertical displayed
                  CRTC.registers[6] = val & 0x7f;
                  if (CRTC.line_count == CRTC.registers[6]) { // matches vertical displayed?
                     new_dt.NewDISPTIMG = 0;
                  }
                  break;
               case 7: // vertical sync position
                  CRTC.registers[7] = val & 0x7f;
                  {
                     register dword temp = 0;
                     if (CRTC.line_count == CRTC.registers[7]) { // matches vertical sync position?
                        temp++;
                        if (CRTC.r7match != temp) {
                           CRTC.r7match = temp;
                           if (CRTC.char_count >= 2) {
                              CRTC.flag_resvsync = 0;
                              if (!CRTC.flag_invsync) {
                                 CRTC.vsw_count = 0;
                                 CRTC.flag_invsync = 1;
                                 flags1.monVSYNC = 26;
                                 GateArray.hs_count = 2; // GA delays its VSYNC by two CRTC HSYNCs
                              }
                           }
                        }
                     }
                     else {
                        CRTC.r7match = 0;
                     }
                  }
                  break;
               case 8: // interlace and skew
                  CRTC.registers[8] = val;
                  update_skew();
                  break;
               case 9: // maximum raster count
                  CRTC.registers[9] = val & 0x1f;
                  {
                     register dword temp = 0;
                     if (CRTC.raster_count == CRTC.registers[9]) { // matches maximum raster address?
                        temp = 1;
                        CRTC.flag_resscan = 1; // request a raster counter reset
                     }
                     if (CRTC.r9match != temp) {
                        CRTC.r9match = temp;
                        if (temp) {
                           CRTC.CharInstMR = CharMR1;
                        }
                     }
                     if (CRTC.raster_count == CRTC.registers[9]) { // matches maximum raster address?
                        if (CRTC.char_count == CRTC.registers[1]) {
                           CRTC.next_addr = CRTC.addr + CRTC.char_count;
                        }
                        if (CRTC.char_count == CRTC.registers[0]) { // matches horizontal total?
                           CRTC.flag_reschar = 1; // request a line count update
                        }
                        if (!CRTC.flag_startvta) {
                           CRTC.flag_resscan = 1;
                        }
                     } else {
                        if (!CRTC.flag_invta) { // not in vertical total adjust?
                           CRTC.flag_resscan = 0;
                        }
                     }
                  }
                  break;
               case 10: // cursor start raster
                  CRTC.registers[10] = val & 0x7f;
                  break;
               case 11: // cursor end raster
                  CRTC.registers[11] = val & 0x1f;
                  break;
               case 12: // start address high byte
                  CRTC.registers[12] = val & 0x3f;
                  CRTC.requested_addr = CRTC.registers[13] + (CRTC.registers[12] << 8);
                  break;
               case 13: // start address low byte
                  CRTC.registers[13] = val;
                  CRTC.requested_addr = CRTC.registers[13] + (CRTC.registers[12] << 8);
                  break;
               case 14: // cursor address high byte
                  CRTC.registers[14] = val & 0x3f;
                  break;
               case 15: // cursor address low byte
                  CRTC.registers[15] = val;
                  break;
            }
         }
         if (CPC.mf2) { // MF2 enabled?
            *(pbMF2ROM + (0x03db0 | (*(pbMF2ROM + 0x03cff) & 0x0f))) = val;
         }
         #ifdef DEBUG_CRTC
         if (dwDebugFlag) {
            fprintf(pfoDebug, "%02x = %02x\r\n", CRTC.reg_select, val);
         }
         #endif
      }
   }
// ROM select -----------------------------------------------------------------
   if (!(port.b.h & 0x20)) { // ROM select?
      GateArray.upper_ROM = val;
      pbExpansionROM = memmap_ROM[val];
      if (pbExpansionROM == nullptr) { // selected expansion ROM not present?
         pbExpansionROM = pbROMhi; // revert to BASIC ROM
      }
      if (!(GateArray.ROM_config & 0x08)) { // upper/expansion ROM is enabled?
         membank_read[3] = pbExpansionROM; // 'page in' upper/expansion ROM
      }
      if (CPC.mf2) { // MF2 enabled?
         *(pbMF2ROM + 0x03aac) = val;
      }
   }
// printer port ---------------------------------------------------------------
   if (!(port.b.h & 0x10)) { // printer port?
      CPC.printer_port = val ^ 0x80; // invert bit 7
      if (pfoPrinter) {
         if (!(CPC.printer_port & 0x80)) { // only grab data bytes; ignore the strobe signal
            fputc(CPC.printer_port, pfoPrinter); // capture printer output to file
         }
      }
   }
// PPI ------------------------------------------------------------------------
   if (!(port.b.h & 0x08)) { // PPI chip select?
      switch (port.b.h & 3) {
         case 0: // write to port A?
            PPI.portA = val;
            if (!(PPI.control & 0x10)) { // port A set to output?
               byte psg_data = val;
               psg_write
            }
            break;
         case 1: // write to port B?
            PPI.portB = val;
            break;
         case 2: // write to port C?
            PPI.portC = val;
            if (!(PPI.control & 1)) { // output lower half?
               CPC.keyboard_line = val;
            }
            if (!(PPI.control & 8)) { // output upper half?
               CPC.tape_motor = val & 0x10; // update tape motor control
               PSG.control = val; // change PSG control
               byte psg_data = PPI.portA;
               psg_write
            }
            break;
         case 3: // modify PPI control
            if (val & 0x80) { // change PPI configuration
               PPI.control = val; // update control byte
               PPI.portA = 0; // clear data for all ports
               PPI.portB = 0;
               PPI.portC = 0;
            } else { // bit manipulation of port C data
               if (val & 1) { // set bit?
                  byte bit = (val >> 1) & 7; // isolate bit to set
                  PPI.portC |= bit_values[bit]; // set requested bit
                  if (!(PPI.control & 1)) { // output lower half?
                     CPC.keyboard_line = PPI.portC;
                  }
                  if (!(PPI.control & 8)) { // output upper half?
                     CPC.tape_motor = PPI.portC & 0x10;
                     PSG.control = PPI.portC; // change PSG control
                     byte psg_data = PPI.portA;
                     psg_write
                  }
               } else {
                  byte bit = (val >> 1) & 7; // isolate bit to reset
                  PPI.portC &= ~(bit_values[bit]); // reset requested bit
                  if (!(PPI.control & 1)) { // output lower half?
                     CPC.keyboard_line = PPI.portC;
                  }
                  if (!(PPI.control & 8)) { // output upper half?
                     CPC.tape_motor = PPI.portC & 0x10;
                     PSG.control = PPI.portC; // change PSG control
                     byte psg_data = PPI.portA;
                     psg_write
                  }
               }
            }
            if (CPC.mf2) { // MF2 enabled?
               *(pbMF2ROM + 0x037ff) = val;
            }
            break;
      }
   }
// ----------------------------------------------------------------------------
   if ((port.b.h == 0xfa) && (!(port.b.l & 0x80))) { // floppy motor control?
      FDC.motor = val & 0x01;
      #ifdef DEBUG_FDC
      fputs(FDC.motor ? "\r\n--- motor on" : "\r\n--- motor off", pfoDebug);
      #endif
      FDC.flags |= STATUSDRVA_flag | STATUSDRVB_flag;
   }
   else if ((port.b.h == 0xfb) && (!(port.b.l & 0x80))) { // FDC data register?
      fdc_write_data(val);
   }
   else if ((CPC.mf2) && (port.b.h == 0xfe)) { // Multiface 2?
      if ((port.b.l == 0xe8) && (!(dwMF2Flags & MF2_INVISIBLE))) { // page in MF2 ROM?
         dwMF2Flags |= MF2_ACTIVE;
         ga_memory_manager();
      }
      else if (port.b.l == 0xea) { // page out MF2 ROM?
         dwMF2Flags &= ~MF2_ACTIVE;
         ga_memory_manager();
      }
   }
}



void print (dword *pdwAddr, char *pchStr, bool bolColour)
{
   int iLen, iIdx, iRow, iCol;
   dword dwColour;
   word wColour;
   byte bRow, bColour;

   switch (CPC.scr_bpp)
   {
      case 32:
         dwColour = bolColour ? 0x00ffffff : 0;
         iLen = strlen(pchStr); // number of characters to process
         for (int n = 0; n < iLen; n++) {
            dword *pdwLine, *pdwPixel;
            iIdx = static_cast<int>(pchStr[n]); // get the ASCII value
            if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) { // limit it to the range of chars in the font
               iIdx = FNT_BAD_CHAR;
            }
            iIdx -= FNT_MIN_CHAR; // zero base the index
            pdwLine = pdwAddr; // keep a reference to the current screen position
            for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
               pdwPixel = pdwLine;
               bRow = bFont[iIdx]; // get the bitmap information for one row
               for (iCol = 0; iCol < FNT_CHAR_WIDTH; iCol++) { // loop for all columns in the font character
                  if (bRow & 0x80) { // is the bit set?
                     *(pdwPixel+1) = 0; // draw the "shadow"
                     *(pdwPixel+CPC.scr_line_offs) = 0;
                     *(pdwPixel+CPC.scr_line_offs+1) = 0;
                     *pdwPixel = dwColour; // draw the character pixel
                  }
                  pdwPixel++; // update the screen position
                  bRow <<= 1; // advance to the next bit
               }
               pdwLine += CPC.scr_line_offs; // advance to next screen line
               iIdx += FNT_CHARS; // advance to next row in font data
            }
            pdwAddr += FNT_CHAR_WIDTH; // set screen address to next character position
         }
         break;

      case 24:
         dwColour = bolColour ? 0x00ffffff : 0;
         iLen = strlen(pchStr); // number of characters to process
         for (int n = 0; n < iLen; n++) {
            dword *pdwLine;
            byte *pbPixel;
            iIdx = static_cast<int>(pchStr[n]); // get the ASCII value
            if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) { // limit it to the range of chars in the font
               iIdx = FNT_BAD_CHAR;
            }
            iIdx -= FNT_MIN_CHAR; // zero base the index
            pdwLine = pdwAddr; // keep a reference to the current screen position
            for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
               pbPixel = reinterpret_cast<byte *>(pdwLine);
               bRow = bFont[iIdx]; // get the bitmap information for one row
               for (iCol = 0; iCol < FNT_CHAR_WIDTH; iCol++) { // loop for all columns in the font character
                  if (bRow & 0x80) { // is the bit set?
                     *(reinterpret_cast<dword *>(pbPixel+CPC.scr_line_offs)) = 0; // draw the "shadow"
                     *reinterpret_cast<dword *>(pbPixel) = dwColour; // draw the character pixel
                  }
                  pbPixel += 3; // update the screen position
                  bRow <<= 1; // advance to the next bit
               }
               pdwLine += CPC.scr_line_offs; // advance to next screen line
               iIdx += FNT_CHARS; // advance to next row in font data
            }
            pdwAddr += FNT_CHAR_WIDTH-2; // set screen address to next character position
         }
         break;

      case 15:
      case 16:
         wColour = bolColour ? 0xffff : 0;
         iLen = strlen(pchStr); // number of characters to process
         for (int n = 0; n < iLen; n++) {
            dword *pdwLine;
            word *pwPixel;
            iIdx = static_cast<int>(pchStr[n]); // get the ASCII value
            if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) { // limit it to the range of chars in the font
               iIdx = FNT_BAD_CHAR;
            }
            iIdx -= FNT_MIN_CHAR; // zero base the index
            pdwLine = pdwAddr; // keep a reference to the current screen position
            for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
               pwPixel = reinterpret_cast<word *>(pdwLine);
               bRow = bFont[iIdx]; // get the bitmap information for one row
               for (iCol = 0; iCol < FNT_CHAR_WIDTH; iCol++) { // loop for all columns in the font character
                  if (bRow & 0x80) { // is the bit set?
                     *(pwPixel+1) = 0; // draw the "shadow"
                     *reinterpret_cast<word *>(reinterpret_cast<dword *>(pwPixel)+CPC.scr_line_offs) = 0;
                     *(reinterpret_cast<word *>(reinterpret_cast<dword *>(pwPixel)+CPC.scr_line_offs)+1) = 0;
                     *pwPixel = wColour; // draw the character pixel
                  }
                  pwPixel++; // update the screen position
                  bRow <<= 1; // advance to the next bit
               }
               pdwLine += CPC.scr_line_offs; // advance to next screen line
               iIdx += FNT_CHARS; // advance to next row in font data
            }
            pdwAddr += FNT_CHAR_WIDTH/2; // set screen address to next character position
         }
         break;

      case 8:
         bColour = bolColour ? SDL_MapRGB(back_surface->format,255,255,255) : SDL_MapRGB(back_surface->format,0,0,0);
         iLen = strlen(pchStr); // number of characters to process
         for (int n = 0; n < iLen; n++) {
            dword *pdwLine;
            byte *pbPixel;
            iIdx = static_cast<int>(pchStr[n]); // get the ASCII value
            if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) { // limit it to the range of chars in the font
               iIdx = FNT_BAD_CHAR;
            }
            iIdx -= FNT_MIN_CHAR; // zero base the index
            pdwLine = pdwAddr; // keep a reference to the current screen position
            for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
               pbPixel = reinterpret_cast<byte *>(pdwLine);
               bRow = bFont[iIdx]; // get the bitmap information for one row
               for (iCol = 0; iCol < FNT_CHAR_WIDTH; iCol++) { // loop for all columns in the font character
                  if (bRow & 0x80) { // is the bit set?
                     *(pbPixel+1) = 0; // draw the "shadow"
                     *reinterpret_cast<byte *>(reinterpret_cast<dword *>(pbPixel)+CPC.scr_line_offs) = 0;
                     *(reinterpret_cast<byte *>(reinterpret_cast<dword *>(pbPixel)+CPC.scr_line_offs)+1) = 0;
                     *pbPixel = bColour; // draw the character pixel
                  }
                  pbPixel++; // update the screen position
                  bRow <<= 1; // advance to the next bit
               }
               pdwLine += CPC.scr_line_offs; // advance to next screen line
               iIdx += FNT_CHARS; // advance to next row in font data
            }
            pdwAddr += FNT_CHAR_WIDTH/4; // set screen address to next character position
         }
         break;
   }
}



int file_size (int file_num)
{
   struct stat s;

   if (!fstat(file_num, &s)) {
      return s.st_size;
   } else {
      return 0;
   }
}



int snapshot_load (FILE *pfile)
{
  int n;
  dword dwSnapSize, dwModel, dwFlags;
  byte val;
  reg_pair port;
  t_SNA_header sh;

  memset(&sh, 0, sizeof(sh));
  if(fread(&sh, sizeof(sh), 1, pfile) != 1) { // read snapshot header
    return ERR_SNA_INVALID;
  }
  if (memcmp(sh.id, "MV - SNA", 8) != 0) { // valid SNApshot image?
    return ERR_SNA_INVALID;
  }
  dwSnapSize = sh.ram_size[0] + (sh.ram_size[1] * 256); // memory dump size
  dwSnapSize &= ~0x3f; // limit to multiples of 64
  if (!dwSnapSize) {
    return ERR_SNA_SIZE;
  }
  if (dwSnapSize > CPC.ram_size) { // memory dump size differs from current RAM size?
    byte *pbTemp;

    pbTemp = new byte [dwSnapSize*1024];
    if (pbTemp) {
      delete [] pbRAM;
      CPC.ram_size = dwSnapSize;
      pbRAM = pbTemp;
    } else {
      return ERR_OUT_OF_MEMORY;
    }
  }
  emulator_reset(false);
  n = fread(pbRAM, dwSnapSize*1024, 1, pfile); // read memory dump into CPC RAM
  if (!n) {
    emulator_reset(false);
    return ERR_SNA_INVALID;
  }

  // Z80
  _A = sh.AF[1];
  _F = sh.AF[0];
  _B = sh.BC[1];
  _C = sh.BC[0];
  _D = sh.DE[1];
  _E = sh.DE[0];
  _H = sh.HL[1];
  _L = sh.HL[0];
  _R = sh.R & 0x7f;
  _Rb7 = sh.R & 0x80; // bit 7 of R
  _I = sh.I;
  if (sh.IFF0)
    _IFF1 = Pflag;
  if (sh.IFF1)
    _IFF2 = Pflag;
  _IXh = sh.IX[1];
  _IXl = sh.IX[0];
  _IYh = sh.IY[1];
  _IYl = sh.IY[0];
  z80.SP.b.h = sh.SP[1];
  z80.SP.b.l = sh.SP[0];
  z80.PC.b.h = sh.PC[1];
  z80.PC.b.l = sh.PC[0];
  _IM = sh.IM; // interrupt mode
  z80.AFx.b.h = sh.AFx[1];
  z80.AFx.b.l = sh.AFx[0];
  z80.BCx.b.h = sh.BCx[1];
  z80.BCx.b.l = sh.BCx[0];
  z80.DEx.b.h = sh.DEx[1];
  z80.DEx.b.l = sh.DEx[0];
  z80.HLx.b.h = sh.HLx[1];
  z80.HLx.b.l = sh.HLx[0];
  // Gate Array
  port.b.h = 0x7f;
  for (n = 0; n < 17; n++) { // loop for all colours + border
    GateArray.pen = n;
    val = sh.ga_ink_values[n]; // GA palette entry
    z80_OUT_handler(port, val | (1<<6));
  }
  val = sh.ga_pen; // GA pen
  z80_OUT_handler(port, (val & 0x3f));
  val = sh.ga_ROM_config; // GA ROM configuration
  z80_OUT_handler(port, (val & 0x3f) | (2<<6));
  val = sh.ga_RAM_config; // GA RAM configuration
  z80_OUT_handler(port, (val & 0x3f) | (3<<6));
  // CRTC
  port.b.h = 0xbd;
  for (n = 0; n < 18; n++) { // loop for all CRTC registers
    val = sh.crtc_registers[n];
    CRTC.reg_select = n;
    z80_OUT_handler(port, val);
  }
  port.b.h = 0xbc;
  val = sh.crtc_reg_select; // CRTC register select
  z80_OUT_handler(port, val);
  // ROM select
  port.b.h = 0xdf;
  val = sh.upper_ROM; // upper ROM number
  z80_OUT_handler(port, val);
  // PPI
  port.b.h = 0xf4; // port A
  z80_OUT_handler(port, sh.ppi_A);
  port.b.h = 0xf5; // port B
  z80_OUT_handler(port, sh.ppi_B);
  port.b.h = 0xf6; // port C
  z80_OUT_handler(port, sh.ppi_C);
  port.b.h = 0xf7; // control
  z80_OUT_handler(port, sh.ppi_control);
  // PSG
  PSG.control = PPI.portC;
  PSG.reg_select = sh.psg_reg_select;
  for (n = 0; n < 16; n++) { // loop for all PSG registers
    SetAYRegister(n, sh.psg_registers[n]);
  }

  if (sh.version > 1) { // does the snapshot have version 2 data?
    dwModel = sh.cpc_model; // determine the model it was saved for
    if (dwModel != CPC.model) { // different from what we're currently running?
      if (dwModel > 2) { // not one of the known models?
        emulator_reset(false);
        return ERR_SNA_CPC_TYPE;
      }
      std::string romFilename = CPC.rom_path + "/" + chROMFile[dwModel];
      if ((pfileObject = fopen(romFilename.c_str(), "rb")) != nullptr) {
        n = fread(pbROMlo, 2*16384, 1, pfileObject);
        fclose(pfileObject);
        if (!n) {
          emulator_reset(false);
          return ERR_CPC_ROM_MISSING;
        }
        CPC.model = dwModel;
      } else { // ROM image load failed
        emulator_reset(false);
        return ERR_CPC_ROM_MISSING;
      }
    }
  }
  if (sh.version > 2) { // does the snapshot have version 3 data?
    FDC.motor = sh.fdc_motor;
    driveA.current_track = sh.drvA_current_track;
    driveB.current_track = sh.drvB_current_track;
    CPC.printer_port = sh.printer_data ^ 0x80; // invert bit 7 again
    PSG.AmplitudeEnv = sh.psg_env_step << 1; // multiply by 2 to bring it into the 0 - 30 range
    PSG.FirstPeriod = false;
    if (sh.psg_env_direction == 0x01) { // up
      switch (PSG.RegisterAY.EnvType)
      {
        case 4:
        case 5:
        case 6:
        case 7:
        case 13:
        case 14:
        case 15:
          PSG.FirstPeriod = true;
          break;
      }
    } else if (sh.psg_env_direction == 0xff) { // down
      switch (PSG.RegisterAY.EnvType)
      {
        case 0:
        case 1:
        case 2:
        case 3:
        case 9:
        case 10:
        case 11:
          PSG.FirstPeriod = true;
          break;
      }
    }
    CRTC.addr = sh.crtc_addr[0] + (sh.crtc_addr[1] * 256);
    VDU.scanline = sh.crtc_scanline[0] + (sh.crtc_scanline[1] * 256);
    if (VDU.scanline > MaxVSync) {
      VDU.scanline = MaxVSync; // limit to max value
    }
    CRTC.char_count = sh.crtc_char_count[0];
    CRTC.line_count = sh.crtc_line_count & 127;
    CRTC.raster_count = sh.crtc_raster_count & 31;
    CRTC.hsw_count = sh.crtc_hsw_count & 15;
    CRTC.vsw_count = sh.crtc_vsw_count & 15;
    dwFlags = sh.crtc_flags[0] + (sh.crtc_flags[1] * 256);
    CRTC.flag_invsync = dwFlags & 1 ? 1 : 0; // vsync active?
    if (dwFlags & 2) { // hsync active?
      flags1.inHSYNC = 0xff;
      CRTC.flag_hadhsync = 1;
      if ((CRTC.hsw_count >= 3) && (CRTC.hsw_count < 7)) {
        CRTC.flag_inmonhsync = 1;
      }
    }
    CRTC.flag_invta = dwFlags & 0x80 ? 1 : 0; // in vertical total adjust?
    GateArray.hs_count = sh.ga_int_delay & 3;
    GateArray.sl_count = sh.ga_sl_count;
    z80.int_pending = sh.z80_int_pending;
  }
  return 0;
}



int snapshot_load (const char *pchFileName)
{
   if ((pfileObject = fopen(pchFileName, "rb")) != nullptr) {
     return snapshot_load(pfileObject);
   }
   return ERR_FILE_NOT_FOUND;
}



int snapshot_save (const char *pchFileName)
{
   t_SNA_header sh;
   int n;
   dword dwFlags;

   memset(&sh, 0, sizeof(sh));
   memcpy(sh.id, "MV - SNA", sizeof(sh.id));
   sh.version = 3;
// Z80
   sh.AF[1] = _A;
   sh.AF[0] = _F;
   sh.BC[1] = _B;
   sh.BC[0] = _C;
   sh.DE[1] = _D;
   sh.DE[0] = _E;
   sh.HL[1] = _H;
   sh.HL[0] = _L;
   sh.R = (_R & 0x7f) | (_Rb7 & 0x80);
   sh.I = _I;
   if (_IFF1)
      sh.IFF0 = 1;
   if (_IFF2)
      sh.IFF1 = 1;
   sh.IX[1] = _IXh;
   sh.IX[0] = _IXl;
   sh.IY[1] = _IYh;
   sh.IY[0] = _IYl;
   sh.SP[1] = z80.SP.b.h;
   sh.SP[0] = z80.SP.b.l;
   sh.PC[1] = z80.PC.b.h;
   sh.PC[0] = z80.PC.b.l;
   sh.IM = _IM;
   sh.AFx[1] = z80.AFx.b.h;
   sh.AFx[0] = z80.AFx.b.l;
   sh.BCx[1] = z80.BCx.b.h;
   sh.BCx[0] = z80.BCx.b.l;
   sh.DEx[1] = z80.DEx.b.h;
   sh.DEx[0] = z80.DEx.b.l;
   sh.HLx[1] = z80.HLx.b.h;
   sh.HLx[0] = z80.HLx.b.l;
// Gate Array
   sh.ga_pen = GateArray.pen;
   for (n = 0; n < 17; n++) { // loop for all colours + border
      sh.ga_ink_values[n] = GateArray.ink_values[n];
   }
   sh.ga_ROM_config = GateArray.ROM_config;
   sh.ga_RAM_config = GateArray.RAM_config;
// CRTC
   sh.crtc_reg_select = CRTC.reg_select;
   for (n = 0; n < 18; n++) { // loop for all CRTC registers
      sh.crtc_registers[n] = CRTC.registers[n];
   }
// ROM select
   sh.upper_ROM = GateArray.upper_ROM;
// PPI
   sh.ppi_A = PPI.portA;
   sh.ppi_B = PPI.portB;
   sh.ppi_C = PPI.portC;
   sh.ppi_control = PPI.control;
// PSG
   sh.psg_reg_select = PSG.reg_select;
   for (n = 0; n < 16; n++) { // loop for all PSG registers
      sh.psg_registers[n] = PSG.RegisterAY.Index[n];
   }

   sh.ram_size[0] = CPC.ram_size & 0xff;
   sh.ram_size[1] = (CPC.ram_size >> 8) & 0xff;
// version 2 info
   sh.cpc_model = CPC.model;
// version 3 info
   sh.fdc_motor = FDC.motor;
   sh.drvA_current_track = driveA.current_track;
   sh.drvB_current_track = driveB.current_track;
   sh.printer_data = CPC.printer_port ^ 0x80; // invert bit 7 again
   sh.psg_env_step = PSG.AmplitudeEnv >> 1; // divide by 2 to bring it into the 0 - 15 range
   if (PSG.FirstPeriod) {
      switch (PSG.RegisterAY.EnvType)
      {
         case 0:
         case 1:
         case 2:
         case 3:
         case 8:
         case 9:
         case 10:
         case 11:
            sh.psg_env_direction = 0xff; // down
            break;
         case 4:
         case 5:
         case 6:
         case 7:
         case 12:
         case 13:
         case 14:
         case 15:
            sh.psg_env_direction = 0x01; // up
            break;
      }
   } else {
      switch (PSG.RegisterAY.EnvType)
      {
         case 0:
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
         case 6:
         case 7:
         case 9:
         case 11:
         case 13:
         case 15:
            sh.psg_env_direction = 0x00; // hold
            break;
         case 8:
         case 14:
            sh.psg_env_direction = 0xff; // down
            break;
         case 10:
         case 12:
            sh.psg_env_direction = 0x01; // up
            break;
      }
   }
   sh.crtc_addr[0] = CRTC.addr & 0xff;
   sh.crtc_addr[1] = (CRTC.addr >> 8) & 0xff;
   sh.crtc_scanline[0] = VDU.scanline & 0xff;
   sh.crtc_scanline[1] = (VDU.scanline >> 8) & 0xff;
   sh.crtc_char_count[0] = CRTC.char_count;
   sh.crtc_line_count = CRTC.line_count;
   sh.crtc_raster_count = CRTC.raster_count;
   sh.crtc_hsw_count = CRTC.hsw_count;
   sh.crtc_vsw_count = CRTC.vsw_count;
   dwFlags = 0;
   if (CRTC.flag_invsync) { // vsync active?
      dwFlags |= 1;
   }
   if (flags1.inHSYNC) { // hsync active?
      dwFlags |= 2;
   }
   if (CRTC.flag_invta) { // in vertical total adjust?
      dwFlags |= 0x80;
   }
   sh.crtc_flags[0] = dwFlags & 0xff;
   sh.crtc_flags[1] = (dwFlags >> 8) & 0xff;
   sh.ga_int_delay = GateArray.hs_count;
   sh.ga_sl_count = GateArray.sl_count;
   sh.z80_int_pending = z80.int_pending;

   if ((pfileObject = fopen(pchFileName, "wb")) != nullptr) {
      if (fwrite(&sh, sizeof(sh), 1, pfileObject) != 1) { // write snapshot header
         fclose(pfileObject);
         return ERR_SNA_WRITE;
      }
      if (fwrite(pbRAM, CPC.ram_size*1024, 1, pfileObject) != 1) { // write memory contents to snapshot file
         fclose(pfileObject);
         return ERR_SNA_WRITE;
      }
      fclose(pfileObject);
   } else {
      return ERR_SNA_WRITE;
   }

   return 0;
}



void dsk_eject (t_drive *drive)
{
   dword track, side;

   for (track = 0; track < DSK_TRACKMAX; track++) { // loop for all tracks
      for (side = 0; side < DSK_SIDEMAX; side++) { // loop for all sides
         if (drive->track[track][side].data) { // track is formatted?
            free(drive->track[track][side].data); // release memory allocated for this track
         }
      }
   }
   dword dwTemp = drive->current_track; // save the drive head position
   memset(drive, 0, sizeof(t_drive)); // clear drive info structure
   drive->current_track = dwTemp;
}



int dsk_load (FILE *pfile, t_drive *drive)
{
  dword dwTrackSize, track, side, sector, dwSectorSize, dwSectors;
  byte *pbPtr, *pbDataPtr, *pbTempPtr, *pbTrackSizeTable;
  if(fread(pbGPBuffer, 0x100, 1, pfile) != 1) { // read DSK header
    return ERR_DSK_INVALID;
  }
  pbPtr = pbGPBuffer;

  if (memcmp(pbPtr, "MV - CPC", 8) == 0) { // normal DSK image?
    drive->tracks = *(pbPtr + 0x30); // grab number of tracks
    if (drive->tracks > DSK_TRACKMAX) { // compare against upper limit
      drive->tracks = DSK_TRACKMAX; // limit to maximum
    }
    drive->sides = *(pbPtr + 0x31); // grab number of sides
    if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
      dsk_eject(drive);
      return ERR_DSK_SIDES;
    }
    dwTrackSize = (*(pbPtr + 0x32) + (*(pbPtr + 0x33) << 8)) - 0x100; // determine track size in bytes, minus track header
    drive->sides--; // zero base number of sides
    for (track = 0; track < drive->tracks; track++) { // loop for all tracks
      for (side = 0; side <= drive->sides; side++) { // loop for all sides
        if(fread(pbGPBuffer+0x100, 0x100, 1, pfile) != 1) { // read track header
          dsk_eject(drive);
          return ERR_DSK_INVALID;
        }
        pbPtr = pbGPBuffer + 0x100;
        if (memcmp(pbPtr, "Track-Info", 10) != 0) { // abort if ID does not match
          dsk_eject(drive);
          return ERR_DSK_INVALID;
        }
        dwSectorSize = 0x80 << *(pbPtr + 0x14); // determine sector size in bytes
        dwSectors = *(pbPtr + 0x15); // grab number of sectors
        if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
          dsk_eject(drive);
          return ERR_DSK_SECTORS;
        }
        drive->track[track][side].sectors = dwSectors; // store sector count
        drive->track[track][side].size = dwTrackSize; // store track size
        drive->track[track][side].data = static_cast<byte *>(malloc(dwTrackSize)); // attempt to allocate the required memory
        if (drive->track[track][side].data == nullptr) { // abort if not enough
          dsk_eject(drive);
          return ERR_OUT_OF_MEMORY;
        }
        pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
        pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
        for (sector = 0; sector < dwSectors; sector++) { // loop for all sectors
          memcpy(drive->track[track][side].sector[sector].CHRN, (pbPtr + 0x18), 4); // copy CHRN
          memcpy(drive->track[track][side].sector[sector].flags, (pbPtr + 0x1c), 2); // copy ST1 & ST2
          drive->track[track][side].sector[sector].size = dwSectorSize;
          drive->track[track][side].sector[sector].data = pbDataPtr; // store pointer to sector data
          pbDataPtr += dwSectorSize;
          pbPtr += 8;
        }
        if (!fread(pbTempPtr, dwTrackSize, 1, pfile)) { // read entire track data in one go
          dsk_eject(drive);
          return ERR_DSK_INVALID;
        }
      }
    }
    drive->altered = 0; // disk is as yet unmodified
  } else {
    if (memcmp(pbPtr, "EXTENDED", 8) == 0) { // extended DSK image?
      drive->tracks = *(pbPtr + 0x30); // number of tracks
      if (drive->tracks > DSK_TRACKMAX) {  // limit to maximum possible
        drive->tracks = DSK_TRACKMAX;
      }
      drive->random_DEs = *(pbPtr + 0x31) & 0x80; // simulate random Data Errors?
      drive->sides = *(pbPtr + 0x31) & 3; // number of sides
      if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
        dsk_eject(drive);
        return ERR_DSK_SIDES;
      }
      pbTrackSizeTable = pbPtr + 0x34; // pointer to track size table in DSK header
      drive->sides--; // zero base number of sides
      for (track = 0; track < drive->tracks; track++) { // loop for all tracks
        for (side = 0; side <= drive->sides; side++) { // loop for all sides
          dwTrackSize = (*pbTrackSizeTable++ << 8); // track size in bytes
          if (dwTrackSize != 0) { // only process if track contains data
            dwTrackSize -= 0x100; // compensate for track header
            if(fread(pbGPBuffer+0x100, 0x100, 1, pfile) != 1) { // read track header
              dsk_eject(drive);
              return ERR_DSK_INVALID;
            }
            pbPtr = pbGPBuffer + 0x100;
            if (memcmp(pbPtr, "Track-Info", 10) != 0) { // valid track header?
              dsk_eject(drive);
              return ERR_DSK_INVALID;
            }
            dwSectors = *(pbPtr + 0x15); // number of sectors for this track
            if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
              dsk_eject(drive);
              return ERR_DSK_SECTORS;
            }
            drive->track[track][side].sectors = dwSectors; // store sector count
            drive->track[track][side].size = dwTrackSize; // store track size
            drive->track[track][side].data = static_cast<byte *>(malloc(dwTrackSize)); // attempt to allocate the required memory
            if (drive->track[track][side].data == nullptr) { // abort if not enough
              dsk_eject(drive);
              return ERR_OUT_OF_MEMORY;
            }
            pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
            pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
            for (sector = 0; sector < dwSectors; sector++) { // loop for all sectors
              memcpy(drive->track[track][side].sector[sector].CHRN, (pbPtr + 0x18), 4); // copy CHRN
              memcpy(drive->track[track][side].sector[sector].flags, (pbPtr + 0x1c), 2); // copy ST1 & ST2
              dwSectorSize = *(pbPtr + 0x1e) + (*(pbPtr + 0x1f) << 8); // sector size in bytes
              drive->track[track][side].sector[sector].size = dwSectorSize;
              drive->track[track][side].sector[sector].data = pbDataPtr; // store pointer to sector data
              pbDataPtr += dwSectorSize;
              pbPtr += 8;
            }
            if (!fread(pbTempPtr, dwTrackSize, 1, pfile)) { // read entire track data in one go
              dsk_eject(drive);
              return ERR_DSK_INVALID;
            }
          } else {
            memset(&drive->track[track][side], 0, sizeof(t_track)); // track not formatted
          }
        }
      }
      drive->altered = 0; // disk is as yet unmodified
    } else {
      dsk_eject(drive);
      return ERR_DSK_INVALID; // file could not be identified as a valid DSK
    }
  }
  return 0;
}



int dsk_load (const char *pchFileName, t_drive *drive)
{
   int iRetCode;

   iRetCode = 0;
   dsk_eject(drive);
   if ((pfileObject = fopen(pchFileName, "rb")) != nullptr) {
     iRetCode = dsk_load(pfileObject, drive);
     fclose(pfileObject);
   } else {
      iRetCode = ERR_FILE_NOT_FOUND;
   }

   if (iRetCode != 0) { // on error, 'eject' disk from drive
      dsk_eject(drive);
   }
   return iRetCode;
}



int dsk_save (const char *pchFileName, t_drive *drive)
{
   t_DSK_header dh;
   t_track_header th;
   dword track, side, pos, sector;

   if ((pfileObject = fopen(pchFileName, "wb")) != nullptr) {
      memset(&dh, 0, sizeof(dh));
      memcpy(dh.id, "EXTENDED CPC DSK File\r\nDisk-Info\r\n", sizeof(dh.id));
      strcpy(dh.unused1, "Caprice32\r\n");
      dh.tracks = drive->tracks;
      dh.sides = (drive->sides+1) | (drive->random_DEs); // correct side count and indicate random DEs, if necessary
      pos = 0;
      for (track = 0; track < drive->tracks; track++) { // loop for all tracks
         for (side = 0; side <= drive->sides; side++) { // loop for all sides
            if (drive->track[track][side].size) { // track is formatted?
               dh.track_size[pos] = (drive->track[track][side].size + 0x100) >> 8; // track size + header in bytes
            }
            pos++;
         }
      }
      if (!fwrite(&dh, sizeof(dh), 1, pfileObject)) { // write header to file
         fclose(pfileObject);
         return ERR_DSK_WRITE;
      }

      memset(&th, 0, sizeof(th));
      memcpy(th.id, "Track-Info\r\n", sizeof(th.id));
      for (track = 0; track < drive->tracks; track++) { // loop for all tracks
         for (side = 0; side <= drive->sides; side++) { // loop for all sides
            if (drive->track[track][side].size) { // track is formatted?
               th.track = track;
               th.side = side;
               th.bps = 2;
               th.sectors = drive->track[track][side].sectors;
               th.gap3 = 0x4e;
               th.filler = 0xe5;
               for (sector = 0; sector < th.sectors; sector++) {
                  memcpy(&th.sector[sector][0], drive->track[track][side].sector[sector].CHRN, 4); // copy CHRN
                  memcpy(&th.sector[sector][4], drive->track[track][side].sector[sector].flags, 2); // copy ST1 & ST2
                  th.sector[sector][6] = drive->track[track][side].sector[sector].size & 0xff;
                  th.sector[sector][7] = (drive->track[track][side].sector[sector].size >> 8) & 0xff; // sector size in bytes
               }
               if (!fwrite(&th, sizeof(th), 1, pfileObject)) { // write track header
                  fclose(pfileObject);
                  return ERR_DSK_WRITE;
               }
               if (!fwrite(drive->track[track][side].data, drive->track[track][side].size, 1, pfileObject)) { // write track data
                  fclose(pfileObject);
                  return ERR_DSK_WRITE;
               }
            }
         }
      }
      fclose(pfileObject);
   } else {
      return ERR_DSK_WRITE; // write attempt failed
   }

   return 0;
}



int dsk_format (t_drive *drive, int iFormat)
{
   int iRetCode = 0;
   drive->tracks = disk_format[iFormat].tracks;
   if (drive->tracks > DSK_TRACKMAX) { // compare against upper limit
      drive->tracks = DSK_TRACKMAX; // limit to maximum
   }
   drive->sides = disk_format[iFormat].sides;
   if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
      iRetCode = ERR_DSK_SIDES;
      goto exit;
   }
   drive->sides--; // zero base number of sides
   for (dword track = 0; track < drive->tracks; track++) { // loop for all tracks
      for (dword side = 0; side <= drive->sides; side++) { // loop for all sides
         dword dwSectorSize = 0x80 << disk_format[iFormat].sector_size; // determine sector size in bytes
         dword dwSectors = disk_format[iFormat].sectors;
         if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
            iRetCode = ERR_DSK_SECTORS;
            goto exit;
         }
         dword dwTrackSize = dwSectorSize * dwSectors; // determine track size in bytes, minus track header
         drive->track[track][side].sectors = dwSectors; // store sector count
         drive->track[track][side].size = dwTrackSize; // store track size
         drive->track[track][side].data = static_cast<byte *>(malloc(dwTrackSize)); // attempt to allocate the required memory
         if (drive->track[track][side].data == nullptr) { // abort if not enough
            iRetCode = ERR_OUT_OF_MEMORY;
            goto exit;
         }
         byte *pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
         byte *pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
         byte CHRN[4];
         CHRN[0] = static_cast<byte>(track);
         CHRN[1] = static_cast<byte>(side);
         CHRN[3] = static_cast<byte>(disk_format[iFormat].sector_size);
         for (dword sector = 0; sector < dwSectors; sector++) { // loop for all sectors
            CHRN[2] = disk_format[iFormat].sector_ids[side][sector];
            memcpy(drive->track[track][side].sector[sector].CHRN, CHRN, 4); // copy CHRN
            drive->track[track][side].sector[sector].size = dwSectorSize;
            drive->track[track][side].sector[sector].data = pbDataPtr; // store pointer to sector data
            pbDataPtr += dwSectorSize;
         }
         memset(pbTempPtr, disk_format[iFormat].filler_byte, dwTrackSize);
      }
   }
   drive->altered = 1; // flag disk as having been modified

exit:
   if (iRetCode != 0) { // on error, 'eject' disk from drive
      dsk_eject(drive);
   }
   return iRetCode;
}



void tape_eject (void)
{
   free(pbTapeImage);
   pbTapeImage = nullptr;
}



int tape_insert (FILE *pfile)
{
   tape_eject();
   if(fread(pbGPBuffer, 10, 1, pfile) != 1) { // read beginning of header
      return ERR_TAP_INVALID;
   }
   // Reset so that the next method can recheck the header
   fseek(pfile, 0, SEEK_SET);
   byte *pbPtr = pbGPBuffer;
   if (memcmp(pbPtr, "ZXTape!\032", 8) == 0) { // CDT file?
      LOG("tape_insert CDT file");
      return tape_insert_cdt(pfile);
   } else if (memcmp(pbPtr, "Creative", 8) == 0) { // VOC file ?
      LOG("tape_insert VOC file");
      return tape_insert_voc(pfile);
   } else { // Unknown file
      LOG("tape_insert unknown file");
      return ERR_TAP_INVALID;
   }
}



int tape_insert (const char *pchFileName)
{
   LOG("tape_insert " << pchFileName);
   FILE *pfile;
   if ((pfile = fopen(pchFileName, "rb")) == nullptr) {
      return ERR_FILE_NOT_FOUND;
   }

   int iRetCode = tape_insert(pfile);
   fclose(pfile);

   return iRetCode;
}

int tape_insert_cdt (FILE *pfile)
{
   long lFileSize;
   int iBlockLength;
   byte bID;
   byte *pbPtr, *pbBlock;

   if(fread(pbGPBuffer, 10, 1, pfile) != 1) { // read CDT header
      LOG("Couldn't read CDT header");
      return ERR_TAP_INVALID;
   }
   pbPtr = pbGPBuffer;
   if (memcmp(pbPtr, "ZXTape!\032", 8) != 0) { // valid CDT file?
      LOG("Invalid CDT header '" << pbPtr << "'");
      return ERR_TAP_INVALID;
   }
   if (*(pbPtr + 0x08) != 1) { // major version must be 1
      LOG("Invalid CDT major version");
      return ERR_TAP_INVALID;
   }
   lFileSize = file_size(fileno(pfile)) - 0x0a;
   if (lFileSize <= 0) { // the tape image should have at least one block...
      LOG("Invalid CDT file size");
      return ERR_TAP_INVALID;
   }
   pbTapeImage = static_cast<byte *>(malloc(lFileSize+6));
   *pbTapeImage = 0x20; // start off with a pause block
   *reinterpret_cast<word *>(pbTapeImage+1) = 2000; // set the length to 2 seconds
   if(fread(pbTapeImage+3, lFileSize, 1, pfile) != 1) { // append the entire CDT file
      LOG("Couldn't read CDT file");
     return ERR_TAP_INVALID;
   }
   *(pbTapeImage+lFileSize+3) = 0x20; // end with a pause block
   *reinterpret_cast<word *>(pbTapeImage+lFileSize+3+1) = 2000; // set the length to 2 seconds

   #ifdef DEBUG_TAPE
   fputs("--- New Tape\r\n", pfoDebug);
   #endif
   pbTapeImageEnd = pbTapeImage + lFileSize+6;
   pbBlock = pbTapeImage;
   bool bolGotDataBlock = false;
   while (pbBlock < pbTapeImageEnd) {
      bID = *pbBlock++;
      switch(bID) {
         case 0x10: // standard speed data block
            iBlockLength = *reinterpret_cast<word *>(pbBlock+2) + 4;
            bolGotDataBlock = true;
            break;
         case 0x11: // turbo loading data block
            iBlockLength = (*reinterpret_cast<dword *>(pbBlock+0x0f) & 0x00ffffff) + 0x12;
            bolGotDataBlock = true;
            break;
         case 0x12: // pure tone
            iBlockLength = 4;
            bolGotDataBlock = true;
            break;
         case 0x13: // sequence of pulses of different length
            iBlockLength = *pbBlock * 2 + 1;
            bolGotDataBlock = true;
            break;
         case 0x14: // pure data block
            iBlockLength = (*reinterpret_cast<dword *>(pbBlock+0x07) & 0x00ffffff) + 0x0a;
            bolGotDataBlock = true;
            break;
         case 0x15: // direct recording
            iBlockLength = (*reinterpret_cast<dword *>(pbBlock+0x05) & 0x00ffffff) + 0x08;
            bolGotDataBlock = true;
            break;
         case 0x20: // pause
            if ((!bolGotDataBlock) && (pbBlock != pbTapeImage+1)) {
               *reinterpret_cast<word *>(pbBlock) = 0; // remove any pauses (execept ours) before the data starts
            }
            iBlockLength = 2;
            break;
         case 0x21: // group start
            iBlockLength = *pbBlock + 1;
            break;
         case 0x22: // group end
            iBlockLength = 0;
            break;
         case 0x23: // jump to block
            LOG("Couldn't load CDT file: unsupported " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = 2;
            break;
         case 0x24: // loop start
            LOG("Couldn't load CDT file: unsupported " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = 2;
            break;
         case 0x25: // loop end
            LOG("Couldn't load CDT file: unsupported " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = 0;
            break;
         case 0x26: // call sequence
            LOG("Couldn't load CDT file: unsupported " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = (*reinterpret_cast<word *>(pbBlock) * 2) + 2;
            break;
         case 0x27: // return from sequence
            LOG("Couldn't load CDT file: unsupported " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = 0;
            break;
         case 0x28: // select block
            LOG("Couldn't load CDT file: unsupported " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = *reinterpret_cast<word *>(pbBlock) + 2;
            break;
         case 0x30: // text description
            iBlockLength = *pbBlock + 1;
            break;
         case 0x31: // message block
            iBlockLength = *(pbBlock+1) + 2;
            break;
         case 0x32: // archive info
            iBlockLength = *reinterpret_cast<word *>(pbBlock) + 2;
            break;
         case 0x33: // hardware type
            iBlockLength = (*pbBlock * 3) + 1;
            break;
         case 0x34: // emulation info
            iBlockLength = 8;
            break;
         case 0x35: // custom info block
            iBlockLength = *reinterpret_cast<dword *>(pbBlock+0x10) + 0x14;
            break;
         case 0x40: // snapshot block
            iBlockLength = (*reinterpret_cast<dword *>(pbBlock+0x01) & 0x00ffffff) + 0x04;
            break;
         case 0x5A: // another tzx/cdt file
            iBlockLength = 9;
            break;

         default: // "extension rule"
            iBlockLength = *reinterpret_cast<dword *>(pbBlock) + 4;
      }

      #ifdef DEBUG_TAPE
      fprintf(pfoDebug, "%02x %d\r\n", bID, iBlockLength);
      #endif

      pbBlock += iBlockLength;
   }
   if (pbBlock != pbTapeImageEnd) {
      LOG("CDT file error: Didn't reach end of tape");
      tape_eject();
      return ERR_TAP_INVALID;
   }
   Tape_Rewind();
   return 0;
}



int tape_insert_voc (FILE *pfile)
{
   long lFileSize, lOffset, lInitialOffset, lSampleLength;
   int iBlockLength;
   byte *pbPtr, *pbTapeImagePtr, *pbVocDataBlock, *pbVocDataBlockPtr;
   bool bolDone;

   tape_eject();
   if(fread(pbGPBuffer, 26, 1, pfile) != 1) { // read VOC header
     return ERR_TAP_BAD_VOC;
   }
   pbPtr = pbGPBuffer;
   if (memcmp(pbPtr, "Creative Voice File\032", 20) != 0) { // valid VOC file?
      return ERR_TAP_BAD_VOC;
   }
   lOffset =
   lInitialOffset = *reinterpret_cast<word *>(pbPtr + 0x14);
   lFileSize = file_size(fileno(pfile));
   if ((lFileSize-26) <= 0) { // should have at least one block...
      return ERR_TAP_BAD_VOC;
   }

   #ifdef DEBUG_TAPE
   fputs("--- New Tape\r\n", pfoDebug);
   #endif
   iBlockLength = 0;
   lSampleLength = 0;
   byte bSampleRate = 0;
   bolDone = false;
   while ((!bolDone) && (lOffset < lFileSize)) {
      fseek(pfile, lOffset, SEEK_SET);
      if(fread(pbPtr, 16, 1, pfile) != 1) { // read block ID + size
        return ERR_TAP_BAD_VOC;
      }
      #ifdef DEBUG_TAPE
      fprintf(pfoDebug, "%02x %d\r\n", *pbPtr, *(dword *)(pbPtr+0x01) & 0x00ffffff);
      #endif
      switch(*pbPtr) {
         case 0x0: // terminator
            bolDone = true;
            break;
         case 0x1: // sound data
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr+0x01) & 0x00ffffff) + 4;
            lSampleLength += iBlockLength - 6;
            if ((bSampleRate) && (bSampleRate != *(pbPtr+0x04))) { // no change in sample rate allowed
               return ERR_TAP_BAD_VOC;
            }
            bSampleRate = *(pbPtr+0x04);
            if (*(pbPtr+0x05) != 0) { // must be 8 bits wide
               return ERR_TAP_BAD_VOC;
            }
            break;
         case 0x2: // sound continue
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr+0x01) & 0x00ffffff) + 4;
            lSampleLength += iBlockLength - 4;
            break;
         case 0x3: // silence
            iBlockLength = 4;
            lSampleLength += *reinterpret_cast<word *>(pbPtr+0x01) + 1;
            if ((bSampleRate) && (bSampleRate != *(pbPtr+0x03))) { // no change in sample rate allowed
               return ERR_TAP_BAD_VOC;
            }
            bSampleRate = *(pbPtr+0x03);
            break;
         case 0x4: // marker
            iBlockLength = 3;
            break;
         case 0x5: // ascii
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr+0x01) & 0x00ffffff) + 4;
            break;
         default:
            return ERR_TAP_BAD_VOC;
      }
      lOffset += iBlockLength;
   }
   #ifdef DEBUG_TAPE
   fprintf(pfoDebug, "--- %ld bytes\r\n", lSampleLength);
   #endif

   dword dwTapePulseCycles = 3500000L / (1000000L / (256 - bSampleRate)); // length of one pulse in ZX Spectrum T states
   dword dwCompressedSize = lSampleLength >> 3; // 8x data reduction
   if (dwCompressedSize > 0x00ffffff) { // we only support one direct recording block right now
      return ERR_TAP_BAD_VOC;
   }
   pbTapeImage = static_cast<byte *>(malloc(dwCompressedSize+1+8+6));
   if (pbTapeImage == nullptr) { // check if the memory allocation has failed
      return ERR_OUT_OF_MEMORY;
   }
   *pbTapeImage = 0x20; // start off with a pause block
   *reinterpret_cast<word *>(pbTapeImage+1) = 2000; // set the length to 2 seconds

   *(pbTapeImage+3) = 0x15; // direct recording block
   *reinterpret_cast<word *>(pbTapeImage+4) = static_cast<word>(dwTapePulseCycles); // number of T states per sample
   *reinterpret_cast<word *>(pbTapeImage+6) = 0; // pause after block
   *(pbTapeImage+8) = lSampleLength & 7 ? lSampleLength & 7 : 8; // bits used in last byte
   *reinterpret_cast<dword *>(pbTapeImage+9) = dwCompressedSize & 0x00ffffff; // data length
   pbTapeImagePtr = pbTapeImage + 12;

   lOffset = lInitialOffset;
   bolDone = false;
   dword dwBit = 8;
   byte bByte = 0;
   while ((!bolDone) && (lOffset < lFileSize)) {
      fseek(pfile, lOffset, SEEK_SET);
      if(fread(pbPtr, 1, 1, pfile) != 1) { // read block ID
        return ERR_TAP_BAD_VOC;
      }
      switch(*pbPtr) {
         case 0x0: // terminator
            bolDone = true;
            break;
         case 0x1: // sound data
            if(fread(pbPtr, 3+2, 1, pfile) != 1) { // get block size and sound info
              return ERR_TAP_BAD_VOC;
            }
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr) & 0x00ffffff) + 4;
            lSampleLength = iBlockLength - 6;
            pbVocDataBlock = static_cast<byte *>(malloc(lSampleLength));
            if (pbVocDataBlock == nullptr) {
               tape_eject();
               return ERR_OUT_OF_MEMORY;
            }
            if(fread(pbVocDataBlock, lSampleLength, 1, pfile) != 1) {
              return ERR_TAP_BAD_VOC;
            }
            pbVocDataBlockPtr = pbVocDataBlock;
            for (int iBytePos = 0; iBytePos < lSampleLength; iBytePos++) {
               byte bVocSample = *pbVocDataBlockPtr++;
               dwBit--;
               if (bVocSample > VOC_THRESHOLD) {
                  bByte |= bit_values[dwBit];
               }
               if (!dwBit) { // got all 8 bits?
                  *pbTapeImagePtr++ = bByte;
                  dwBit = 8;
                  bByte = 0;
               }
            }
            free(pbVocDataBlock);
            break;
         case 0x2: // sound continue
            if(fread(pbPtr, 3, 1, pfile) != 1) { // get block size
              return ERR_TAP_BAD_VOC;
            }
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr) & 0x00ffffff) + 4;
            lSampleLength = iBlockLength - 4;
            pbVocDataBlock = static_cast<byte *>(malloc(lSampleLength));
            if (pbVocDataBlock == nullptr) {
               tape_eject();
               return ERR_OUT_OF_MEMORY;
            }
            if(fread(pbVocDataBlock, lSampleLength, 1, pfile) != 1) {
              return ERR_TAP_BAD_VOC;
            }
            pbVocDataBlockPtr = pbVocDataBlock;
            for (int iBytePos = 0; iBytePos < lSampleLength; iBytePos++) {
               byte bVocSample = *pbVocDataBlockPtr++;
               dwBit--;
               if (bVocSample > VOC_THRESHOLD) {
                  bByte |= bit_values[dwBit];
               }
               if (!dwBit) { // got all 8 bits?
                  *pbTapeImagePtr++ = bByte;
                  dwBit = 8;
                  bByte = 0;
               }
            }
            free(pbVocDataBlock);
            break;
         case 0x3: // silence
            iBlockLength = 4;
            lSampleLength = *reinterpret_cast<word *>(pbPtr) + 1;
            for (int iBytePos = 0; iBytePos < lSampleLength; iBytePos++) {
               dwBit--;
               if (!dwBit) { // got all 8 bits?
                  *pbTapeImagePtr++ = bByte;
                  dwBit = 8;
                  bByte = 0;
               }
            }
            break;
         case 0x4: // marker
            iBlockLength = 3;
            break;
         case 0x5: // ascii
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr) & 0x00ffffff) + 4;
            break;
      }
      lOffset += iBlockLength;
   }

   *pbTapeImagePtr = 0x20; // end with a pause block
   *reinterpret_cast<word *>(pbTapeImagePtr+1) = 2000; // set the length to 2 seconds

   pbTapeImageEnd = pbTapeImagePtr + 3;

   Tape_Rewind();
   return 0;
}



int emulator_patch_ROM (void)
{
   byte *pbPtr;

   std::string romFilename = CPC.rom_path + "/" + chROMFile[CPC.model];
   if ((pfileObject = fopen(romFilename.c_str(), "rb")) != nullptr) { // load CPC OS + Basic
      if(fread(pbROMlo, 2*16384, 1, pfileObject) != 1) {
        fclose(pfileObject);
        return ERR_NOT_A_CPC_ROM;
      }
      fclose(pfileObject);
   } else {
      return ERR_CPC_ROM_MISSING;
   }

   if (CPC.keyboard) {
      pbPtr = pbROMlo;
      switch(CPC.model) {
         case 0: // 464
            pbPtr += 0x1d69; // location of the keyboard translation table
            break;
         case 1: // 664
         case 2: // 6128
            pbPtr += 0x1eef; // location of the keyboard translation table
            break;
      }
      if (pbPtr != pbROMlo) {
         memcpy(pbPtr, cpc_keytrans[CPC.keyboard-1], 240); // patch the CPC OS ROM with the chosen keyboard layout
         pbPtr = pbROMlo + 0x3800;
         memcpy(pbPtr, cpc_charset[CPC.keyboard-1], 2048); // add the corresponding character set
      }
   }

   return 0;
}



void emulator_reset (bool bolMF2Reset)
{
   int n;

// Z80
   memset(&z80, 0, sizeof(z80)); // clear all Z80 registers and support variables
   _IX =
  _IY = 0xffff; // IX and IY are FFFF after a reset!
   _F = Zflag; // set zero flag
   z80.break_point = 0xffffffff; // clear break point

// CPC
   CPC.cycle_count = CYCLE_COUNT_INIT;
   memset(keyboard_matrix, 0xff, sizeof(keyboard_matrix)); // clear CPC keyboard matrix
   CPC.tape_motor = 0;
   CPC.tape_play_button = 0;
   CPC.printer_port = 0xff;

// VDU
   memset(&VDU, 0, sizeof(VDU)); // clear VDU data structure
   VDU.flag_drawing = 1;

// CRTC
   crtc_reset();

// Gate Array
   memset(&GateArray, 0, sizeof(GateArray)); // clear GA data structure
   GateArray.scr_mode =
   GateArray.requested_scr_mode = 1; // set to mode 1
   ga_init_banking();

// PPI
   memset(&PPI, 0, sizeof(PPI)); // clear PPI data structure

// PSG
   PSG.control = 0;
   ResetAYChipEmulation();

// FDC
   memset(&FDC, 0, sizeof(FDC)); // clear FDC data structure
   FDC.phase = CMD_PHASE;
   FDC.flags = STATUSDRVA_flag | STATUSDRVB_flag;

// memory
   if (bolMF2Reset) {
      memset(pbRAM, 0, 64*1024); // clear only the first 64K of CPC memory
   } else {
      memset(pbRAM, 0, CPC.ram_size*1024); // clear all memory used for CPC RAM
      if (pbMF2ROM) {
         memset(pbMF2ROM+8192, 0, 8192); // clear the MF2's RAM area
      }
   }
   for (n = 0; n < 4; n++) { // initialize active read/write bank configuration
      membank_read[n] = membank_config[0][n];
      membank_write[n] = membank_config[0][n];
   }
   membank_read[0] = pbROMlo; // 'page in' lower ROM
   membank_read[3] = pbROMhi; // 'page in' upper ROM

// Multiface 2
   dwMF2Flags = 0;
   dwMF2ExitAddr = 0xffffffff; // clear MF2 return address
   if ((pbMF2ROM) && (pbMF2ROMbackup)) {
      memcpy(pbMF2ROM, pbMF2ROMbackup, 8192); // copy the MF2 ROM to its proper place
   }
}



int emulator_init (void)
{
   int iErr, iRomNum;
   char *pchRomData;

   pbGPBuffer = new byte [128*1024]; // attempt to allocate the general purpose buffer
   pbRAM = new byte [CPC.ram_size*1024]; // allocate memory for desired amount of RAM
   pbROMlo = new byte [32*1024]; // allocate memory for 32K of ROM
   if ((!pbGPBuffer) || (!pbRAM) || (!pbROMlo)) {
      return ERR_OUT_OF_MEMORY;
   }
   pbROMhi =
   pbExpansionROM = pbROMlo + 16384;
   memset(memmap_ROM, 0, sizeof(memmap_ROM[0]) * 256); // clear the expansion ROM map
   ga_init_banking(); // init the CPC memory banking map
   if ((iErr = emulator_patch_ROM())) {
      return iErr;
   }

   for (iRomNum = 0; iRomNum < 16; iRomNum++) { // loop for ROMs 0-15
      if (!CPC.rom_file[iRomNum].empty()) { // is a ROM image specified for this slot?
         pchRomData = new char [16384]; // allocate 16K
         memset(pchRomData, 0, 16384); // clear memory
         std::string romFilename = CPC.rom_path + "/" + CPC.rom_file[iRomNum];
         if ((pfileObject = fopen(romFilename.c_str(), "rb")) != nullptr) { // attempt to open the ROM image
            if(fread(pchRomData, 128, 1, pfileObject) != 1) { // read 128 bytes of ROM data
              fclose(pfileObject);
              return ERR_NOT_A_CPC_ROM;
            }
            word checksum = 0;
            for (int n = 0; n < 0x43; n++) {
               checksum += pchRomData[n];
            }
            if (checksum == ((pchRomData[0x43] << 8) + pchRomData[0x44])) { // if the checksum matches, we got us an AMSDOS header
               if(fread(pchRomData, 128, 1, pfileObject) != 1) { // skip it
                 fclose(pfileObject);
                 return ERR_NOT_A_CPC_ROM;
               }
            }
            if (!(pchRomData[0] & 0xfc)) { // is it a valid CPC ROM image (0 = forground, 1 = background, 2 = extension)?
               if(fread(pchRomData+128, 16384-128, 1, pfileObject) != 1) { // read the rest of the ROM file
                 fclose(pfileObject);
                 return ERR_NOT_A_CPC_ROM;
               }
               memmap_ROM[iRomNum] = reinterpret_cast<byte *>(pchRomData); // update the ROM map
            } else { // not a valid ROM file
               fprintf(stderr, "ERROR: %s is not a CPC ROM file - clearing ROM slot %d.\n", CPC.rom_file[iRomNum].c_str(), iRomNum);
               delete [] pchRomData; // free memory on error
               CPC.rom_file[iRomNum] = "";
            }
            fclose(pfileObject);
         } else { // file not found
            fprintf(stderr, "ERROR: The %s file is missing - clearing ROM slot %d.\n", CPC.rom_file[iRomNum].c_str(), iRomNum);
            delete [] pchRomData; // free memory on error
            CPC.rom_file[iRomNum] = "";
         }
      }
   }
   if (CPC.mf2) { // Multiface 2 enabled?
      if (!pbMF2ROM) {
         pbMF2ROM = new byte [16384]; // allocate the space needed for the Multiface 2: 8K ROM + 8K RAM
         pbMF2ROMbackup = new byte [8192]; // allocate the space needed for the backup of the MF2 ROM
         if ((!pbMF2ROM) || (!pbMF2ROMbackup)) {
            return ERR_OUT_OF_MEMORY;
         }
         memset(pbMF2ROM, 0, 16384); // clear memory
         std::string romFilename = CPC.rom_path + "/" + CPC.rom_mf2;
         bool MF2error = false;
         if ((pfileObject = fopen(romFilename.c_str(), "rb")) != nullptr) { // attempt to open the ROM image
            if((fread(pbMF2ROMbackup, 8192, 1, pfileObject) != 1) || (memcmp(pbMF2ROMbackup+0x0d32, "MULTIFACE 2", 11) != 0)) { // does it have the required signature?
               fprintf(stderr, "ERROR: The file selected as the MF2 ROM is either corrupt or invalid.\n");
               MF2error = true;
            }
            fclose(pfileObject);
         } else { // error opening file
            fprintf(stderr, "ERROR: The file selected as the MF2 ROM (%s) couldn't be opened.\n", romFilename.c_str());
            MF2error = true;
         }
         if(MF2error) {
           delete [] pbMF2ROMbackup;
           delete [] pbMF2ROM;
           pbMF2ROM = nullptr;
           pbMF2ROMbackup = nullptr;
           CPC.rom_mf2 = "";
           CPC.mf2 = 0; // disable MF2 support
         }
      }
   }

   emulator_reset(false);
   CPC.paused &= ~1;

   return 0;
}



void emulator_shutdown (void)
{
   int iRomNum;

   delete [] pbMF2ROMbackup;
   delete [] pbMF2ROM;
   pbMF2ROM = nullptr;
   pbMF2ROMbackup = nullptr;
   for (iRomNum = 2; iRomNum < 16; iRomNum++) // loop for ROMs 2-15
   {
      if (memmap_ROM[iRomNum] != nullptr) // was a ROM assigned to this slot?
         delete [] memmap_ROM[iRomNum]; // if so, release the associated memory
   }

   delete [] pbROMlo;
   delete [] pbRAM;
   delete [] pbGPBuffer;
}



int printer_start (void)
{
   if (!pfoPrinter) {
      if(!(pfoPrinter = fopen(CPC.printer_file.c_str(), "wb"))) {
         return 0; // failed to open/create file
      }
   }
   return 1; // ready to capture printer output
}



void printer_stop (void)
{
   if (pfoPrinter) {
      fclose(pfoPrinter);
   }
   pfoPrinter = nullptr;
}



void audio_update (void *userdata __attribute__((unused)), byte *stream, int len)
{
   memcpy(stream, pbSndBuffer, len);
   dwSndBufferCopied = 1;
}



int audio_align_samples (int given)
{
   int actual = 1;
   while (actual < given) {
      actual <<= 1;
   }
   return actual; // return the closest match as 2^n
}




int audio_init (void)
{
   SDL_AudioSpec *desired, *obtained;

   if (!CPC.snd_enabled) {
      return 0;
   }

   desired = static_cast<SDL_AudioSpec *>(malloc(sizeof(SDL_AudioSpec)));
   obtained = static_cast<SDL_AudioSpec *>(malloc(sizeof(SDL_AudioSpec)));

   desired->freq = freq_table[CPC.snd_playback_rate];
   desired->format = CPC.snd_bits ? AUDIO_S16LSB : AUDIO_S8;
   desired->channels = CPC.snd_stereo+1;
   desired->samples = audio_align_samples(desired->freq / 50); // desired is 20ms at the given frequency
   desired->callback = audio_update;
   desired->userdata = nullptr;

   if (SDL_OpenAudio(desired, obtained) < 0) {
      fprintf(stderr, "Could not open audio: %s\n", SDL_GetError());
      return 1;
   }

   free(desired);
   audio_spec = obtained;

   CPC.snd_buffersize = audio_spec->size; // size is samples * channels * bytes per sample (1 or 2)
   pbSndBuffer = static_cast<byte *>(malloc(CPC.snd_buffersize)); // allocate the sound data buffer
   pbSndBufferEnd = pbSndBuffer + CPC.snd_buffersize;
   memset(pbSndBuffer, 0, CPC.snd_buffersize);
   CPC.snd_bufferptr = pbSndBuffer; // init write cursor

   InitAY();

   for (int n = 0; n < 16; n++) {
      SetAYRegister(n, PSG.RegisterAY.Index[n]); // init sound emulation with valid values
   }

   return 0;
}



void audio_shutdown (void)
{
   SDL_CloseAudio();
   if (pbSndBuffer) {
      free(pbSndBuffer);
   }
   if (audio_spec) {
      free(audio_spec);
   }
}



void audio_pause (void)
{
   if (CPC.snd_enabled) {
      SDL_PauseAudio(1);
   }
}



void audio_resume (void)
{
   if (CPC.snd_enabled) {
      SDL_PauseAudio(0);
   }
}



int video_set_palette (void)
{
            int n;

         if (!CPC.scr_tube) {
            int n;
            for (n = 0; n < 32; n++) {
               dword red = static_cast<dword>(colours_rgb[n][0] * (CPC.scr_intensity / 10.0) * 255);
               if (red > 255) { // limit to the maximum
                  red = 255;
               }
               dword green = static_cast<dword>(colours_rgb[n][1] * (CPC.scr_intensity / 10.0) * 255);
               if (green > 255) {
                  green = 255;
               }
               dword blue = static_cast<dword>(colours_rgb[n][2] * (CPC.scr_intensity / 10.0) * 255);
               if (blue > 255) {
                  blue = 255;
               }
               colours[n].r = red;
               colours[n].g = green;
               colours[n].b = blue;
            }
         } else {
            int n;
            for (n = 0; n < 32; n++) {
               dword green = static_cast<dword>(colours_green[n] * (CPC.scr_intensity / 10.0) * 255);
               if (green > 255) {
                  green = 255;
               }
               colours[n].r = 0;
               colours[n].g = green;
               colours[n].b = 0;
         }
         }

         vid_plugin->set_palette(colours);

   for (n = 0; n < 17; n++) { // loop for all colours + border
     int i=GateArray.ink_values[n];
     GateArray.palette[n] = SDL_MapRGB(back_surface->format,colours[i].r,colours[i].g,colours[i].b);
   }

   return 0;
}



void video_set_style (void)
{
   if (vid_plugin->half_pixels)
   {
      dwXScale = 1;
      dwYScale = 1;
   }
   else
   {
      dwXScale = 2;
      dwYScale = 2;
   }
   switch (dwXScale) {
      case 1:
         CPC.scr_prerendernorm = prerender_normal_half;
         CPC.scr_prerenderbord = prerender_border_half;
         CPC.scr_prerendersync = prerender_sync_half;
         break;
      case 2:
         CPC.scr_prerendernorm = prerender_normal;
         CPC.scr_prerenderbord = prerender_border;
         CPC.scr_prerendersync = prerender_sync;
         break;
   }

   switch(CPC.scr_bpp)
   {
      case 32:
               switch(dwYScale) {
                 case 1:
                   CPC.scr_render = render32bpp;
                   break;
                 case 2:
                   CPC.scr_render = render32bpp_doubleY;
                   break;
               }
               break;

      case 24:
               switch(dwYScale) {
                 case 1:
                   CPC.scr_render = render24bpp;
                   break;
                 case 2:
                   CPC.scr_render = render24bpp_doubleY;
                   break;
               }
               break;

      case 16:
      case 15:
               switch(dwYScale) {
                 case 1:
                   CPC.scr_render = render16bpp;
                   break;
                 case 2:
                   CPC.scr_render = render16bpp_doubleY;
                   break;
               }
               break;

      case 8:
               switch(dwYScale) {
                 case 1:
                   CPC.scr_render = render8bpp;
                   break;
                 case 2:
                   CPC.scr_render = render8bpp_doubleY;
                   break;
               }
               break;
   }
}


int video_init (void)
{
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) { // initialize the video subsystem
      return ERR_VIDEO_INIT;
   }

   vid_plugin=&video_plugin_list[CPC.scr_style];

   back_surface=vid_plugin->init(vid_plugin,CPC.scr_fs_width, CPC.scr_fs_height, CPC.scr_fs_bpp, CPC.scr_window==0);

   if (!back_surface) { // attempt to set the required video mode
      fprintf(stderr, "Could not set requested video mode: %s\n", SDL_GetError());
      return ERR_VIDEO_SET_MODE;
   }

   CPC.scr_bpp = back_surface->format->BitsPerPixel; // bit depth of the surface
   video_set_style(); // select rendering style

   int iErrCode = video_set_palette(); // init CPC colours
   if (iErrCode) {
      return iErrCode;
   }

   vid_plugin->lock();
   CPC.scr_bps = back_surface->pitch / 4; // rendered screen line length (changing bytes to dwords)
   CPC.scr_line_offs = CPC.scr_bps * dwYScale;
   CPC.scr_pos =
   CPC.scr_base = static_cast<dword *>(back_surface->pixels); // memory address of back buffer

   vid_plugin->unlock();

   SDL_ShowCursor(SDL_DISABLE); // hide the mouse cursor

   SDL_WM_SetCaption("Caprice32 " VERSION_STRING, "Caprice32");

   crtc_init();

   return 0;
}



void video_shutdown (void)
{
   if (back_surface) {
      vid_plugin->unlock();
   }
   vid_plugin->close();
   SDL_QuitSubSystem(SDL_INIT_VIDEO);
}



void video_display (void)
{
   vid_plugin->flip();
}



int joysticks_init (void)
{
   if(CPC.joysticks == 0) {
      return 0;
   }

   if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
      fprintf(stderr, "Failed to initialize joystick subsystem. Error: %s\n", SDL_GetError());
      return ERR_JOYSTICKS_INIT;
   }

   int nbJoysticks = SDL_NumJoysticks();
   if (nbJoysticks < 0) {
      fprintf(stderr, "Failed to count joysticks. Error: %s\n", SDL_GetError());
      return ERR_JOYSTICKS_INIT;
   }
   if (nbJoysticks == 0) {
      fprintf(stderr, "No joystick found.\n");
      return ERR_JOYSTICKS_INIT;
   }

   if (SDL_JoystickEventState(SDL_ENABLE) != 1) {
      fprintf(stderr, "Failed to activate joystick events. Error: %s\n", SDL_GetError());
      return ERR_JOYSTICKS_INIT;
   }

   if(nbJoysticks > MAX_NB_JOYSTICKS) {
      nbJoysticks = MAX_NB_JOYSTICKS;
   }

   for(int i = 0; i < MAX_NB_JOYSTICKS; i++) {
      if(i < nbJoysticks) {
        joysticks[i] = SDL_JoystickOpen(i);
        if(joysticks[i] == nullptr) {
          fprintf(stderr, "Failed to open joystick %d. Error: %s\n", i, SDL_GetError());
          //return ERR_JOYSTICKS_INIT;
        }
      } else {
        joysticks[i] = nullptr;
      }
   }

   return 0;
}



void joysticks_shutdown (void)
{
/* This cores for an unknown reason - anyway, SDL_QuitSubSystem will do the job
   for(int i = 0; i < MAX_NB_JOYSTICKS; i++) {
      if(joysticks[i] != nullptr) {
         SDL_JoystickClose(joysticks[i]);
      }
   }
*/

   SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}



void input_swap_joy (void)
{
   dword n, pc_idx, val;

   for (n = 0; n < 6; n++) {
      pc_idx = joy_layout[n][1]; // get the PC key to change the assignment for
      if (pc_idx) {
         val = keyboard_normal[pc_idx]; // keep old value
         keyboard_normal[pc_idx] = cpc_kbd[CPC.keyboard][joy_layout[n][0]]; // assign new function
         cpc_kbd[CPC.keyboard][joy_layout[n][0]] = val; // store old value
      }
   }
}

// Recalculate emulation speed (to verify, seems to work reasonably well)
void update_cpc_speed(void)
{
   dwTicksOffset = static_cast<int>(20.0 / ((CPC.speed * 25) / 100.0));
   dwTicksTarget = SDL_GetTicks();
   dwTicksTargetFPS = dwTicksTarget;
   dwTicksTarget += dwTicksOffset;
   InitAY();
}


int input_init (void)
{
   dword n, pc_key, pc_idx, cpc_idx, cpc_key;

   memset(keyboard_normal, 0xff, sizeof(keyboard_normal));
   memset(keyboard_shift, 0xff, sizeof(keyboard_shift));
   memset(keyboard_ctrl, 0xff, sizeof(keyboard_ctrl));
   memset(keyboard_mode, 0xff, sizeof(keyboard_mode));

   for (n = 0; n < KBD_MAX_ENTRIES; n++) {
      pc_key = kbd_layout[CPC.kbd_layout][n][1]; // PC key assigned to CPC key
      if (pc_key) {
         pc_idx = pc_key & 0xffff; // strip off modifier
         cpc_idx = kbd_layout[CPC.kbd_layout][n][0];
         if (cpc_idx & MOD_EMU_KEY) {
            cpc_key = cpc_idx;
         } else {
            cpc_key = cpc_kbd[CPC.keyboard][cpc_idx];
         }
         if (pc_key & MOD_PC_SHIFT) { // key + SHIFT?
            keyboard_shift[pc_idx] = cpc_key; // copy CPC key matrix value to SHIFT table
         } else if (pc_key & MOD_PC_CTRL) { // key + CTRL?
            keyboard_ctrl[pc_idx] = cpc_key; // copy CPC key matrix value to CTRL table
         } else if (pc_key & MOD_PC_MODE) { // key + AltGr?
            keyboard_mode[pc_idx] = cpc_key; // copy CPC key matrix value to AltGr table
         } else {
            keyboard_normal[pc_idx] = cpc_key; // copy CPC key matrix value to normal table
            if (!(cpc_key & MOD_EMU_KEY)) { // not an emulator function key?
               if (keyboard_shift[pc_idx] == 0xffffffff) { // SHIFT table entry has no value yet?
                  keyboard_shift[pc_idx] = cpc_key; // duplicate entry in SHIFT table
               }
               if (keyboard_ctrl[pc_idx] == 0xffffffff) { // CTRL table entry has no value yet?
                  keyboard_ctrl[pc_idx] = cpc_key | MOD_CPC_CTRL; // duplicate entry in CTRL table
               }
               if (keyboard_mode[pc_idx] == 0xffffffff) { // AltGr table entry has no value yet?
                  keyboard_mode[pc_idx] = cpc_key; // duplicate entry in AltGr table
               }
            }
         }
      }
   }

   if (CPC.joystick_emulation) { // enable keyboard joystick emulation?
      input_swap_joy();
   }

   return 0;
}



std::string getConfigurationFilename(bool forWrite)
{
  // First look for cap32.cfg in the same directory as executable
  std::string configFilename = std::string(chAppPath) + "/cap32.cfg";
  if(access(configFilename.c_str(), F_OK) != 0) {
    // If not found, look for .cap32.cfg in the home of current user
    configFilename = std::string(getenv("HOME")) + "/.cap32.cfg";
    // If still not found, look for cap32.cfg in /etc
    if(!forWrite && access(configFilename.c_str(), F_OK) != 0) {
      configFilename = "/etc/cap32.cfg";
    }
  }
  std::cout << "Using configuration file" << (forWrite ? " to save" : "") << ": " << configFilename << std::endl;
  return configFilename;
}



t_disk_format parseDiskFormat(const std::string& format)
{
  t_disk_format result;
  dword dwVal;
  std::vector<std::string> tokens = stringutils::split(format, ',');
  if (tokens.size() < 7) { // Minimum number of values required
    return result;
  }
  dwVal = strtoul(tokens[1].c_str(), nullptr, 0);
  if ((dwVal < 1) || (dwVal > DSK_TRACKMAX)) { // invalid value?
    return result;
  }
  result.tracks = dwVal;
  dwVal = strtoul(tokens[2].c_str(), nullptr, 0);
  if ((dwVal < 1) || (dwVal > DSK_SIDEMAX)) { // invalid value?
    return result;
  }
  result.sides = dwVal;
  dwVal = strtoul(tokens[3].c_str(), nullptr, 0);
  if ((dwVal < 1) || (dwVal > DSK_SECTORMAX)) { // invalid value?
    return result;
  }
  result.sectors = dwVal;
  dwVal = strtoul(tokens[4].c_str(), nullptr, 0);
  if ((dwVal < 1) || (dwVal > 6)) { // invalid value?
    return result;
  }
  result.sector_size = dwVal;
  dwVal = strtoul(tokens[5].c_str(), nullptr, 0);
  if ((dwVal < 1) || (dwVal > 255)) { // invalid value?
    return result;
  }
  result.gap3_length = dwVal;
  dwVal = strtoul(tokens[6].c_str(), nullptr, 0);
  result.filler_byte = static_cast<byte>(dwVal);
  unsigned int i = 7;
  for (int iSide = 0; iSide < static_cast<int>(result.sides); iSide++) {
    for (int iSector = 0; iSector < static_cast<int>(result.sectors); iSector++) {
      if (i >= tokens.size()) { // value missing?
        dwVal = iSector+1;
      } else {
        dwVal = strtoul(tokens[i++].c_str(), nullptr, 0);
      }
      result.sector_ids[iSide][iSector] = static_cast<byte>(dwVal);
    }
  }
  // Fill the label only if the disk format is valid
  result.label = tokens[0];
  return result;
}



std::string serializeDiskFormat(const t_disk_format& format)
{
  std::ostringstream oss;
  oss << format.label << ",";
  oss << format.tracks << ",";
  oss << format.sides << ",";
  oss << format.sectors << ",";
  oss << format.sector_size << ",";
  oss << format.gap3_length << ",";
  oss << static_cast<unsigned int>(format.filler_byte);
  for (int iSide = 0; iSide < static_cast<int>(format.sides); iSide++) {
    for (int iSector = 0; iSector < static_cast<int>(format.sectors); iSector++) {
      oss << "," << static_cast<unsigned int>(format.sector_ids[iSide][iSector]);
    }
  }
  return oss.str();
}



void loadConfiguration (t_CPC &CPC, const std::string& configFilename)
{
   config::Config conf;
   conf.parseFile(configFilename);

   std::string appPath = chAppPath;
   const char *chFileName = configFilename.c_str();

   CPC.model = conf.getIntValue("system", "model", 2); // CPC 6128
   if (CPC.model > 2) {
      CPC.model = 2;
   }
   CPC.jumpers = conf.getIntValue("system", "jumpers", 0x1e) & 0x1e; // OEM is Amstrad, video refresh is 50Hz
   CPC.ram_size = conf.getIntValue("system", "ram_size", 128) & 0x02c0; // 128KB RAM
   if (CPC.ram_size > 576) {
      CPC.ram_size = 576;
   } else if ((CPC.model == 2) && (CPC.ram_size < 128)) {
      CPC.ram_size = 128; // minimum RAM size for CPC 6128 is 128KB
   }
   CPC.speed = conf.getIntValue("system", "speed", DEF_SPEED_SETTING); // original CPC speed
   if ((CPC.speed < MIN_SPEED_SETTING) || (CPC.speed > MAX_SPEED_SETTING)) {
      CPC.speed = DEF_SPEED_SETTING;
   }
   CPC.limit_speed = 1;
   CPC.auto_pause = conf.getIntValue("system", "auto_pause", 1) & 1;
   CPC.printer = conf.getIntValue("system", "printer", 0) & 1;
   CPC.mf2 = conf.getIntValue("system", "mf2", 0) & 1;
   CPC.keyboard = conf.getIntValue("system", "keyboard", 0);
   if (CPC.keyboard > MAX_ROM_MODS) {
      CPC.keyboard = 0;
   }
   CPC.joystick_emulation = conf.getIntValue("system", "joystick_emulation", 0) & 1;
   CPC.joysticks = conf.getIntValue("system", "joysticks", 1) & 1;
   CPC.joystick_menu_button = conf.getIntValue("system", "joystick_menu_button", 9) - 1;
   CPC.joystick_vkeyboard_button = conf.getIntValue("system", "joystick_vkeyboard_button", 10) - 1;
   CPC.resources_path = conf.getStringValue("system", "resources_path", appPath + "/resources");

   CPC.scr_fs_width = conf.getIntValue("video", "scr_width", 800);
   CPC.scr_fs_height = conf.getIntValue("video", "scr_height", 600);
   CPC.scr_fs_bpp = conf.getIntValue("video", "scr_bpp", 8);
   CPC.scr_style = conf.getIntValue("video", "scr_style", 0);
   if (CPC.scr_style >= nb_video_plugins) {
    CPC.scr_style = 0;
   }
   CPC.scr_oglfilter = conf.getIntValue("video", "scr_oglfilter", 1) & 1;
   CPC.scr_oglscanlines = conf.getIntValue("video", "scr_oglscanlines", 30);
   if (CPC.scr_oglscanlines > 100) {
      CPC.scr_oglscanlines = 30;
   }
   CPC.scr_vsync = conf.getIntValue("video", "scr_vsync", 1) & 1;
   CPC.scr_led = conf.getIntValue("video", "scr_led", 1) & 1;
   CPC.scr_fps = conf.getIntValue("video", "scr_fps", 0) & 1;
   CPC.scr_tube = conf.getIntValue("video", "scr_tube", 0) & 1;
   CPC.scr_intensity = conf.getIntValue("video", "scr_intensity", 10);
   CPC.scr_remanency = conf.getIntValue("video", "scr_remanency", 0) & 1;
   if ((CPC.scr_intensity < 5) || (CPC.scr_intensity > 15)) {
      CPC.scr_intensity = 10;
   }
   CPC.scr_window = conf.getIntValue("video", "scr_window", 0) & 1;

   CPC.snd_enabled = conf.getIntValue("sound", "enabled", 1) & 1;
   CPC.snd_playback_rate = conf.getIntValue("sound", "playback_rate", 2);
   if (CPC.snd_playback_rate > (MAX_FREQ_ENTRIES-1)) {
      CPC.snd_playback_rate = 2;
   }
   CPC.snd_bits = conf.getIntValue("sound", "bits", 1) & 1;
   CPC.snd_stereo = conf.getIntValue("sound", "stereo", 1) & 1;
   CPC.snd_volume = conf.getIntValue("sound", "volume", 80);
   if (CPC.snd_volume > 100) {
      CPC.snd_volume = 80;
   }
   CPC.snd_pp_device = conf.getIntValue("sound", "pp_device", 0) & 1;

   CPC.kbd_layout = conf.getIntValue("control", "kbd_layout", 0);
   if (CPC.kbd_layout > 3) {
      CPC.kbd_layout = 0;
   }

   CPC.max_tracksize = conf.getIntValue("file", "max_track_size", 6144-154);
   CPC.snap_path = conf.getStringValue("file", "snap_path", appPath + "/snap");
   CPC.snap_file = conf.getStringValue("file", "snap_file", "");
   CPC.snap_zip = conf.getIntValue("file", "snap_zip", 0) & 1;
   CPC.drvA_path = conf.getStringValue("file", "drvA_path", appPath + "/disk");
   CPC.drvA_file = conf.getStringValue("file", "drvA_file", "");
   CPC.drvA_zip = conf.getIntValue("file", "drvA_zip", 0) & 1;
   CPC.drvA_format = conf.getIntValue("file", "drvA_format", DEFAULT_DISK_FORMAT);
   CPC.drvB_path = conf.getStringValue("file", "drvB_path", appPath + "/disk");
   CPC.drvB_file = conf.getStringValue("file", "drvB_file", "");
   CPC.drvB_zip = conf.getIntValue("file", "drvB_zip", 0) & 1;
   CPC.drvB_format = conf.getIntValue("file", "drvB_format", DEFAULT_DISK_FORMAT);
   CPC.tape_path = conf.getStringValue("file", "tape_path", appPath + "/tape");
   CPC.tape_file = conf.getStringValue("file", "tape_file", "");
   CPC.tape_zip = conf.getIntValue("file", "tape_zip", 0) & 1;

   int iFmt = FIRST_CUSTOM_DISK_FORMAT;
   for (int i = iFmt; i < MAX_DISK_FORMAT; i++) { // loop through all user definable disk formats
      char chFmtId[14];
      sprintf(chFmtId, "fmt%02d", i); // build format ID
      std::string formatString = conf.getStringValue("file", chFmtId, "");
      disk_format[iFmt] = parseDiskFormat(formatString);
      if (!disk_format[iFmt].label.empty()) { // found format definition for this slot?
         iFmt++; // entry is valid
      }
   }
   CPC.printer_file = conf.getStringValue("file", "printer_file", appPath + "/printer.dat");
   CPC.sdump_file = conf.getStringValue("file", "sdump_file", appPath + "/screen.png");

   CPC.rom_path = conf.getStringValue("rom", "rom_path", appPath + "/rom");
   for (int iRomNum = 0; iRomNum < 16; iRomNum++) { // loop for ROMs 0-15
      char chRomId[14];
      sprintf(chRomId, "slot%02d", iRomNum); // build ROM ID
      CPC.rom_file[iRomNum] = conf.getStringValue("rom", chRomId, "");
   }
   if ((pfileObject = fopen(chFileName, "rt")) == nullptr) {
      CPC.rom_file[7] = "amsdos.rom"; // insert AMSDOS in slot 7 if the config file does not exist yet
   } else {
      fclose(pfileObject);
   }
   CPC.rom_mf2 = conf.getStringValue("rom", "rom_mf2", "");
}



void saveConfiguration (t_CPC &CPC, const std::string& configFilename)
{
   config::Config conf;

   conf.setIntValue("system", "model", CPC.model);
   conf.setIntValue("system", "jumpers", CPC.jumpers);

   conf.setIntValue("system", "ram_size", CPC.ram_size); // 128KB RAM
   conf.setIntValue("system", "speed", CPC.speed); // original CPC speed
   conf.setIntValue("system", "auto_pause", CPC.auto_pause);
   conf.setIntValue("system", "printer", CPC.printer);
   conf.setIntValue("system", "mf2", CPC.mf2);
   conf.setIntValue("system", "keyboard", CPC.keyboard);
   conf.setIntValue("system", "joystick_emulation", CPC.joystick_emulation);
   conf.setIntValue("system", "joysticks", CPC.joysticks);
   conf.setStringValue("system", "resources_path", CPC.resources_path);

   conf.setIntValue("video", "scr_width", CPC.scr_fs_width);
   conf.setIntValue("video", "scr_height", CPC.scr_fs_height);
   conf.setIntValue("video", "scr_bpp", CPC.scr_fs_bpp);
   conf.setIntValue("video", "scr_style", CPC.scr_style);
   conf.setIntValue("video", "scr_oglfilter", CPC.scr_oglfilter);
   conf.setIntValue("video", "scr_oglscanlines", CPC.scr_oglscanlines);
   conf.setIntValue("video", "scr_vsync", CPC.scr_vsync);
   conf.setIntValue("video", "scr_led", CPC.scr_led);
   conf.setIntValue("video", "scr_fps", CPC.scr_fps);
   conf.setIntValue("video", "scr_tube", CPC.scr_tube);
   conf.setIntValue("video", "scr_intensity", CPC.scr_intensity);
   conf.setIntValue("video", "scr_remanency", CPC.scr_remanency);
   conf.setIntValue("video", "scr_window", CPC.scr_window);

   conf.setIntValue("sound", "enabled", CPC.snd_enabled);
   conf.setIntValue("sound", "playback_rate", CPC.snd_playback_rate);
   conf.setIntValue("sound", "bits", CPC.snd_bits);
   conf.setIntValue("sound", "stereo", CPC.snd_stereo);
   conf.setIntValue("sound", "volume", CPC.snd_volume);
   conf.setIntValue("sound", "pp_device", CPC.snd_pp_device);

   conf.setIntValue("control", "kbd_layout", CPC.kbd_layout);

   conf.setIntValue("file", "max_track_size", CPC.max_tracksize);
   conf.setStringValue("file", "snap_path", CPC.snap_path);
   conf.setStringValue("file", "snap_file", CPC.snap_file);
   conf.setIntValue("file", "snap_zip", CPC.snap_zip);
   conf.setStringValue("file", "drvA_path", CPC.drvA_path);
   conf.setStringValue("file", "drvA_file", CPC.drvA_file);
   conf.setIntValue("file", "drvA_zip", CPC.drvA_zip);
   conf.setIntValue("file", "drvA_format", CPC.drvA_format);
   conf.setStringValue("file", "drvB_path", CPC.drvB_path);
   conf.setStringValue("file", "drvB_file", CPC.drvB_file);
   conf.setIntValue("file", "drvB_zip", CPC.drvB_zip);
   conf.setIntValue("file", "drvB_format", CPC.drvB_format);
   conf.setStringValue("file", "tape_path", CPC.tape_path);
   conf.setStringValue("file", "tape_file", CPC.tape_file);
   conf.setIntValue("file", "tape_zip", CPC.tape_zip);

   for (int iFmt = FIRST_CUSTOM_DISK_FORMAT; iFmt < MAX_DISK_FORMAT; iFmt++) { // loop through all user definable disk formats
      char chFmtId[14];
      sprintf(chFmtId, "fmt%02d", iFmt); // build format ID
      conf.setStringValue("file", chFmtId, serializeDiskFormat(disk_format[iFmt]));
   }
   conf.setStringValue("file", "printer_file", CPC.printer_file);
   conf.setStringValue("file", "sdump_file", CPC.sdump_file);

   conf.setStringValue("rom", "rom_path", CPC.rom_path);
   for (int iRomNum = 0; iRomNum < 16; iRomNum++) { // loop for ROMs 0-15
      char chRomId[14];
      sprintf(chRomId, "slot%02d", iRomNum); // build ROM ID
      conf.setStringValue("rom", chRomId, CPC.rom_file[iRomNum]);
   }
   conf.setStringValue("rom", "rom_mf2", CPC.rom_mf2);

   conf.saveToFile(configFilename);
}



void doCleanUp (void)
{
   printer_stop();
   emulator_shutdown();

   dsk_eject(&driveA);
   dsk_eject(&driveB);
   tape_eject();
   if (zip_info.pchFileNames) {
      free(zip_info.pchFileNames);
   }

   joysticks_shutdown();
   audio_shutdown();
   video_shutdown();

   #ifdef DEBUG
   if(pfoDebug) {
     fclose(pfoDebug);
   }
   #endif

   SDL_Quit();
}



// TODO(cpitrat): refactor
void parseArgs (int argc, const char **argv, t_CPC& CPC)
{
   bool have_DSKA = false;
   bool have_DSKB = false;
   bool have_SNA = false;
   bool have_TAP = false;

   for (int i = 1; i < argc; i++) { // loop for all command line arguments
      std::string fullpath = stringutils::trim(argv[i], '"'); // remove quotes if arguments quoted
      if (fullpath.length() > 5) { // minumum for a valid filename
         int pos = fullpath.length() - 4;
         std::string dirname;
         std::string filename;
         bool zip = false;
         std::string extension = stringutils::lower(fullpath.substr(pos));
         stringutils::splitPath(fullpath, dirname, filename);
         if (extension == ".zip") { // are we dealing with a zip archive?
            zip_info.filename = fullpath;
            zip_info.extensions = ".dsk.sna.cdt.voc";
            if (zip::dir(&zip_info)) {
               continue; // error or nothing relevant found
            } else {
               dirname = fullpath;
               filename = zip_info.pchFileNames;
               pos = filename.length() - 4;
               extension = filename.substr(pos); // grab the extension
               zip = true;
            }
         }
         if (extension == ".dsk") { // a disk image?
            if(!have_DSKA) {
               CPC.drvA_path = dirname; // if the image loads, copy the infos to the config structure
               CPC.drvA_file = filename;
               CPC.drvA_zip = (zip ? 1 : 0);
               have_DSKA = true;
            } else if(!have_DSKB) {
               CPC.drvB_path = dirname; // if the image loads, copy the infos to the config structure
               CPC.drvB_file = filename;
               CPC.drvB_zip = (zip ? 1 : 0);
               have_DSKB = true;
            }
         }
         if ((!have_SNA) && (extension == ".sna")) {
            CPC.snap_path = dirname;
            CPC.snap_file = filename;
            CPC.snap_zip = (zip ? 1 : 0);
            have_SNA = true;
         }
         if ((!have_TAP) && (extension == ".cdt" || extension == ".voc")) {
            CPC.tape_path = dirname;
            CPC.tape_file = filename;
            CPC.tape_zip = (zip ? 1 : 0);
            have_TAP = true;
         }
      }
   }
}



void showVKeyboard()
{
  // Activate virtual keyboard
  audio_pause();
  SDL_ShowCursor(SDL_ENABLE);
  CapriceGui capriceGui;
  capriceGui.Init();
  // guiBackSurface will allow the GUI to capture the current frame
  SDL_Surface* guiBackSurface(SDL_CreateRGBSurface(SDL_SWSURFACE, back_surface->w, back_surface->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000));
  SDL_BlitSurface(back_surface, nullptr, guiBackSurface, nullptr);
  CapriceVKeyboardView capriceVKeyboardView(back_surface, guiBackSurface, CRect(0, 0, back_surface->w, back_surface->h));
  capriceGui.SetMouseVisibility(true);
  capriceGui.Exec();
  auto newEvents = capriceVKeyboardView.GetEvents();
  virtualKeyboardEvents.splice(virtualKeyboardEvents.end(), newEvents);
  SDL_FreeSurface(guiBackSurface);
  // Clear SDL surface:
  SDL_FillRect(back_surface, nullptr, SDL_MapRGB(back_surface->format, 0, 0, 0));
  SDL_ShowCursor(SDL_DISABLE);
  audio_resume();
}



void showGui()
{
  // Activate gui
  audio_pause();
  SDL_ShowCursor(SDL_ENABLE);
  CapriceGui capriceGui;
  capriceGui.Init();
  // guiBackSurface will allow the GUI to capture the current frame
  SDL_Surface* guiBackSurface(SDL_CreateRGBSurface(SDL_SWSURFACE, back_surface->w, back_surface->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000));
  SDL_BlitSurface(back_surface, nullptr, guiBackSurface, nullptr);
  CapriceGuiView capriceGuiView(back_surface, guiBackSurface, CRect(0, 0, back_surface->w, back_surface->h));
  capriceGui.SetMouseVisibility(true);
  capriceGui.Exec();
  SDL_FreeSurface(guiBackSurface);
  // Clear SDL surface:
  SDL_FillRect(back_surface, nullptr, SDL_MapRGB(back_surface->format, 0, 0, 0));
  SDL_ShowCursor(SDL_DISABLE);
  audio_resume();
}



int cap32_main (int argc, char **argv)
{
   dword dwOffset;
   int iExitCondition;
   bool bolDone;
   SDL_Event event;

   if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) < 0) { // initialize SDL
      fprintf(stderr, "SDL_Init() failed: %s\n", SDL_GetError());
      exit(-1);
   }
   atexit(doCleanUp); // install the clean up routine

   if(getcwd(chAppPath, sizeof(chAppPath)-1) == nullptr) { // get the location of the executable
      fprintf(stderr, "getcwd failed: %s\n", strerror(errno));
      exit(-1);
   }

   loadConfiguration(CPC, getConfigurationFilename()); // retrieve the emulator configuration
   if (CPC.printer) {
      if (!printer_start()) { // start capturing printer output, if enabled
         CPC.printer = 0;
      }
   }

   z80_init_tables(); // init Z80 emulation

   if (input_init()) {
      fprintf(stderr, "input_init() failed. Aborting.\n");
      exit(-1);
   }

   if (video_init()) {
      fprintf(stderr, "video_init() failed. Aborting.\n");
      exit(-1);
   }

   if (audio_init()) {
      fprintf(stderr, "audio_init() failed. Disabling sound.\n");
      CPC.snd_enabled = 0; // disable sound emulation
   }

   if (joysticks_init()) {
      fprintf(stderr, "joysticks_init() failed. Disabling joysticks.\n");
      CPC.joysticks = 0;
   }

   if (emulator_init()) {
      fprintf(stderr, "emulator_init() failed. Aborting.\n");
      exit(-1);
   }

   #ifdef DEBUG
   pfoDebug = fopen("./debug.txt", "wt");
   #endif

   parseArgs(argc, const_cast<const char**>(argv), CPC);

   // TODO(cpitrat): refactor this: duplication + should be tested + cleanup
   memset(&driveA, 0, sizeof(t_drive)); // clear disk drive A data structure
   if (!CPC.drvA_file.empty()) { // insert disk in drive A?
      char chFileName[_MAX_PATH + 1];
      char *pchPtr;

      if (CPC.drvA_zip) { // compressed image?
         zip_info.filename = CPC.drvA_path; // pchPath already has path and zip file combined
         zip_info.extensions = ".dsk";
         if (!zip::dir(&zip_info)) { // parse the zip for relevant files
            dword n;
            pchPtr = zip_info.pchFileNames;
            for (n = zip_info.iFiles; n; n--) { // loop through all entries
               if (!strcasecmp(CPC.drvA_file.c_str(), pchPtr)) { // do we have a match?
                  break;
               }
               pchPtr += strlen(pchPtr) + 5; // skip offset
            }
            if (n) {
               FILE *file = nullptr;
               zip_info.dwOffset = *reinterpret_cast<dword *>(pchPtr + (strlen(pchPtr)+1)); // get the offset into the zip archive
               if (!zip::extract(zip_info, &file)) {
                  dsk_load(file, &driveA);
                  fclose(file);
               }
            }
         } else {
            CPC.drvA_zip = 0;
         }
      } else {
         strncpy(chFileName, CPC.drvA_path.c_str(), sizeof(chFileName)-1);
         strncat(chFileName, CPC.drvA_file.c_str(), sizeof(chFileName)-1 - strlen(chFileName));
         dsk_load(chFileName, &driveA);
      }
   }
   memset(&driveB, 0, sizeof(t_drive)); // clear disk drive B data structure
   if (!CPC.drvB_file.empty()) { // insert disk in drive B?
      char chFileName[_MAX_PATH + 1];
      char *pchPtr;

      if (CPC.drvB_zip) { // compressed image?
         zip_info.filename = CPC.drvB_path; // pchPath already has path and zip file combined
         zip_info.extensions = ".dsk";
         if (!zip::dir(&zip_info)) { // parse the zip for relevant files
            dword n;
            pchPtr = zip_info.pchFileNames;
            for (n = zip_info.iFiles; n; n--) { // loop through all entries
               if (!strcasecmp(CPC.drvB_file.c_str(), pchPtr)) { // do we have a match?
                  break;
               }
               pchPtr += strlen(pchPtr) + 5; // skip offset
            }
            if (n) {
               FILE *file = nullptr;
               zip_info.dwOffset = *reinterpret_cast<dword *>(pchPtr + (strlen(pchPtr)+1)); // get the offset into the zip archive
               if (!zip::extract(zip_info, &file)) {
                  dsk_load(file, &driveB);
                  fclose(file);
               }
            }
         }
         else {
            CPC.drvB_zip = 0;
         }
      }
      else {
         strncpy(chFileName, CPC.drvB_path.c_str(), sizeof(chFileName)-1);
         strncat(chFileName, CPC.drvB_file.c_str(), sizeof(chFileName)-1 - strlen(chFileName));
         dsk_load(chFileName, &driveB);
      }
   }
   if (!CPC.tape_file.empty()) { // insert a tape?
      char chFileName[_MAX_PATH + 1];
      char *pchPtr;

      if (CPC.tape_zip) { // compressed image?
         zip_info.filename = CPC.tape_path; // pchPath already has path and zip file combined
         zip_info.extensions = ".cdt.voc";
         if (!zip::dir(&zip_info)) { // parse the zip for relevant files
            dword n;
            pchPtr = zip_info.pchFileNames;
            for (n = zip_info.iFiles; n; n--) { // loop through all entries
               if (!strcasecmp(CPC.tape_file.c_str(), pchPtr)) { // do we have a match?
                  break;
               }
               pchPtr += strlen(pchPtr) + 5; // skip offset
            }
            if (n) {
               FILE *file = nullptr;
               zip_info.dwOffset = *reinterpret_cast<dword *>(pchPtr + (strlen(pchPtr)+1)); // get the offset into the zip archive
               if(!zip::extract(zip_info, &file)) {
                 tape_insert(file);
                 fclose(file);
               }
            }
            else {
               CPC.tape_zip = 0;
            }
         }
         else {
            CPC.tape_zip = 0;
         }
      }
      else {
         strncpy(chFileName, CPC.tape_path.c_str(), sizeof(chFileName)-1);
         strncat(chFileName, CPC.tape_file.c_str(), sizeof(chFileName)-1 - strlen(chFileName));
         tape_insert(chFileName);
      }
   }
   if (!CPC.snap_file.empty()) { // load a snapshot ?
      char chFileName[_MAX_PATH + 1];
      char *pchPtr;

      if (CPC.snap_zip) { // compressed image?
         zip_info.filename = CPC.snap_path; // pchPath already has path and zip file combined
         zip_info.extensions = ".sna";
         if(!zip::dir(&zip_info)) { // parse the zip for relevant files
            dword n;
            pchPtr = zip_info.pchFileNames;
            for (n = zip_info.iFiles; n; n--) { // loop through all entries
               if (!strcasecmp(CPC.snap_file.c_str(), pchPtr)) { // do we have a match?
                  break;
               }
               pchPtr += strlen(pchPtr) + 5; // skip offset
            }
            if (n) {
              FILE *file = nullptr;
               zip_info.dwOffset = *reinterpret_cast<dword *>(pchPtr + (strlen(pchPtr)+1)); // get the offset into the zip archive
               if (!zip::extract(zip_info, &file)) {
                  snapshot_load(file);
                  fclose(file);
               }
            }
            else {
               CPC.snap_zip = 0;
            }
         }
         else {
            CPC.snap_zip = 0;
         }
      }
      else {
         strncpy(chFileName, CPC.snap_path.c_str(), sizeof(chFileName)-1);
         strncat(chFileName, CPC.snap_file.c_str(), sizeof(chFileName)-1 - strlen(chFileName));
         snapshot_load(chFileName);
      }
   }

// ----------------------------------------------------------------------------

   // TODO: deduplicate this and update_cpc_speed
   dwTicksOffset = static_cast<int>(20.0 / (CPC.speed * 25) / 100.0);
   dwTicksTarget = SDL_GetTicks();
   dwTicksTargetFPS = dwTicksTarget;
   dwTicksTarget += dwTicksOffset;

   audio_resume();

   iExitCondition = EC_FRAME_COMPLETE;
   bolDone = false;

   while (!bolDone) {
      if(!virtualKeyboardEvents.empty() && lastVirtualEventTicks + 100 < SDL_GetTicks()) {
        lastVirtualEventTicks = SDL_GetTicks();
        SDL_PushEvent(&virtualKeyboardEvents.front());
        virtualKeyboardEvents.pop_front();
      }
      while (SDL_PollEvent(&event)) {
         switch (event.type) {
            case SDL_KEYDOWN:
               {
                  dword cpc_key;
                  if (event.key.keysym.mod & KMOD_SHIFT) { // PC SHIFT key held down?
                     cpc_key = keyboard_shift[event.key.keysym.sym]; // consult the SHIFT table
                  } else if (event.key.keysym.mod & KMOD_CTRL) { // PC CTRL key held down?
                     cpc_key = keyboard_ctrl[event.key.keysym.sym]; // consult the CTRL table
                  } else if (event.key.keysym.mod & KMOD_MODE) { // PC AltGr key held down?
                     cpc_key = keyboard_mode[event.key.keysym.sym]; // consult the AltGr table
                  } else {
                     cpc_key = keyboard_normal[event.key.keysym.sym]; // consult the normal table
                  }
                  if ((!(cpc_key & MOD_EMU_KEY)) && (!CPC.paused) && (static_cast<byte>(cpc_key) != 0xff)) {
                     keyboard_matrix[static_cast<byte>(cpc_key) >> 4] &= ~bit_values[static_cast<byte>(cpc_key) & 7]; // key is being held down
                     if (cpc_key & MOD_CPC_SHIFT) { // CPC SHIFT key required?
                        keyboard_matrix[0x25 >> 4] &= ~bit_values[0x25 & 7]; // key needs to be SHIFTed
                     } else {
                        keyboard_matrix[0x25 >> 4] |= bit_values[0x25 & 7]; // make sure key is unSHIFTed
                     }
                     if (cpc_key & MOD_CPC_CTRL) { // CPC CONTROL key required?
                        keyboard_matrix[0x27 >> 4] &= ~bit_values[0x27 & 7]; // CONTROL key is held down
                     } else {
                        keyboard_matrix[0x27 >> 4] |= bit_values[0x27 & 7]; // make sure CONTROL key is released
                     }
                  }
               }
               break;

            case SDL_KEYUP:
               {
                  dword cpc_key;
                  if (event.key.keysym.mod & KMOD_SHIFT) { // PC SHIFT key held down?
                     cpc_key = keyboard_shift[event.key.keysym.sym]; // consult the SHIFT table
                  } else if (event.key.keysym.mod & KMOD_CTRL) { // PC CTRL key held down?
                     cpc_key = keyboard_ctrl[event.key.keysym.sym]; // consult the CTRL table
                  } else if (event.key.keysym.mod & KMOD_MODE) { // PC AltGr key held down?
                     cpc_key = keyboard_mode[event.key.keysym.sym]; // consult the AltGr table
                  } else {
                     cpc_key = keyboard_normal[event.key.keysym.sym]; // consult the normal table
                  }
                  if (!(cpc_key & MOD_EMU_KEY)) { // a key of the CPC keyboard?
                     if ((!CPC.paused) && (static_cast<byte>(cpc_key) != 0xff)) {
                        keyboard_matrix[static_cast<byte>(cpc_key) >> 4] |= bit_values[static_cast<byte>(cpc_key) & 7]; // key has been released
                        keyboard_matrix[0x25 >> 4] |= bit_values[0x25 & 7]; // make sure key is unSHIFTed
                        keyboard_matrix[0x27 >> 4] |= bit_values[0x27 & 7]; // make sure CONTROL key is not held down
                     }
                  } else { // process emulator specific keys
                     switch (cpc_key) {

                        case CAP32_GUI:
                          {
                            showGui();
                            break;
                          }

                        case CAP32_FULLSCRN:
                           audio_pause();
                           SDL_Delay(20);
                           video_shutdown();
                           CPC.scr_window = CPC.scr_window ? 0 : 1;
                           if (video_init()) {
                              fprintf(stderr, "video_init() failed. Aborting.\n");
                              exit(-1);
                           }
                           audio_resume();
                           break;

                        case CAP32_TAPEPLAY:
                           LOG("Request to play tape");
                           Tape_Rewind();
                           if (pbTapeImage) {
                              if (CPC.tape_play_button) {
                                 LOG("Play button released");
                                 CPC.tape_play_button = 0;
                              } else {
                                 LOG("Play button pushed");
                                 CPC.tape_play_button = 0x10;
                              }
                           }
                           break;

                        case CAP32_MF2STOP:
                           if(CPC.mf2 && !(dwMF2Flags & MF2_ACTIVE)) {
                             reg_pair port;

                             // Set mode to activate ROM_config
                             //port.b.h = 0x40;
                             //z80_OUT_handler(port, 128);

                             // Attempt to load MF2 in lower ROM (can fail if lower ROM is not active)
                             port.b.h = 0xfe;
                             port.b.l = 0xe8;
                             dwMF2Flags &= ~MF2_INVISIBLE;
                             z80_OUT_handler(port, 0);

                             // Stop execution if load succeeded
                             if(dwMF2Flags & MF2_ACTIVE) {
                               z80_mf2stop();
                             }
                           }
                           break;

                        case CAP32_RESET:
                           emulator_reset(false);
                           break;

                        case CAP32_JOY:
                           CPC.joystick_emulation = CPC.joystick_emulation ? 0 : 1;
                           input_swap_joy();
                           break;

                        case CAP32_EXIT:
                           exit (0);
                           break;

                        case CAP32_FPS:
                           CPC.scr_fps = CPC.scr_fps ? 0 : 1; // toggle fps display on or off
                           break;

                        case CAP32_SPEED:
                           CPC.limit_speed = CPC.limit_speed ? 0 : 1;
                           break;

                        #ifdef DEBUG
                        case DEBUG_KEY:
                           dwDebugFlag = dwDebugFlag ? 0 : 1;
                           #ifdef DEBUG_CRTC
                           if (dwDebugFlag) {
                              for (int n = 0; n < 14; n++) {
                                 fprintf(pfoDebug, "%02x = %02x\r\n", n, CRTC.registers[n]);
                              }
                           }
                           #endif
                           break;
                        #endif
                     }
                  }
               }
               break;

            case SDL_JOYBUTTONDOWN:
            {
              dword cpc_key(0xff);
              switch(event.jbutton.button) {
                case 0:
                  switch(event.jbutton.which) {
                    case 0:
                      cpc_key = cpc_kbd[CPC.keyboard][CPC_J0_FIRE1];
                      break;
                    case 1:
                      cpc_key = cpc_kbd[CPC.keyboard][CPC_J1_FIRE1];
                      break;
                  }
                  break;
                case 1:
                  switch(event.jbutton.which) {
                    case 0:
                      cpc_key = cpc_kbd[CPC.keyboard][CPC_J0_FIRE2];
                      break;
                    case 1:
                      cpc_key = cpc_kbd[CPC.keyboard][CPC_J1_FIRE2];
                      break;
                  }
                  break;
                default:
                  if (event.jbutton.button == CPC.joystick_menu_button)
                  {
                    showGui();
                  }
                  if (event.jbutton.button == CPC.joystick_vkeyboard_button)
                  {
                    showVKeyboard();
                  }
                  break;
              }
              // TODO: deduplicate this from SDL_KEYDOWN, SDL_KEYUP, SDL_JOYBUTTONDOWN, SDL_JOYBUTTONUP and SDL_JOYAXISMOTION
              if (!CPC.paused && cpc_key != 0xff) {
                 keyboard_matrix[static_cast<byte>(cpc_key) >> 4] &= ~bit_values[static_cast<byte>(cpc_key) & 7]; // key is being held down
                 if (cpc_key & MOD_CPC_SHIFT) { // CPC SHIFT key required?
                    keyboard_matrix[0x25 >> 4] &= ~bit_values[0x25 & 7]; // key needs to be SHIFTed
                 } else {
                    keyboard_matrix[0x25 >> 4] |= bit_values[0x25 & 7]; // make sure key is unSHIFTed
                 }
                 if (cpc_key & MOD_CPC_CTRL) { // CPC CONTROL key required?
                    keyboard_matrix[0x27 >> 4] &= ~bit_values[0x27 & 7]; // CONTROL key is held down
                 } else {
                    keyboard_matrix[0x27 >> 4] |= bit_values[0x27 & 7]; // make sure CONTROL key is released
                 }
              }
            }
            break;

            case SDL_JOYBUTTONUP:
            {
              dword cpc_key(0xff);
              switch(event.jbutton.button) {
                case 0:
                  switch(event.jbutton.which) {
                    case 0:
                      cpc_key = cpc_kbd[CPC.keyboard][CPC_J0_FIRE1];
                      break;
                    case 1:
                      cpc_key = cpc_kbd[CPC.keyboard][CPC_J1_FIRE1];
                      break;
                  }
                  break;
                case 1:
                  switch(event.jbutton.which) {
                    case 0:
                      cpc_key = cpc_kbd[CPC.keyboard][CPC_J0_FIRE2];
                      break;
                    case 1:
                      cpc_key = cpc_kbd[CPC.keyboard][CPC_J1_FIRE2];
                      break;
                  }
                  break;
              }
              if (!CPC.paused && cpc_key != 0xff) {
                 keyboard_matrix[static_cast<byte>(cpc_key) >> 4] |= bit_values[static_cast<byte>(cpc_key) & 7]; // key has been released
                 keyboard_matrix[0x25 >> 4] |= bit_values[0x25 & 7]; // make sure key is unSHIFTed
                 keyboard_matrix[0x27 >> 4] |= bit_values[0x27 & 7]; // make sure CONTROL key is not held down
              }
            }
            break;

            case SDL_JOYAXISMOTION:
            {
              dword cpc_key(0xff), cpc_key2(0xff);
              bool release = false;
              switch(event.jaxis.axis) {
                case 0:
                case 2:
                  switch(event.jaxis.which) {
                    case 0:
                      if(event.jaxis.value < -JOYSTICK_AXIS_THRESHOLD) {
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J0_LEFT];
                      } else if(event.jaxis.value > JOYSTICK_AXIS_THRESHOLD) {
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J0_RIGHT];
                      } else {
                        // release both LEFT and RIGHT
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J0_LEFT];
                        cpc_key2 = cpc_kbd[CPC.keyboard][CPC_J0_RIGHT];
                        release = true;
                      }
                      break;
                    case 1:
                      if(event.jaxis.value < -JOYSTICK_AXIS_THRESHOLD) {
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J1_LEFT];
                      } else if(event.jaxis.value > JOYSTICK_AXIS_THRESHOLD) {
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J1_RIGHT];
                      } else {
                        // release both LEFT and RIGHT
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J1_LEFT];
                        cpc_key2 = cpc_kbd[CPC.keyboard][CPC_J1_RIGHT];
                        release = true;
                      }
                      break;
                  }
                  break;
                case 1:
                case 3:
                  switch(event.jaxis.which) {
                    case 0:
                      if(event.jaxis.value < -JOYSTICK_AXIS_THRESHOLD) {
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J0_UP];
                      } else if(event.jaxis.value > JOYSTICK_AXIS_THRESHOLD) {
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J0_DOWN];
                      } else {
                        // release both UP and DOWN
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J0_UP];
                        cpc_key2 = cpc_kbd[CPC.keyboard][CPC_J0_DOWN];
                        release = true;
                      }
                      break;
                    case 1:
                      if(event.jaxis.value < -JOYSTICK_AXIS_THRESHOLD) {
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J1_UP];
                      } else if(event.jaxis.value > JOYSTICK_AXIS_THRESHOLD) {
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J1_DOWN];
                      } else {
                        // release both UP and DOWN
                        cpc_key = cpc_kbd[CPC.keyboard][CPC_J1_UP];
                        cpc_key2 = cpc_kbd[CPC.keyboard][CPC_J1_DOWN];
                        release = true;
                      }
                      break;
                  }
                  break;
              }
              if (!CPC.paused && cpc_key != 0xff) {
                if(release) {
                 keyboard_matrix[static_cast<byte>(cpc_key) >> 4] |= bit_values[static_cast<byte>(cpc_key) & 7]; // key has been released
                 keyboard_matrix[static_cast<byte>(cpc_key2) >> 4] |= bit_values[static_cast<byte>(cpc_key2) & 7]; // key has been released
                 keyboard_matrix[0x25 >> 4] |= bit_values[0x25 & 7]; // make sure key is unSHIFTed
                 keyboard_matrix[0x27 >> 4] |= bit_values[0x27 & 7]; // make sure CONTROL key is not held down
                } else {
                 keyboard_matrix[static_cast<byte>(cpc_key) >> 4] &= ~bit_values[static_cast<byte>(cpc_key) & 7]; // key is being held down
                 if (cpc_key & MOD_CPC_SHIFT) { // CPC SHIFT key required?
                    keyboard_matrix[0x25 >> 4] &= ~bit_values[0x25 & 7]; // key needs to be SHIFTed
                 } else {
                    keyboard_matrix[0x25 >> 4] |= bit_values[0x25 & 7]; // make sure key is unSHIFTed
                 }
                 if (cpc_key & MOD_CPC_CTRL) { // CPC CONTROL key required?
                    keyboard_matrix[0x27 >> 4] &= ~bit_values[0x27 & 7]; // CONTROL key is held down
                 } else {
                    keyboard_matrix[0x27 >> 4] |= bit_values[0x27 & 7]; // make sure CONTROL key is released
                 }
                }
              }
            }
            break;

            case SDL_QUIT:
               exit(0);
         }
      }

      if (!CPC.paused) { // run the emulation, as long as the user doesn't pause it
         dwTicks = SDL_GetTicks();
         if (dwTicks >= dwTicksTargetFPS) { // update FPS counter?
            dwFPS = dwFrameCount;
            dwFrameCount = 0;
            dwTicksTargetFPS = dwTicks + 1000; // prep counter for the next run
         }

         if (CPC.limit_speed) { // limit to original CPC speed?
            if (CPC.snd_enabled) {
               if (iExitCondition == EC_SOUND_BUFFER) {
                  if (!dwSndBufferCopied) { // limit speed?
                     continue; // delay emulation
                  }
                  dwSndBufferCopied = 0;
               }
            } else if (iExitCondition == EC_CYCLE_COUNT) {
               dwTicks = SDL_GetTicks();
               if (dwTicks < dwTicksTarget) { // limit speed?
                  continue; // delay emulation
               }
               dwTicksTarget = dwTicks + dwTicksOffset; // prep counter for the next run
            }
         }

         if (!vid_plugin->lock()) { // lock the video buffer
           continue; // skip the emulation if we can't get a lock
         }
         dwOffset = CPC.scr_pos - CPC.scr_base; // offset in current surface row
         if (VDU.scrln > 0) {
            CPC.scr_base = static_cast<dword *>(back_surface->pixels) + (VDU.scrln * CPC.scr_line_offs); // determine current position
         } else {
            CPC.scr_base = static_cast<dword *>(back_surface->pixels); // reset to surface start
         }
         CPC.scr_pos = CPC.scr_base + dwOffset; // update current rendering position

         iExitCondition = z80_execute(); // run the emulation until an exit condition is met

         if (iExitCondition == EC_FRAME_COMPLETE) { // emulation finished rendering a complete frame?
            dwFrameCount++;
            if (CPC.scr_fps) {
               char chStr[15];
               sprintf(chStr, "%3dFPS %3d%%", static_cast<int>(dwFPS), static_cast<int>(dwFPS) * 100 / 50);
               print(static_cast<dword *>(back_surface->pixels) + CPC.scr_line_offs, chStr, true); // display the frames per second counter
            }
            vid_plugin->unlock();
            video_display(); // update PC display
         } else {
            vid_plugin->unlock();
         }
      }
   }

   exit(0);
}
