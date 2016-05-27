#include <gtest/gtest.h>

#include "cap32.h"

TEST(Cap32Test, parseArgsNoArg)
{
  const char *argv[] = { "./cap32" };
  t_CPC CPC;

  parseArgs(sizeof(argv)/sizeof(char*), argv, CPC); 

  ASSERT_EQ("", CPC.drvA_path);
  ASSERT_EQ("", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_path);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);
}

TEST(Cap32Test, parseArgsOneLocalDskFile)
{
  const char *argv[] = { "./cap32", "./test.dsk" };
  t_CPC CPC;

  parseArgs(sizeof(argv)/sizeof(char*), argv, CPC); 

  ASSERT_EQ("./", CPC.drvA_path);
  ASSERT_EQ("test.dsk", CPC.drvA_file);
  ASSERT_EQ(0, CPC.drvA_zip);
  ASSERT_EQ("", CPC.drvB_path);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);
}

TEST(Cap32Test, parseArgsTwoDskFiles)
{
  const char *argv[] = { "./cap32", "/tmp/foo.dsk", "/var/bar.dsk" };
  t_CPC CPC;

  parseArgs(sizeof(argv)/sizeof(char*), argv, CPC); 

  ASSERT_EQ("/tmp/", CPC.drvA_path);
  ASSERT_EQ("foo.dsk", CPC.drvA_file);
  ASSERT_EQ(0, CPC.drvA_zip);
  ASSERT_EQ("/var/", CPC.drvB_path);
  ASSERT_EQ("bar.dsk", CPC.drvB_file);
  ASSERT_EQ(0, CPC.drvB_zip);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);
}

TEST(Cap32Test, parseArgsOneLocalCdtFile)
{
  const char *argv[] = { "./cap32", "./test.cdt" };
  t_CPC CPC;

  parseArgs(sizeof(argv)/sizeof(char*), argv, CPC); 

  ASSERT_EQ("", CPC.drvA_path);
  ASSERT_EQ("", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_path);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("./", CPC.tape_path);
  ASSERT_EQ("test.cdt", CPC.tape_file);
  ASSERT_EQ(0, CPC.tape_zip);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);
}

TEST(Cap32Test, parseArgsOneLocalVocFile)
{
  const char *argv[] = { "./cap32", "./test.voc" };
  t_CPC CPC;

  parseArgs(sizeof(argv)/sizeof(char*), argv, CPC); 

  ASSERT_EQ("", CPC.drvA_path);
  ASSERT_EQ("", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_path);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("./", CPC.tape_path);
  ASSERT_EQ("test.voc", CPC.tape_file);
  ASSERT_EQ(0, CPC.tape_zip);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);
}

TEST(Cap32Test, parseArgsOneLocalSnaFile)
{
  const char *argv[] = { "./cap32", "./test.sna" };
  t_CPC CPC;

  parseArgs(sizeof(argv)/sizeof(char*), argv, CPC); 

  ASSERT_EQ("", CPC.drvA_path);
  ASSERT_EQ("", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_path);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape_file);
  ASSERT_EQ("./", CPC.snap_path);
  ASSERT_EQ("test.sna", CPC.snap_file);
  ASSERT_EQ(0, CPC.snap_zip);
}

TEST(Cap32Test, parseArgsOneFileOfEachKind)
{
  const char *argv[] = { "./cap32", "/tmp/foo.dsk", "/var/bar.cdt", "/usr/test.sna" };
  t_CPC CPC;

  parseArgs(sizeof(argv)/sizeof(char*), argv, CPC); 

  ASSERT_EQ("/tmp/", CPC.drvA_path);
  ASSERT_EQ("foo.dsk", CPC.drvA_file);
  ASSERT_EQ(0, CPC.drvA_zip);
  ASSERT_EQ("", CPC.drvB_path);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("/var/", CPC.tape_path);
  ASSERT_EQ("bar.cdt", CPC.tape_file);
  ASSERT_EQ(0, CPC.tape_zip);
  ASSERT_EQ("/usr/", CPC.snap_path);
  ASSERT_EQ("test.sna", CPC.snap_file);
  ASSERT_EQ(0, CPC.snap_zip);
}

TEST(Cap32Test, parseArgsManyFilesOfEachKind)
{
  const char *argv[] = { "./cap32", "/tmp/foo.dsk", "/var/test.dsk", "/tmp/other.dsk", "/var/bar.cdt", "/tmp/test.voc", "/usr/test.sna", "/tmp/other.sna" };
  t_CPC CPC;

  parseArgs(sizeof(argv)/sizeof(char*), argv, CPC); 

  ASSERT_EQ("/tmp/", CPC.drvA_path);
  ASSERT_EQ("foo.dsk", CPC.drvA_file);
  ASSERT_EQ(0, CPC.drvA_zip);
  ASSERT_EQ("/var/", CPC.drvB_path);
  ASSERT_EQ("test.dsk", CPC.drvB_file);
  ASSERT_EQ(0, CPC.drvB_zip);
  ASSERT_EQ("/var/", CPC.tape_path);
  ASSERT_EQ("bar.cdt", CPC.tape_file);
  ASSERT_EQ(0, CPC.tape_zip);
  ASSERT_EQ("/usr/", CPC.snap_path);
  ASSERT_EQ("test.sna", CPC.snap_file);
  ASSERT_EQ(0, CPC.snap_zip);
}

