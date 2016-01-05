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
   This file includes video filters from the SMS Plus/SDL 
   sega master system emulator :
   (c) Copyright Gregory Montoir
   http://membres.lycos.fr/cyxdown/smssdl/
*/

/*
   This file includes video filters from MAME
   (Multiple Arcade Machine Emulator) :
   (c) Copyright The MAME Team
   http://www.mame.net/
*/

#include "video.h"
#include "cap32.h"
#include "SDL_opengl.h"
#include "glfuncs.h"
#include <math.h>

// the real video surface
static SDL_Surface* vid;
// the video surface shown by the plugin to the application
static SDL_Surface* pub;

extern t_CPC CPC;

#ifndef min
#define min(a,b) (a<b ? a : b)
#endif

#ifndef max
#define max(a,b) (a>b ? a : b)
#endif

// checks for an OpenGL extension
bool have_gl_extension (char *nom_ext)
{
   const char *ext;
   ext = (const char *) (eglGetString (GL_EXTENSIONS));
   const char *f;
   if (ext == NULL)
      return false;
   f = ext + strlen (ext);
   while (ext < f)
   {
      unsigned int n = strcspn (ext, " ");
      if ((strlen (nom_ext) == n) && (strncmp (nom_ext, ext, n) == 0))
         return true;
      ext += (n + 1);
   }
   return false;
}

// computes the clipping of two rectangles and changes src and dst accordingly
// dst is the screen
// src is the internal window
void compute_rects(SDL_Rect* src, SDL_Rect* dst)
{
	/* initialise the source rect to full source */
	src->x=0;
	src->y=0;
	src->w=pub->w;
	src->h=pub->h;
	
	dst->x=(vid->w-CPC_VISIBLE_SCR_WIDTH*2)/2,
	dst->y=(vid->h-CPC_VISIBLE_SCR_HEIGHT*2)/2;
	dst->w=vid->w;
	dst->h=vid->h;
	
	int dw=src->w*2-dst->w;
	/* the src width is too big */
	if (dw>0)
	{
		src->w-=dw/2;
		src->x+=dw/4;

		dst->x=0;
		dst->w=vid->w;
	}
	else
	{
		dst->w=CPC_VISIBLE_SCR_WIDTH*2;
	}
	int dh=src->h*2-dst->h;
	/* the src height is too big */
	if (dh>0)
	{
		src->h-=dh/2;
		src->y+=dh/4;
		
		dst->y=0;
		dst->h=vid->h;
	}
	else
	{
		src->h-=2*3;
		dst->h=CPC_VISIBLE_SCR_HEIGHT*2;
	}
}

/* ------------------------------------------------------------------------------------ */
/* Half size video plugin ------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
SDL_Surface* half_init(int w,int h, int bpp,bool fs)
{
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH;
		h=CPC_VISIBLE_SCR_HEIGHT;
	}
	vid=SDL_SetVideoMode(w,h,bpp,SDL_ANYFORMAT | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH,CPC_VISIBLE_SCR_HEIGHT,bpp,0,0,0,0);
	return pub;
}

void half_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool half_lock()
{
	return true;
}

void half_unlock()
{
}

void half_flip()
{
	SDL_Rect dr;
	dr.x=(vid->w-CPC_VISIBLE_SCR_WIDTH)/2;
	dr.y=(vid->h-CPC_VISIBLE_SCR_HEIGHT)/2;
	dr.w=CPC_VISIBLE_SCR_WIDTH;
	dr.h=CPC_VISIBLE_SCR_HEIGHT;
	SDL_BlitSurface(pub,NULL,vid,&dr);
	SDL_UpdateRects(vid,1,&dr);
}

void half_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
}

/* ------------------------------------------------------------------------------------ */
/* Half size with hardware flip video plugin ------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
SDL_Surface* halfhw_init(int w,int h, int bpp, bool fs)
{
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH;
		h=CPC_VISIBLE_SCR_HEIGHT;
	}
	vid=SDL_SetVideoMode(CPC_VISIBLE_SCR_WIDTH,CPC_VISIBLE_SCR_HEIGHT,bpp,SDL_ANYFORMAT | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	return vid;
}

void halfhw_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool halfhw_lock()
{
	if (SDL_MUSTLOCK(vid))
		return (SDL_LockSurface(vid)==0);
	return true;
}

void halfhw_unlock()
{
	if (SDL_MUSTLOCK(vid))
		SDL_UnlockSurface(vid);
}

void halfhw_flip()
{
	SDL_Flip(vid);
}

void halfhw_close()
{
}

/* ------------------------------------------------------------------------------------ */
/* Double width video plugin ---------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
SDL_Surface* doublew_init(int w,int h, int bpp, bool fs)
{
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH*2;
		h=CPC_VISIBLE_SCR_HEIGHT;
	}
	vid=SDL_SetVideoMode(w,h,bpp,SDL_ANYFORMAT | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH*2,CPC_VISIBLE_SCR_HEIGHT,bpp,0,0,0,0);
	return pub;
}

void doublew_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool doublew_lock()
{
	return true;
}

void doublew_unlock()
{
}

void doublew_flip()
{
	SDL_Rect dr;
	dr.x=(vid->w-CPC_VISIBLE_SCR_WIDTH*2)/2;
	dr.y=(vid->h-CPC_VISIBLE_SCR_HEIGHT)/2;
	dr.w=CPC_VISIBLE_SCR_WIDTH*2;
	dr.h=CPC_VISIBLE_SCR_HEIGHT;
	SDL_BlitSurface(pub,NULL,vid,&dr);
	SDL_UpdateRects(vid,1,&dr);
}

void doublew_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
}

/* ------------------------------------------------------------------------------------ */
/* Scanlines video plugin ------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
SDL_Surface* scanline_init(int w,int h, int bpp, bool fs)
{
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH*2;
		h=CPC_VISIBLE_SCR_HEIGHT*2;
	}
	vid=SDL_SetVideoMode(w,h,bpp,SDL_ANYFORMAT | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH*2,CPC_VISIBLE_SCR_HEIGHT,bpp,0,0,0,0);
	return pub;
}

void scanline_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool scanline_lock()
{
	return true;
}

void scanline_unlock()
{
}

void scanline_flip()
{
	int line;
	SDL_Rect sr,dr;
	if (CPC_VISIBLE_SCR_WIDTH*2>vid->w)
		sr.x=(CPC_VISIBLE_SCR_WIDTH*2-vid->w)/2;
	else
		sr.x=0;
	if (CPC_VISIBLE_SCR_HEIGHT*2>vid->h)
		sr.y=(CPC_VISIBLE_SCR_HEIGHT*2-vid->h)/4;
	else
		sr.y=0;
	sr.w=pub->w;
	sr.h=1;
	dr.x=(vid->w-CPC_VISIBLE_SCR_WIDTH*2)/2;
	dr.y=(vid->h-CPC_VISIBLE_SCR_HEIGHT*2)/2;
	dr.w=pub->w;
	dr.h=1;
	for(line=0;line<pub->h;line++)
	{
		SDL_BlitSurface(pub,&sr,vid,&dr);
		sr.y++;
		dr.y+=2;
	}
	dr.x=max((vid->w-CPC_VISIBLE_SCR_WIDTH*2)/2,0);
	dr.y=max((vid->h-CPC_VISIBLE_SCR_HEIGHT*2)/2,0);
	dr.w=min(pub->w,vid->w);
	dr.h=min(pub->h*2,vid->h);
	SDL_UpdateRects(vid,1,&dr);
}

void scanline_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
}

/* ------------------------------------------------------------------------------------ */
/* Line doubling video plugin --------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
SDL_Surface* double_init(int w,int h, int bpp, bool fs)
{
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH*2;
		h=CPC_VISIBLE_SCR_HEIGHT*2;
	}
	vid=SDL_SetVideoMode(w,h,bpp,SDL_ANYFORMAT | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH*2,CPC_VISIBLE_SCR_HEIGHT,bpp,0,0,0,0);
	return pub;
}

void double_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool double_lock()
{
	return true;
}

void double_unlock()
{
}

void double_flip()
{
	int line;
	SDL_Rect sr,dr;
	if (CPC_VISIBLE_SCR_WIDTH*2>vid->w)
		sr.x=(CPC_VISIBLE_SCR_WIDTH*2-vid->w)/2;
	else
		sr.x=0;
	if (CPC_VISIBLE_SCR_HEIGHT*2>vid->h)
		sr.y=(CPC_VISIBLE_SCR_HEIGHT*2-vid->h)/4;
	else
		sr.y=0;
	sr.w=pub->w;
	sr.h=1;
	dr.x=(vid->w-CPC_VISIBLE_SCR_WIDTH*2)/2;
	dr.y=(vid->h-CPC_VISIBLE_SCR_HEIGHT*2)/2;
	dr.w=pub->w;
	dr.h=1;
	for(line=0;line<pub->h;line++)
	{
		SDL_BlitSurface(pub,&sr,vid,&dr);
		dr.y++;
		SDL_BlitSurface(pub,&sr,vid,&dr);
		dr.y++;
		sr.y++;
	}
	dr.x=max((vid->w-CPC_VISIBLE_SCR_WIDTH*2)/2,0);
	dr.y=max((vid->h-CPC_VISIBLE_SCR_HEIGHT*2)/2,0);
	dr.w=min(pub->w,vid->w);
	dr.h=min(pub->h*2,vid->h);
	SDL_UpdateRects(vid,1,&dr);
}

void double_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
}

/* ------------------------------------------------------------------------------------ */
/* OpenGL scaling video plugin -------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
static int tex_x,tex_y;
static GLuint screen_texnum,modulate_texnum;
static int gl_scanlines;

SDL_Surface* gl_init(int w,int h, int bpp, bool fs)
{
#ifdef _WIN32
	char *gl_library = "OpenGL32.DLL";
#else
	char *gl_library = "libGL.so.1";
#endif
	int surface_bpp;

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	if (SDL_GL_LoadLibrary(gl_library)<0)
	{
		fprintf(stderr,"Unable to dynamically open GL lib : %s\n",SDL_GetError());
		return NULL;
	}

	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH*2;
		h=CPC_VISIBLE_SCR_HEIGHT*2;
	}
	vid=SDL_SetVideoMode(w,h,0,SDL_OPENGL | (fs?SDL_FULLSCREEN:0));
	if (!vid)
	{
		fprintf(stderr, "Could not set requested video mode: %s\n", SDL_GetError());
		return NULL;
	}
	if (init_glfuncs()!=0)
	{
		fprintf(stderr, "Cannot init OpenGL functions: %s\n", SDL_GetError());
		return NULL;
	}

	int major, minor;
	const char *version;
	version = (char *) eglGetString(GL_VERSION); 
	if (sscanf(version, "%d.%d", &major, &minor) != 2) {
		fprintf(stderr, "Unable to get OpenGL version\n");
		return NULL;
	}

	GLint max_texsize;
	eglGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_texsize);
	if (max_texsize<512) {
		fprintf(stderr, "Your OpenGL implementation doesn't support 512x512 textures\n");
		return NULL;
	}

	// We have to react differently to the bpp parameter than with software rendering
	// Here are the rules :
	// for 8bpp OpenGL, we need the GL_EXT_paletted_texture extension
	// for 16bpp OpenGL, we need OpenGL 1.2+
	// for 24bpp reversed OpenGL, we need OpenGL 1.2+
	surface_bpp=0;
	switch(bpp)
	{
		case 8:
			surface_bpp = (have_gl_extension("GL_EXT_paletted_texture"))?8:0;
			break;
		case 15:
		case 16:
			surface_bpp = ((major>1)||(major == 1 && minor >= 2))?16:0;
			break;
		case 24:
		case 32:
		default:
			surface_bpp = ((major>1)||(major == 1 && minor >= 2))?24:0;
			break;
	}
	if (surface_bpp==0) {
		fprintf(stderr, "Your OpenGL implementation doesn't support %dbpp textures\n",surface_bpp);
		return NULL;
	}

	eglDisable(GL_FOG);
	eglDisable(GL_LIGHTING);
	eglDisable(GL_CULL_FACE);
	eglDisable(GL_DEPTH_TEST);
	eglDisable(GL_BLEND);
	eglDisable(GL_NORMALIZE);
	eglDisable(GL_ALPHA_TEST);
	eglEnable(GL_TEXTURE_2D);
	eglBlendFunc (GL_SRC_ALPHA, GL_ONE);

	eglGenTextures(1,&screen_texnum);
	eglBindTexture(GL_TEXTURE_2D,screen_texnum);
	eglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, CPC.scr_oglfilter?GL_LINEAR:GL_NEAREST);
	eglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, CPC.scr_oglfilter?GL_LINEAR:GL_NEAREST);
	tex_x=512;
	tex_y=512;

	switch(surface_bpp)
	{
		case 24:
			eglTexImage2D(GL_TEXTURE_2D, 0,GL_RGB,tex_x,tex_y, 0,
					GL_RGB,
					GL_UNSIGNED_BYTE, NULL);
			break;
		case 16:
			eglTexImage2D(GL_TEXTURE_2D, 0,GL_RGB5,tex_x,tex_y, 0,
					GL_RGB,
					GL_UNSIGNED_BYTE, NULL);
			break;
		case 8:
			eglTexImage2D(GL_TEXTURE_2D, 0,GL_COLOR_INDEX8_EXT,tex_x,tex_y, 0,
					GL_COLOR_INDEX,
					GL_UNSIGNED_BYTE, NULL);
			break;
	}

	if (gl_scanlines!=0)
	{
		Uint8 texmod;
		switch(gl_scanlines)
		{
			case 25:
			texmod=192; break;
			case 50:
			texmod=128; break;
			case 75:
			texmod=64; break;
			case 100:
			texmod=0; break;
			default:
			texmod=255;
		}
		eglGenTextures(1,&modulate_texnum);
		eglBindTexture(GL_TEXTURE_2D,modulate_texnum);
		eglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, CPC.scr_oglfilter?GL_LINEAR:GL_NEAREST);
		eglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, CPC.scr_oglfilter?GL_LINEAR:GL_NEAREST);

		Uint8 modulate_texture[]={
			255,255,255,
			0,0,0};
		modulate_texture[3]=texmod;
		modulate_texture[4]=texmod;
		modulate_texture[5]=texmod;
		eglTexImage2D(GL_TEXTURE_2D, 0,GL_RGB8,1,2, 0,GL_RGB,GL_UNSIGNED_BYTE, modulate_texture);
	}
	eglViewport(0,0,w,h);
	eglMatrixMode(GL_PROJECTION);
	eglLoadIdentity();
	eglOrtho(0,w,h,0,-1.0, 1.0);

	eglMatrixMode(GL_MODELVIEW);
	eglLoadIdentity();

	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH,CPC_VISIBLE_SCR_HEIGHT,surface_bpp,0,0,0,0);
	return pub;
}

SDL_Surface* glscale_init(int w,int h, int bpp, bool fs)
{
	gl_scanlines=0;
	return gl_init(w,h,bpp,fs);
}
SDL_Surface* glscale25_init(int w,int h, int bpp, bool fs)
{
	gl_scanlines=25;
	return gl_init(w,h,bpp,fs);
}
SDL_Surface* glscale50_init(int w,int h, int bpp, bool fs)
{
	gl_scanlines=50;
	return gl_init(w,h,bpp,fs);
}
SDL_Surface* glscale75_init(int w,int h, int bpp, bool fs)
{
	gl_scanlines=75;
	return gl_init(w,h,bpp,fs);
}
SDL_Surface* glscale100_init(int w,int h, int bpp, bool fs)
{
	gl_scanlines=100;
	return gl_init(w,h,bpp,fs);
}

void glscale_setpal(SDL_Color* c)
{
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32);
	if (pub->format->palette)
	{
		Uint8* pal=(Uint8*)malloc(sizeof(Uint8)*256*3);
		for(int i=0;i<256;i++)
		{
			pal[3*i  ] = pub->format->palette->colors[i].r;
			pal[3*i+1] = pub->format->palette->colors[i].g;
			pal[3*i+2] = pub->format->palette->colors[i].b;
		}
		eglBindTexture(GL_TEXTURE_2D,screen_texnum);
		eglColorTableEXT(GL_TEXTURE_2D,GL_RGB8,256,GL_RGB,GL_UNSIGNED_BYTE,pal);
		free(pal);
	}
}

bool glscale_lock()
{
	return true;
}

void glscale_unlock()
{
}

void glscale_flip()
{
	eglDisable(GL_BLEND);
	
	if (gl_scanlines!=0)
	{
		eglActiveTextureARB(GL_TEXTURE1_ARB);
		eglEnable(GL_TEXTURE_2D);
		eglBindTexture(GL_TEXTURE_2D,modulate_texnum);
		eglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		eglColor4f(1.0,1.0,1.0,1.0);
		eglActiveTextureARB(GL_TEXTURE0_ARB);
	}

	eglEnable(GL_TEXTURE_2D);
	eglBindTexture(GL_TEXTURE_2D,screen_texnum);
	
	if (CPC.scr_remanency)
	{
		/* draw again using the old texture */
		eglBegin(GL_QUADS);
		eglColor4f(1.0,1.0,1.0,1.0);

		eglTexCoord2f(0.f, 0.f);
		if (gl_scanlines!=0)
			eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,0.f, 0.f);
		eglVertex2i(0, 0);

		eglTexCoord2f(0.f, (float)(pub->h)/tex_y);
		if (gl_scanlines!=0)
			eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,0.f, vid->h/2);
		eglVertex2i(0, vid->h);

		eglTexCoord2f((float)(pub->w)/tex_x, (float)(pub->h)/tex_y);
		if (gl_scanlines!=0)
			eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,vid->w, vid->h/2);
		eglVertex2i(vid->w, vid->h);

		eglTexCoord2f((float)(pub->w)/tex_x, 0.f);
		if (gl_scanlines!=0)
			eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,vid->w, 0);
		eglVertex2i(vid->w, 0);
		eglEnd();

		/* enable blending for the subsequent pass */
		eglEnable(GL_BLEND);
		eglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	/* upload the texture */
	switch(pub->format->BitsPerPixel)
	{
		case 24:
			eglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
					pub->w, pub->h,
					GL_BGR,GL_UNSIGNED_BYTE,
					pub->pixels);
			break;
		case 16:
			eglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
					pub->w, pub->h,
					GL_RGB,GL_UNSIGNED_SHORT_5_6_5,
					pub->pixels);
			break;
		case 8:
			eglTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0,
					pub->w,pub->h, 
					GL_COLOR_INDEX, GL_UNSIGNED_BYTE, 
					pub->pixels);
			break;
	}

	/* draw ! */
	eglBegin(GL_QUADS);
	eglColor4f(1.0,1.0,1.0,0.5);

	eglTexCoord2f(0.f, 0.f);
	if (gl_scanlines!=0)
		eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,0.f, 0.f);
	eglVertex2i(0, 0);

	eglTexCoord2f(0.f, (float)(pub->h)/tex_y);
	if (gl_scanlines!=0)
		eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,0.f, vid->h/2);
	eglVertex2i(0, vid->h);

	eglTexCoord2f((float)(pub->w)/tex_x, (float)(pub->h)/tex_y);
	if (gl_scanlines!=0)
		eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,vid->w, vid->h/2);
	eglVertex2i(vid->w, vid->h);

	eglTexCoord2f((float)(pub->w)/tex_x, 0.f);
	if (gl_scanlines!=0)
		eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,vid->w, 0);
	eglVertex2i(vid->w, 0);
	eglEnd();

	SDL_GL_SwapBuffers();
}

void glscale_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
	if (eglIsTexture(screen_texnum))
		eglDeleteTextures(1,&screen_texnum);
	if (eglIsTexture(modulate_texnum))
		eglDeleteTextures(1,&modulate_texnum);
}

/* ------------------------------------------------------------------------------------ */
/* Super eagle video plugin ----------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */

/* 2X SAI Filter */
static Uint32 colorMask = 0xF7DEF7DE;
static Uint32 lowPixelMask = 0x08210821;
static Uint32 qcolorMask = 0xE79CE79C;
static Uint32 qlowpixelMask = 0x18631863;
static Uint32 redblueMask = 0xF81F;
static Uint32 greenMask = 0x7E0;

__inline__ int GetResult1 (Uint32 A, Uint32 B, Uint32 C, Uint32 D, Uint32 E)
{
	int x = 0;
	int y = 0;
	int r = 0;

	if (A == C)
		x += 1;
	else if (B == C)
		y += 1;
	if (A == D)
		x += 1;
	else if (B == D)
		y += 1;
	if (x <= 1)
		r += 1;
	if (y <= 1)
		r -= 1;
	return r;
}

__inline__ int GetResult2 (Uint32 A, Uint32 B, Uint32 C, Uint32 D, Uint32 E)
{
	int x = 0;
	int y = 0;
	int r = 0;

	if (A == C)
		x += 1;
	else if (B == C)
		y += 1;
	if (A == D)
		x += 1;
	else if (B == D)
		y += 1;
	if (x <= 1)
		r -= 1;
	if (y <= 1)
		r += 1;
	return r;
}

__inline__ int GetResult (Uint32 A, Uint32 B, Uint32 C, Uint32 D)
{
	int x = 0;
	int y = 0;
	int r = 0;

	if (A == C)
		x += 1;
	else if (B == C)
		y += 1;
	if (A == D)
		x += 1;
	else if (B == D)
		y += 1;
	if (x <= 1)
		r += 1;
	if (y <= 1)
		r -= 1;
	return r;
}


__inline__ Uint32 INTERPOLATE (Uint32 A, Uint32 B)
{
	if (A != B)
	{
		return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) +
				(A & B & lowPixelMask));
	}
	else
		return A;
}

__inline__ Uint32 Q_INTERPOLATE (Uint32 A, Uint32 B, Uint32 C, Uint32 D)
{
	register Uint32 x = ((A & qcolorMask) >> 2) +
		((B & qcolorMask) >> 2) +
		((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
	register Uint32 y = (A & qlowpixelMask) +
		(B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);
	y = (y >> 2) & qlowpixelMask;
	return x + y;
}

void filter_supereagle(Uint8 *srcPtr, Uint32 srcPitch, /* Uint8 *deltaPtr,  */
		 Uint8 *dstPtr, Uint32 dstPitch, int width, int height)
{
    Uint8  *dP;
    Uint16 *bP;
    Uint32 inc_bP;



	Uint32 finish;
	Uint32 Nextline = srcPitch >> 1;

	inc_bP = 1;

	for (; height ; height--)
	{
	    bP = (Uint16 *) srcPtr;
	    dP = dstPtr;
	    for (finish = width; finish; finish -= inc_bP)
	    {
		Uint32 color4, color5, color6;
		Uint32 color1, color2, color3;
		Uint32 colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
		Uint32 product1a, product1b, product2a, product2b;
		colorB1 = *(bP - Nextline);
		colorB2 = *(bP - Nextline + 1);

		color4 = *(bP - 1);
		color5 = *(bP);
		color6 = *(bP + 1);
		colorS2 = *(bP + 2);

		color1 = *(bP + Nextline - 1);
		color2 = *(bP + Nextline);
		color3 = *(bP + Nextline + 1);
		colorS1 = *(bP + Nextline + 2);

		colorA1 = *(bP + Nextline + Nextline);
		colorA2 = *(bP + Nextline + Nextline + 1);
		// --------------------------------------
		if (color2 == color6 && color5 != color3)
		{
		    product1b = product2a = color2;
		    if ((color1 == color2) || (color6 == colorB2))
		    {
			product1a = INTERPOLATE (color2, color5);
			product1a = INTERPOLATE (color2, product1a);
//                       product1a = color2;
		    }
		    else
		    {
			product1a = INTERPOLATE (color5, color6);
		    }

		    if ((color6 == colorS2) || (color2 == colorA1))
		    {
			product2b = INTERPOLATE (color2, color3);
			product2b = INTERPOLATE (color2, product2b);
//                       product2b = color2;
		    }
		    else
		    {
			product2b = INTERPOLATE (color2, color3);
		    }
		}
		else if (color5 == color3 && color2 != color6)
		{
		    product2b = product1a = color5;

		    if ((colorB1 == color5) || (color3 == colorS1))
		    {
			product1b = INTERPOLATE (color5, color6);
			product1b = INTERPOLATE (color5, product1b);
//                       product1b = color5;
		    }
		    else
		    {
			product1b = INTERPOLATE (color5, color6);
		    }

		    if ((color3 == colorA2) || (color4 == color5))
		    {
			product2a = INTERPOLATE (color5, color2);
			product2a = INTERPOLATE (color5, product2a);
//                       product2a = color5;
		    }
		    else
		    {
			product2a = INTERPOLATE (color2, color3);
		    }

		}
		else if (color5 == color3 && color2 == color6)
		{
		    register int r = 0;

		    r += GetResult (color6, color5, color1, colorA1);
		    r += GetResult (color6, color5, color4, colorB1);
		    r += GetResult (color6, color5, colorA2, colorS1);
		    r += GetResult (color6, color5, colorB2, colorS2);

		    if (r > 0)
		    {
			product1b = product2a = color2;
			product1a = product2b = INTERPOLATE (color5, color6);
		    }
		    else if (r < 0)
		    {
			product2b = product1a = color5;
			product1b = product2a = INTERPOLATE (color5, color6);
		    }
		    else
		    {
			product2b = product1a = color5;
			product1b = product2a = color2;
		    }
		}
		else
		{
		    product2b = product1a = INTERPOLATE (color2, color6);
		    product2b =
			Q_INTERPOLATE (color3, color3, color3, product2b);
		    product1a =
			Q_INTERPOLATE (color5, color5, color5, product1a);

		    product2a = product1b = INTERPOLATE (color5, color3);
		    product2a =
			Q_INTERPOLATE (color2, color2, color2, product2a);
		    product1b =
			Q_INTERPOLATE (color6, color6, color6, product1b);

//                    product1a = color5;
//                    product1b = color6;
//                    product2a = color2;
//                    product2b = color3;
		}
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		product1a = product1a | (product1b << 16);
		product2a = product2a | (product2b << 16);
#else
    product1a = (product1a << 16) | product1b;
    product2a = (product2a << 16) | product2b;
#endif

		*((Uint32 *) dP) = product1a;
		*((Uint32 *) (dP + dstPitch)) = product2a;

		bP += inc_bP;
		dP += sizeof (Uint32);
	    }			// end of for ( finish= width etc..)
	    srcPtr += srcPitch;
	    dstPtr += dstPitch * 2;
	}			// endof: for (height; height; height--)
}

SDL_Surface* seagle_init(int w,int h, int bpp, bool fs)
{
	if (bpp!=16)
		return NULL;
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH*2;
		h=CPC_VISIBLE_SCR_HEIGHT*2;
	}
	vid=SDL_SetVideoMode(w,h,bpp,SDL_ANYFORMAT | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	if (vid->format->BitsPerPixel!=16)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH,CPC_VISIBLE_SCR_HEIGHT,bpp,0,0,0,0);
	return pub;
}

void seagle_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool seagle_lock()
{
	return true;
}

void seagle_unlock()
{
}

void seagle_flip()
{
	if (SDL_MUSTLOCK(vid))
		SDL_LockSurface(vid);
	SDL_Rect src;
	SDL_Rect dst;
	compute_rects(&src,&dst);
	filter_supereagle((Uint8*)pub->pixels + (2*src.x+src.y*pub->pitch), pub->pitch,
		 (Uint8*)vid->pixels + (2*dst.x+dst.y*vid->pitch), vid->pitch, src.w, src.h);
	if (SDL_MUSTLOCK(vid))
		SDL_UnlockSurface(vid);
	SDL_UpdateRects(vid,1,&dst);
}

void seagle_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
}

/* ------------------------------------------------------------------------------------ */
/* Scale2x video plugin --------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
void filter_scale2x(Uint8 *srcPtr, Uint32 srcPitch, 
                      Uint8 *dstPtr, Uint32 dstPitch,
		      int width, int height)
{
	unsigned int nextlineSrc = srcPitch / sizeof(short);
	short *p = (short *)srcPtr;

	unsigned int nextlineDst = dstPitch / sizeof(short);
	short *q = (short *)dstPtr;

	while(height--) {
		int i = 0, j = 0;
		for(i = 0; i < width; ++i, j += 2) {
			short B = *(p + i - nextlineSrc);
			short D = *(p + i - 1);
			short E = *(p + i);
			short F = *(p + i + 1);
			short H = *(p + i + nextlineSrc);

			*(q + j) = D == B && B != F && D != H ? D : E;
			*(q + j + 1) = B == F && B != D && F != H ? F : E;
			*(q + j + nextlineDst) = D == H && D != B && H != F ? D : E;
			*(q + j + nextlineDst + 1) = H == F && D != H && B != F ? F : E;
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}

SDL_Surface* scale2x_init(int w,int h, int bpp, bool fs)
{
	if (bpp!=16)
		return NULL;
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH*2;
		h=CPC_VISIBLE_SCR_HEIGHT*2;
	}
	vid=SDL_SetVideoMode(w,h,bpp,SDL_ANYFORMAT | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	if (vid->format->BitsPerPixel!=16)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH,CPC_VISIBLE_SCR_HEIGHT,bpp,0,0,0,0);
	return pub;
}

void scale2x_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool scale2x_lock()
{
	return true;
}

void scale2x_unlock()
{
}

void scale2x_flip()
{
	if (SDL_MUSTLOCK(vid))
		SDL_LockSurface(vid);
	SDL_Rect src;
	SDL_Rect dst;
	compute_rects(&src,&dst);
	filter_scale2x((Uint8*)pub->pixels + (2*src.x+src.y*pub->pitch), pub->pitch,
		 (Uint8*)vid->pixels + (2*dst.x+dst.y*vid->pitch), vid->pitch, src.w, src.h);
	if (SDL_MUSTLOCK(vid))
		SDL_UnlockSurface(vid);
	SDL_UpdateRects(vid,1,&dst);
}

void scale2x_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
}

/* ------------------------------------------------------------------------------------ */
/* ascale2x video plugin --------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
void filter_ascale2x (Uint8 *srcPtr, Uint32 srcPitch,
	     Uint8 *dstPtr, Uint32 dstPitch, int width, int height)
{
    Uint8  *dP;
    Uint16 *bP;
    Uint32 inc_bP;


	Uint32 finish;
	Uint32 Nextline = srcPitch >> 1;
	inc_bP = 1;


	for (; height; height--)
	{
	    bP = (Uint16 *) srcPtr;
	    dP = dstPtr;

	    for (finish = width; finish; finish -= inc_bP)
	    {

		register Uint32 colorA, colorB;
		Uint32 colorC, colorD,
		    colorE, colorF, colorG, colorH,
		    colorI, colorJ, colorK, colorL,

		    colorM, colorN, colorO, colorP;
		Uint32 product, product1, product2;

//---------------------------------------
// Map of the pixels:                    I|E F|J
//                                       G|A B|K
//                                       H|C D|L
//                                       M|N O|P
		colorI = *(bP - Nextline - 1);
		colorE = *(bP - Nextline);
		colorF = *(bP - Nextline + 1);
		colorJ = *(bP - Nextline + 2);

		colorG = *(bP - 1);
		colorA = *(bP);
		colorB = *(bP + 1);
		colorK = *(bP + 2);

		colorH = *(bP + Nextline - 1);
		colorC = *(bP + Nextline);
		colorD = *(bP + Nextline + 1);
		colorL = *(bP + Nextline + 2);

		colorM = *(bP + Nextline + Nextline - 1);
		colorN = *(bP + Nextline + Nextline);
		colorO = *(bP + Nextline + Nextline + 1);
		colorP = *(bP + Nextline + Nextline + 2);

		if ((colorA == colorD) && (colorB != colorC))
		{
		    if (((colorA == colorE) && (colorB == colorL)) ||
			    ((colorA == colorC) && (colorA == colorF)
			     && (colorB != colorE) && (colorB == colorJ)))
		    {
			product = colorA;
		    }
		    else
		    {
			product = INTERPOLATE (colorA, colorB);
		    }

		    if (((colorA == colorG) && (colorC == colorO)) ||
			    ((colorA == colorB) && (colorA == colorH)
			     && (colorG != colorC) && (colorC == colorM)))
		    {
			product1 = colorA;
		    }
		    else
		    {
			product1 = INTERPOLATE (colorA, colorC);
		    }
		    product2 = colorA;
		}
		else if ((colorB == colorC) && (colorA != colorD))
		{
		    if (((colorB == colorF) && (colorA == colorH)) ||
			    ((colorB == colorE) && (colorB == colorD)
			     && (colorA != colorF) && (colorA == colorI)))
		    {
			product = colorB;
		    }
		    else
		    {
			product = INTERPOLATE (colorA, colorB);
		    }

		    if (((colorC == colorH) && (colorA == colorF)) ||
			    ((colorC == colorG) && (colorC == colorD)
			     && (colorA != colorH) && (colorA == colorI)))
		    {
			product1 = colorC;
		    }
		    else
		    {
			product1 = INTERPOLATE (colorA, colorC);
		    }
		    product2 = colorB;
		}
		else if ((colorA == colorD) && (colorB == colorC))
		{
		    if (colorA == colorB)
		    {
			product = colorA;
			product1 = colorA;
			product2 = colorA;
		    }
		    else
		    {
			register int r = 0;

			product1 = INTERPOLATE (colorA, colorC);
			product = INTERPOLATE (colorA, colorB);

			r +=
			    GetResult1 (colorA, colorB, colorG, colorE,
					colorI);
			r +=
			    GetResult2 (colorB, colorA, colorK, colorF,
					colorJ);
			r +=
			    GetResult2 (colorB, colorA, colorH, colorN,
					colorM);
			r +=
			    GetResult1 (colorA, colorB, colorL, colorO,
					colorP);

			if (r > 0)
			    product2 = colorA;
			else if (r < 0)
			    product2 = colorB;
			else
			{
			    product2 =
				Q_INTERPOLATE (colorA, colorB, colorC,
					       colorD);
			}
		    }
		}
		else
		{
		    product2 = Q_INTERPOLATE (colorA, colorB, colorC, colorD);

		    if ((colorA == colorC) && (colorA == colorF)
			    && (colorB != colorE) && (colorB == colorJ))
		    {
			product = colorA;
		    }
		    else
			if ((colorB == colorE) && (colorB == colorD)
			    && (colorA != colorF) && (colorA == colorI))
		    {
			product = colorB;
		    }
		    else
		    {
			product = INTERPOLATE (colorA, colorB);
		    }

		    if ((colorA == colorB) && (colorA == colorH)
			    && (colorG != colorC) && (colorC == colorM))
		    {
			product1 = colorA;
		    }
		    else
			if ((colorC == colorG) && (colorC == colorD)
			    && (colorA != colorH) && (colorA == colorI))
		    {
			product1 = colorC;
		    }
		    else
		    {
			product1 = INTERPOLATE (colorA, colorC);
		    }
		}
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		product = colorA | (product << 16);
		product1 = product1 | (product2 << 16);
#else
    product = (colorA << 16) | product;
    product1 = (product1 << 16) | product2;
#endif
		*((Uint32 *) dP) = product;
		*((Uint32 *) (dP + dstPitch)) = product1;

		bP += inc_bP;
		dP += sizeof (Uint32);
	    }			// end of for ( finish= width etc..)

	    srcPtr += srcPitch;
	    dstPtr += dstPitch * 2;
	}			// endof: for (height; height; height--)
}



SDL_Surface* ascale2x_init(int w,int h, int bpp, bool fs)
{
	if (bpp!=16)
		return NULL;
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH*2;
		h=CPC_VISIBLE_SCR_HEIGHT*2;
	}
	vid=SDL_SetVideoMode(w,h,bpp,SDL_ANYFORMAT | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	if (vid->format->BitsPerPixel!=16)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH,CPC_VISIBLE_SCR_HEIGHT,bpp,0,0,0,0);
	return pub;
}

void ascale2x_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool ascale2x_lock()
{
	return true;
}

void ascale2x_unlock()
{
}

void ascale2x_flip()
{
	if (SDL_MUSTLOCK(vid))
		SDL_LockSurface(vid);
	SDL_Rect src;
	SDL_Rect dst;
	compute_rects(&src,&dst);
	filter_ascale2x((Uint8*)pub->pixels + (2*src.x+src.y*pub->pitch), pub->pitch,
		 (Uint8*)vid->pixels + (2*dst.x+dst.y*vid->pitch), vid->pitch, src.w, src.h);
	if (SDL_MUSTLOCK(vid))
		SDL_UnlockSurface(vid);
	SDL_UpdateRects(vid,1,&dst);
}

void ascale2x_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
}


/* ------------------------------------------------------------------------------------ */
/* tv2x video plugin ------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
void filter_tv2x(Uint8 *srcPtr, Uint32 srcPitch, 
		Uint8 *dstPtr, Uint32 dstPitch, 
		int width, int height)
{
	unsigned int nextlineSrc = srcPitch / sizeof(Uint16);
	Uint16 *p = (Uint16 *)srcPtr;

	unsigned int nextlineDst = dstPitch / sizeof(Uint16);
	Uint16 *q = (Uint16 *)dstPtr;

	while(height--) {
		int i = 0, j = 0;
		for(; i < width; ++i, j += 2) {
			Uint16 p1 = *(p + i);
			Uint32 pi;

			pi = (((p1 & redblueMask) * 7) >> 3) & redblueMask;
			pi |= (((p1 & greenMask) * 7) >> 3) & greenMask;

			*(q + j) = p1;
			*(q + j + 1) = p1;
			*(q + j + nextlineDst) = pi;
			*(q + j + nextlineDst + 1) = pi;
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}

SDL_Surface* tv2x_init(int w,int h, int bpp, bool fs)
{
	if (bpp!=16)
		return NULL;
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH*2;
		h=CPC_VISIBLE_SCR_HEIGHT*2;
	}
	vid=SDL_SetVideoMode(w,h,bpp,SDL_ANYFORMAT | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	if (vid->format->BitsPerPixel!=16)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH,CPC_VISIBLE_SCR_HEIGHT,bpp,0,0,0,0);
	return pub;
}

void tv2x_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool tv2x_lock()
{
	return true;
}

void tv2x_unlock()
{
}

void tv2x_flip()
{
	if (SDL_MUSTLOCK(vid))
		SDL_LockSurface(vid);
	SDL_Rect src;
	SDL_Rect dst;
	compute_rects(&src,&dst);
	filter_tv2x((Uint8*)pub->pixels + (2*src.x+src.y*pub->pitch), pub->pitch,
		 (Uint8*)vid->pixels + (2*dst.x+dst.y*vid->pitch), vid->pitch, src.w, src.h);
	if (SDL_MUSTLOCK(vid))
		SDL_UnlockSurface(vid);
	SDL_UpdateRects(vid,1,&dst);
}

void tv2x_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
}

/* ------------------------------------------------------------------------------------ */
/* Software bilinear video plugin ----------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
void filter_bilinear(Uint8 *srcPtr, Uint32 srcPitch, 
		Uint8 *dstPtr, Uint32 dstPitch, 
		int width, int height)
{
	unsigned int nextlineSrc = srcPitch / sizeof(Uint16);
	Uint16 *p = (Uint16 *)srcPtr;
	unsigned int nextlineDst = dstPitch / sizeof(Uint16);
	Uint16 *q = (Uint16 *)dstPtr;

	while(height--) {
		int i, ii;
		for(i = 0, ii = 0; i < width; ++i, ii += 2) {
			Uint16 A = *(p + i);
			Uint16 B = *(p + i + 1);
			Uint16 C = *(p + i + nextlineSrc);
			Uint16 D = *(p + i + nextlineSrc + 1);
			*(q + ii) = A;
			*(q + ii + 1) = INTERPOLATE(A, B);
			*(q + ii + nextlineDst) = INTERPOLATE(A, C);
			*(q + ii + nextlineDst + 1) = Q_INTERPOLATE(A, B, C, D);
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}

SDL_Surface* swbilin_init(int w,int h, int bpp, bool fs)
{
	if (bpp!=16)
		return NULL;
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH*2;
		h=CPC_VISIBLE_SCR_HEIGHT*2;
	}
	vid=SDL_SetVideoMode(w,h,bpp,SDL_ANYFORMAT | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	if (vid->format->BitsPerPixel!=16)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH,CPC_VISIBLE_SCR_HEIGHT,bpp,0,0,0,0);
	return pub;
}

void swbilin_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool swbilin_lock()
{
	return true;
}

void swbilin_unlock()
{
}

void swbilin_flip()
{
	if (SDL_MUSTLOCK(vid))
		SDL_LockSurface(vid);
	SDL_Rect src;
	SDL_Rect dst;
	compute_rects(&src,&dst);
	filter_bilinear((Uint8*)pub->pixels + (2*src.x+src.y*pub->pitch), pub->pitch,
		 (Uint8*)vid->pixels + (2*dst.x+dst.y*vid->pitch), vid->pitch, src.w, src.h);
	if (SDL_MUSTLOCK(vid))
		SDL_UnlockSurface(vid);
	SDL_UpdateRects(vid,1,&dst);
}

void swbilin_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
}

/* ------------------------------------------------------------------------------------ */
/* Software bicubic video plugin ------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
#define BLUE_MASK565 0x001F001F
#define RED_MASK565 0xF800F800
#define GREEN_MASK565 0x07E007E0

#define BLUE_MASK555 0x001F001F
#define RED_MASK555 0x7C007C00
#define GREEN_MASK555 0x03E003E0

__inline__ static void MULT(Uint16 c, float* r, float* g, float* b, float alpha) {
  *r += alpha * ((c & RED_MASK565  ) >> 11);
  *g += alpha * ((c & GREEN_MASK565) >>  5);
  *b += alpha * ((c & BLUE_MASK565 ) >>  0);
}

__inline__ static Uint16 MAKE_RGB565(float r, float g, float b) {
  return 
  ((((Uint8)r) << 11) & RED_MASK565  ) |
  ((((Uint8)g) <<  5) & GREEN_MASK565) |
  ((((Uint8)b) <<  0) & BLUE_MASK565 );
}

__inline__ float CUBIC_WEIGHT(float x) {
  // P(x) = { x, x>0 | 0, x<=0 }
  // P(x + 2) ^ 3 - 4 * P(x + 1) ^ 3 + 6 * P(x) ^ 3 - 4 * P(x - 1) ^ 3
  double r = 0.;
  if(x + 2 > 0) r +=      pow(x + 2, 3);
  if(x + 1 > 0) r += -4 * pow(x + 1, 3);
  if(x     > 0) r +=  6 * pow(x    , 3);
  if(x - 1 > 0) r += -4 * pow(x - 1, 3);
  return (float)r / 6;
}

void filter_bicubic(Uint8 *srcPtr, Uint32 srcPitch, 
                    Uint8 *dstPtr, Uint32 dstPitch, 
                    int width, int height)
{
  unsigned int nextlineSrc = srcPitch / sizeof(Uint16);
  Uint16 *p = (Uint16 *)srcPtr;
  unsigned int nextlineDst = dstPitch / sizeof(Uint16);
  Uint16 *q = (Uint16 *)dstPtr;
  int dx = width << 1, dy = height << 1;
  float fsx = (float)width / dx;
	float fsy = (float)height / dy;
	float v = 0.0f;
	int j = 0;
	for(; j < dy; ++j) {
	  float u = 0.0f;
	  int iv = (int)v;
    float decy = v - iv;
    int i = 0;
	  for(; i < dx; ++i) {
		  int iu = (int)u;
		  float decx = u - iu;
      float r, g, b;
      int m;
      r = g = b = 0.;
      for(m = -1; m <= 2; ++m) {
        float r1 = CUBIC_WEIGHT(decy - m);
        int n;
        for(n = -1; n <= 2; ++n) {
          float r2 = CUBIC_WEIGHT(n - decx);
          Uint16* pIn = p + (iu  + n) + (iv + m) * nextlineSrc;
          MULT(*pIn, &r, &g, &b, r1 * r2);
        }
      }
      *(q + i) = MAKE_RGB565(r, g, b);
      u += fsx;
	  }
    q += nextlineDst;
	  v += fsy;
  }
}

SDL_Surface* swbicub_init(int w,int h, int bpp, bool fs)
{
	if (bpp!=16)
		return NULL;
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH*2;
		h=CPC_VISIBLE_SCR_HEIGHT*2;
	}
	vid=SDL_SetVideoMode(w,h,bpp,SDL_ANYFORMAT | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	if (vid->format->BitsPerPixel!=16)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH,CPC_VISIBLE_SCR_HEIGHT,bpp,0,0,0,0);
	return pub;
}

void swbicub_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool swbicub_lock()
{
	return true;
}

void swbicub_unlock()
{
}

void swbicub_flip()
{
	if (SDL_MUSTLOCK(vid))
		SDL_LockSurface(vid);
	SDL_Rect src;
	SDL_Rect dst;
	compute_rects(&src,&dst);
	filter_bicubic((Uint8*)pub->pixels + (2*src.x+src.y*pub->pitch), pub->pitch,
		 (Uint8*)vid->pixels + (2*dst.x+dst.y*vid->pitch), vid->pitch, src.w, src.h);
	if (SDL_MUSTLOCK(vid))
		SDL_UnlockSurface(vid);
	SDL_UpdateRects(vid,1,&dst);
}

void swbicub_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
}


/* ------------------------------------------------------------------------------------ */
/* Dot matrix video plugin ------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
static Uint16 DOT_16(Uint16 c, int j, int i) {
  static const Uint16 dotmatrix[16] = {
	  0x01E0, 0x0007, 0x3800, 0x0000,
	  0x39E7, 0x0000, 0x39E7, 0x0000,
	  0x3800, 0x0000, 0x01E0, 0x0007,
	  0x39E7, 0x0000, 0x39E7, 0x0000
  };
  return c - ((c >> 2) & *(dotmatrix + ((j & 3) << 2) + (i & 3)));
}

void filter_dotmatrix(Uint8 *srcPtr, Uint32 srcPitch, 
		Uint8 *dstPtr, Uint32 dstPitch,
		int width, int height)
{
	unsigned int nextlineSrc = srcPitch / sizeof(Uint16);
	Uint16 *p = (Uint16 *)srcPtr;

	unsigned int nextlineDst = dstPitch / sizeof(Uint16);
	Uint16 *q = (Uint16 *)dstPtr;

	int i, ii, j, jj;
	for(j = 0, jj = 0; j < height; ++j, jj += 2) {
		for(i = 0, ii = 0; i < width; ++i, ii += 2) {
			Uint16 c = *(p + i);
			*(q + ii) = DOT_16(c, jj, ii);
			*(q + ii + 1) = DOT_16(c, jj, ii + 1);
			*(q + ii + nextlineDst) = DOT_16(c, jj + 1, ii);
			*(q + ii + nextlineDst + 1) = DOT_16(c, jj + 1, ii + 1);
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}

SDL_Surface* dotmat_init(int w,int h, int bpp, bool fs)
{
	if (bpp!=16)
		return NULL;
	if (!fs)
	{
		w=CPC_VISIBLE_SCR_WIDTH*2;
		h=CPC_VISIBLE_SCR_HEIGHT*2;
	}
	vid=SDL_SetVideoMode(w,h,bpp,SDL_ANYFORMAT | SDL_HWSURFACE | SDL_HWPALETTE | (fs?SDL_FULLSCREEN:0));
	if (!vid)
		return NULL;
	if (vid->format->BitsPerPixel!=16)
		return NULL;
	SDL_FillRect(vid,NULL,SDL_MapRGB(vid->format,0,0,0));
	pub=SDL_CreateRGBSurface(SDL_SWSURFACE,CPC_VISIBLE_SCR_WIDTH,CPC_VISIBLE_SCR_HEIGHT,bpp,0,0,0,0);
	return pub;
}

void dotmat_setpal(SDL_Color* c)
{
	SDL_SetPalette(vid, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
	SDL_SetPalette(pub, SDL_LOGPAL | SDL_PHYSPAL, c, 0, 32); 
}

bool dotmat_lock()
{
	return true;
}

void dotmat_unlock()
{
}

void dotmat_flip()
{
	if (SDL_MUSTLOCK(vid))
		SDL_LockSurface(vid);
	SDL_Rect src;
	SDL_Rect dst;
	compute_rects(&src,&dst);
	filter_dotmatrix((Uint8*)pub->pixels + (2*src.x+src.y*pub->pitch), pub->pitch,
		 (Uint8*)vid->pixels + (2*dst.x+dst.y*vid->pitch), vid->pitch, src.w, src.h);
	if (SDL_MUSTLOCK(vid))
		SDL_UnlockSurface(vid);
	SDL_UpdateRects(vid,1,&dst);
}

void dotmat_close()
{
	if (!vid)
		return;
	SDL_FreeSurface(pub);
}


/* ------------------------------------------------------------------------------------ */
/* End of video plugins --------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */

video_plugin video_plugin_list[]=
{
/* Name					Init func	Palette func 	Lock func		Unlock func		Flip func	Close func 		Pixel formats	Half size ?		*/
{"Scanlines",				scanline_init,	scanline_setpal,scanline_lock,		scanline_unlock,	scanline_flip,	scanline_close,		ALL,		0			},
{"Double size",				double_init,	double_setpal,	double_lock,		double_unlock,		double_flip,	double_close,		ALL,		0			},
{"Double width",			doublew_init,	doublew_setpal,	doublew_lock,		doublew_unlock,		doublew_flip,	doublew_close,		ALL,		0			},
{"Half size",				half_init,	half_setpal,	half_lock,		half_unlock,		half_flip,	half_close,		ALL,		1			},
{"Half size with hardware flip",	halfhw_init,	halfhw_setpal,	halfhw_lock,		halfhw_unlock,		halfhw_flip,	half_close,		ALL,		1			},
{"Super eagle",				seagle_init,	seagle_setpal,	seagle_lock,		seagle_unlock,		seagle_flip,	seagle_close,		F16_BPP,	1			},
{"Scale2x",				scale2x_init,	scale2x_setpal,	scale2x_lock,		scale2x_unlock,		scale2x_flip,	scale2x_close,		F16_BPP,	1			},
{"Advanced Scale2x",			ascale2x_init,	ascale2x_setpal,ascale2x_lock,		ascale2x_unlock,	ascale2x_flip,	ascale2x_close,		F16_BPP,	1			},
{"TV 2x",				tv2x_init,	tv2x_setpal,	tv2x_lock,		tv2x_unlock,		tv2x_flip,	tv2x_close,		F16_BPP,	1			},
{"Software bilinear",			swbilin_init,	swbilin_setpal,	swbilin_lock,		swbilin_unlock,		swbilin_flip,	swbilin_close,		F16_BPP,	1			},
{"Software bicubic",			swbicub_init,	swbicub_setpal,	swbicub_lock,		swbicub_unlock,		swbicub_flip,	swbicub_close,		F16_BPP,	1			},
{"Dot matrix",				dotmat_init,	dotmat_setpal,	dotmat_lock,		dotmat_unlock,		dotmat_flip,	dotmat_close,		F16_BPP,	1			},
{"OpenGL scaling",			glscale_init,	glscale_setpal,	glscale_lock,		glscale_unlock,		glscale_flip,	glscale_close,		ALL,		1			},
{"OpenGL scaling, 25%% scanlines",	glscale25_init,	glscale_setpal,	glscale_lock,		glscale_unlock,		glscale_flip,	glscale_close,		ALL,		1			},
{"OpenGL scaling, 50%% scanlines",	glscale50_init,	glscale_setpal,	glscale_lock,		glscale_unlock,		glscale_flip,	glscale_close,		ALL,		1			},
{"OpenGL scaling, 75%% scanlines",	glscale75_init,	glscale_setpal,	glscale_lock,		glscale_unlock,		glscale_flip,	glscale_close,		ALL,		1			},
{"OpenGL scaling, 100%% scanlines",	glscale100_init,glscale_setpal,	glscale_lock,		glscale_unlock,		glscale_flip,	glscale_close,		ALL,		1			},
{NULL,					NULL,		NULL,		NULL,			NULL,			NULL,		NULL,			0,		0			}
};

