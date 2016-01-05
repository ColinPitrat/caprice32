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


#include "glfuncs.h"

#define GL_FUNC(ret,func,params) ptr##func e##func=0;
#define GL_FUNC_OPTIONAL(ret,func,params) ptr##func e##func=0;
#include "glfunclist.h"
#undef GL_FUNC
#undef GL_FUNC_OPTIONAL


#define GL_FUNC(ret,func,params) \
do { \
	e##func = ( ptr##func ) SDL_GL_GetProcAddress(#func); \
	if ( ! e##func ) { \
		printf("Unable to load GL function %s\n", #func); \
		return(1); \
	} \
} while ( 0 );

#define GL_FUNC_OPTIONAL(ret,func,params) \
do { \
	e##func = ( ptr##func ) SDL_GL_GetProcAddress(#func); \
} while ( 0 );

int init_glfuncs()
{
#include "glfunclist.h"
return 0;
}

#undef GL_FUNC
#undef GL_FUNC_OPTIONAL

