/* Caprice32 - Amstrad CPC Emulator
   (c) Copyright 1997-2004 Ulrich Doewich

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

#ifndef Z80_H
#define Z80_H

#include "SDL.h"
#include "types.h"
#include "crtc.h"

typedef union {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
   struct { byte l, h, h2, h3; } b;
   struct { word l, h; } w;
#else
   struct { byte h3, h2, h, l; } b;
   struct { word h, l; } w;
#endif
   dword d;
}  reg_pair;

#define Sflag  0x80 // sign flag
#define Zflag  0x40 // zero flag
#define Hflag  0x10 // halfcarry flag
#define Pflag  0x04 // parity flag
#define Vflag  0x04 // overflow flag
#define Nflag  0x02 // negative flag
#define Cflag  0x01 // carry flag
#define Xflags 0x28 // bit 5 & 3 flags

struct Breakpoint {
  Breakpoint(word val) : address(val) {};
  dword address;
};

enum WatchpointType {
  READ = 1,
  WRITE = 2,
  READWRITE = 3,
};

struct Watchpoint {
  Watchpoint(word val, WatchpointType t) : address(val), type(t) {};
  dword address;
  WatchpointType type;
};

typedef struct {
   reg_pair AF, BC, DE, HL, PC, SP, AFx, BCx, DEx, HLx, IX, IY;
   byte I, R, Rb7, IFF1, IFF2, IM, HALT, EI_issued, int_pending;
   byte watchpoint_reached;
   byte breakpoint_reached;
   byte step_in;
   dword break_point, trace;
} t_z80regs;


#define EC_BREAKPOINT      10
#define EC_TRACE           20
#define EC_FRAME_COMPLETE  30
#define EC_CYCLE_COUNT     40
#define EC_SOUND_BUFFER    50


byte z80_read_mem(word addr);
void z80_write_mem(word addr, byte val);

// TODO: put declaration or definition of these two methods somewhere else !!!
byte z80_IN_handler(reg_pair port); // not provided by Z80.c
void z80_OUT_handler(reg_pair port, byte val); // not provided by Z80.c

void z80_reset();
void z80_init_tables();
void z80_mf2stop();

int z80_execute();

// Handle main z80 instructions.
void z80_execute_instruction();

// Handle prefixed bits instructions.
void z80_execute_pfx_cb_instruction();

// Handle prefixed IX instructions.
void z80_execute_pfx_dd_instruction();

// Handle prefixed IX bit instructions.
void z80_execute_pfx_ddcb_instruction();

// Handle prefixed extended instructions.
void z80_execute_pfx_ed_instruction();

// Handle prefixed IY instructions.
void z80_execute_pfx_fd_instruction();

// Handle prefixed IY bit instructions.
void z80_execute_pfx_fdcb_instruction();

#endif
