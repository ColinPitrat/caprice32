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

// Makefile doesn't pass HAVE_GL directly to reduce the likelihood
// of using HAVE_GL without including this header.
// We need the header included for every use of HAVE_GL (at least) to be able to
// deactivate OpenGL on MacOS.
#ifdef WITH_GL
#define HAVE_GL
#endif

// It seems there's no OpenGL on MacOS anymore:
// https://github.com/ColinPitrat/caprice32/pull/201
#ifdef __APPLE__
#undef HAVE_GL
#endif

#ifdef HAVE_GL

#ifndef GLFUNCS_H
#define GLFUNCS_H

#include "SDL.h"
#include "SDL_opengl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GL_FUNC(ret,func,params) typedef ret (APIENTRY * ptr##func) params; // NOLINT(misc-macro-parentheses): Caller is expected to provide parenthesis otherwise this will cause a syntax error
#define GL_FUNC_OPTIONAL(ret,func,params) typedef ret (APIENTRY * ptr##func) params; // NOLINT(misc-macro-parentheses): Caller is expected to provide parenthesis otherwise this will cause a syntax error
#include "glfunclist.h"
#undef GL_FUNC
#undef GL_FUNC_OPTIONAL

#define GL_FUNC(ret,func,params) extern ptr##func e##func;
#define GL_FUNC_OPTIONAL(ret,func,params) extern ptr##func e##func;
#include "glfunclist.h"
#undef GL_FUNC
#undef GL_FUNC_OPTIONAL

extern int init_glfuncs();

#ifdef __cplusplus
}
#endif

#endif // GLFUNCS_H

#endif // HAVE_GL
