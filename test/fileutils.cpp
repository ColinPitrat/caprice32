#include <gtest/gtest.h>
#include "fileutils.h"
#include <string>

TEST(FileUtils, Listdirectory)
{
  std::string directory = "test/zip";
  
  std::vector<std::string> result = listDirectory(directory);

  ASSERT_EQ(1, result.size());
}

TEST(FileUtils, ListdirectoryMatchingExtension)
{
  std::string directory = "test/zip";
  
  std::vector<std::string> result = listDirectoryExt(directory, "zip");

  ASSERT_EQ(1, result.size());
}

TEST(FileUtils, ListdirectoryNonMatchingExtension)
{
  std::string directory = "test/zip";
  
  std::vector<std::string> result = listDirectoryExt(directory, "zup");

  ASSERT_EQ(0, result.size());
}
