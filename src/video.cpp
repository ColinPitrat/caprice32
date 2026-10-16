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
#include "log.h"
#include "glfuncs.h"
#ifdef HAVE_GL
#include "SDL_opengl.h"
#endif
#include <math.h>
#include <memory>
#include <iostream>

SDL_Window* mainSDLWindow = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;
SDL_GLContext glcontext;

// the video surface ready to display
SDL_Surface* vid = nullptr;
// the video surface scaled with same format as pub
SDL_Surface* scaled = nullptr;
// the video surface shown by the plugin to the application
SDL_Surface* pub = nullptr;

extern t_CPC CPC;

#ifndef min
#define min(a,b) ((a)<(b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b))
#endif

// checks for an OpenGL extension
#ifdef HAVE_GL
static bool have_gl_extension (const char *nom_ext)
{
   const char *ext;
   ext = reinterpret_cast<const char *> (eglGetString (GL_EXTENSIONS));
   const char *f;
   if (ext == nullptr)
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
#endif

// Returns a bpp compatible with the renderer
int renderer_bpp(SDL_Renderer *sdl_renderer)
{
  SDL_RendererInfo infos;
  SDL_GetRendererInfo(sdl_renderer, &infos);
  return SDL_BITSPERPIXEL(infos.texture_formats[0]);
}

// TODO: Cleanup sw_scaling if really not needed
void compute_scale(video_plugin* t, int w, int h)
{
  int win_width, win_height;
  SDL_GetWindowSize(mainSDLWindow, &win_width, &win_height);
  if (CPC.scr_preserve_aspect_ratio != 0) {
    float win_x_scale, win_y_scale;
    win_x_scale = w/static_cast<float>(win_width);
    win_y_scale = h/static_cast<float>(win_height);
    float scale = max(win_x_scale, win_y_scale);
    t->width=w/scale;
    t->height=h/scale;
    float x_offset = 0.5*(win_width-t->width);
    float y_offset = 0.5*(win_height-t->height);
    t->x_offset=x_offset;
    t->y_offset=y_offset;
    t->x_scale=scale;
    t->y_scale=scale;
  } else {
    t->x_offset=0;
    t->y_offset=0;
    t->x_scale=w/static_cast<float>(win_width);
    t->y_scale=h/static_cast<float>(win_height);
    t->width = win_width;
    t->height = win_height;
  }
}

/* ------------------------------------------------------------------------------------ */
/* Half size video plugin ------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
SDL_Surface* direct_init(video_plugin* t, int scale, bool fs)
{
  SDL_CreateWindowAndRenderer(CPC_VISIBLE_SCR_WIDTH*scale, CPC_VISIBLE_SCR_HEIGHT*scale, (fs?SDL_WINDOW_FULLSCREEN_DESKTOP:SDL_WINDOW_SHOWN), &mainSDLWindow, &renderer);
  if (!mainSDLWindow || !renderer) return nullptr;
  SDL_SetWindowTitle(mainSDLWindow, "Caprice32 " VERSION_STRING);
  int surface_width, surface_height;
  if (scale > 1) {
    t->half_pixels = 0;
    surface_width = CPC_VISIBLE_SCR_WIDTH * 2;
    surface_height = CPC_VISIBLE_SCR_HEIGHT * 2;
  } else {
    t->half_pixels = 1;
    surface_width = CPC_VISIBLE_SCR_WIDTH;
    surface_height = CPC_VISIBLE_SCR_HEIGHT;
  }
  vid = SDL_CreateRGBSurface(0, surface_width, surface_height, renderer_bpp(renderer), 0, 0, 0, 0);
  if (!vid) return nullptr;
  texture = SDL_CreateTextureFromSurface(renderer, vid);
  if (!texture) return nullptr;
  SDL_FillRect(vid, nullptr, SDL_MapRGB(vid->format,0,0,0));
  compute_scale(t, surface_width, surface_height);
  return vid;
}

void direct_setpal(SDL_Color* c)
{
  SDL_SetPaletteColors(vid->format->palette, c, 0, 32);
}

void direct_flip(video_plugin* t)
{
  SDL_UpdateTexture(texture, nullptr, vid->pixels, vid->pitch);
  SDL_RenderClear(renderer);
  if (CPC.scr_preserve_aspect_ratio != 0) {
    SDL_Rect dest_rect = { t->x_offset, t->y_offset, t->width, t->height };
    SDL_RenderCopy(renderer, texture, nullptr, &dest_rect);
  } else {
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  }
  SDL_RenderPresent(renderer);
}

void direct_close()
{
  if (texture) SDL_DestroyTexture(texture);
  if (vid) SDL_FreeSurface(vid);
  if (renderer) SDL_DestroyRenderer(renderer);
  if (mainSDLWindow) SDL_DestroyWindow(mainSDLWindow);
}


#ifdef HAVE_GL
/* ------------------------------------------------------------------------------------ */
/* OpenGL scaling video plugin -------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
static int tex_x,tex_y;
static GLuint screen_texnum,modulate_texnum;
static int gl_scanlines;

SDL_Surface* glscale_init(video_plugin* t, int scale, bool fs)
{
#ifdef _WIN32
  const char *gl_library = "OpenGL32.DLL";
#else
  const char *gl_library = "libGL.so.1";
#endif

  gl_scanlines=CPC.scr_oglscanlines;
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  if (SDL_GL_LoadLibrary(gl_library)<0)
  {
    fprintf(stderr,"Unable to dynamically open GL lib : %s\n",SDL_GetError());
    return nullptr;
  }

  int width = CPC_VISIBLE_SCR_WIDTH*scale;
  int height = CPC_VISIBLE_SCR_HEIGHT*scale;
  SDL_CreateWindowAndRenderer(width, height, (fs?SDL_WINDOW_FULLSCREEN_DESKTOP:SDL_WINDOW_SHOWN) | SDL_WINDOW_OPENGL, &mainSDLWindow, &renderer);
  if (!mainSDLWindow || !renderer) return nullptr;
  if (fs) {
    SDL_DisplayMode display;
    SDL_GetCurrentDisplayMode(0, &display);
    width = display.w;
    height = display.h;
  }
  vid = SDL_CreateRGBSurface(0, width, height, renderer_bpp(renderer), 0, 0, 0, 0);
  if (!vid) return nullptr;
  glcontext = SDL_GL_CreateContext(mainSDLWindow);
  if (init_glfuncs()!=0)
  {
    fprintf(stderr, "Cannot init OpenGL functions: %s\n", SDL_GetError());
    return nullptr;
  }

  int major, minor;
  const char *version;
  version = reinterpret_cast<const char *>(eglGetString(GL_VERSION));
  if (sscanf(version, "%d.%d", &major, &minor) != 2) {
    fprintf(stderr, "Unable to get OpenGL version: got %s.\n", version);
    return nullptr;
  }

  GLint max_texsize;
  eglGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_texsize);
  if (max_texsize<1024) {
      printf("Your OpenGL implementation doesn't support 1024x1024 textures: max size = %d\n", max_texsize);
      t->half_pixels = 1;
   }
  if (max_texsize<512) {
    fprintf(stderr, "Your OpenGL implementation doesn't support 512x512 textures\n");
    return nullptr;
  }

   unsigned int original_width, original_height, tex_size;
   if (t->half_pixels) {
      tex_size = 512;
      original_width = CPC_VISIBLE_SCR_WIDTH;
      original_height = CPC_VISIBLE_SCR_HEIGHT;
   } else {
      tex_size = 1024;
      original_width = CPC_VISIBLE_SCR_WIDTH * 2;
      original_height = CPC_VISIBLE_SCR_HEIGHT * 2;
   }

  compute_scale(t, original_width, original_height);

  // We have to react differently to the bpp parameter than with software rendering
  // Here are the rules :
  // for 8bpp OpenGL, we need the GL_EXT_paletted_texture extension
  // for 16bpp OpenGL, we need OpenGL 1.2+
  // for 24bpp reversed OpenGL, we need OpenGL 1.2+
  std::vector<int> candidates_bpp{32, 24, 16, 8};
  int surface_bpp = 0;
  for (int try_bpp : candidates_bpp) {
    switch(try_bpp)
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
    if (surface_bpp == 0) {
      fprintf(stderr, "Your OpenGL implementation doesn't support %dbpp textures\n", try_bpp);
    } else {
      break;
    }
  }
  if (surface_bpp == 0) {
    fprintf(stderr, "FATAL: Couldn't find a supported OpenGL color depth.\n");
    return nullptr;
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
  tex_x=tex_size;
  tex_y=tex_size;

  switch(surface_bpp)
  {
    case 24:
      eglTexImage2D(GL_TEXTURE_2D, 0,GL_RGB,tex_x,tex_y, 0,
          GL_RGB,
          GL_UNSIGNED_BYTE, nullptr);
      break;
    case 16:
      eglTexImage2D(GL_TEXTURE_2D, 0,GL_RGB5,tex_x,tex_y, 0,
          GL_RGB,
          GL_UNSIGNED_BYTE, nullptr);
      break;
    case 8:
      eglTexImage2D(GL_TEXTURE_2D, 0,GL_COLOR_INDEX8_EXT,tex_x,tex_y, 0,
          GL_COLOR_INDEX,
          GL_UNSIGNED_BYTE, nullptr);
      break;
  }

  if (gl_scanlines!=0)
  {
    uint8_t texmod;
    texmod=(100-gl_scanlines)*255/100;
    eglGenTextures(1,&modulate_texnum);
    eglBindTexture(GL_TEXTURE_2D,modulate_texnum);
    eglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, CPC.scr_oglfilter?GL_LINEAR:GL_NEAREST);
    eglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, CPC.scr_oglfilter?GL_LINEAR:GL_NEAREST);

    uint8_t modulate_texture[]={
      255,255,255,
      0,0,0};
    modulate_texture[3]=texmod;
    modulate_texture[4]=texmod;
    modulate_texture[5]=texmod;
    eglTexImage2D(GL_TEXTURE_2D, 0,GL_RGB8,1,2, 0,GL_RGB,GL_UNSIGNED_BYTE, modulate_texture);
  }
  if (CPC.scr_preserve_aspect_ratio) {
    eglViewport(t->x_offset, t->y_offset, t->width, t->height);
  } else {
    eglViewport(0, 0, width, height);
  }
  eglMatrixMode(GL_PROJECTION);
  eglLoadIdentity();
  eglOrtho(0, width, height, 0, -1.0, 1.0);

  eglMatrixMode(GL_MODELVIEW);
  eglLoadIdentity();

  pub=SDL_CreateRGBSurface(0, original_width, original_height, surface_bpp, 0, 0, 0, 0);
  return pub;
}

void glscale_setpal(SDL_Color* c)
{
  SDL_SetPaletteColors(pub->format->palette, c, 0, 32);
  if (pub->format->palette)
  {
    std::unique_ptr<uint8_t[]> pal = std::make_unique<uint8_t[]>(256*3);
    for(int i=0;i<256;i++)
    {
      pal[3*i  ] = pub->format->palette->colors[i].r;
      pal[3*i+1] = pub->format->palette->colors[i].g;
      pal[3*i+2] = pub->format->palette->colors[i].b;
    }
    eglBindTexture(GL_TEXTURE_2D,screen_texnum);
    eglColorTableEXT(GL_TEXTURE_2D,GL_RGB8,256,GL_RGB,GL_UNSIGNED_BYTE,pal.get());
  }
}

void glscale_flip(video_plugin* t __attribute__((unused)))
{
  eglDisable(GL_BLEND);
  eglClearColor(0,0,0,1);
  eglClear(GL_COLOR_BUFFER_BIT);
  
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
  
  if (CPC.scr_remanency && !CPC.scr_gui_is_currently_on)
  {
    /* draw again using the old texture */
    eglBegin(GL_QUADS);
    eglColor4f(1.0,1.0,1.0,1.0);

    eglTexCoord2f(0.f, 0.f);
    if (gl_scanlines!=0)
      eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,0.f, 0.f);
    eglVertex2i(0, 0);

    eglTexCoord2f(0.f, static_cast<float>(pub->h)/tex_y);
    if (gl_scanlines!=0)
      eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,0.f, vid->h/2);
    eglVertex2i(0, vid->h);

    eglTexCoord2f(static_cast<float>(pub->w)/tex_x, static_cast<float>(pub->h)/tex_y);
    if (gl_scanlines!=0)
      eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,vid->w, vid->h/2);
    eglVertex2i(vid->w, vid->h);

    eglTexCoord2f(static_cast<float>(pub->w)/tex_x, 0.f);
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

  eglTexCoord2f(0.f, static_cast<float>(pub->h)/tex_y);
  if (gl_scanlines!=0)
    eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,0.f, vid->h/2);
  eglVertex2i(0, vid->h);

  eglTexCoord2f(static_cast<float>(pub->w)/tex_x, static_cast<float>(pub->h)/tex_y);
  if (gl_scanlines!=0)
    eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,vid->w, vid->h/2);
  eglVertex2i(vid->w, vid->h);

  eglTexCoord2f(static_cast<float>(pub->w)/tex_x, 0.f);
  if (gl_scanlines!=0)
    eglMultiTexCoord2fARB(GL_TEXTURE1_ARB,vid->w, 0);
  eglVertex2i(vid->w, 0);
  eglEnd();

  SDL_GL_SwapWindow(mainSDLWindow);
}

void glscale_close()
{
  direct_close();
  SDL_FreeSurface(pub);
  pub = nullptr;
}
#endif // HAVE_GL

/* ------------------------------------------------------------------------------------ */
/* Common 2x software scaling code ---------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */

/* Computes the clipping of pub and scaled surfaces and put the result in src and dst accordingly.
 *
 * This provides the rectangles to clip to obtain a centered doubled CPC display
 * in the middle of the dst surface if it fits
 *
 * dst is the screen
 * src is the internal window
 *
 * Only exposed for testing purposes. Shouldn't be used outside of video.cpp
 */
static void compute_rects(SDL_Rect* src, SDL_Rect* dst, uint8_t half_pixels)
{
  int surface_width = CPC_VISIBLE_SCR_WIDTH*4;
  int surface_height = CPC_VISIBLE_SCR_HEIGHT*4;
  if (half_pixels) {
    surface_width = CPC_VISIBLE_SCR_WIDTH*2;
    surface_height = CPC_VISIBLE_SCR_HEIGHT*2;
  }
  /* initialise the source rect to full source */
  src->x=0;
  src->y=0;
  src->w=pub->w;
  src->h=pub->h;
  
  dst->x=(scaled->w-surface_width)/2,
  dst->y=(scaled->h-surface_height)/2;
  dst->w=scaled->w;
  dst->h=scaled->h;
  
  int dw=src->w*2-dst->w;
  /* the src width is too big */
  if (dw>0)
  {
    // To ensure src is not bigger than dst for odd widths.
    dw += 1;
    src->w-=dw/2;
    src->x+=dw/4;

    dst->x=0;
    dst->w=scaled->w;
  }
  else
  {
    dst->w=surface_width;
  }
  int dh=src->h*2-dst->h;
  /* the src height is too big */
  if (dh>0)
  {
    // To ensure src is not bigger than dst for odd heights.
    dh += 1;
    src->h-=dh/2;
    src->y+=dh/4;
    
    dst->y=0;
    dst->h=scaled->h;
  }
  else
  {
    // Without this -=, the bottom of the screen has line with random pixels.
    // With this, they are black instead which is slightly better.
    // Investigating where this comes from and how to avoid it would be nice!
    src->h-=2*2;
    dst->h=surface_height;
  }
}

void compute_rects_for_tests(SDL_Rect* src, SDL_Rect* dst, uint8_t half_pixels)
{
  compute_rects(src, dst, half_pixels);
}

SDL_Surface* swscale_init(video_plugin* t, int scale, bool fs)
{
  SDL_CreateWindowAndRenderer(CPC_VISIBLE_SCR_WIDTH*scale, CPC_VISIBLE_SCR_HEIGHT*scale, (fs?SDL_WINDOW_FULLSCREEN_DESKTOP:SDL_WINDOW_SHOWN), &mainSDLWindow, &renderer);
  if (!mainSDLWindow || !renderer) return nullptr;
  SDL_SetWindowTitle(mainSDLWindow, "Caprice32 " VERSION_STRING);
  int surface_width, surface_height;
  if (scale < 4) {
    t->half_pixels = 1;
    surface_width = CPC_VISIBLE_SCR_WIDTH;
    surface_height = CPC_VISIBLE_SCR_HEIGHT;
  } else {
    t->half_pixels = 0;
    surface_width = CPC_VISIBLE_SCR_WIDTH * 2;
    surface_height = CPC_VISIBLE_SCR_HEIGHT * 2;
  }
  vid = SDL_CreateRGBSurface(0, surface_width*2, surface_height*2, renderer_bpp(renderer), 0, 0, 0, 0);
  if (!vid) return nullptr;
  texture = SDL_CreateTextureFromSurface(renderer, vid);
  if (!texture) return nullptr;

  scaled = SDL_CreateRGBSurface(0, surface_width*2, surface_height*2, 16, 0, 0, 0, 0);
  if (!scaled) return nullptr;
  if (scaled->format->BitsPerPixel!=16)
  {
    LOG_ERROR(t->name << ": SDL didn't return a 16 bpp surface but a " << static_cast<int>(scaled->format->BitsPerPixel) << " bpp one.");
    return nullptr;
  }
  SDL_FillRect(vid, nullptr, SDL_MapRGB(vid->format,0,0,0));
  compute_scale(t, surface_width, surface_height);
  pub = SDL_CreateRGBSurface(0, surface_width, surface_height, 16, 0, 0, 0, 0);
  if (pub->format->BitsPerPixel!=16)
  {
    LOG_ERROR(t->name << ": SDL didn't return a 16 bpp surface but a " << static_cast<int>(pub->format->BitsPerPixel) << " bpp one.");
    return nullptr;
  }

  return pub;
}

// Common code to all software plugin to display the vid surface after it's been computed.
void swscale_blit(video_plugin* t)
{
  // Blit to convert from 16bpp to pixel format compatible with renderer.
  SDL_BlitSurface(scaled, nullptr, vid, nullptr);
  SDL_UpdateTexture(texture, nullptr, vid->pixels, vid->pitch);
  SDL_RenderClear(renderer);
  if (CPC.scr_preserve_aspect_ratio != 0) {
    SDL_Rect dest_rect = { t->x_offset, t->y_offset, t->width, t->height };
    SDL_RenderCopy(renderer, texture, nullptr, &dest_rect);
  } else {
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  }
  SDL_RenderPresent(renderer);
}

void swscale_setpal(SDL_Color* c)
{
  SDL_SetPaletteColors(scaled->format->palette, c, 0, 32);
  SDL_SetPaletteColors(pub->format->palette, c, 0, 32);
}

void swscale_close()
{
  direct_close();
  SDL_FreeSurface(pub);
  pub = nullptr;
}

/* ------------------------------------------------------------------------------------ */
/* Super eagle video plugin ----------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */

/* 2X SAI Filter */
static uint32_t colorMask = 0xF7DEF7DE;
static uint32_t lowPixelMask = 0x08210821;
static uint32_t qcolorMask = 0xE79CE79C;
static uint32_t qlowpixelMask = 0x18631863;
static uint32_t redblueMask = 0xF81F;
static uint32_t greenMask = 0x7E0;

__inline__ int GetResult1 (uint32_t A, uint32_t B, uint32_t C, uint32_t D)
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

__inline__ int GetResult2 (uint32_t A, uint32_t B, uint32_t C, uint32_t D)
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

__inline__ int GetResult (uint32_t A, uint32_t B, uint32_t C, uint32_t D)
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


__inline__ uint32_t INTERPOLATE (uint32_t A, uint32_t B)
{
  if (A != B)
  {
    return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) +
        (A & B & lowPixelMask));
  }
  return A;
}

__inline__ uint32_t Q_INTERPOLATE (uint32_t A, uint32_t B, uint32_t C, uint32_t D)
{
  uint32_t x = ((A & qcolorMask) >> 2) +
    ((B & qcolorMask) >> 2) +
    ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
  uint32_t y = (A & qlowpixelMask) +
    (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);
  y = (y >> 2) & qlowpixelMask;
  return x + y;
}

void filter_supereagle(uint8_t *srcPtr, uint32_t srcPitch, /* uint8_t *deltaPtr,  */
     uint8_t *dstPtr, uint32_t dstPitch, int width, int height)
{
  uint8_t  *dP;
  uint16_t *bP;
  uint32_t inc_bP;



  uint32_t finish;
  uint32_t Nextline = srcPitch >> 1;

  inc_bP = 1;

  for (; height ; height--)
  {
    bP = reinterpret_cast<uint16_t *>(srcPtr);
    dP = dstPtr;
    for (finish = width; finish; finish -= inc_bP)
    {
      uint32_t color4, color5, color6;
      uint32_t color1, color2, color3;
      uint32_t colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
      uint32_t product1a, product1b, product2a, product2b;
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
        int r = 0;

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

      *(reinterpret_cast<uint32_t *>(dP)) = product1a;
      *(reinterpret_cast<uint32_t *>(dP + dstPitch)) = product2a;

      bP += inc_bP;
      dP += sizeof (uint32_t);
    }      // end of for ( finish= width etc..)
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
  }      // endof: for (height; height; height--)
}

void seagle_flip(video_plugin* t)
{
  if (SDL_MUSTLOCK(scaled))
    SDL_LockSurface(scaled);
  SDL_Rect src;
  SDL_Rect dst;
  compute_rects(&src,&dst,t->half_pixels);
  filter_supereagle(static_cast<uint8_t*>(pub->pixels) + (2*src.x+src.y*pub->pitch) + (pub->pitch), pub->pitch,
     static_cast<uint8_t*>(scaled->pixels) + (2*dst.x+dst.y*scaled->pitch), scaled->pitch, src.w, src.h);
  if (SDL_MUSTLOCK(scaled))
    SDL_UnlockSurface(scaled);
  swscale_blit(t);
}

/* ------------------------------------------------------------------------------------ */
/* Scale2x video plugin --------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
void filter_scale2x(uint8_t *srcPtr, uint32_t srcPitch, 
                      uint8_t *dstPtr, uint32_t dstPitch,
          int width, int height)
{
  unsigned int nextlineSrc = srcPitch / sizeof(short);
  short *p = reinterpret_cast<short *>(srcPtr);

  unsigned int nextlineDst = dstPitch / sizeof(short);
  short *q = reinterpret_cast<short *>(dstPtr);

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

void scale2x_flip(video_plugin* t __attribute__((unused)))
{
  if (SDL_MUSTLOCK(scaled))
    SDL_LockSurface(scaled);
  SDL_Rect src;
  SDL_Rect dst;
  compute_rects(&src,&dst,t->half_pixels);
  filter_scale2x(static_cast<uint8_t*>(pub->pixels) + (2*src.x+src.y*pub->pitch) + (pub->pitch), pub->pitch,
     static_cast<uint8_t*>(scaled->pixels) + (2*dst.x+dst.y*scaled->pitch), scaled->pitch, src.w, src.h);
  if (SDL_MUSTLOCK(scaled))
    SDL_UnlockSurface(scaled);
  swscale_blit(t);
}

/* ------------------------------------------------------------------------------------ */
/* ascale2x video plugin --------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
void filter_ascale2x (uint8_t *srcPtr, uint32_t srcPitch,
       uint8_t *dstPtr, uint32_t dstPitch, int width, int height)
{
  uint8_t  *dP;
  uint16_t *bP;
  uint32_t inc_bP;

  uint32_t finish;
  uint32_t Nextline = srcPitch >> 1;
  inc_bP = 1;

  for (; height; height--)
  {
    bP = reinterpret_cast<uint16_t *>(srcPtr);
    dP = dstPtr;

    for (finish = width; finish; finish -= inc_bP)
    {

      uint32_t colorA, colorB;
      uint32_t colorC, colorD,
             colorE, colorF, colorG, colorH,
             colorI, colorJ, colorK, colorL,

             colorM, colorN, colorO;
      uint32_t product, product1, product2;

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
          int r = 0;

          product1 = INTERPOLATE (colorA, colorC);
          product = INTERPOLATE (colorA, colorB);

          r += GetResult1 (colorA, colorB, colorG, colorE);
          r += GetResult2 (colorB, colorA, colorK, colorF);
          r += GetResult2 (colorB, colorA, colorH, colorN);
          r += GetResult1 (colorA, colorB, colorL, colorO);

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
      *(reinterpret_cast<uint32_t *>(dP)) = product;
      *(reinterpret_cast<uint32_t *>(dP + dstPitch)) = product1;

      bP += inc_bP;
      dP += sizeof (uint32_t);
    }      // end of for ( finish= width etc..)

    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
  }      // endof: for (height; height; height--)
}



void ascale2x_flip(video_plugin* t __attribute__((unused)))
{
  if (SDL_MUSTLOCK(scaled))
    SDL_LockSurface(scaled);
  SDL_Rect src;
  SDL_Rect dst;
  compute_rects(&src,&dst,t->half_pixels);
  filter_ascale2x(static_cast<uint8_t*>(pub->pixels) + (2*src.x+src.y*pub->pitch) + (pub->pitch), pub->pitch,
      static_cast<uint8_t*>(scaled->pixels) + (2*dst.x+dst.y*scaled->pitch), scaled->pitch, src.w, src.h);
  if (SDL_MUSTLOCK(scaled))
    SDL_UnlockSurface(scaled);
  swscale_blit(t);
}


/* ------------------------------------------------------------------------------------ */
/* tv2x video plugin ------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
void filter_tv2x(uint8_t *srcPtr, uint32_t srcPitch, 
    uint8_t *dstPtr, uint32_t dstPitch, 
    int width, int height)
{
  unsigned int nextlineSrc = srcPitch / sizeof(uint16_t);
  uint16_t *p = reinterpret_cast<uint16_t *>(srcPtr);

  unsigned int nextlineDst = dstPitch / sizeof(uint16_t);
  uint16_t *q = reinterpret_cast<uint16_t *>(dstPtr);

  while(height--) {
    int i = 0, j = 0;
    for(; i < width; ++i, j += 2) {
      uint16_t p1 = *(p + i);
      uint32_t pi;

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

void tv2x_flip(video_plugin* t __attribute__((unused)))
{
  if (SDL_MUSTLOCK(scaled))
    SDL_LockSurface(scaled);
  SDL_Rect src;
  SDL_Rect dst;
  compute_rects(&src,&dst,t->half_pixels);
  filter_tv2x(static_cast<uint8_t*>(pub->pixels) + (2*src.x+src.y*pub->pitch) + (pub->pitch), pub->pitch,
      static_cast<uint8_t*>(scaled->pixels) + (2*dst.x+dst.y*scaled->pitch), scaled->pitch, src.w, src.h);
  if (SDL_MUSTLOCK(scaled))
    SDL_UnlockSurface(scaled);
  swscale_blit(t);
}

/* ------------------------------------------------------------------------------------ */
/* Software bilinear video plugin ----------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */
void filter_bilinear(uint8_t *srcPtr, uint32_t srcPitch, 
    uint8_t *dstPtr, uint32_t dstPitch, 
    int width, int height)
{
  unsigned int nextlineSrc = srcPitch / sizeof(uint16_t);
  uint16_t *p = reinterpret_cast<uint16_t *>(srcPtr);
  unsigned int nextlineDst = dstPitch / sizeof(uint16_t);
  uint16_t *q = reinterpret_cast<uint16_t *>(dstPtr);

  while(height--) {
    int i, ii;
    for(i = 0, ii = 0; i < width; ++i, ii += 2) {
      uint16_t A = *(p + i);
      uint16_t B = *(p + i + 1);
      uint16_t C = *(p + i + nextlineSrc);
      uint16_t D = *(p + i + nextlineSrc + 1);
      *(q + ii) = A;
      *(q + ii + 1) = INTERPOLATE(A, B);
      *(q + ii + nextlineDst) = INTERPOLATE(A, C);
      *(q + ii + nextlineDst + 1) = Q_INTERPOLATE(A, B, C, D);
    }
    p += nextlineSrc;
    q += nextlineDst << 1;
  }
}

void swbilin_flip(video_plugin* t __attribute__((unused)))
{
  if (SDL_MUSTLOCK(scaled))
    SDL_LockSurface(scaled);
  SDL_Rect src;
  SDL_Rect dst;
  compute_rects(&src,&dst,t->half_pixels);
  filter_bilinear(static_cast<uint8_t*>(pub->pixels) + (2*src.x+src.y*pub->pitch) + (pub->pitch), pub->pitch,
      static_cast<uint8_t*>(scaled->pixels) + (2*dst.x+dst.y*scaled->pitch), scaled->pitch, src.w, src.h);
  if (SDL_MUSTLOCK(scaled))
    SDL_UnlockSurface(scaled);
  swscale_blit(t);
}

/* ------------------------------------------------------------------------------------ */
/* Software bicubic video plugin ------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
// For fixed 2x upscaling (fsx = fsy = 0.5), the cubic B-spline kernel
//   CUBIC_WEIGHT(x) = (P(x+2)^3 - 4*P(x+1)^3 + 6*P(x)^3 - 4*P(x-1)^3) / 6
// is separable and only evaluated at fractional offsets 0.0 (even) and 0.5 (odd):
//   - dec = 0.0 (m, n in {-1, 0, 1, 2}): [1,  4,  1, 0] / 6
//   - dec = 0.5 (m, n in {-1, 0, 1, 2}): [1, 23, 23, 1] / 48
// This yields 2D rational denominators of 36 (even, even), 288 (even, odd /
// odd, even), and 2304 (odd, odd).
static constexpr int kBicubicMaxRowSize = CPC_VISIBLE_SCR_WIDTH * 2 + 32;

struct BicubicUnpackedRow {
  alignas(32) uint16_t r[kBicubicMaxRowSize];
  alignas(32) uint16_t g[kBicubicMaxRowSize];
  alignas(32) uint16_t b[kBicubicMaxRowSize];
};

static inline uint16_t bicubic_div36(uint16_t x) { return x / 36U; }
static inline uint16_t bicubic_div288(uint16_t x) { return x / 288U; }

// Computes (x0 + 23*(x1 + x2) + x3) / 2304 in 16-bit unsigned arithmetic
// without overflowing uint16_t (raw sum reaches 63 * 2304 = 145152 > 65535):
//   floor((16*s_in + 7*s_in + s_out) / 64) == ((s_in << 2) + ((7*s_in + s_out) >> 2)) >> 4
// followed by division by 36 (since 2304 = 64 * 36).
static inline uint16_t bicubic_div2304(uint16_t x0, uint16_t x1, uint16_t x2, uint16_t x3) {
  uint16_t s_in = x1 + x2;
  uint16_t s_out = x0 + x3;
  uint16_t b_val = 7 * s_in + s_out;
  uint16_t shr6 = static_cast<uint16_t>((s_in << 2) + (b_val >> 2)) >> 4;
  return shr6 / 36U;
}

static inline void bicubic_unpack_row(const uint16_t * __restrict src,
                                      uint16_t * __restrict dst_r,
                                      uint16_t * __restrict dst_g,
                                      uint16_t * __restrict dst_b, int total) {
  for (int u = 0; u < total; ++u) {
    uint16_t c = src[u];
    dst_r[u] = c >> 11;
    dst_g[u] = (c >> 5) & 0x3F;
    dst_b[u] = c & 0x1F;
  }
}

static inline void bicubic_vpass_channel(const uint16_t * __restrict p0,
                                         const uint16_t * __restrict p1,
                                         const uint16_t * __restrict p2,
                                         const uint16_t * __restrict p3,
                                         uint16_t * __restrict ve,
                                         uint16_t * __restrict vo, int total) {
  for (int u = 0; u < total; ++u) {
    ve[u] = p0[u] + 4 * p1[u] + p2[u];
    vo[u] = p0[u] + 23 * (p1[u] + p2[u]) + p3[u];
  }
}

static inline void bicubic_hpass_even(const uint16_t * __restrict ve_r,
                                      const uint16_t * __restrict ve_g,
                                      const uint16_t * __restrict ve_b,
                                      uint16_t * __restrict q_even, int width) {
  for (int u = 0; u < width; ++u) {
    uint16_t re0 = bicubic_div36(ve_r[u] + 4 * ve_r[u + 1] + ve_r[u + 2]);
    uint16_t ge0 = bicubic_div36(ve_g[u] + 4 * ve_g[u + 1] + ve_g[u + 2]);
    uint16_t be0 = bicubic_div36(ve_b[u] + 4 * ve_b[u + 1] + ve_b[u + 2]);

    uint16_t re1 = bicubic_div288(ve_r[u] + 23 * (ve_r[u + 1] + ve_r[u + 2]) + ve_r[u + 3]);
    uint16_t ge1 = bicubic_div288(ve_g[u] + 23 * (ve_g[u + 1] + ve_g[u + 2]) + ve_g[u + 3]);
    uint16_t be1 = bicubic_div288(ve_b[u] + 23 * (ve_b[u + 1] + ve_b[u + 2]) + ve_b[u + 3]);

    q_even[2 * u] = (re0 << 11) | (ge0 << 5) | be0;
    q_even[2 * u + 1] = (re1 << 11) | (ge1 << 5) | be1;
  }
}

static inline void bicubic_hpass_odd(const uint16_t * __restrict vo_r,
                                     const uint16_t * __restrict vo_g,
                                     const uint16_t * __restrict vo_b,
                                     uint16_t * __restrict q_odd, int width) {
  for (int u = 0; u < width; ++u) {
    uint16_t ro0 = bicubic_div288(vo_r[u] + 4 * vo_r[u + 1] + vo_r[u + 2]);
    uint16_t go0 = bicubic_div288(vo_g[u] + 4 * vo_g[u + 1] + vo_g[u + 2]);
    uint16_t bo0 = bicubic_div288(vo_b[u] + 4 * vo_b[u + 1] + vo_b[u + 2]);

    uint16_t ro1 = bicubic_div2304(vo_r[u], vo_r[u + 1], vo_r[u + 2], vo_r[u + 3]);
    uint16_t go1 = bicubic_div2304(vo_g[u], vo_g[u + 1], vo_g[u + 2], vo_g[u + 3]);
    uint16_t bo1 = bicubic_div2304(vo_b[u], vo_b[u + 1], vo_b[u + 2], vo_b[u + 3]);

    q_odd[2 * u] = (ro0 << 11) | (go0 << 5) | bo0;
    q_odd[2 * u + 1] = (ro1 << 11) | (go1 << 5) | bo1;
  }
}

void filter_bicubic(uint8_t *srcPtr, uint32_t srcPitch, 
    uint8_t *dstPtr, uint32_t dstPitch, 
    int width, int height)
{
  if (width <= 0 || height <= 0) return;
  unsigned int nextlineSrc = srcPitch / sizeof(uint16_t);
  const uint16_t *p = reinterpret_cast<const uint16_t *>(srcPtr);
  unsigned int nextlineDst = dstPitch / sizeof(uint16_t);
  uint16_t *q = reinterpret_cast<uint16_t *>(dstPtr);

  BicubicUnpackedRow ubuf[4];
  int total_h = width + 3;
  bicubic_unpack_row(p - static_cast<int>(nextlineSrc) - 1, ubuf[0].r, ubuf[0].g, ubuf[0].b, total_h);
  bicubic_unpack_row(p - 1, ubuf[1].r, ubuf[1].g, ubuf[1].b, total_h);
  bicubic_unpack_row(p + nextlineSrc - 1, ubuf[2].r, ubuf[2].g, ubuf[2].b, total_h);

  alignas(32) uint16_t ve_r[kBicubicMaxRowSize], ve_g[kBicubicMaxRowSize], ve_b[kBicubicMaxRowSize];
  alignas(32) uint16_t vo_r[kBicubicMaxRowSize], vo_g[kBicubicMaxRowSize], vo_b[kBicubicMaxRowSize];

  for (int v = 0; v < height; ++v) {
    const BicubicUnpackedRow &row0 = ubuf[(v + 0) & 3];
    const BicubicUnpackedRow &row1 = ubuf[(v + 1) & 3];
    const BicubicUnpackedRow &row2 = ubuf[(v + 2) & 3];
    BicubicUnpackedRow &row3 = ubuf[(v + 3) & 3];
    bicubic_unpack_row(p + (v + 2) * static_cast<int>(nextlineSrc) - 1, row3.r, row3.g, row3.b, total_h);

    bicubic_vpass_channel(row0.r, row1.r, row2.r, row3.r, ve_r, vo_r, total_h);
    bicubic_vpass_channel(row0.g, row1.g, row2.g, row3.g, ve_g, vo_g, total_h);
    bicubic_vpass_channel(row0.b, row1.b, row2.b, row3.b, ve_b, vo_b, total_h);

    bicubic_hpass_even(ve_r, ve_g, ve_b, q, width);
    bicubic_hpass_odd(vo_r, vo_g, vo_b, q + nextlineDst, width);

    q += nextlineDst << 1;
  }
}

void swbicub_flip(video_plugin* t __attribute__((unused)))
{
  if (SDL_MUSTLOCK(scaled))
    SDL_LockSurface(scaled);
  SDL_Rect src;
  SDL_Rect dst;
  compute_rects(&src,&dst,t->half_pixels);
  filter_bicubic(static_cast<uint8_t*>(pub->pixels) + (2*src.x+src.y*pub->pitch) + (pub->pitch), pub->pitch,
      static_cast<uint8_t*>(scaled->pixels) + (2*dst.x+dst.y*scaled->pitch), scaled->pitch, src.w, src.h);
  if (SDL_MUSTLOCK(scaled))
    SDL_UnlockSurface(scaled);
  swscale_blit(t);
}

/* ------------------------------------------------------------------------------------ */
/* Dot matrix video plugin ------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
static uint16_t DOT_16(uint16_t c, int j, int i) {
  static constexpr uint16_t dotmatrix[16] = {
    0x01E0, 0x0007, 0x3800, 0x0000,
    0x39E7, 0x0000, 0x39E7, 0x0000,
    0x3800, 0x0000, 0x01E0, 0x0007,
    0x39E7, 0x0000, 0x39E7, 0x0000
  };
  return c - ((c >> 2) & *(dotmatrix + ((j & 3) << 2) + (i & 3)));
}

void filter_dotmatrix(uint8_t *srcPtr, uint32_t srcPitch, 
    uint8_t *dstPtr, uint32_t dstPitch,
    int width, int height)
{
  unsigned int nextlineSrc = srcPitch / sizeof(uint16_t);
  uint16_t *p = reinterpret_cast<uint16_t *>(srcPtr);

  unsigned int nextlineDst = dstPitch / sizeof(uint16_t);
  uint16_t *q = reinterpret_cast<uint16_t *>(dstPtr);

  int i, ii, j, jj;
  for(j = 0, jj = 0; j < height; ++j, jj += 2) {
    for(i = 0, ii = 0; i < width; ++i, ii += 2) {
      uint16_t c = *(p + i);
      *(q + ii) = DOT_16(c, jj, ii);
      *(q + ii + 1) = DOT_16(c, jj, ii + 1);
      *(q + ii + nextlineDst) = DOT_16(c, jj + 1, ii);
      *(q + ii + nextlineDst + 1) = DOT_16(c, jj + 1, ii + 1);
    }
    p += nextlineSrc;
    q += nextlineDst << 1;
  }
}

void dotmat_flip(video_plugin* t __attribute__((unused)))
{
  if (SDL_MUSTLOCK(scaled))
    SDL_LockSurface(scaled);
  SDL_Rect src;
  SDL_Rect dst;
  compute_rects(&src,&dst,t->half_pixels);
  filter_dotmatrix(static_cast<uint8_t*>(pub->pixels) + (2*src.x+src.y*pub->pitch) + (pub->pitch), pub->pitch,
      static_cast<uint8_t*>(scaled->pixels) + (2*dst.x+dst.y*scaled->pitch), scaled->pitch, src.w, src.h);
  if (SDL_MUSTLOCK(scaled))
    SDL_UnlockSurface(scaled);
  swscale_blit(t);
}

/* ------------------------------------------------------------------------------------ */
/* End of video plugins --------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------ */

std::vector<video_plugin> video_plugin_list =
{
  // Hardware flip version are the same as software ones since switch to SDL2. Kept for compatibility of config, would be nice to not display them in the UI.
  /* Name                     Hidden Init func      Palette func     Flip func      Close func      Half size  X, Y offsets   X, Y scale  width, height */
  {"Direct",                  false, direct_init,   direct_setpal,   direct_flip,   direct_close,   1,         0, 0,          0, 0, 0, 0 },
  {"Direct double",           true,  direct_init,   direct_setpal,   direct_flip,   direct_close,   0,         0, 0,          0, 0, 0, 0 },
  {"Half size",               true,  direct_init,   direct_setpal,   direct_flip,   direct_close,   1,         0, 0,          0, 0, 0, 0 },
  {"Double size",             true,  direct_init,   direct_setpal,   direct_flip,   direct_close,   0,         0, 0,          0, 0, 0, 0 },
  {"Super eagle",             false, swscale_init,  swscale_setpal,  seagle_flip,   swscale_close,  1,         0, 0,          0, 0, 0, 0 },
  {"Scale2x",                 false, swscale_init,  swscale_setpal,  scale2x_flip,  swscale_close,  1,         0, 0,          0, 0, 0, 0 },
  {"Advanced Scale2x",        false, swscale_init,  swscale_setpal,  ascale2x_flip, swscale_close,  1,         0, 0,          0, 0, 0, 0 },
  {"TV 2x",                   false, swscale_init,  swscale_setpal,  tv2x_flip,     swscale_close,  1,         0, 0,          0, 0, 0, 0 },
  {"Software bilinear",       false, swscale_init,  swscale_setpal,  swbilin_flip,  swscale_close,  1,         0, 0,          0, 0, 0, 0 },
  {"Software bicubic",        false, swscale_init,  swscale_setpal,  swbicub_flip,  swscale_close,  1,         0, 0,          0, 0, 0, 0 },
  {"Dot matrix",              false, swscale_init,  swscale_setpal,  dotmat_flip,   swscale_close,  1,         0, 0,          0, 0, 0, 0 },
#ifdef HAVE_GL
  {"OpenGL scaling",          false, glscale_init,  glscale_setpal,  glscale_flip,  glscale_close,  0,         0, 0,          0, 0, 0, 0 },
#endif
};
