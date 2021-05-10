#ifndef ZIP_H
#define ZIP_H

#include <string>
#include <vector>
#include "types.h"

namespace zip
{
  typedef struct {
    std::string filename;
    std::string extensions;
    std::vector<std::pair<std::string, dword>> filesOffsets;
    unsigned int dwOffset;
  } t_zip_info;

  int dir (t_zip_info *zi);
  int extract (const t_zip_info& zi, FILE **pfileOut);
}

#endif
