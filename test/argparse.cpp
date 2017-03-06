#include <gtest/gtest.h>

#include "cap32.h"
#include "argparse.h"

TEST(ArgParseTest, parseArgsNoArg)
{
   const char *argv[] = {"./cap32"};
   std::vector<std::string> slot_list;

   parseArguments(1, const_cast<char **>(argv), slot_list);

   ASSERT_EQ(0, slot_list.size());
}

TEST(ArgParseTest, parseArgsOneArg)
{
   const char *argv[] = {"./cap32", "./foo.dsk"};
   std::vector<std::string> slot_list;

   parseArguments(2, const_cast<char **>(argv), slot_list);

   ASSERT_EQ(1, slot_list.size());
   ASSERT_EQ("./foo.dsk", slot_list.at(0));
}

TEST(ArgParseTest, parseArgsSeveralArgs)
{
   const char *argv[] = {"./cap32", "./foo.dsk", "bar.zip", "0", "__"};
   std::vector<std::string> slot_list;

   parseArguments(5, const_cast<char **>(argv), slot_list);

   ASSERT_EQ(4, slot_list.size());
   for (int i=1; i < 5; i++)
	   ASSERT_EQ(argv[i], slot_list.at(i-1));
}

