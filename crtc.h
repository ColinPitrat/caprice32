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

#include "z80.h"

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

void update_skew(void);
void CharMR1(void);
void CharMR2(void);
void prerender_border(void);
void prerender_border_half(void);
void prerender_sync(void);
void prerender_sync_half(void);
void prerender_normal(void);
void prerender_normal_half(void);
void crtc_cycle(int repeat_count);
void crtc_init(void);
void crtc_reset(void);

void render8bpp(void);
void render8bpp_doubleY(void);
void render16bpp(void);
void render16bpp_doubleY(void);
void render24bpp(void);
void render24bpp_doubleY(void);
void render32bpp(void);
void render32bpp_doubleY(void);

#endif
