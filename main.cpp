#include "cap32.h"
#include <SDL_main.h>

#ifdef __WIN32__
#error __WIN32__ is defined
#endif

extern "C"
int main(int argc, char **argv)
{
  return cap32_main(argc, argv);
}
