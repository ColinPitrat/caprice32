#include <gtest/gtest.h>
#include "cap32.h"
#include "configuration.h"
#include <stdlib.h>
#include <unistd.h>
#include <fstream>

class ConfigurationTest : public testing::Test
{
  public:
    void SetUp()
    {
      // TODO(cpitrat): remove all this once migrated to config::Config
      std::ofstream configFile(getTmpFilename());
      configFile << "# A comment in top\n"
                 << "[system] # A comment at the end of the line\n"
                 << "model=42\n"
                 << "# This is an unused param:\n"
                 << "unused=1\n"
                 << "# Here is an empty line:\n"
                 << "\n"
                 << "resources_path=./resources\n"
                 << "\n"
                 << "[video]\n"
                 << "resources_path=./toto\n"
                 << "model = 8\n"
                 << "[input]\n"
                 << "resources_path=./directory with spaces\n"
                 << "[sound]\n"
                 << "enabled=1";

      defaultPathValue_ = "./default";
    }

    void TearDown()
    {
      if (!tmpFilename_.empty())
      {
        ASSERT_EQ(0, unlink(tmpFilename_.c_str()));
      }
    }

    const std::string getTmpFilename()
    {
      if (tmpFilename_.empty())
      {
        createTmpFile();
      }
      return tmpFilename_;
    }

  protected:
    std::string tmpFilename_;
    std::string defaultPathValue_;
    config::Config configuration_;

  private:
    void createTmpFile()
    {
      char tmpFilename[] = "/tmp/cap32_test_XXXXXX";
      int fd = mkstemp(tmpFilename);
      ASSERT_GE(fd, 0);
      close(fd);
      tmpFilename_ = tmpFilename;
    }
};

// "Integrated" tests (on a real file)
// TODO(cpitrat): Remove this once fully migrated to config::Config 
// --8<-------
TEST_F(ConfigurationTest, getConfigValueAsInt)
{
  ASSERT_EQ(42, getConfigValueInt(tmpFilename_.c_str(), "system", "model", 0));
};

TEST_F(ConfigurationTest, getConfigValueAsIntMatchesRightSection)
{
  ASSERT_EQ(8, getConfigValueInt(tmpFilename_.c_str(), "video", "model", 0));
};

TEST_F(ConfigurationTest, getConfigValueAsIntReturnsDefault)
{
  ASSERT_EQ(10, getConfigValueInt(tmpFilename_.c_str(), "sound", "model", 10));
};

TEST_F(ConfigurationTest, getConfigValueAsString)
{
  char chPath[256];
  getConfigValueString(tmpFilename_.c_str(), "system", "resources_path", chPath, sizeof(chPath)-1, defaultPathValue_.c_str());
  ASSERT_STREQ("./resources", chPath);
};

TEST_F(ConfigurationTest, getConfigValueAsStringMatchesRightSection)
{
  char chPath[256];
  getConfigValueString(tmpFilename_.c_str(), "video", "resources_path", chPath, sizeof(chPath)-1, defaultPathValue_.c_str());
  ASSERT_STREQ("./toto", chPath);
};

TEST_F(ConfigurationTest, getConfigValueAsStringWithSpaces)
{
  char chPath[256];
  getConfigValueString(tmpFilename_.c_str(), "input", "resources_path", chPath, sizeof(chPath)-1, defaultPathValue_.c_str());
  ASSERT_STREQ("./directory with spaces", chPath);
};

TEST_F(ConfigurationTest, getConfigValueAsStringReturnsDefault)
{
  char chPath[256];
  getConfigValueString(tmpFilename_.c_str(), "sound", "resources_path", chPath, sizeof(chPath)-1, defaultPathValue_.c_str());
  ASSERT_STREQ(defaultPathValue_.c_str(), chPath);
};
// ------>8--

TEST_F(ConfigurationTest, parseFileAndMore)
{
  std::ofstream configFile(getTmpFilename());
  configFile << "# A comment in top\n"
             << "[system] # A comment at the end of the line\n"
             << "model=42\n"
             << "# This is an unused param:\n"
             << "unused=1\n"
             << "# Here is an empty line:\n"
             << "\n"
             << "resources_path=./resources\n"
             << "\n"
             << "[video]\n"
             << "resources_path=./toto\n"
             << "model = 8\n"
             << "[input]\n"
             << "resources_path=./directory with spaces\n"
             << "[sound]\n"
             << "enabled=1";
  configFile.close();

  configuration_.parseFile(tmpFilename_);

  ASSERT_EQ(42, configuration_.getIntValue("system", "model", 0));
  ASSERT_EQ(8, configuration_.getIntValue("video", "model", 0));
  ASSERT_EQ(10, configuration_.getIntValue("sound", "model", 10));
  ASSERT_EQ("./resources", configuration_.getStringValue("system", "resources_path", "none"));
  ASSERT_EQ("./toto", configuration_.getStringValue("video", "resources_path", "none"));
  ASSERT_EQ("./directory with spaces", configuration_.getStringValue("input", "resources_path", "none"));
  ASSERT_EQ("./default", configuration_.getStringValue("sound", "resources_path", "./default"));
};

TEST_F(ConfigurationTest, saveToFileAndMore)
{
  std::string initalConfig = "# A comment in top\n"
                             "[system] # A comment at the end of the line\n"
                             "model=42\n";
  std::string expectedConfig = "[system]\n"
                               "model=42\n";
  configuration_.parseString(initalConfig);

  configuration_.saveToFile(getTmpFilename());

  std::ifstream ifs(tmpFilename_);
  std::stringstream buffer;
  buffer << ifs.rdbuf();
  ASSERT_EQ(expectedConfig, buffer.str());
};

// Real unit tests

TEST_F(ConfigurationTest, hasValueReturnsFalseOnNonExistingSystem)
{
  ASSERT_FALSE(configuration_.hasValue("system", "model"));
}

TEST_F(ConfigurationTest, hasValueReturnsFalseOnNonExistingKey)
{
  std::string config = "[system]\nmodel=42";
  configuration_.parseString(config);

  ASSERT_FALSE(configuration_.hasValue("system", "something"));
}

TEST_F(ConfigurationTest, hasValueReturnsTrueOnExistingKey)
{
  std::string config = "[system]\nmodel=42";
  configuration_.parseString(config);

  ASSERT_TRUE(configuration_.hasValue("system", "model"));
}

TEST_F(ConfigurationTest, getIntOnEmptyConfigReturnsDefault)
{
  ASSERT_EQ(0, configuration_.getIntValue("system", "model", 0));
  ASSERT_EQ(18, configuration_.getIntValue("system", "model", 18));
}

TEST_F(ConfigurationTest, getIntOnParsedStringConfigReturnsParsedValue)
{
  std::string config = "[system]\nmodel=42";
  configuration_.parseString(config);

  ASSERT_EQ(42, configuration_.getIntValue("system", "model", 0));
}

TEST_F(ConfigurationTest, getIntWithValueInMultipleSectionsReturnsTheRightOne)
{
  std::string config = "[system]\nmodel=42\n[video]\nmodel=17";
  configuration_.parseString(config);

  ASSERT_EQ(42, configuration_.getIntValue("system", "model", 0));
  ASSERT_EQ(17, configuration_.getIntValue("video", "model", 0));
}

TEST_F(ConfigurationTest, getStringOnEmptyConfigReturnsDefault)
{
  ASSERT_EQ("/path/by/default", configuration_.getStringValue("system", "path", "/path/by/default"));
  ASSERT_EQ("/other/path/by/default", configuration_.getStringValue("system", "path", "/other/path/by/default"));
}

TEST_F(ConfigurationTest, getStringOnParsedStringConfigReturnsParsedValue)
{
  std::string config = "[system]\npath=/path/defined";
  configuration_.parseString(config);

  ASSERT_EQ("/path/defined", configuration_.getStringValue("system", "path", "/path/by/default"));
}

TEST_F(ConfigurationTest, getStringWithValueInMultipleSectionsReturnsTheRightOne)
{
  std::string config = "[system]\npath=/path/system\n[video]\npath=/path/video";
  configuration_.parseString(config);

  ASSERT_EQ("/path/system", configuration_.getStringValue("system", "path", "/path/by/default"));
  ASSERT_EQ("/path/video", configuration_.getStringValue("video", "path", "/path/by/default"));
}

TEST_F(ConfigurationTest, setStringValueModifiesValue)
{
  std::string config = "[system]\npath=/path/original";
  configuration_.parseString(config);
  ASSERT_EQ("/path/original", configuration_.getStringValue("system", "path", "/path/default"));

  configuration_.setStringValue("system", "path", "/path/modified");
  ASSERT_EQ("/path/modified", configuration_.getStringValue("system", "path", "/path/default"));
}

TEST_F(ConfigurationTest, setIntValueModifiesValue)
{
  std::string config = "[system]\nmodel=17";
  configuration_.parseString(config);
  ASSERT_EQ(17, configuration_.getIntValue("system", "model", 0));

  configuration_.setIntValue("system", "model", 42);
  ASSERT_EQ(42, configuration_.getIntValue("system", "model", 0));
}

TEST_F(ConfigurationTest, toStreamDumpsConfig)
{
  std::string config = "[system]\nmodel=1\n";
  configuration_.parseString(config);

  std::ostringstream oss;
  configuration_.toStream(oss);

  ASSERT_EQ(config, oss.str());
}
