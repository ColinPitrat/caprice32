#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <map>
#include <string>

namespace config
{
  using ConfigSection = std::map<std::string, std::string>;
  using ConfigMap = std::map<std::string, ConfigSection>;

  bool hasValue(const ConfigMap& configMap, const std::string& section, const std::string& key);

  class Config
  {
    public:
      std::istream& parseStream(std::istream& configStream);
      void parseString(const std::string& configString);
      void parseFile(const std::string& configFilename);

      std::ostream& toStream(std::ostream& out) const;
      bool saveToFile(const std::string& configFilename) const;

      void setOverrides(const ConfigMap& overrides);

      int getIntValue(const std::string& section, const std::string& key, const int defaultValue) const;
      void setIntValue(const std::string& section, const std::string& key, const int value);

      std::string getStringValue(const std::string& section, const std::string& key, const std::string& defaultValue) const;
      void setStringValue(const std::string& section, const std::string& key, const std::string& value);

      // Do not use this for anything else than testing.
      ConfigMap getConfigMapForTests() const;

    private:
      ConfigMap config_;
      ConfigMap overrides_;
  };
}

#endif
