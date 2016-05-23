#include <gtest/gtest.h>
#include "stringutils.h"

TEST(StringUtils, SplitOnEmptyStringReturnsEmptyVector)
{
  std::string toSplit = "";

  std::vector<std::string> result = stringutils::split(toSplit, ',');

  ASSERT_EQ(0, result.size());
}

TEST(StringUtils, SplitOnStringWithoutDelimiterReturnsOneElement)
{
  std::string toSplit = "a string;without comma.";

  std::vector<std::string> result = stringutils::split(toSplit, ',');

  ASSERT_EQ(1, result.size());
  ASSERT_EQ(toSplit, result[0]);
}

TEST(StringUtils, SplitOnComma)
{
  std::string toSplit = "value1,value2,value3";

  std::vector<std::string> result = stringutils::split(toSplit, ',');

  ASSERT_EQ(3, result.size());
  ASSERT_EQ("value1", result[0]);
  ASSERT_EQ("value2", result[1]);
  ASSERT_EQ("value3", result[2]);
}

TEST(StringUtils, SplitOnColon)
{
  std::string toSplit = "v1:v2:v3:v4:v5";

  std::vector<std::string> result = stringutils::split(toSplit, ':');

  ASSERT_EQ(5, result.size());
  ASSERT_EQ("v1", result[0]);
  ASSERT_EQ("v2", result[1]);
  ASSERT_EQ("v3", result[2]);
  ASSERT_EQ("v4", result[3]);
  ASSERT_EQ("v5", result[4]);
}
