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
#include <sys/stat.h>

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
#include "cartridge.h"
#include "asic.h"
#include "argparse.h"
#include "slotshandler.h"

#include <errno.h>
#include <string.h>

#include "CapriceGui.h"
#include "CapriceGuiView.h"
#include "CapriceVKeyboardView.h"

#include "errors.h"
#include "log.h"

#include "savepng.h"

#define MAX_LINE_LEN 256

#define MIN_SPEED_SETTING 2
#define MAX_SPEED_SETTING 32
#define DEF_SPEED_SETTING 4

#define MAX_NB_JOYSTICKS 2


extern byte bTapeLevel;
extern t_z80regs z80;

extern dword *ScanPos;
extern dword *ScanStart;
extern word MaxVSync;
extern t_flags1 flags1;
extern t_new_dt new_dt;
extern t_disk_format disk_format[];

extern byte* pbCartridgePages[];

SDL_AudioSpec *audio_spec = nullptr;

SDL_Surface *back_surface = nullptr;
video_plugin* vid_plugin;
SDL_Joystick* joysticks[MAX_NB_JOYSTICKS];

dword dwTicks, dwTicksOffset, dwTicksTarget, dwTicksTargetFPS;
dword dwFPS, dwFrameCount;
dword dwXScale, dwYScale;
dword dwSndBufferCopied;

dword osd_timing;
std::string osd_message;

dword dwBreakPoint, dwTrace, dwMF2ExitAddr;
dword dwMF2Flags = 0;
byte *pbGPBuffer = nullptr;
byte *pbSndBuffer = nullptr;
byte *pbSndBufferEnd = nullptr;
byte *pbSndStream = nullptr;
byte *membank_read[4], *membank_write[4], *memmap_ROM[256];
byte *pbRAM = nullptr;
byte *pbRAMbuffer = nullptr;
byte *pbROM = nullptr;
byte *pbROMlo = nullptr;
byte *pbROMhi = nullptr;
byte *pbExpansionROM = nullptr;
byte *pbMF2ROMbackup = nullptr;
byte *pbMF2ROM = nullptr;
byte *pbTapeImage = nullptr;
byte keyboard_matrix[16];

std::list<SDL_Event> virtualKeyboardEvents;
dword lastVirtualEventTicks;

byte *membank_config[8][4];

FILE *pfileObject;
FILE *pfoPrinter;

#ifdef DEBUG
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

double colours_rgb[32][3] = {
   { 0.5, 0.5, 0.5 }, { 0.5, 0.5, 0.5 },{ 0.0, 1.0, 0.5 }, { 1.0, 1.0, 0.5 },
   { 0.0, 0.0, 0.5 }, { 1.0, 0.0, 0.5 },{ 0.0, 0.5, 0.5 }, { 1.0, 0.5, 0.5 },
   { 1.0, 0.0, 0.5 }, { 1.0, 1.0, 0.5 },{ 1.0, 1.0, 0.0 }, { 1.0, 1.0, 1.0 },
   { 1.0, 0.0, 0.0 }, { 1.0, 0.0, 1.0 },{ 1.0, 0.5, 0.0 }, { 1.0, 0.5, 1.0 },
   { 0.0, 0.0, 0.5 }, { 0.0, 1.0, 0.5 },{ 0.0, 1.0, 0.0 }, { 0.0, 1.0, 1.0 },
   { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 },{ 0.0, 0.5, 0.0 }, { 0.0, 0.5, 1.0 },
   { 0.5, 0.0, 0.5 }, { 0.5, 1.0, 0.5 },{ 0.5, 1.0, 0.0 }, { 0.5, 1.0, 1.0 },
   { 0.5, 0.0, 0.0 }, { 0.5, 0.0, 1.0 },{ 0.5, 0.5, 0.0 }, { 0.5, 0.5, 1.0 }
};

double colours_green[32] = {
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

byte bit_values[8] = {
   0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

static dword keyboard_normal[SDLK_LAST];
static dword keyboard_shift[SDLK_LAST];
static dword keyboard_ctrl[SDLK_LAST];
static dword keyboard_mode[SDLK_LAST];

#define MAX_ROM_MODS 2
#include "rom_mods.h"

char chAppPath[_MAX_PATH + 1];
char chROMSelected[_MAX_PATH + 1];
std::string chROMFile[4] = {
   "cpc464.rom",
   "cpc664.rom",
   "cpc6128.rom",
   "system.cpr"
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

enum ApplicationWindowState
{
   Minimized,              // application window has been iconified
   Restored,               // application window has been restored
   GainedFocus,            // application window got input focus
   LostFocus               // application window lost input focus
} _appWindowState;

CapriceArgs args;

void ga_init_banking ()
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



void ga_memory_manager ()
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
         // TODO: I think this is why the MF2 doesn't work properly:
         // ROM should be loaded R/O at 0x0000-0x1FFF (i.e not loaded in membank_write ?)
         // Writes should probably be disabled in membank_write (pointing to a dummy buffer, but not MF2 ROM ?)
         // MF2 also has a RAM (8kB) that should be loaded as R/W at 0x2000-0x3FFF
         membank_read[GateArray.lower_ROM_bank] = pbMF2ROM;
         membank_write[GateArray.lower_ROM_bank] = pbMF2ROM;
      } else {
         membank_read[GateArray.lower_ROM_bank] = pbROMlo; // 'page in' lower ROM
      }
   }
   if (CPC.model >= 3 && GateArray.registerPageOn) {
      membank_read[1] = pbRegisterPage;
      membank_write[1] = pbRegisterPage;
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
                        LOG_DEBUG("PPI read from portA (keyboard_line): " << CPC.keyboard_line << " - " << static_cast<int>(ret_val));
                     } else if (PSG.reg_select == 15) { // PSG port B?
                        if ((PSG.RegisterAY.Index[7] & 0x80)) { // port B in output mode?
                           ret_val = PSG.RegisterAY.Index[15]; // return stored value
                           LOG_DEBUG("PPI read from portA (PSG portB): " << CPC.keyboard_line << " - " << static_cast<int>(ret_val));
                        }
                     } else {
                        ret_val = PSG.RegisterAY.Index[PSG.reg_select]; // read PSG register
                        LOG_DEBUG("PPI read from portA (registers): " << CPC.keyboard_line << " - " << static_cast<int>(ret_val));
                     }
                  }
               }
            } else {
               ret_val = PPI.portA; // return last programmed value
               LOG_DEBUG("PPI read from portA (last value): " << CPC.keyboard_line << " - " << static_cast<int>(ret_val));
            }
            break;

         case 1: // read from port B?
            // 6128+: always use port B as input as this fixes Tintin on the moon.
            // This should always be the case anyway but do not activate it for other model for now, let's validate it before.
            // TODO: verify with CPC (non-plus) if we go in the else in some cases
            if (CPC.model > 2 || PPI.control & 2) { // port B set to input?
               LOG_DEBUG("PPI read from portB: bTapeLevel=" << static_cast<int>(bTapeLevel) << ", CPC.printer=" << CPC.printer << ", CPC.jumpers=" << CPC.jumpers << ", CRTC.flag_invsync=" << CRTC.flag_invsync)
               ret_val = bTapeLevel | // tape level when reading
                         (CPC.printer ? 0 : 0x40) | // ready line of connected printer
                         (CPC.jumpers & 0x7f) | // manufacturer + 50Hz
                         (CRTC.flag_invsync ? 1 : 0); // VSYNC status
            } else {
               LOG_DEBUG("PPI read from portB: " << static_cast<int>(PPI.portB))
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
                  LOG_DEBUG("PPI read from portC (upper half): " << static_cast<int>(ret_val));
               }
               if (!(direction & 1)) { // lower half set to output?
                  ret_val |= 0x0f; // invalid - set all bits
                  LOG_DEBUG("PPI read from portC (lower half): " << static_cast<int>(ret_val));
               }
            }
            LOG_DEBUG("PPI read from portC: " << static_cast<int>(ret_val));
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
   LOG_DEBUG("port.b.h3=" << std::hex << static_cast<int>(port.b.h3) << ", port.b.h2=" << static_cast<int>(port.b.h2) << ", port.b.h=" << std::hex << static_cast<int>(port.b.h) << ", port.b.l=" << static_cast<int>(port.b.l) << ", ret_val=" << static_cast<int>(ret_val) << std::dec);
   return ret_val;
}



void z80_OUT_handler (reg_pair port, byte val)
{
   LOG_DEBUG("port.b.h3=" << std::hex << static_cast<int>(port.b.h3) << ", port.b.h2=" << static_cast<int>(port.b.h2) << ", port.b.h=" << std::hex << static_cast<int>(port.b.h) << ", port.b.l=" << static_cast<int>(port.b.l) << ", val=" << static_cast<int>(val) << std::dec);
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
            LOG_DEBUG("Set pen value to " << static_cast<int>(GateArray.pen));
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
               LOG_DEBUG("Set ink value " << static_cast<int>(GateArray.pen) << " to " << static_cast<int>(colour));
               GateArray.ink_values[GateArray.pen] = colour;
               GateArray.palette[GateArray.pen] = SDL_MapRGB(back_surface->format,
                     colours[colour].r, colours[colour].g, colours[colour].b);
               if (GateArray.pen < 2) {
                  byte r = (static_cast<dword>(colours[GateArray.ink_values[0]].r) + static_cast<dword>(colours[GateArray.ink_values[1]].r)) >> 1;
                  byte g = (static_cast<dword>(colours[GateArray.ink_values[0]].g) + static_cast<dword>(colours[GateArray.ink_values[1]].g)) >> 1;
                  byte b = (static_cast<dword>(colours[GateArray.ink_values[0]].b) + static_cast<dword>(colours[GateArray.ink_values[1]].b)) >> 1;
                  GateArray.palette[33] = SDL_MapRGB(back_surface->format, r, g, b); // update the mode 2 'anti-aliasing' colour
               }
               // TODO: update pbRegisterPage
            }
            if (CPC.mf2) { // MF2 enabled?
               int iPen = *(pbMF2ROM + 0x03fcf);
               *(pbMF2ROM + (0x03f90 | ((iPen & 0x10) << 2) | (iPen & 0x0f))) = val;
            }
            break;
         case 2: // set mode
            if (!asic.locked && (val & 0x20) && CPC.model > 2) {
               // 6128+ RMR2 register
               int membank = (val >> 3) & 3;
               if (membank == 3) { // Map register page at 0x4000
                  LOG_DEBUG("Register page on");
                  GateArray.registerPageOn = true;
                  membank = 0;
               } else {
                  LOG_DEBUG("Register page off");
                  GateArray.registerPageOn = false;
               }
               int page = (val & 0x7);
               LOG_DEBUG("RMR2: Low bank rom = 0x" << std::hex << (4*membank) << std::dec << "000 - page " << page);
               GateArray.lower_ROM_bank = membank;
               pbROMlo = pbCartridgePages[page];
               ga_memory_manager();
            } else {
               #ifdef DEBUG_GA
               if (dwDebugFlag) {
                  fprintf(pfoDebug, "rom 0x%02x\r\n", val);
               }
               #endif
               LOG_DEBUG("MRER: ROM config = " << std::hex << static_cast<int>(val) << std::dec << " - mode=" << static_cast<int>(val & 0x03));
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
            }
            break;
         case 3: // set memory configuration
            if (asic.locked) {
               #ifdef DEBUG_GA
               if (dwDebugFlag) {
                  fprintf(pfoDebug, "mem 0x%02x\r\n", val);
               }
               #endif
               LOG_DEBUG("RAM config: " << static_cast<int>(val));
               GateArray.RAM_config = val;
               ga_memory_manager();
               if (CPC.mf2) { // MF2 enabled?
                  *(pbMF2ROM + 0x03fff) = val;
               }
            } else {
               // 6128+ memory mapping register
               LOG_DEBUG("Memory mapping register (RAM)");
            }
            break;
      }
   }
// CRTC -----------------------------------------------------------------------
   if (!(port.b.h & 0x40)) { // CRTC chip select?
      byte crtc_port = port.b.h & 3;
      if (crtc_port == 0) { // CRTC register select?
         // 6128+: this is where we detect the ASIC (un)locking sequence
         asic_poke_lock_sequence(val);
         CRTC.reg_select = val;
         if (CPC.mf2) { // MF2 enabled?
            *(pbMF2ROM + 0x03cff) = val;
         }
      }
      else if (crtc_port == 1) { // CRTC write data?
         if (CRTC.reg_select < 16) { // only registers 0 - 15 can be written to
            LOG_DEBUG("CRTC write to register " << static_cast<int>(CRTC.reg_select) << ": " << static_cast<int>(val));
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
                     dword temp = 0;
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
                     dword temp = 0;
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
      if (CPC.model <= 2) {
         GateArray.upper_ROM = val;
         pbExpansionROM = memmap_ROM[val];
         if (pbExpansionROM == nullptr) { // selected expansion ROM not present?
            pbExpansionROM = pbROMhi; // revert to BASIC ROM
         }
      } else {
         uint32_t page = 1; // Default to basic page
         LOG_DEBUG("ROM select: " << static_cast<int>(val));
         if (val == 7) {
            page = 3;
         } else if (val >= 128) {
            page = val & 31;
         }
         GateArray.upper_ROM = page;
         pbExpansionROM = pbCartridgePages[page];
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
            LOG_DEBUG("PPI write to portA: " << static_cast<int>(val));
            PPI.portA = val;
            if (!(PPI.control & 0x10)) { // port A set to output?
               LOG_DEBUG("PPI write to portA (PSG): " << static_cast<int>(val));
               byte psg_data = val;
               psg_write
            }
            break;
         case 1: // write to port B?
            LOG_DEBUG("PPI write to portB (upper half): " << static_cast<int>(val));
            PPI.portB = val;
            break;
         case 2: // write to port C?
            LOG_DEBUG("PPI write to portC: " << static_cast<int>(val));
            PPI.portC = val;
            if (!(PPI.control & 1)) { // output lower half?
               LOG_DEBUG("PPI write to portC (keyboard_line): " << static_cast<int>(val));
               CPC.keyboard_line = val;
            }
            if (!(PPI.control & 8)) { // output upper half?
               LOG_DEBUG("PPI write to portC (upper half): " << static_cast<int>(val));
               CPC.tape_motor = val & 0x10; // update tape motor control
               PSG.control = val; // change PSG control
               byte psg_data = PPI.portA;
               psg_write
            }
            break;
         case 3: // modify PPI control
            if (val & 0x80) { // change PPI configuration
               LOG_DEBUG("PPI.control " << static_cast<int>(PPI.control) << " => " << static_cast<int>(val));
               PPI.control = val; // update control byte
               PPI.portA = 0; // clear data for all ports
               PPI.portB = 0;
               PPI.portC = 0;
            } else { // bit manipulation of port C data
               LOG_DEBUG("PPI.portC update: " << static_cast<int>(val));
               byte bit = (val >> 1) & 7; // isolate bit to set
               if (val & 1) { // set bit?
                  PPI.portC |= bit_values[bit]; // set requested bit
               } else {
                  PPI.portC &= ~(bit_values[bit]); // reset requested bit
               }
               if (!(PPI.control & 1)) { // output lower half?
                  LOG_DEBUG("PPI.portC update (keyboard_line): " << static_cast<int>(PPI.portC));
                  CPC.keyboard_line = PPI.portC;
               }
               if (!(PPI.control & 8)) { // output upper half?
                  LOG_DEBUG("PPI.portC update (upper half): " << static_cast<int>(PPI.portC));
                  CPC.tape_motor = PPI.portC & 0x10;
                  PSG.control = PPI.portC; // change PSG control
                  byte psg_data = PPI.portA;
                  psg_write
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
      LOG_DEBUG("FDC motor control access: " << static_cast<int>(port.b.l) << " - " << static_cast<int>(val));
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



void print (dword *pdwAddr, const char *pchStr, bool bolColour)
{
   int iLen, iIdx;
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
            for (int iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
               pdwPixel = pdwLine;
               bRow = bFont[iIdx]; // get the bitmap information for one row
               for (int iCol = 0; iCol < FNT_CHAR_WIDTH; iCol++) { // loop for all columns in the font character
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
            for (int iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
               pbPixel = reinterpret_cast<byte *>(pdwLine);
               bRow = bFont[iIdx]; // get the bitmap information for one row
               for (int iCol = 0; iCol < FNT_CHAR_WIDTH; iCol++) { // loop for all columns in the font character
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
            for (int iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
               pwPixel = reinterpret_cast<word *>(pdwLine);
               bRow = bFont[iIdx]; // get the bitmap information for one row
               for (int iCol = 0; iCol < FNT_CHAR_WIDTH; iCol++) { // loop for all columns in the font character
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
            for (int iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
               pbPixel = reinterpret_cast<byte *>(pdwLine);
               bRow = bFont[iIdx]; // get the bitmap information for one row
               for (int iCol = 0; iCol < FNT_CHAR_WIDTH; iCol++) { // loop for all columns in the font character
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

int emulator_patch_ROM ()
{
   byte *pbPtr;

   if(CPC.model <= 2) { // Normal CPC range
      std::string romFilename = CPC.rom_path + "/" + chROMFile[CPC.model];
      if ((pfileObject = fopen(romFilename.c_str(), "rb")) != nullptr) { // load CPC OS + Basic
         if(fread(pbROM, 2*16384, 1, pfileObject) != 1) {
            fclose(pfileObject);
            LOG_ERROR("Couldn't read ROM file '" << romFilename << "'");
            return ERR_NOT_A_CPC_ROM;
         }
         pbROMlo = pbROM;
         fclose(pfileObject);
      } else {
         LOG_ERROR("Couldn't open ROM file '" << romFilename << "'");
         return ERR_CPC_ROM_MISSING;
      }
   } else { // Plus range
      if (pbCartridgePages[0] != nullptr) {
         pbROMlo = pbCartridgePages[0];
      }
   }

   // Patch ROM for non-english keyboards
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
         case 3: // 6128+
            if(CPC.cart_file == CPC.rom_path + "/" + chROMFile[3]) { // Only patch system cartridge - we don't want to break another one by messing with it
               pbPtr += 0x1eef; // location of the keyboard translation table
            }
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
   if (CPC.model > 2) {
      if (pbCartridgePages[0] != nullptr) {
         pbROMlo = pbCartridgePages[0];
      }
   }

// ASIC
   asic_reset();
   video_set_palette();

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
   GateArray.registerPageOn = false;
   GateArray.lower_ROM_bank = 0;
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
   for (int n = 0; n < 4; n++) { // initialize active read/write bank configuration
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

int input_init ()
{
   memset(keyboard_normal, 0xff, sizeof(keyboard_normal));
   memset(keyboard_shift, 0xff, sizeof(keyboard_shift));
   memset(keyboard_ctrl, 0xff, sizeof(keyboard_ctrl));
   memset(keyboard_mode, 0xff, sizeof(keyboard_mode));

   init_kbd_layout(CPC.resources_path + "/" + CPC.kbd_layout);
   for (dword n = 0; n < KBD_MAX_ENTRIES; n++) {
      dword pc_key = kbd_layout[n][1]; // PC key assigned to CPC key
      if (pc_key) {
         dword pc_idx = pc_key & 0xffff; // strip off modifier
         dword cpc_idx = kbd_layout[n][0];
         dword cpc_key;
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

   init_joystick_emulation();

   return 0;
}

int emulator_init ()
{
   if (input_init()) {
      fprintf(stderr, "input_init() failed. Aborting.\n");
      exit(-1);
   }

   // Cartridge must be loaded before init as ROM needs to be present.
   cartridge_load();
   int iErr, iRomNum;
   byte *pchRomData;

   pbGPBuffer = new byte [128*1024]; // attempt to allocate the general purpose buffer
   pbRAMbuffer = new byte [CPC.ram_size*1024 + 1]; // allocate memory for desired amount of RAM
   // Ensure 1 byte is available before pbRAM as prerender_normal*_plus can read it
   pbRAM = pbRAMbuffer + 1;
   pbROM = new byte [32*1024]; // allocate memory for 32K of ROM
   pbRegisterPage = new byte [16*1024];
   if ((!pbGPBuffer) || (!pbRAMbuffer) || (!pbROM) || (!pbRegisterPage)) {
      LOG_ERROR("Failed allocating memory in emulator_init. Out of memory ?");
      return ERR_OUT_OF_MEMORY;
   }
   pbROMlo = pbROM;
   pbROMhi =
   pbExpansionROM = pbROM + 16384;
   memset(memmap_ROM, 0, sizeof(memmap_ROM[0]) * 256); // clear the expansion ROM map
   ga_init_banking(); // init the CPC memory banking map
   if ((iErr = emulator_patch_ROM())) {
      LOG_ERROR("Failed patching the ROM");
      return iErr;
   }

   for (iRomNum = 0; iRomNum < 16; iRomNum++) { // loop for ROMs 0-15
      if (!CPC.rom_file[iRomNum].empty()) { // is a ROM image specified for this slot?
         pchRomData = new byte [16384]; // allocate 16K
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
               memmap_ROM[iRomNum] = pchRomData; // update the ROM map
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
   CPC.paused = false;

   return 0;
}



void emulator_shutdown ()
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

   delete [] pbROM;
   delete [] pbRAMbuffer;
   delete [] pbGPBuffer;
}



int printer_start ()
{
   if (!pfoPrinter) {
      if(!(pfoPrinter = fopen(CPC.printer_file.c_str(), "wb"))) {
         return 0; // failed to open/create file
      }
   }
   return 1; // ready to capture printer output
}



void printer_stop ()
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




int audio_init ()
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



void audio_shutdown ()
{
   SDL_CloseAudio();
   if (pbSndBuffer) {
      free(pbSndBuffer);
   }
   if (audio_spec) {
      free(audio_spec);
   }
}



void audio_pause ()
{
   if (CPC.snd_enabled) {
      SDL_PauseAudio(1);
   }
}



void audio_resume ()
{
   if (CPC.snd_enabled) {
      SDL_PauseAudio(0);
   }
}

void cpc_pause()
{
   audio_pause();
   CPC.paused = true;
}

void cpc_resume()
{
   CPC.paused = false;
   audio_resume();
}

int video_set_palette ()
{
   if (!CPC.scr_tube) {
      for (int n = 0; n < 32; n++) {
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
      for (int n = 0; n < 32; n++) {
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

   for (int n = 0; n < 17; n++) { // loop for all colours + border
      int i=GateArray.ink_values[n];
      GateArray.palette[n] = SDL_MapRGB(back_surface->format,colours[i].r,colours[i].g,colours[i].b);
   }

   return 0;
}



void video_set_style ()
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
         if (CPC.model > 2) {
            CPC.scr_prerendernorm = prerender_normal_half_plus;
         } else {
            CPC.scr_prerendernorm = prerender_normal_half;
         }
         CPC.scr_prerenderbord = prerender_border_half;
         CPC.scr_prerendersync = prerender_sync_half;
         break;
      case 2:
         if (CPC.model > 2) {
            CPC.scr_prerendernorm = prerender_normal_plus;
         } else {
            CPC.scr_prerendernorm = prerender_normal;
         }
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


int video_init ()
{
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) { // initialize the video subsystem
      std::cerr << "Init of video subsystem failed: " << SDL_GetError() << std::endl;
      return ERR_VIDEO_INIT;
   }

   vid_plugin=&video_plugin_list[CPC.scr_style];

   back_surface=vid_plugin->init(vid_plugin, CPC.scr_fs_width, CPC.scr_fs_height, CPC.scr_fs_bpp, CPC.scr_window==0);

   if (!back_surface) { // attempt to set the required video mode
      std::cerr << "Could not set requested video mode: " << SDL_GetError() << std::endl;
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



void video_shutdown ()
{
   if (back_surface) {
      vid_plugin->unlock();
   }
   vid_plugin->close();
   SDL_QuitSubSystem(SDL_INIT_VIDEO);
}



void video_display ()
{
   vid_plugin->flip();
}



int joysticks_init ()
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



void joysticks_shutdown ()
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



void init_joystick_emulation ()
{
  // CPC joy key, CPC original key
  static int joy_layout[12][2] = {
    { CPC_J0_UP,      CPC_CUR_UP },
    { CPC_J0_DOWN,    CPC_CUR_DOWN },
    { CPC_J0_LEFT,    CPC_CUR_LEFT },
    { CPC_J0_RIGHT,   CPC_CUR_RIGHT },
    { CPC_J0_FIRE1,   CPC_z },
    { CPC_J0_FIRE2,   CPC_x },
    { CPC_J1_UP,      0 },
    { CPC_J1_DOWN,    0 },
    { CPC_J1_LEFT,    0 },
    { CPC_J1_RIGHT,   0 },
    { CPC_J1_FIRE1,   0 },
    { CPC_J1_FIRE2,   0 }
  };

  for (dword n = 0; n < 6; n++) {
    int cpc_idx = joy_layout[n][1]; // get the CPC key to change the assignment for
    if (cpc_idx) {
      for (int i=0; i < KBD_MAX_ENTRIES; i++) {
        if (kbd_layout[i][0] == cpc_idx) {
	  dword pc_idx = kbd_layout[i][1]; // SDL key corresponding to the CPC key to remap
	  if (CPC.joystick_emulation) {
            keyboard_normal[pc_idx] = cpc_kbd[CPC.keyboard][joy_layout[n][0]];
	  }
	  else {
            keyboard_normal[pc_idx] = cpc_kbd[CPC.keyboard][cpc_idx];
          }
          break;
	}
      }
    }
  }
}

void update_timings()
{
   dwTicksOffset = static_cast<int>(FRAME_PERIOD_MS / (CPC.speed/CPC_BASE_FREQUENCY_MHZ));
   dwTicksTarget = SDL_GetTicks();
   dwTicksTargetFPS = dwTicksTarget;
   dwTicksTarget += dwTicksOffset;
}

// Recalculate emulation speed (to verify, seems to work reasonably well)
void update_cpc_speed()
{
   update_timings();
   InitAY();
}


std::string getConfigurationFilename(bool forWrite)
{
  // First look in any user supplied configuration file path
  std::string configFilename = args.cfgFilePath;
  if(access(configFilename.c_str(), F_OK) != 0) {
     // If not found, cap32.cfg in the same directory as the executable
     configFilename = std::string(chAppPath) + "/cap32.cfg";
     // If not found, look for .cap32.cfg in the home of current user
     if (access(configFilename.c_str(), F_OK) != 0) {
        configFilename = std::string(getenv("HOME")) + "/.cap32.cfg";
        // If still not found, look for cap32.cfg in /etc
        if (!forWrite && access(configFilename.c_str(), F_OK) != 0) {
           configFilename = "/etc/cap32.cfg";
        }
     }
  }
  std::cout << "Using configuration file" << (forWrite ? " to save" : "") << ": " << configFilename << std::endl;
  return configFilename;
}



void loadConfiguration (t_CPC &CPC, const std::string& configFilename)
{
   config::Config conf;
   conf.parseFile(configFilename);

   std::string appPath = chAppPath;
   const char *chFileName = configFilename.c_str();

   CPC.model = conf.getIntValue("system", "model", 2); // CPC 6128
   if (CPC.model > 3) {
      CPC.model = 2;
   }
   CPC.jumpers = conf.getIntValue("system", "jumpers", 0x1e) & 0x1e; // OEM is Amstrad, video refresh is 50Hz
   CPC.ram_size = conf.getIntValue("system", "ram_size", 128) & 0x02c0; // 128KB RAM
   if (CPC.ram_size > 576) {
      CPC.ram_size = 576;
   } else if ((CPC.model >= 2) && (CPC.ram_size < 128)) {
      CPC.ram_size = 128; // minimum RAM size for CPC 6128 is 128KB
   }
   CPC.speed = conf.getIntValue("system", "speed", DEF_SPEED_SETTING); // original CPC speed
   if ((CPC.speed < MIN_SPEED_SETTING) || (CPC.speed > MAX_SPEED_SETTING)) {
      CPC.speed = DEF_SPEED_SETTING;
   }
   CPC.limit_speed = 1;
   CPC.auto_pause = conf.getIntValue("system", "auto_pause", 1) & 1;
   CPC.boot_time = conf.getIntValue("system", "boot_time", 5);
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
      CPC.scr_style = DEFAULT_VIDEO_PLUGIN;
      LOG_ERROR("Unsupported video plugin specified - defaulting to plugin " << video_plugin_list[DEFAULT_VIDEO_PLUGIN].name);
   }
   CPC.scr_oglfilter = conf.getIntValue("video", "scr_oglfilter", 1) & 1;
   CPC.scr_oglscanlines = conf.getIntValue("video", "scr_oglscanlines", 30);
   if (CPC.scr_oglscanlines > 100) {
      CPC.scr_oglscanlines = 30;
   }
   CPC.scr_led = conf.getIntValue("video", "scr_led", 1) & 1;
   CPC.scr_fps = conf.getIntValue("video", "scr_fps", 0) & 1;
   CPC.scr_tube = conf.getIntValue("video", "scr_tube", 0) & 1;
   CPC.scr_intensity = conf.getIntValue("video", "scr_intensity", 10);
   CPC.scr_remanency = conf.getIntValue("video", "scr_remanency", 0) & 1;
   if ((CPC.scr_intensity < 5) || (CPC.scr_intensity > 15)) {
      CPC.scr_intensity = 10;
   }
   CPC.scr_window = conf.getIntValue("video", "scr_window", 1) & 1;

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

   CPC.kbd_layout = conf.getStringValue("control", "kbd_layout", "keymap_us.map");

   CPC.max_tracksize = conf.getIntValue("file", "max_track_size", 6144-154);
   CPC.snap_path = conf.getStringValue("file", "snap_path", appPath + "/snap/");
   CPC.cart_path = conf.getStringValue("file", "cart_path", appPath + "/cart/");
   CPC.dsk_path = conf.getStringValue("file", "dsk_path", appPath + "/disk/");
   CPC.tape_path = conf.getStringValue("file", "tape_path", appPath + "/tape/");

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
   CPC.sdump_dir = conf.getStringValue("file", "sdump_dir", appPath + "/screenshots");

   CPC.rom_path = conf.getStringValue("rom", "rom_path", appPath + "/rom/");
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

   CPC.cart_file = CPC.rom_path + "/system.cpr"; // Only default path defined. Needed for CPC6128+
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
   conf.setIntValue("system", "boot_time", CPC.boot_time);
   conf.setIntValue("system", "joystick_emulation", CPC.joystick_emulation);
   conf.setIntValue("system", "joysticks", CPC.joysticks);
   conf.setIntValue("system", "joystick_menu_button", CPC.joystick_menu_button + 1);
   conf.setIntValue("system", "joystick_vkeyboard_button", CPC.joystick_vkeyboard_button + 1);
   conf.setStringValue("system", "resources_path", CPC.resources_path);

   conf.setIntValue("video", "scr_width", CPC.scr_fs_width);
   conf.setIntValue("video", "scr_height", CPC.scr_fs_height);
   conf.setIntValue("video", "scr_bpp", CPC.scr_fs_bpp);
   conf.setIntValue("video", "scr_style", CPC.scr_style);
   conf.setIntValue("video", "scr_oglfilter", CPC.scr_oglfilter);
   conf.setIntValue("video", "scr_oglscanlines", CPC.scr_oglscanlines);
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

   conf.setStringValue("control", "kbd_layout", CPC.kbd_layout);

   conf.setIntValue("file", "max_track_size", CPC.max_tracksize);
   conf.setStringValue("file", "snap_path", CPC.snap_path);
   conf.setStringValue("file", "cart_path", CPC.cart_path);
   conf.setStringValue("file", "dsk_path", CPC.dsk_path);
   conf.setStringValue("file", "tape_path", CPC.tape_path);

   for (int iFmt = FIRST_CUSTOM_DISK_FORMAT; iFmt < MAX_DISK_FORMAT; iFmt++) { // loop through all user definable disk formats
      char chFmtId[14];
      sprintf(chFmtId, "fmt%02d", iFmt); // build format ID
      conf.setStringValue("file", chFmtId, serializeDiskFormat(disk_format[iFmt]));
   }
   conf.setStringValue("file", "printer_file", CPC.printer_file);
   conf.setStringValue("file", "sdump_dir", CPC.sdump_dir);

   conf.setStringValue("rom", "rom_path", CPC.rom_path);
   for (int iRomNum = 0; iRomNum < 16; iRomNum++) { // loop for ROMs 0-15
      char chRomId[14];
      sprintf(chRomId, "slot%02d", iRomNum); // build ROM ID
      conf.setStringValue("rom", chRomId, CPC.rom_file[iRomNum]);
   }
   conf.setStringValue("rom", "rom_mf2", CPC.rom_mf2);

   conf.saveToFile(configFilename);
}



void doCleanUp ()
{
   printer_stop();
   emulator_shutdown();

   dsk_eject(&driveA);
   dsk_eject(&driveB);
   tape_eject();

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



// TODO: Deduplicate showVKeyboard and showGui
void showVKeyboard()
{
  // Activate virtual keyboard
  audio_pause();
  SDL_ShowCursor(SDL_ENABLE);
  // guiBackSurface will allow the GUI to capture the current frame
  SDL_Surface* guiBackSurface(SDL_CreateRGBSurface(SDL_SWSURFACE, back_surface->w, back_surface->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000));
  SDL_BlitSurface(back_surface, nullptr, guiBackSurface, nullptr);
  try {
    CapriceGui capriceGui;
    capriceGui.Init();
    CapriceVKeyboardView capriceVKeyboardView(back_surface, guiBackSurface, CRect(0, 0, back_surface->w, back_surface->h));
    capriceGui.SetMouseVisibility(true);
    capriceGui.Exec();
    auto newEvents = capriceVKeyboardView.GetEvents();
    virtualKeyboardEvents.splice(virtualKeyboardEvents.end(), newEvents);
  } catch(wGui::Wg_Ex_App& e) {
    // TODO: improve: this is pretty silent if people don't look at the console
    std::cout << "Failed displaying the virtual keyboard: " << e.what() << std::endl;
  }
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
  // guiBackSurface will allow the GUI to capture the current frame
  SDL_Surface* guiBackSurface(SDL_CreateRGBSurface(SDL_SWSURFACE, back_surface->w, back_surface->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000));
  SDL_BlitSurface(back_surface, nullptr, guiBackSurface, nullptr);
  try {
    CapriceGui capriceGui;
    capriceGui.Init();
    CapriceGuiView capriceGuiView(back_surface, guiBackSurface, CRect(0, 0, back_surface->w, back_surface->h));
    capriceGui.SetMouseVisibility(true);
    capriceGui.Exec();
  } catch(wGui::Wg_Ex_App& e) {
    // TODO: improve: this is pretty silent if people don't look at the console
    std::cout << "Failed displaying the GUI: " << e.what() << std::endl;
  }
  SDL_FreeSurface(guiBackSurface);
  // Clear SDL surface:
  SDL_FillRect(back_surface, nullptr, SDL_MapRGB(back_surface->format, 0, 0, 0));
  SDL_ShowCursor(SDL_DISABLE);
  audio_resume();
}



void set_osd_message(const std::string& message) {
   osd_timing = SDL_GetTicks() + 1000;
   osd_message = " " + message;
}

void dumpScreen() {
   static int dump_num=0;
   struct stat _stat;

   if (stat(CPC.sdump_dir.c_str(), &_stat) == 0) {
      if (S_ISDIR(_stat.st_mode)) {
         SDL_Surface* shot;
         shot = SDL_PNGFormatAlpha(back_surface);
         std::string dumpPath;
         do {
            dumpPath = CPC.sdump_dir + "/" + std::string("dump") + std::to_string(dump_num) + ".png";
            dump_num++;
         } while (access(dumpPath.c_str(), F_OK) == 0);
         LOG_DEBUG("Dumping screen to " + dumpPath);
         SDL_SavePNG(shot, dumpPath.c_str());
         return;
      }
   }
   LOG_ERROR("Unable to find or open directory " + CPC.sdump_dir + " when trying to take a screenshot.");
}

int cap32_main (int argc, char **argv)
{
   dword dwOffset;
   int iExitCondition;
   bool bolDone;
   SDL_Event event;
   std::vector<std::string> slot_list;

   parseArguments(argc, argv, slot_list, args);

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

   if (video_init()) {
      fprintf(stderr, "video_init() failed. Aborting.\n");
      exit(-1);
   }

   if (audio_init()) {
      fprintf(stderr, "audio_init() failed. Disabling sound.\n");
      // TODO(cpitrat): Do not set this to 0 when audio_init fail as this affect
      // configuration when saving from GUI. Rather use some other indicator to
      // know whether snd_bufferptr is usable or not.
      // To test it, set SDL_AUDIODRIVER=dsp or some other unsupported value.
      CPC.snd_enabled = 0; // disable sound emulation
   }

   if (joysticks_init()) {
      fprintf(stderr, "joysticks_init() failed. Joysticks won't work.\n");
   }

#ifdef DEBUG
   pfoDebug = fopen("./debug.txt", "wt");
#endif

   // Extract files to be loaded from the command line args
   fillSlots(slot_list, CPC);

   // emulator_init must be called before loading files as they require
   // pbGPBuffer to be initialized.
   if (emulator_init()) {
      fprintf(stderr, "emulator_init() failed. Aborting.\n");
      exit(-1);
   }

   // Really load the various drives, if needed
   loadSlots();

   // Fill the buffer with autocmd if provided
   virtualKeyboardEvents = CapriceVKeyboard::StringToEvents(args.autocmd);
   // Give some time to the CPC to start before sending any command
   lastVirtualEventTicks = SDL_GetTicks() + CPC.boot_time * 1000;

// ----------------------------------------------------------------------------

   update_timings();
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

                        case CAP32_VKBD:
                          {
                            showVKeyboard();
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

                        case CAP32_SCRNSHOT:
                           dumpScreen();
                           break;

                        case CAP32_TAPEPLAY:
                           LOG_DEBUG("Request to play tape");
                           Tape_Rewind();
                           if (pbTapeImage) {
                              if (CPC.tape_play_button) {
                                 LOG_DEBUG("Play button released");
                                 CPC.tape_play_button = 0;
                              } else {
                                 LOG_DEBUG("Play button pushed");
                                 CPC.tape_play_button = 0x10;
                              }
                           }
                           set_osd_message(std::string("Play tape: ") + (CPC.tape_play_button ? "on" : "off"));
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
                           LOG_DEBUG("User requested emulator reset");
                           emulator_reset(false);
                           break;

                        case CAP32_JOY:
                           CPC.joystick_emulation = CPC.joystick_emulation ? 0 : 1;
                           init_joystick_emulation();
                           set_osd_message(std::string("Joystick emulation: ") + (CPC.joystick_emulation ? "on" : "off"));
                           break;

                        case CAP32_EXIT:
                           exit (0);
                           break;

                        case CAP32_FPS:
                           CPC.scr_fps = CPC.scr_fps ? 0 : 1; // toggle fps display on or off
                           set_osd_message(std::string("Performances info: ") + (CPC.scr_fps ? "on" : "off"));
                           break;

                        case CAP32_SPEED:
                           CPC.limit_speed = CPC.limit_speed ? 0 : 1;
                           set_osd_message(std::string("Limit speed: ") + (CPC.limit_speed ? "on" : "off"));
                           break;

                        case CAP32_DEBUG:
                           log_verbose = !log_verbose;
                           #ifdef DEBUG
                           dwDebugFlag = dwDebugFlag ? 0 : 1;
                           #endif
                           #ifdef DEBUG_CRTC
                           if (dwDebugFlag) {
                              for (int n = 0; n < 14; n++) {
                                 fprintf(pfoDebug, "%02x = %02x\r\n", n, CRTC.registers[n]);
                              }
                           }
                           #endif
                           set_osd_message(std::string("Debug mode: ") + (log_verbose ? "on" : "off"));
                           break;
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

            // Code shamelessly copied from http://sdl.beuc.net/sdl.wiki/Event_Examples
            // TODO: What if we were paused because of other reason than losing focus and then only lost focus
            //       the right thing to do here is to restore focus but keep paused... implementing this require
            //       keeping track of pause source, which will be a pain.
            case SDL_ACTIVEEVENT:
               if (event.active.state == (SDL_APPINPUTFOCUS | SDL_APPACTIVE) ) {
                  if (event.active.gain == 0) {
                     _appWindowState = Minimized;
                     cpc_pause(); // Always paused when iconified
                  } else {
                     if (_appWindowState == LostFocus ) {
                         _appWindowState = GainedFocus;
                         if (CPC.auto_pause)
                            cpc_resume();
                     } else {
                         _appWindowState = Restored;
                         cpc_resume(); // Always unpause when restoring from iconified
                     }
                  }
               }
               else if (event.active.state & SDL_APPINPUTFOCUS) {
                  if (event.active.gain == 0) {
                      _appWindowState = LostFocus;
                      if (CPC.auto_pause)
                         cpc_pause();
                  }
                  else {
                      _appWindowState = GainedFocus;
                      if (CPC.auto_pause)
                         cpc_resume();
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
            if (SDL_GetTicks() < osd_timing) {
               print(static_cast<dword *>(back_surface->pixels) + CPC.scr_line_offs, osd_message.c_str(), true);
            } else if (CPC.scr_fps) {
               char chStr[15];
               sprintf(chStr, "%3dFPS %3d%%", static_cast<int>(dwFPS), static_cast<int>(dwFPS) * 100 / 50);
               print(static_cast<dword *>(back_surface->pixels) + CPC.scr_line_offs, chStr, true); // display the frames per second counter
            }
            asic_draw_sprites();
            vid_plugin->unlock();
            video_display(); // update PC display
         } else {
            vid_plugin->unlock();
         }
      }
   }

   exit(0);
}

