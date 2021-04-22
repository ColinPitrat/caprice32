#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <map>
#include <string>
#include <vector>

class CapriceArgs
{
   public:
      CapriceArgs();
      std::string autocmd;
      std::string cfgFilePath;
      std::string binFile;
      size_t binOffset;
      std::map<std::string, std::map<std::string, std::string>> cfgOverrides;
};

std::string replaceCap32Keys(std::string command);
void parseArguments(int argc, char** argv, std::vector<std::string>& slot_list, CapriceArgs& args);

#endif
