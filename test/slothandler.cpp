#include <gtest/gtest.h>

#include "cap32.h"
#include "slotshandler.h"
#include <string>

extern t_drive driveA;

TEST(SlotHandlerTest, slotsInitializedWithProperDriveTypes)
{
  t_CPC CPC;

  ASSERT_EQ(DRIVE::DSK_A, CPC.driveA.drive);
  ASSERT_EQ(DRIVE::DSK_B, CPC.driveB.drive);
  ASSERT_EQ(DRIVE::TAPE, CPC.tape.drive);
  ASSERT_EQ(DRIVE::SNAPSHOT, CPC.snapshot.drive);
  ASSERT_EQ(DRIVE::CARTRIDGE, CPC.cartridge.drive);
}

TEST(SlotHandlerTest, fillSlotsNoArg)
{
  std::vector<std::string> slot_list;
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cartridge.file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.driveA.file);
  ASSERT_EQ("", CPC.driveB.file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape.file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snapshot.file);
}

TEST(SlotHandlerTest, fillSlotsOneLocalDskFile)
{
  std::vector<std::string> slot_list = { "./test.dsk"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cartridge.file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("./test.dsk", CPC.driveA.file);
  ASSERT_EQ("", CPC.driveB.file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape.file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snapshot.file);

}

TEST(SlotHandlerTest, fillSlotsTwoDskFiles)
{
  std::vector<std::string> slot_list = { "/tmp/foo.dsk", "/var/bar.dsk"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cartridge.file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("/tmp/foo.dsk", CPC.driveA.file);
  ASSERT_EQ("/var/bar.dsk", CPC.driveB.file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape.file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snapshot.file);
}

TEST(SlotHandlerTest, fillSlotsOneLocalCdtFile)
{
  std::vector<std::string> slot_list = { "./test.cdt"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cartridge.file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.driveA.file);
  ASSERT_EQ("", CPC.driveB.file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("./test.cdt", CPC.tape.file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snapshot.file);
}

TEST(SlotHandlerTest, fillSlotsOneLocalVocFile)
{
  std::vector<std::string> slot_list = { "./test.voc"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cartridge.file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.driveA.file);
  ASSERT_EQ("", CPC.driveB.file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("./test.voc", CPC.tape.file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snapshot.file);
}

TEST(SlotHandlerTest, fillSlotsOneLocalSnaFile)
{
  std::vector<std::string> slot_list = { "./test.sna"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cartridge.file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.driveA.file);
  ASSERT_EQ("", CPC.driveB.file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape.file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("./test.sna", CPC.snapshot.file);
}

TEST(SlotHandlerTest, fillSlotsOneCprFile)
{
  std::vector<std::string> slot_list = { "./test.cpr"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("./test.cpr", CPC.cartridge.file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.driveA.file);
  ASSERT_EQ("", CPC.driveB.file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape.file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snapshot.file);
}

TEST(SlotHandlerTest, fillSlotsOneZippedCprFile)
{
  std::vector<std::string> slot_list = { "test/cartridge/testplus.zip"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("test/cartridge/testplus.zip", CPC.cartridge.file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("", CPC.driveA.file);
  ASSERT_EQ("", CPC.driveB.file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape.file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snapshot.file);
}

unsigned int checksum(const t_drive* drive)
{
  unsigned int checksum = 0;
  for (unsigned int track = 0; track < drive->tracks; track++) {
    for (unsigned int side = 0; side <= drive->sides; side++) {
      for (unsigned int byte = 0; byte < drive->track[track][side].size; byte++) {
        checksum += drive->track[track][side].data[byte];
      }
    }
  }
  return checksum;
}

TEST(SlotHandlerTest, fillSlotsDiskAZipMultipleFiles)
{
  std::vector<std::string> slot_list = { "test/zip/test1.zip"};
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("", CPC.cartridge.file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("test/zip/test1.zip", CPC.driveA.file);
  ASSERT_EQ("", CPC.driveB.file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("", CPC.tape.file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("", CPC.snapshot.file);

  // The zip contains 2 files.
  // First file is empty.dsk
  file_load(CPC.driveA);
  auto checksum1 = checksum(&driveA);
  // Second file is hello.dsk
  CPC.driveA.zip_index += 1;
  file_load(CPC.driveA);
  auto checksum2 = checksum(&driveA);
  // Switching again should loop back to empty.dsk
  CPC.driveA.zip_index += 1;
  file_load(CPC.driveA);
  auto checksum3 = checksum(&driveA);
  // And back to hello.dsk
  CPC.driveA.zip_index += 1;
  file_load(CPC.driveA);
  auto checksum4 = checksum(&driveA);

  // 1 and 3 are both "empty.dsk" so they are equals
  // 2 and 4 are both "hello.dsk" so they are equals
  ASSERT_EQ(checksum1, checksum3);
  ASSERT_EQ(checksum2, checksum4);

  // 1 and 2 are "empty.dsk" and "hello.dsk" so they are different
  ASSERT_NE(checksum1, checksum2);
}

TEST(SlotHandlerTest, fillSlotsOneFileOfEachKind)
{
  std::vector<std::string> slot_list = { "/tmp/foo.dsk", "/var/bar.cdt", "/usr/test.sna", "/home/cart.cpr" };
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("/home/cart.cpr", CPC.cartridge.file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("/tmp/foo.dsk", CPC.driveA.file);
  ASSERT_EQ("", CPC.driveB.file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("/var/bar.cdt", CPC.tape.file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("/usr/test.sna", CPC.snapshot.file);
}

TEST(SlotHandlerTest, fillSlotsManyFilesOfEachKind)
{
  std::vector<std::string> slot_list = { "rom/system.cpr", "/tmp/foo.dsk", "/var/test.dsk", "/tmp/other.dsk", "/var/bar.cdt", "/tmp/test.voc", "/usr/test.sna", "/tmp/other.sna", "test/test.cpr" };
  t_CPC CPC;

  fillSlots(slot_list, CPC);

  ASSERT_EQ("", CPC.cart_path);
  ASSERT_EQ("rom/system.cpr", CPC.cartridge.file);
  ASSERT_EQ("", CPC.dsk_path);
  ASSERT_EQ("/tmp/foo.dsk", CPC.driveA.file);
  ASSERT_EQ("/var/test.dsk", CPC.driveB.file);
  ASSERT_EQ("", CPC.tape_path);
  ASSERT_EQ("/var/bar.cdt", CPC.tape.file);
  ASSERT_EQ("", CPC.snap_path);
  ASSERT_EQ("/usr/test.sna", CPC.snapshot.file);

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
