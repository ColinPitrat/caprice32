#ifndef ZIP_H
#define ZIP_H

#include <string>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

namespace zip
{
  typedef struct {
    std::string filename;
    std::string extensions;
    char *pchFileNames;
    char *pchSelection;
    int iFiles;
    unsigned int dwOffset;
  } t_zip_info;

  int dir (t_zip_info *zi);
  int extract (const t_zip_info& zi, FILE **pfileOut);
}

#endif
