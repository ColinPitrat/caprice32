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

#ifndef VIDEO_H
#define VIDEO_H

#include "SDL.h"
#include <vector>

typedef struct video_plugin
{
	/* the user-displayed name of this plugin */
	const char* name;
	/* initializes the video plugin ; returns the surface that you must draw into, nullptr in the (unlikely ;) event of a failure */
	SDL_Surface* (*init)(video_plugin* t,int w,int h,int bpp,bool fs);

	void (*set_palette)(SDL_Color* c);
	/* locks/unlocks the surface if needed */
	bool (*lock)();
	void (*unlock)();
	/* "flips" the video surface. Note that this might not always do a real flip */
	void (*flip)();
	/* closes the plugin */
	void (*close)();

	/* this plugin wants : 0 half sized pixels (320x200 screen)/1 full sized pixels (640x200 screen)*/
	Uint8 half_pixels;

	/* mouse offset/scaling info */
	int x_offset,y_offset;
	float x_scale,y_scale;
}
video_plugin;

extern std::vector<video_plugin> video_plugin_list;

#endif
