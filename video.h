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

#include "SDL.h"

typedef struct 
{
	/* the user-displayed name of this plugin */
	char* name;
	/* initializes the video plugin ; returns the surface that you must draw into, NULL in the (unlikely ;) event of a failure */
	SDL_Surface* (*init)(int w,int h,int bpp,bool fs);
	
	void (*set_palette)(SDL_Color* c);
	/* locks/unlocks the surface if needed */
	bool (*lock)();
	void (*unlock)();
	/* "flips" the video surface. Note that this might not always do a real flip */
	void (*flip)();
	/* closes the plugin */
	void (*close)();
	
	
	/* what you can feed to this plugin : */
	/* the pixel formats supported */
	Uint32 formats;
	/* this plugin wants : 0 half sized pixels (320x200 screen)/1 full sized pixels (640x200 screen)*/
	Uint8 half_pixels;
}
video_plugin;

/* the pixel formats video plugins can support */
#define F8_BPP		1<<0
#define F15_BPP		1<<1
#define F15_BPP_REV	1<<2
#define F16_BPP		1<<3
#define F16_BPP_REV	1<<4
#define F24_BPP		1<<5
#define F24_BPP_REV	1<<6
#define F32_BPP		1<<7
#define F32_BPP_REV	1<<8

#define ALL		0xffffffff

extern video_plugin video_plugin_list[];

