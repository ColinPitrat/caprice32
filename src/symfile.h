#ifndef _SYMFILE_H
#define _SYMFILE_H

#include <map>
#include <string>
#include <vector>
#include "types.h"

class Symfile
{
  public:
    Symfile() = default;
    explicit Symfile(const std::string& filename);

    bool SaveTo(const std::string& filename);

    void addBreakpoint(word addr);
    void addEntrypoint(word addr);
    void addSymbol(word addr, const std::string& symbol);

    std::map<word, std::string> Symbols() { return symbols; };
    std::vector<word> Breakpoints() { return breakpoints; };
    std::vector<word> Entrypoints() { return entrypoints; };

  private:
    std::vector<word> breakpoints;
    std::vector<word> entrypoints;
    std::map<word, std::string> symbols;
};

#endif
