#include "cap32.h"
// Needed on mingw32 to resolve SDL_main.
#include <SDL.h>

int main(int argc, char **argv)
{
  return cap32_main(argc, argv);
}
