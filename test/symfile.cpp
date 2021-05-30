#include <gtest/gtest.h>
#include "symfile.h"

#include <fstream>
#include <iostream>
#include <map>
#include <string>

class SymfileTest : public testing::Test
{
  public:
    void TearDown()
    {
      for(auto f : tmpFilenames_)
      {
        ASSERT_EQ(0, unlink(f.c_str()));
      }
    }
  protected:
    std::string createTmpFile()
    {
      char tmpFilename[] = "test/.cap32_tmp_XXXXXX";
      int fd = mkstemp(tmpFilename);
      if (fd <= 0) return "";
      close(fd);
      tmpFilenames_.push_back(tmpFilename);
      return tmpFilename;
    }

    std::string readWholeFile(const std::string& filename)
    {
      std::ifstream is(filename);
      if (is.bad()) return "**bad stream after open**";
      if (is.fail()) return "**fail stream after open**";
      is.seekg(0, is.end);
      int length = is.tellg();
      is.seekg(0, is.beg);
      char * buffer = new char [length+1];
      is.read(buffer, length);
      if (is.bad()) return "**bad stream after read**";
      // For some reason, the stream from the tmp file is failed after read on Windows
      // It seems that length has an invalid value.
      if (is.fail()) length = is.gcount();
      buffer[length] = 0;
      is.close();
      return std::string(buffer, length);
    }

    std::vector<std::string> tmpFilenames_;
};

TEST_F(SymfileTest, parseSymfile)
{
  Symfile symfile("test/symfile/example.sym");

  std::vector<word> expected_breakpoints = {0x4042};
  std::vector<word> expected_entrypoints = {0x1729};
  std::map<word, std::string> expected_symbols = {
    {0x4005, "hello_world"},
    {0x4012, "foo_bar"},
  };

  ASSERT_EQ(expected_breakpoints, symfile.Breakpoints());
  ASSERT_EQ(expected_entrypoints, symfile.Entrypoints());
  ASSERT_EQ(expected_symbols, symfile.Symbols());
}

TEST_F(SymfileTest, saveSymfile)
{
  Symfile symfile;
  symfile.addBreakpoint(0x1234);
  symfile.addBreakpoint(0x1235);
  symfile.addEntrypoint(0x2345);
  symfile.addEntrypoint(0x2346);
  symfile.addSymbol(0x3456, "foobar");
  symfile.addSymbol(0x3457, "foobaz");

  auto filename = createTmpFile();
  ASSERT_FALSE(filename.empty());
  symfile.SaveTo(filename);

  auto got = readWholeFile(filename);
  auto want = readWholeFile("test/symfile/expected.sym");
  ASSERT_EQ(got, want);
}
