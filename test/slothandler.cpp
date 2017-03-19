#include <gtest/gtest.h>

#include "cap32.h"
#include "slotshandler.h"

TEST(SlotHandlerTest, fillSlotsNoArg)
{
  std::vector<std::string> slot_list;
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cart_file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);
}

TEST(SlotHandlerTest, fillSlotsOneLocalDskFile)
{
  std::vector<std::string> slot_list = { "./test.dsk"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cart_file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("./test.dsk", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);

}

TEST(SlotHandlerTest, fillSlotsTwoDskFiles)
{
  std::vector<std::string> slot_list = { "/tmp/foo.dsk", "/var/bar.dsk"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cart_file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("/tmp/foo.dsk", CPC.drvA_file);
  ASSERT_EQ("/var/bar.dsk", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);
}

TEST(SlotHandlerTest, fillSlotsOneLocalCdtFile)
{
  std::vector<std::string> slot_list = { "./test.cdt"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cart_file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("./test.cdt", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);
}

TEST(SlotHandlerTest, fillSlotsOneLocalVocFile)
{
  std::vector<std::string> slot_list = { "./test.voc"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cart_file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("./test.voc", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);
}

TEST(SlotHandlerTest, fillSlotsOneLocalSnaFile)
{
  std::vector<std::string> slot_list = { "./test.sna"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cart_file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("./test.sna", CPC.snap_file);
}

TEST(SlotHandlerTest, fillSlotsOneCprFile)
{
  std::vector<std::string> slot_list = { "./test.cpr"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("./test.cpr", CPC.cart_file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);
}

TEST(SlotHandlerTest, fillSlotsOneZippedCprFile)
{
  std::vector<std::string> slot_list = { "test/cartridge/testplus.zip"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("test/cartridge/testplus.zip", CPC.cart_file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snap_file);
}

TEST(SlotHandlerTest, fillSlotsOneFileOfEachKind)
{
  std::vector<std::string> slot_list = { "/tmp/foo.dsk", "/var/bar.cdt", "/usr/test.sna", "/home/cart.cpr" };
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("/home/cart.cpr", CPC.cart_file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("/tmp/foo.dsk", CPC.drvA_file);
  ASSERT_EQ("", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("/var/bar.cdt", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("/usr/test.sna", CPC.snap_file);
}

TEST(SlotHandlerTest, fillSlotsManyFilesOfEachKind)
{
  std::vector<std::string> slot_list = { "rom/system.cpr", "/tmp/foo.dsk", "/var/test.dsk", "/tmp/other.dsk", "/var/bar.cdt", "/tmp/test.voc", "/usr/test.sna", "/tmp/other.sna", "test/test.cpr" };
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("rom/system.cpr", CPC.cart_file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("/tmp/foo.dsk", CPC.drvA_file);
  ASSERT_EQ("/var/test.dsk", CPC.drvB_file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("/var/bar.cdt", CPC.tape_file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("/usr/test.sna", CPC.snap_file);

}

TEST(SlotHandlerTest, serializeDiskFormat)
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

TEST(SlotHandlerTest, serializeUnnamedDiskFormatReturnsEmptyString)
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
