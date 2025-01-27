#include "stringutils.h"

#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
#include <strings.h>

namespace stringutils
{
  std::vector<std::string> split(const std::string& s, char delim, bool ignore_empty)
  {
      std::vector<std::string> elems;
      std::stringstream ss(s);
      std::string item;
      while (std::getline(ss, item, delim)) 
      {
          if (ignore_empty && item.empty()) continue;
          elems.push_back(item);
      }
      return elems;
  }

  std::string join(const std::vector<std::string>& v, const std::string& delim)
  {
    std::string result;
    for(auto it = v.begin(); it != v.end(); ++it)
    {
      result += *it;
      if (it != v.end() - 1) result += delim;
    }
    return result;
  }

  std::string trim(const std::string& s, char c)
  {
    auto b = s.begin();
    auto e = s.end();
    e--;
    while(*b == c) b++;
    while(*e == c) e--;
    if(e++ >= b) return std::string(b, e);
    return "";
  }

  std::string lower(const std::string& s)
  {
    std::string result(s);
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
  }

  std::string upper(const std::string& s)
  {
    std::string result(s);
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
  }

  std::string replace(std::string s, const std::string& search, const std::string& replace)
  {
    auto start_pos = s.find(search);
    if (start_pos == std::string::npos) return s;
    return s.replace(start_pos, search.size(), replace);
  }

  void splitPath(const std::string& path, std::string& dirname, std::string& filename)
  {
    auto delimiter = path.rfind('/');
    if(delimiter == std::string::npos) {
      delimiter = path.rfind('\\');
    }
    if(delimiter != std::string::npos) {
      delimiter++;
      dirname = path.substr(0, delimiter);
      filename = path.substr(delimiter);
    } else {
      dirname = "./";
      filename = path;
    }
  }

  bool caseInsensitiveCompare(const std::string& str1, const std::string& str2)
  {
    return strncasecmp(str1.c_str(), str2.c_str(), std::max(str1.size(), str2.size())) < 0;
  }
}
