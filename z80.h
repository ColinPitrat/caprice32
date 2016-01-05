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

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

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

typedef struct {
   reg_pair AF, BC, DE, HL, PC, SP, AFx, BCx, DEx, HLx, IX, IY;
   byte I, R, Rb7, IFF1, IFF2, IM, HALT, EI_issued, int_pending;
   dword break_point, trace;
} t_z80regs;

#define _A        z80.AF.b.h
#define _F        z80.AF.b.l
#define _AF       z80.AF.w.l
#define _AFdword  z80.AF.d
#define _B        z80.BC.b.h
#define _C        z80.BC.b.l
#define _BC       z80.BC.w.l
#define _BCdword  z80.BC.d
#define _D        z80.DE.b.h
#define _E        z80.DE.b.l
#define _DE       z80.DE.w.l
#define _DEdword  z80.DE.d
#define _H        z80.HL.b.h
#define _L        z80.HL.b.l
#define _HL       z80.HL.w.l
#define _HLdword  z80.HL.d
#define _PC       z80.PC.w.l
#define _PCdword  z80.PC.d
#define _SP       z80.SP.w.l

#define _IXh      z80.IX.b.h
#define _IXl      z80.IX.b.l
#define _IX       z80.IX.w.l
#define _IXdword  z80.IX.d
#define _IYh      z80.IY.b.h
#define _IYl      z80.IY.b.l
#define _IY       z80.IY.w.l
#define _IYdword  z80.IY.d

#define _I        z80.I
#define _R        z80.R
#define _Rb7      z80.Rb7
#define _IFF1     z80.IFF1
#define _IFF2     z80.IFF2
#define _IM       z80.IM
#define _HALT     z80.HALT

#define EC_BREAKPOINT      10
#define EC_TRACE           20
#define EC_FRAME_COMPLETE  30
#define EC_CYCLE_COUNT     40
#define EC_SOUND_BUFFER    50



byte read_mem(word addr);
void write_mem(word addr, byte val);
void crtc_cycle(int repeat_count); // not provided by Z80.c
byte z80_IN_handler(reg_pair port); // not provided by Z80.c
void z80_OUT_handler(reg_pair port, byte val); // not provided by Z80.c

void z80_init_tables(void);
void z80_mf2stop(void);
int z80_execute(void);
void z80_pfx_cb(void);
void z80_pfx_dd(void);
void z80_pfx_ddcb(void);
void z80_pfx_ed(void);
void z80_pfx_fd(void);
void z80_pfx_fdcb(void);

#endif
