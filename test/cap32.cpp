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

TEST(Cap32Test, serializeDiskFormat)
{
  t_disk_format fmt;
  fmt.label = "test";
  fmt.tracks = 42;
  fmt.sides = 2;
  fmt.sectors = 4;
  fmt.sector_size = 1;
  fmt.gap3_length = 82;
  fmt.filler_byte = 229;
  fmt.sector_ids[0][0] = 193;
  fmt.sector_ids[0][1] = 198;
  fmt.sector_ids[0][2] = 194;
  fmt.sector_ids[0][3] = 199;
  fmt.sector_ids[1][0] = 195;
  fmt.sector_ids[1][1] = 200;
  fmt.sector_ids[1][2] = 196;
  fmt.sector_ids[1][3] = 201;

  auto result = serializeDiskFormat(fmt);

  ASSERT_EQ("test,42,2,4,1,82,229,193,198,194,199,195,200,196,201", result);
}

TEST(Cap32Test, serializeUnnamedDiskFormatReturnsEmptyString)
{
  t_disk_format fmt;
  fmt.tracks = 42;
  fmt.sides = 2;
  fmt.sectors = 4;
  fmt.sector_size = 1;
  fmt.gap3_length = 82;
  fmt.filler_byte = 229;
  fmt.sector_ids[0][0] = 193;
  fmt.sector_ids[0][1] = 198;
  fmt.sector_ids[0][2] = 194;
  fmt.sector_ids[0][3] = 199;
  fmt.sector_ids[1][0] = 195;
  fmt.sector_ids[1][1] = 200;
  fmt.sector_ids[1][2] = 196;
  fmt.sector_ids[1][3] = 201;

  auto result = serializeDiskFormat(fmt);

  ASSERT_EQ("", result);
}
