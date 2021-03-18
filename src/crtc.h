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

#ifndef CRTC_H
#define CRTC_H

#include "types.h"

// The next 4 bytes must remain together
typedef union {
   dword combined;
   struct {
      byte monVSYNC;
      byte inHSYNC;
      union {
         word combined;
         struct {
            byte DISPTIMG;
            byte HDSPTIMG;
         };
      } dt;
   };
} t_flags1;
// The next two bytes must remain together
typedef union {
   word combined;
   struct {
      byte NewDISPTIMG;
      byte NewHDSPTIMG;
   };
} t_new_dt;

void update_skew();
void CharMR1();
void CharMR2();
void prerender_border();
void prerender_border_half();
void prerender_sync();
void prerender_sync_half();
void prerender_normal();
void prerender_normal_half();
void prerender_normal_plus();
void prerender_normal_half_plus();
void crtc_cycle(int repeat_count);
void crtc_init();
void crtc_reset();
dword shiftLittleEndianDwordTriplet(dword val1, dword val2, dword val3, unsigned int byteShift);

void render8bpp();
void render8bpp_doubleY();
void render16bpp();
void render16bpp_doubleY();
void render24bpp();
void render24bpp_doubleY();
void render32bpp();
void render32bpp_doubleY();

#endif
