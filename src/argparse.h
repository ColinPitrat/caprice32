#ifndef ARGPARSE_H
#define ARGPARSE_H

struct capriceArgs {
   std::string cfgFilePath;
};

void parseArguments(int, char**, std::vector<std::string>&, struct capriceArgs&);

#endif
