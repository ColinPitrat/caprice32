#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>
#include <vector>

class CapriceArgs
{
   public:
      CapriceArgs(void);
      std::string autocmd;
      std::string cfgFilePath;
};

std::string replaceCap32Keys(std::string command);
void parseArguments(int, char**, std::vector<std::string>&, CapriceArgs&);

#endif
