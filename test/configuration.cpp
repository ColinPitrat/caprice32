#include <gtest/gtest.h>
#include "cap32.h"
#include <stdlib.h>
#include <unistd.h>

// TODO(cpitrat): Find a clean way to create a unique /tmp file
// Maybe gtest provides something ?
class ConfigurationTest : public testing::Test
{
  public:
    void SetUp()
    {
      char tmpFileName[] = "/tmp/cap32_test_XXXXXX";
      int fd = mkstemp(tmpFileName);
      ASSERT_GE(fd, 0);
      std::string fileContent = "# A comment in top\n"
                                "[system] # A comment at the end of the line\n"
                                "model=42\n"
                                "# This is an unused param:\n"
                                "unused=1\n"
                                "# Here is an empty line:\n"
                                "\n"
                                "resources_path=./resources\n"
                                "\n"
                                "[video]\n"
                                "resources_path=./toto\n"
                                "model = 8\n"
                                "[sound]\n"
                                "enabled=1";
      dprintf(fd, fileContent.c_str());
      close(fd);
      configFilePath = tmpFileName;
    };

    void TearDown()
    {
      ASSERT_EQ(0, unlink(configFilePath.c_str()));
    }

  protected:
    std::string configFilePath;
};

TEST_F(ConfigurationTest, getConfigValueAsInt)
{
  ASSERT_EQ(42, getConfigValueInt(configFilePath.c_str(), "system", "model", 0));
};

TEST_F(ConfigurationTest, getConfigValueAsIntMatchesRightSection)
{
  ASSERT_EQ(8, getConfigValueInt(configFilePath.c_str(), "video", "model", 0));
};

TEST_F(ConfigurationTest, getConfigValueAsIntReturnsDefault)
{
  ASSERT_EQ(10, getConfigValueInt(configFilePath.c_str(), "sound", "model", 10));
};

TEST_F(ConfigurationTest, getConfigValueAsString)
{
  char chPath[256];
  getConfigValueString(configFilePath.c_str(), "system", "resources_path", chPath, sizeof(chPath)-1, "./default");
  ASSERT_STREQ("./resources", chPath);
};

TEST_F(ConfigurationTest, getConfigValueAsStringMatchesRightSection)
{
  char chPath[256];
  getConfigValueString(configFilePath.c_str(), "video", "resources_path", chPath, sizeof(chPath)-1, "./default");
  ASSERT_STREQ("./toto", chPath);
};

TEST_F(ConfigurationTest, getConfigValueAsStringReturnsDefault)
{
  char chPath[256];
  getConfigValueString(configFilePath.c_str(), "sound", "resources_path", chPath, sizeof(chPath)-1, "./default");
  ASSERT_STREQ("./default", chPath);
};

