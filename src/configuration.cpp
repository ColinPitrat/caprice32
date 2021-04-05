#include "configuration.h"
#include "log.h"
#include <sstream>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

namespace config
{
  std::istream& Config::parseStream(std::istream& configStream)
  {
    std::streamsize maxSize = 256;
    char chLine[maxSize];
    std::string section;
    while(configStream.good())
    {
      configStream.getline(chLine, maxSize);
      if(chLine[0] == '[') // check if there's a section key
      {
        char *pchToken = strtok(chLine, "[]");
        if(pchToken != nullptr) {
          section = pchToken;
        }
      }
      else
      {
        char *pchToken = strtok(chLine, "\t =\n\r"); // check if it has a key=value pair
        if((pchToken != nullptr) && (pchToken[0] != '#')) {
          std::string key = pchToken;
          char* pchPtr = strtok(nullptr, "\t=#\n\r"); // get the value if it matches our key
          if (pchPtr != nullptr) {
            std::string value = pchPtr;
            config_[section][key] = value;
          }
        }
      }
    }
    return configStream;
  };

  void Config::parseString(const std::string& configString)
  {
    std::istringstream configStream(configString);
    parseStream(configStream);
  };

  void Config::parseFile(const std::string& configFilename)
  {
    std::ifstream configStream(configFilename);
    parseStream(configStream);
    configStream.close();
  };

  std::ostream& Config::toStream(std::ostream& out) const
  {
    for(auto section : config_)
    {
      out << "[" << section.first << "]" << std::endl;
      for(auto keyval : section.second)
      {
        out << keyval.first << "=" << keyval.second << std::endl;
      }
    }
    return out;
  }

  bool Config::saveToFile(const std::string& configFilename) const
  {
    std::ofstream configStream(configFilename);
    toStream(configStream);
    configStream.close();
    bool success = configStream.good();
    if (!success) {
      LOG_ERROR("Couldn't save configuration to '" << configFilename << "'. Is the file writable?");
    }
    return success;
  }

  bool Config::hasValue(std::string section, std::string key) const
  {
    if(config_.find(section) != config_.end())
    {
      auto configSection = config_.at(section);
      if(configSection.find(key) != configSection.end())
      {
        return true;
      }
    }
    return false;
  }

  int Config::getIntValue(const std::string& section, const std::string& key, const int defaultValue) const
  {
    if(hasValue(section, key))
    {
      return atoi(config_.at(section).at(key).c_str());
    }
    return defaultValue;
  }

  std::string Config::getStringValue(const std::string& section, const std::string& key, const std::string& defaultValue) const
  {
    if(hasValue(section, key))
    {
      return std::string(config_.at(section).at(key));
    }
    return defaultValue;
  }

  void Config::setStringValue(const std::string& section, const std::string& key, const std::string& value)
  {
    config_[section][key] = value;
  }

  void Config::setIntValue(const std::string& section, const std::string& key, const int value)
  {
    std::ostringstream oss;
    oss << value;
    config_[section][key] = oss.str();
  }
}
