#include <gtest/gtest.h>
#include "stringutils.h"
#include <string>

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

TEST(StringUtils, TrimCharNotPresentInString)
{
  std::string toTrim = "a test string";

  auto result = stringutils::trim(toTrim, '"');

  ASSERT_EQ(toTrim, result);
}

TEST(StringUtils, TrimCharPresentInMiddleOfStringOnly)
{
  std::string toTrim = "a test string";

  auto result = stringutils::trim(toTrim, ' ');

  ASSERT_EQ(toTrim, result);
}

TEST(StringUtils, TrimCharAtBeginningOfString)
{
  std::string toTrim = "--a test string";

  auto result = stringutils::trim(toTrim, '-');

  ASSERT_EQ("a test string", result);
}

TEST(StringUtils, TrimCharAtEndOfString)
{
  std::string toTrim = "a test string--";

  auto result = stringutils::trim(toTrim, '-');

  ASSERT_EQ("a test string", result);
}

TEST(StringUtils, TrimCharOccuringMultipleTimesInString)
{
  std::string toTrim = "\"a \"test\" string\"";

  auto result = stringutils::trim(toTrim, '"');

  ASSERT_EQ("a \"test\" string", result);
}

TEST(StringUtils, TrimStringWithOnlyOneCharAndTwoDelimitors)
{
  std::string toTrim = "\"a\"";

  auto result = stringutils::trim(toTrim, '"');

  ASSERT_EQ("a", result);
}

TEST(StringUtils, TrimStringWithOnlyTwoDelimitors)
{
  std::string toTrim = "\"\"";

  auto result = stringutils::trim(toTrim, '"');

  ASSERT_EQ("", result);
}

TEST(StringUtils, TrimStringWithOnlyOneDelimitor)
{
  std::string toTrim = "\"";

  auto result = stringutils::trim(toTrim, '"');

  ASSERT_EQ("", result);
}

TEST(StringUtils, LowerOnEmptyString)
{
  std::string toLower = "";

  auto result = stringutils::lower(toLower);

  ASSERT_EQ("", result);
}

TEST(StringUtils, LowerOnUpperCaseString)
{
  std::string toLower = "ABCDEF";

  auto result = stringutils::lower(toLower);

  ASSERT_EQ("abcdef", result);
}

TEST(StringUtils, LowerOnLowerCaseString)
{
  std::string toLower = "ghijkl";

  auto result = stringutils::lower(toLower);

  ASSERT_EQ("ghijkl", result);
}

TEST(StringUtils, UpperOnEmptyString)
{
  std::string toUpper = "";

  auto result = stringutils::upper(toUpper);

  ASSERT_EQ("", result);
}

TEST(StringUtils, UpperOnUpperCaseString)
{
  std::string toUpper = "ABCDEF";

  auto result = stringutils::upper(toUpper);

  ASSERT_EQ("ABCDEF", result);
}

TEST(StringUtils, UpperOnLowerCaseString)
{
  std::string toUpper = "ghijkl";

  auto result = stringutils::upper(toUpper);

  ASSERT_EQ("GHIJKL", result);
}

TEST(StringUtils, SplitPathOnUnix)
{
  std::string path = "/usr/bin/cap32";
  std::string dirname, filename;

  stringutils::splitPath(path, dirname, filename);

  ASSERT_EQ("/usr/bin/", dirname);
  ASSERT_EQ("cap32", filename);
}

TEST(StringUtils, SplitPathOnUnixWithExtension)
{
  std::string path = "/home/user/caprice32/disk/test.dsk";
  std::string dirname, filename;

  stringutils::splitPath(path, dirname, filename);

  ASSERT_EQ("/home/user/caprice32/disk/", dirname);
  ASSERT_EQ("test.dsk", filename);
}

TEST(StringUtils, SplitPathOnUnixWithSpaces)
{
  std::string path = "/home/user name/caprice32/disk/test disk.dsk";
  std::string dirname, filename;

  stringutils::splitPath(path, dirname, filename);

  ASSERT_EQ("/home/user name/caprice32/disk/", dirname);
  ASSERT_EQ("test disk.dsk", filename);
}

TEST(StringUtils, SplitPathOnUnixWithBackslash)
{
  std::string path = "/home/user\\ name/caprice32/disk/test\\ disk.dsk";
  std::string dirname, filename;

  stringutils::splitPath(path, dirname, filename);

  ASSERT_EQ("/home/user\\ name/caprice32/disk/", dirname);
  ASSERT_EQ("test\\ disk.dsk", filename);
}

TEST(StringUtils, SplitPathWithoutDelimiter)
{
  std::string path = "filename";
  std::string dirname, filename;

  stringutils::splitPath(path, dirname, filename);

  ASSERT_EQ("./", dirname);
  ASSERT_EQ("filename", filename);
}

TEST(StringUtils, SplitPathOnWindows)
{
  std::string path = "C:\\Users\\username\\caprice32\\disk\\test.dsk";
  std::string dirname, filename;

  stringutils::splitPath(path, dirname, filename);

  ASSERT_EQ("C:\\Users\\username\\caprice32\\disk\\", dirname);
  ASSERT_EQ("test.dsk", filename);
}

// This basically doesn't work but it's how it used to be implemented
// It should be OK as long as people don't mix slash and backslashes
// TODO(cpitrat): Could be nice (and easy) to fix though.
TEST(StringUtils, SplitInvalidPathOnWindowsWithSlash)
{
  std::string path = "C:\\Users\\username\\caprice32/disk\\test.dsk";
  std::string dirname, filename;

  stringutils::splitPath(path, dirname, filename);

  ASSERT_EQ("C:\\Users\\username\\caprice32/", dirname);
  ASSERT_EQ("disk\\test.dsk", filename);
}

