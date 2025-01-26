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

/*
 * TODO: single letter macros are reserved for compiler internal use and
 * should be renamed/refactored in future.
 * See: https://github.com/ColinPitrat/caprice32/issues/150
 */

#ifndef Z80_MACROS_H
#define Z80_MACROS_H

// _B, _C and _L are already defined in ctype.h on some versions of MSYS2.
#ifdef _B
#undef _B
#endif

#ifdef _C
#undef _C
#endif

#ifdef _L
#undef _L
#endif

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

#endif
