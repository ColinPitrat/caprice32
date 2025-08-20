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

#ifndef CAP32_H
#define CAP32_H

#include <array>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "types.h"
#include "phazer.h"

class InputMapper;
//#define DEBUG
//#define DEBUG_CRTC
//#define DEBUG_FDC
//#define DEBUG_GA
//#define DEBUG_NO_VIDEO
//#define DEBUG_TAPE
//#define DEBUG_Z80

#define VERSION_STRING "v4.6.0"

#ifndef _MAX_PATH
 #ifdef _POSIX_PATH_MAX
 #define _MAX_PATH _POSIX_PATH_MAX
 #else
 #define _MAX_PATH 256
 #endif
#endif

#define CPC_BASE_FREQUENCY_MHZ  4.0
#define FRAME_PERIOD_MS        20.0
#define CYCLE_COUNT_INIT (CPC_BASE_FREQUENCY_MHZ*FRAME_PERIOD_MS*1000)

#define CPC_SCR_WIDTH 1024 // max width
#define CPC_SCR_HEIGHT 312 // max height
#define CPC_VISIBLE_SCR_WIDTH 384 // visible width: 4+40+4 * 8
#define CPC_VISIBLE_SCR_HEIGHT 270

// Emulation speed range
#define MIN_SPEED_SETTING 1
#define MAX_SPEED_SETTING 32
#define DEF_SPEED_SETTING 4

#define ICN_DISK_WIDTH 14
#define ICN_DISK_HEIGHT 16
#define ICN_TAPE_WIDTH 18
#define ICN_TAPE_HEIGHT 13

#define VOC_THRESHOLD 128

// CRTC flags
#define VS_flag      1
#define HS_flag      2
#define HDT_flag     4
#define VDT_flag     8
#define HT_flag      16
#define VT_flag      32
#define MR_flag      64
#define VTadj_flag   128
#define VSf_flag     256

// Multiface 2 flags
#define MF2_ACTIVE      1
#define MF2_RUNNING     2
#define MF2_INVISIBLE   4

// TODO: Tune threshold based on different joysticks or make it configurable ?
#define JOYSTICK_AXIS_THRESHOLD 16384

#define DEFAULT_VIDEO_PLUGIN 0

#ifdef _WIN32
// As opposed to ms_struct, needs to be explicit on Windows
#define ATTR_PACKED __attribute__((packed, gcc_struct))
#else
#define ATTR_PACKED __attribute__((packed))
#endif

typedef struct {
   char id[8];
   char unused1[8];
   unsigned char version;
   unsigned char AF[2];
   unsigned char BC[2];
   unsigned char DE[2];
   unsigned char HL[2];
   unsigned char R;
   unsigned char I;
   unsigned char IFF0;
   unsigned char IFF1;
   unsigned char IX[2];
   unsigned char IY[2];
   unsigned char SP[2];
   unsigned char PC[2];
   unsigned char IM;
   unsigned char AFx[2];
   unsigned char BCx[2];
   unsigned char DEx[2];
   unsigned char HLx[2];
   unsigned char ga_pen;
   unsigned char ga_ink_values[17];
   unsigned char ga_ROM_config;
   unsigned char ga_RAM_config;
   unsigned char crtc_reg_select;
   unsigned char crtc_registers[18];
   unsigned char upper_ROM;
   unsigned char ppi_A;
   unsigned char ppi_B;
   unsigned char ppi_C;
   unsigned char ppi_control;
   unsigned char psg_reg_select;
   unsigned char psg_registers[16];
   unsigned char ram_size[2];
// version 2 info follows
   unsigned char cpc_model;
   unsigned char last_interrupt;
   unsigned char scr_modes[6];
// version 3 info follows
   unsigned char drvA_DOSfilename[13];
   unsigned char drvB_DOSfilename[13];
   unsigned char cart_DOSfilename[13];
   unsigned char fdc_motor;
   unsigned char drvA_current_track;
   unsigned char drvB_current_track;
   unsigned char drvC_current_track;
   unsigned char drvD_current_track;
   unsigned char printer_data;
   unsigned char psg_env_step;
   unsigned char psg_env_direction;
   unsigned char crtc_type;
   unsigned char crtc_addr[2];
   unsigned char crtc_scanline[2];
   unsigned char crtc_char_count[2];
   unsigned char crtc_line_count;
   unsigned char crtc_raster_count;
   unsigned char crtc_vt_adjust_count;
   unsigned char crtc_hsw_count;
   unsigned char crtc_vsw_count;
   unsigned char crtc_flags[2];
   unsigned char ga_int_delay;
   unsigned char ga_sl_count;
   unsigned char z80_int_pending;
   unsigned char unused2[75];
} t_SNA_header;

enum class DRIVE {
  DSK_A,
  DSK_B,
  TAPE,
  CARTRIDGE,
  SNAPSHOT,
};

struct t_slot {
   DRIVE drive;
   std::string file;
   unsigned int zip_index;
};

class t_CPC {
  public:
   t_CPC();

   unsigned int model;
   unsigned int jumpers;
   unsigned int ram_size;
   unsigned int speed;
   unsigned int limit_speed;
   bool paused;
   unsigned int auto_pause;
   unsigned int boot_time;
   unsigned int keyboard_line;
   unsigned int tape_motor;
   unsigned int tape_play_button;
   unsigned int printer;
   unsigned int printer_port;
   unsigned int mf2;
   unsigned int keyboard;
   unsigned int joystick_emulation;
   unsigned int joysticks;
   unsigned int joystick_menu_button;
   unsigned int joystick_vkeyboard_button;
   PhazerType phazer_emulation;
   bool phazer_pressed;
   unsigned int phazer_x;
   unsigned int phazer_y;
   int cycle_count;
   std::string resources_path;

   unsigned int scr_style;
   unsigned int scr_scale;
   unsigned int scr_oglfilter;
   unsigned int scr_oglscanlines;
   unsigned int scr_led;
   unsigned int scr_fps;
   unsigned int scr_tube;
   unsigned int scr_intensity;
   unsigned int scr_remanency;
   unsigned int scr_window;
   unsigned int scr_bpp;        // bits per pixel of the SDL back_surface
   unsigned int scr_preserve_aspect_ratio;
   dword dwYScale;              // Y scale (i.e. number of lines in SDL back_surface per CPC line)
   unsigned int scr_bps;        // bytes per line in the SDL back_surface
   unsigned int scr_line_offs;  // bytes per CPC line in the SDL back_surface (2*scr_bps if doubling Y)
   unsigned int scr_green_mode;
   unsigned int scr_green_blue_percent;
   unsigned char *scr_base;     // begining of current line in the SDL back_surface
   unsigned char *scr_pos;      // current position in the SDL back_surface
   void (*scr_render)();
   void (*scr_prerendernorm)();
   void (*scr_prerenderbord)();
   void (*scr_prerendersync)();
   bool scr_is_ogl;
   bool scr_gui_is_currently_on;

   int devtools_scale;
   unsigned int devtools_max_stack_size;

   unsigned int snd_enabled;
   bool snd_ready;
   unsigned int snd_playback_rate;
   unsigned int snd_bits;
   unsigned int snd_stereo;
   unsigned int snd_volume;
   unsigned int snd_pp_device;
   unsigned int snd_buffersize;
   unsigned char *snd_bufferptr;
   union {
      struct {
         unsigned int low;
         unsigned int high;
      };
      int64_t both;
   } snd_cycle_count_init;

   std::string kbd_layout;

   unsigned int max_tracksize;

   std::string dsk_path;
   std::string tape_path;
   std::string snap_path; // Path where machine state snapshots will be loaded/saved by default.
   std::string cart_path;

   t_slot driveA;
   t_slot driveB;
   t_slot tape;
   t_slot snapshot;
   t_slot cartridge;

   std::string printer_file;
   std::string sdump_dir;

   std::string rom_path;
   std::string rom_file[16];
   std::string rom_mf2;

   std::string current_snap_path; // Last used snapshot path in the file dialog.
   std::string current_cart_path; // Last used cartridge path in the file dialog.
   std::string current_dsk_path;  // Last used disk path in the file dialog.
   std::string current_tape_path; // Last used tape path in the file dialog.

   class InputMapper *InputMapper;
};

typedef struct {
   unsigned int requested_addr;
   unsigned int next_addr;
   unsigned int addr;
   unsigned int next_address;
   unsigned int scr_base;
   unsigned int char_count;
   unsigned int line_count;
   unsigned int raster_count;
   unsigned int hsw;
   unsigned int hsw_count;
   unsigned int vsw;
   unsigned int vsw_count;
   unsigned int flag_hadhsync;
   unsigned int flag_inmonhsync;
   unsigned int flag_invsync;
   unsigned int flag_invta;
   unsigned int flag_newscan;
   unsigned int flag_reschar;
   unsigned int flag_resframe;
   unsigned int flag_resnext;
   unsigned int flag_resscan;
   unsigned int flag_resvsync;
   unsigned int flag_startvta;
   unsigned int last_hend;
   unsigned int reg5;
   unsigned int r7match;
   unsigned int r9match;
   unsigned int hstart;
   unsigned int hend;
   void (*CharInstMR)();
   void (*CharInstSL)();
   unsigned char reg_select;
   unsigned char registers[18];
   // 6128+ split screen support
   unsigned int split_addr;
   unsigned char split_sl;
   unsigned int sl_count;
   unsigned char interrupt_sl;
} t_CRTC;

typedef struct {
   int timeout;
   int motor;
   int led;
   int flags;
   int phase;
   int byte_count;
   int buffer_count;
   int cmd_length;
   int res_length;
   int cmd_direction;
   void (*cmd_handler)();
   unsigned char *buffer_ptr;
   unsigned char *buffer_endptr;
   unsigned char command[12];
   unsigned char result[8];
} t_FDC;

typedef struct {
   unsigned int hs_count;
   unsigned char ROM_config;
   unsigned char lower_ROM_bank;
   bool registerPageOn;
   unsigned char RAM_bank;
   unsigned char RAM_config;
   unsigned char upper_ROM;
   unsigned int requested_scr_mode;
   unsigned int scr_mode;
   unsigned char pen;
   unsigned char ink_values[17];
   unsigned int palette[34];
   unsigned char sl_count;
   unsigned char int_delay;
} t_GateArray;

typedef struct {
   unsigned char control;
   unsigned char portA;
   unsigned char portB;
   unsigned char portC;
} t_PPI;

typedef struct {
   union {
      struct {
         unsigned int low;
         unsigned int high;
      };
      int64_t both;
   } cycle_count;
   unsigned int buffer_full;
   unsigned char control;
   unsigned char reg_select;
   union {
      unsigned char Index[16];
      struct {
         unsigned char TonALo, TonAHi;
         unsigned char TonBLo, TonBHi;
         unsigned char TonCLo, TonCHi;
         unsigned char Noise;
         unsigned char Mixer;
         unsigned char AmplitudeA, AmplitudeB, AmplitudeC;
         unsigned char EnvelopeLo, EnvelopeHi;
         unsigned char EnvType;
         unsigned char PortA;
         unsigned char PortB;
      };
      struct {
         unsigned short TonA;
         unsigned short TonB;
         unsigned short TonC;
         unsigned char _noise, _mixer, _ampa, _ampb, _ampc;
         unsigned short Envelope;
         unsigned char _envtype, _porta, portb;
      } ATTR_PACKED;
   } RegisterAY;
   int AmplitudeEnv;
   bool FirstPeriod;
   void (*Synthesizer)();
} t_PSG;

typedef struct {
   int scrln;
   int scanline;
   unsigned int flag_drawing;
   unsigned int frame_completed;
} t_VDU;

using t_MemBankConfig = std::array<std::array<byte*, 4>, 8>;

// cap32.cpp
void set_osd_message(const std::string& message, uint32_t for_milliseconds = 1000);
void ga_init_banking(t_MemBankConfig& membank_config, unsigned char RAM_bank);
bool driveAltered();
void emulator_reset();
int  emulator_init();
int  video_set_palette();
void init_joystick_emulation();
void update_cpc_speed();
int  printer_start();
void printer_stop();
int audio_init ();
void audio_shutdown ();
void audio_pause ();
void audio_resume ();
void mouse_init ();
int video_init ();
void video_shutdown ();
void cleanExit(int returnCode, bool askIfUnsaved = true);

// Return the path to the best (i.e: most specific) configuration file.
// Priority order is:
//  - cap32.cfg in the same directory as cap32 binary
//  - $XDG_CONFIG_HOME/cap32.cfg
//  - $HOME/.config/cap32.cfg
//  - $HOME/.cap32.cfg
//  - /etc/cap32.cfg
std::string getConfigurationFilename(bool forWrite = false);
void loadConfiguration (t_CPC &CPC, const std::string& configFilename);
bool saveConfiguration (t_CPC &CPC, const std::string& configFilename);

void ShowCursor(bool show);

int cap32_main(int argc, char **argv);

// fdc.c
void fdc_write_data(unsigned char val);
unsigned char fdc_read_status();
unsigned char fdc_read_data();

// psg.c
void SetAYRegister(int Num, unsigned char Value);
void Calculate_Level_Tables();
void ResetAYChipEmulation();
void InitAYCounterVars();
void InitAY();

double *video_get_green_palette(int mode);
double *video_get_rgb_color(int color);
#endif
