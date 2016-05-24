#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <map>

namespace config
{
  typedef std::map<std::string, std::string> ConfigSection;
  typedef std::map<std::string, ConfigSection> ConfigMap;

  class Config
  {
    public:
      std::istream& parseStream(std::istream& configStream);
      void parseString(const std::string& configString);
      void parseFile(const std::string& configFilename);

      std::ostream& toStream(std::ostream& out) const;
      void saveToFile(const std::string& configFilename) const;

      bool hasValue(std::string section, std::string key) const;
      int getIntValue(const std::string& section, const std::string& key, const int defaultValue) const;
      void setIntValue(const std::string& section, const std::string& key, const int value);

      std::string getStringValue(const std::string& section, const std::string& key, const std::string& defaultValue) const;
      void setStringValue(const std::string& section, const std::string& key, const std::string& value);

    private:
      ConfigMap config_;
  };
}

#endif
