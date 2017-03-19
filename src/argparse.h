#ifndef ARGPARSE_H
#define ARGPARSE_H

class CapriceArgs
{
   public:
      CapriceArgs(void);
      std::string autocmd;
      std::string cfgFilePath;
};

void parseArguments(int, char**, std::vector<std::string>&, CapriceArgs&);

#endif
