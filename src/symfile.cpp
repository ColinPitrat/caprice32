#include "symfile.h"

#include <fstream>
#include <iomanip>
#include <string>
#include "stringutils.h"
#include "log.h"

Symfile::Symfile(const std::string& filename)
{
  std::ifstream infile(filename);
  std::string line;
  while (std::getline(infile, line))
  {
    // Remove any comment
    line = stringutils::trim(line.substr(0, line.find(';')), ' ');
    auto elems = stringutils::split(line, ' ', /*ignore_empty=*/true);
    if (elems.empty()) continue;
    if (elems[0] == "al") {
      if (elems.size() < 3 || elems[1][0] != '$' || elems[2][0] != '.') {
        LOG_ERROR("Invalid `al` entry in " << filename << ": " << line);
        continue;
      }
      word addr = std::stol(elems[1].substr(1), nullptr, 16);
      addSymbol(addr, elems[2].substr(1));
    }
    if (elems[0] == "b" or elems[0] == "break") {
      if (elems.size() < 2 || elems[1][0] != '$') {
        LOG_ERROR("Invalid `al` entry in " << filename << ": " << line);
        continue;
      }
      word addr = std::stol(elems[1].substr(1), nullptr, 16);
      addBreakpoint(addr);
    }
    if (elems[0] == "d") {
      if (elems.size() < 2 || elems[1][0] != '$') {
        LOG_ERROR("Invalid `al` entry in " << filename << ": " << line);
        continue;
      }
      word addr = std::stol(elems[1].substr(1), nullptr, 16);
      addEntrypoint(addr);
    }
  }
}

bool Symfile::SaveTo(const std::string& filename)
{
  std::ofstream outfile;
  outfile.open(filename);
  outfile << "; labels" << std::endl;
  for (const auto& [addr, sym] : symbols)
  {
    outfile << "al  $" << std::hex << std::setw(4) << std::setfill('0') << addr << " ." << sym << std::endl;
  }
  outfile << "; breakpoints" << std::endl;
  for (auto addr : breakpoints)
  {
    outfile << "b  $" << std::hex << std::setw(4) << std::setfill('0') << addr << std::endl;
  }
  outfile << "; entrypoints" << std::endl;
  for (auto addr : entrypoints)
  {
    outfile << "d  $" << std::hex << std::setw(4) << std::setfill('0') << addr << std::endl;
  }
  outfile.close();
  return true;
}

void Symfile::addBreakpoint(word addr)
{
  breakpoints.push_back(addr);
}

void Symfile::addEntrypoint(word addr)
{
  entrypoints.push_back(addr);
}

void Symfile::addSymbol(word addr, const std::string& symbol)
{
  symbols[addr] = symbol;
}
