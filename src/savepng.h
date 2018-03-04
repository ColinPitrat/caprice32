#ifndef _SDL_SAVEPNG
#define _SDL_SAVEPNG
/*
 * SDL_SavePNG -- libpng-based SDL_Surface writer.
 *
 * This code is free software, available under zlib/libpng license.
 * http://www.libpng.org/pub/png/src/libpng-LICENSE.txt
 * Code was copied and slightly adapted from driedfruit savepng.
 * See https://github.com/driedfruit/SDL_SavePNG
 */
#include <SDL_video.h>

/*
 * Save an SDL_Surface as a PNG file.
 * 
 * surface - the SDL_Surface structure containing the image to be saved
 * file - the filename to save to
 *
 * Returns 0 success or -1 on failure, the error message is then retrievable
 * via SDL_GetError().
 */
extern int SDL_SavePNG(SDL_Surface *surface, const std::string& file);

#endif
