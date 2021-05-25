#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <vector>
#include <string>

namespace stringutils
{
  std::vector<std::string> split(const std::string& s, char delim, bool ignore_empty=false);
  std::string trim(const std::string& s, char c);
  std::string lower(const std::string& s);
  std::string upper(const std::string& s);
  std::string replace(std::string s, const std::string& search, const std::string& replace);
  void splitPath(const std::string& path, std::string& dirname, std::string& filename);
  bool caseInsensitiveCompare(const std::string& str1, const std::string& str2);
}

#endif
