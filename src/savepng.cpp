/*
 * SDL_SavePNG -- libpng-based SDL_Surface writer.
 *
 * This code is free software, available under zlib/libpng license.
 * http://www.libpng.org/pub/png/src/libpng-LICENSE.txt
 * Code was copied and slightly adapted from driedfruit savepng.
 * See https://github.com/driedfruit/SDL_SavePNG
 */
#include <SDL.h>
#include <png.h>
#include <cstdint>
#include <string>

#define SUCCESS 0
#define ERROR (-1)

#define USE_ROW_POINTERS

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define rmask 0xFF000000
#define gmask 0x00FF0000
#define bmask 0x0000FF00
#define amask 0x000000FF
#else
#define rmask 0x000000FF
#define gmask 0x0000FF00
#define bmask 0x00FF0000
#define amask 0xFF000000
#endif

/* libpng callbacks */
static void png_error_SDL(png_structp ctx, png_const_charp str)
{
	SDL_SetError("libpng: %s (ctx is %08x)\n", str, reinterpret_cast<std::uintptr_t>(ctx));
}
static void png_write_SDL(png_structp png_ptr, png_bytep data, png_size_t length)
{
	SDL_RWops *rw = static_cast<SDL_RWops*>(png_get_io_ptr(png_ptr));
	SDL_RWwrite(rw, data, sizeof(png_byte), length);
}

SDL_Surface *SDL_PNGFormatAlpha(SDL_Surface *src)
{
	SDL_Surface *surf;
	SDL_Rect rect = { 0, 0, 0, 0 };
	
	/* Convert 32bpp alpha-less image to 24bpp alpha-less image */
	rect.w = src->w;
	rect.h = src->h;
	surf = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 24, rmask, gmask, bmask, 0);
	SDL_LowerBlit(src, &rect, surf, &rect);

	return surf;
}

int SDL_SavePNG(SDL_Surface *src, const std::string& file)
{
	/* Initialize and do basic error checking */
	if (!src)
	{
		SDL_SetError("Argument 1 to SDL_SavePNG_RW can't be NULL, expecting SDL_Surface*\n");
		return (ERROR);
	}
  SDL_Surface *surface = SDL_PNGFormatAlpha(src);
	if (!src)
	{
		return (ERROR);
	}

	SDL_RWops *dst = SDL_RWFromFile(file.c_str(), "wb");
	if (!dst)
	{
		SDL_SetError("Failed to open file for writing: %s\n", file.c_str());
    SDL_FreeSurface(surface);
		return (ERROR);
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, png_error_SDL, nullptr); /* err_ptr, err_fn, warn_fn */
	if (!png_ptr)
	{
		SDL_SetError("Unable to png_create_write_struct on %s\n", PNG_LIBPNG_VER_STRING);
		SDL_RWclose(dst);
    SDL_FreeSurface(surface);
		return (ERROR);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		SDL_SetError("Unable to png_create_info_struct\n");
		png_destroy_write_struct(&png_ptr, nullptr);
		SDL_RWclose(dst);
    SDL_FreeSurface(surface);
		return (ERROR);
	}

	if (setjmp(png_jmpbuf(png_ptr)))	/* All other errors, see also "png_error_SDL" */
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		SDL_RWclose(dst);
    SDL_FreeSurface(surface);
		return (ERROR);
	}

	int i, colortype;
	/* Setup our RWops writer */
	png_set_write_fn(png_ptr, dst, png_write_SDL, nullptr); /* w_ptr, write_fn, flush_fn */

	SDL_Palette *pal;
	/* Prepare chunks */
	colortype = PNG_COLOR_MASK_COLOR;
	if (surface->format->BytesPerPixel > 0
      && surface->format->BytesPerPixel <= 8
      && (pal = surface->format->palette))
	{
		colortype |= PNG_COLOR_MASK_PALETTE;
		png_colorp pal_ptr = static_cast<png_colorp>(malloc(pal->ncolors * sizeof(png_color)));
		for (i = 0; i < pal->ncolors; i++) {
			pal_ptr[i].red   = pal->colors[i].r;
			pal_ptr[i].green = pal->colors[i].g;
			pal_ptr[i].blue  = pal->colors[i].b;
		}
		png_set_PLTE(png_ptr, info_ptr, pal_ptr, pal->ncolors);
		free(pal_ptr);
	}
	else if (surface->format->BytesPerPixel > 3 || surface->format->Amask)
		colortype |= PNG_COLOR_MASK_ALPHA;

	png_set_IHDR(png_ptr, info_ptr, surface->w, surface->h, 8, colortype,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

//	png_set_packing(png_ptr);

	/* Allow BGR surfaces */
	if (surface->format->Rmask == bmask
      && surface->format->Gmask == gmask
      && surface->format->Bmask == rmask)
		png_set_bgr(png_ptr);

	/* Write everything */
	png_write_info(png_ptr, info_ptr);
#ifdef USE_ROW_POINTERS
	png_bytep *row_pointers = static_cast<png_bytep*>(malloc(sizeof(png_bytep)*surface->h));
	for (i = 0; i < surface->h; i++)
		row_pointers[i] = static_cast<png_bytep>(static_cast<void*>(static_cast<char*>(surface->pixels) + i * surface->pitch));
	png_write_image(png_ptr, row_pointers);
	free(row_pointers);
#else
	for (i = 0; i < surface->h; i++)
		png_write_row(png_ptr, (png_bytep)(Uint8*)surface->pixels + i * surface->pitch);
#endif
	png_write_end(png_ptr, info_ptr);

	/* Done */
	png_destroy_write_struct(&png_ptr, &info_ptr);
	SDL_RWclose(dst);
  SDL_FreeSurface(surface);

	return (SUCCESS);
}
