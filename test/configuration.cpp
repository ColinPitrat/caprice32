#include <gtest/gtest.h>
#include "cap32.h"
#include "slotshandler.h"
#include "configuration.h"
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <string.h>
#include <errno.h>

// TODO(cpitrat): Cleaner way to handle this
extern char chAppPath[_MAX_PATH + 1];
// TODO(cpitrat): Make this a list or vector in t_CPC
extern t_disk_format disk_format[8];

class ConfigurationTest : public testing::Test
{
  public:
    void SetUp() {}

    void TearDown()
    {
      for(auto f : tmpFilenames_)
      {
        ASSERT_EQ(0, unlink(f.c_str()));
      }
    }

    const std::string getTmpFilename(unsigned int idx)
    {
      while (tmpFilenames_.size() <= idx)
      {
        std::string fn;
        createTmpFile(fn);
        tmpFilenames_.push_back(fn);
      }
      return tmpFilenames_[idx];
    }

  protected:
    std::vector<std::string> tmpFilenames_;
    config::Config configuration_;

  private:
    void createTmpFile(std::string &filename)
    {
      char tmpFilename[] = "test/.cap32_tmp_XXXXXX";
      int fd = mkstemp(tmpFilename);
      ASSERT_GE(fd, 0);
      close(fd);
      filename = tmpFilename;
    }
};

// "Integrated" tests (on a real file)
TEST_F(ConfigurationTest, parseFileAndSaveBack)
{
  std::ofstream configFile(getTmpFilename(0));
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

  configuration_.parseFile(getTmpFilename(0));

  ASSERT_EQ(42, configuration_.getIntValue("system", "model", 0));
  ASSERT_EQ(8, configuration_.getIntValue("video", "model", 0));
  ASSERT_EQ(10, configuration_.getIntValue("sound", "model", 10));
  ASSERT_EQ("./resources", configuration_.getStringValue("system", "resources_path", "none"));
  ASSERT_EQ("./toto", configuration_.getStringValue("video", "resources_path", "none"));
  ASSERT_EQ("./directory with spaces", configuration_.getStringValue("input", "resources_path", "none"));
  ASSERT_EQ("./default", configuration_.getStringValue("sound", "resources_path", "./default"));

  configuration_.saveToFile(getTmpFilename(1));
  config::Config otherConfig;
  otherConfig.parseFile(getTmpFilename(1));

  ASSERT_EQ(42, otherConfig.getIntValue("system", "model", 0));
  ASSERT_EQ(8, otherConfig.getIntValue("video", "model", 0));
  ASSERT_EQ(10, otherConfig.getIntValue("sound", "model", 10));
  ASSERT_EQ("./resources", otherConfig.getStringValue("system", "resources_path", "none"));
  ASSERT_EQ("./toto", otherConfig.getStringValue("video", "resources_path", "none"));
  ASSERT_EQ("./directory with spaces", otherConfig.getStringValue("input", "resources_path", "none"));
  ASSERT_EQ("./default", otherConfig.getStringValue("sound", "resources_path", "./default"));
}

TEST_F(ConfigurationTest, parseFileDoesntExist)
{
  configuration_.parseFile("/a/non/existing/file");
}

TEST_F(ConfigurationTest, saveToFileAndMore)
{
  std::string initalConfig = "# A comment in top\n"
                             "[system] # A comment at the end of the line\n"
                             "model=42\n";
  std::string expectedConfig = "[system]\n"
                               "model=42\n";
  configuration_.parseString(initalConfig);

  configuration_.saveToFile(getTmpFilename(0));

  std::ifstream ifs(getTmpFilename(0));
  std::stringstream buffer;
  buffer << ifs.rdbuf();
  ASSERT_EQ(expectedConfig, buffer.str());
}

TEST_F(ConfigurationTest, saveToNonWritableFile)
{
  // Make a non-writable config file
  auto configFileName = getTmpFilename(0);
  ASSERT_EQ(0, chmod(configFileName.c_str(), S_IRUSR|S_IRGRP|S_IROTH));
  std::string initalConfig = "[system]\nmodel=42\n";
  configuration_.parseString(initalConfig);

  EXPECT_FALSE(configuration_.saveToFile(getTmpFilename(0)));

  // This works when the file is writable
  ASSERT_EQ(0, chmod(configFileName.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH));
  EXPECT_TRUE(configuration_.saveToFile(getTmpFilename(0)));
}

// TODO(cpitrat): test about every value in conf ?
TEST_F(ConfigurationTest, loadConfigurationWithValidContent)
{
  std::ofstream configFile(getTmpFilename(0));
  configFile << "[system]\n"
             << "model=1\n"
             << "jumpers=30\n"
             << "ram_size=128\n"
             << "speed=32\n"
             << "printer=1\n"
             << "resources_path=./resources\n"
             << "[file]\n"
             << "fmt02=Test disk format,40,1,9,2,82,229,193,198,194,199,195,200,196,201,197\n"
             ;
  configFile.close();

  t_CPC CPC[2];
  loadConfiguration(CPC[0], getTmpFilename(0));
  saveConfiguration(CPC[0], getTmpFilename(1));
  loadConfiguration(CPC[1], getTmpFilename(1));

  for(int i = 0; i < 2; ++i)
  {
    ASSERT_EQ(1, CPC[i].model) << "with i = " << i;
    ASSERT_EQ(30, CPC[i].jumpers) << "with i = " << i;
    ASSERT_EQ(128, CPC[i].ram_size) << "with i = " << i;
    ASSERT_EQ(32, CPC[i].speed) << "with i = " << i;
    ASSERT_EQ(1, CPC[i].limit_speed) << "with i = " << i;
    ASSERT_EQ(1, CPC[i].printer) << "with i = " << i;
    ASSERT_EQ("./resources", std::string(CPC[i].resources_path)) << "with i = " << i;
    // TODO(cpitrat): move disk_format array in t_CPC
    ASSERT_EQ("Test disk format", disk_format[2].label);
    ASSERT_EQ(40, disk_format[0].tracks);
    ASSERT_EQ(1, disk_format[0].sides);
    ASSERT_EQ(9, disk_format[0].sectors);
    ASSERT_EQ(2, disk_format[0].sector_size);
    ASSERT_EQ(82, disk_format[0].gap3_length);
    ASSERT_EQ(229, disk_format[0].filler_byte);
    ASSERT_EQ(193, disk_format[0].sector_ids[0][0]);
    ASSERT_EQ(198, disk_format[0].sector_ids[0][1]);
    ASSERT_EQ(194, disk_format[0].sector_ids[0][2]);
    ASSERT_EQ(199, disk_format[0].sector_ids[0][3]);
    ASSERT_EQ(195, disk_format[0].sector_ids[0][4]);
    ASSERT_EQ(200, disk_format[0].sector_ids[0][5]);
    ASSERT_EQ(196, disk_format[0].sector_ids[0][6]);
    ASSERT_EQ(201, disk_format[0].sector_ids[0][7]);
    ASSERT_EQ(197, disk_format[0].sector_ids[0][8]);
  }
}

// TODO(cpitrat): test about every value in conf ?
TEST_F(ConfigurationTest, loadConfigurationWithInvalidValues)
{
  std::ofstream configFile(getTmpFilename(0));
  configFile << "[system]\n"
             << "model=4\n" // model should be <= 3 - default to 2
             << "jumpers=255\n" // jumpers is & with 0x1e == 30
             << "ram_size=704\n" // max ram size is 576 - moreover it's & with 704
             << "speed=64\n" // max speed is 32 - will default to 4
             << "printer=2\n" // printer should be 0 or 1 - it's & with 1
             << "resources_path=\n"
             ;
  configFile.close();
  ASSERT_NE(nullptr, getcwd(chAppPath, sizeof(chAppPath)-1)) << "getcwd error: " << strerror(errno);

  t_CPC CPC;
  loadConfiguration(CPC, getTmpFilename(0));

  ASSERT_EQ(2, CPC.model);
  ASSERT_EQ(30, CPC.jumpers);
  ASSERT_EQ(576, CPC.ram_size);
  ASSERT_EQ(4, CPC.speed);
  ASSERT_EQ(1, CPC.limit_speed);
  ASSERT_EQ(0, CPC.printer);
  ASSERT_EQ(std::string(chAppPath) + "/resources", std::string(CPC.resources_path));
}

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

TEST_F(ConfigurationTest, parseDiskFormatValid2sides4sectors)
{
  std::string diskFormat("DSK format label,42,2,4,1,82,229,1,2,3,4,5,6,7,8");

  auto df = parseDiskFormat(diskFormat);
  auto dfString = serializeDiskFormat(df);

  ASSERT_EQ("DSK format label", df.label);
  ASSERT_EQ(42, df.tracks);
  ASSERT_EQ(2, df.sides);
  ASSERT_EQ(4, df.sectors);
  ASSERT_EQ(1, df.sector_size);
  ASSERT_EQ(82, df.gap3_length);
  ASSERT_EQ(229, df.filler_byte);
  ASSERT_EQ(1, df.sector_ids[0][0]);
  ASSERT_EQ(2, df.sector_ids[0][1]);
  ASSERT_EQ(3, df.sector_ids[0][2]);
  ASSERT_EQ(4, df.sector_ids[0][3]);
  ASSERT_EQ(5, df.sector_ids[1][0]);
  ASSERT_EQ(6, df.sector_ids[1][1]);
  ASSERT_EQ(7, df.sector_ids[1][2]);
  ASSERT_EQ(8, df.sector_ids[1][3]);
  ASSERT_EQ(diskFormat, dfString);
}

TEST_F(ConfigurationTest, parseDiskFormatValid1side5sectors)
{
  std::string diskFormat("DSK format label,17,1,5,2,31,119,1,2,3,4,5");

  auto df = parseDiskFormat(diskFormat);
  auto dfString = serializeDiskFormat(df);

  ASSERT_EQ("DSK format label", df.label);
  ASSERT_EQ(17, df.tracks);
  ASSERT_EQ(1, df.sides);
  ASSERT_EQ(5, df.sectors);
  ASSERT_EQ(2, df.sector_size);
  ASSERT_EQ(31, df.gap3_length);
  ASSERT_EQ(119, df.filler_byte);
  ASSERT_EQ(1, df.sector_ids[0][0]);
  ASSERT_EQ(2, df.sector_ids[0][1]);
  ASSERT_EQ(3, df.sector_ids[0][2]);
  ASSERT_EQ(4, df.sector_ids[0][3]);
  ASSERT_EQ(5, df.sector_ids[0][4]);
  ASSERT_EQ(diskFormat, dfString);
}

// TODO(cpitrat): Clarify why it behaves like this.
// This test validates existing behaviour. The reason why missing sector is accepted
// and why its default value is the sector's index + 1 is a mystery to me.
TEST_F(ConfigurationTest, parseDiskFormatValidMissingSectors)
{
  std::string diskFormat("DSK format label,17,2,4,2,31,119,9,10");

  auto df = parseDiskFormat(diskFormat);
  auto dfString = serializeDiskFormat(df);

  ASSERT_EQ("DSK format label", df.label);
  ASSERT_EQ(17, df.tracks);
  ASSERT_EQ(2, df.sides);
  ASSERT_EQ(4, df.sectors);
  ASSERT_EQ(2, df.sector_size);
  ASSERT_EQ(31, df.gap3_length);
  ASSERT_EQ(119, df.filler_byte);
  ASSERT_EQ(9, df.sector_ids[0][0]);
  ASSERT_EQ(10, df.sector_ids[0][1]);
  ASSERT_EQ(3, df.sector_ids[0][2]);
  ASSERT_EQ(4, df.sector_ids[0][3]);
  ASSERT_EQ(1, df.sector_ids[1][0]);
  ASSERT_EQ(2, df.sector_ids[1][1]);
  ASSERT_EQ(3, df.sector_ids[1][2]);
  ASSERT_EQ(4, df.sector_ids[1][3]);
  ASSERT_EQ(diskFormat + ",3,4,1,2,3,4", dfString); // Description is completed with missing sectors
}
