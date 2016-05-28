#include <gtest/gtest.h>
#include "zip.h"
#include "errors.h"

/*
 * These tests are not really unit tests as they use files that can be found
 * in test/zip/
 * The files content are the following:
 *  - test1.zip
 *     - README.txt
 *     - empty.dsk
 *     - hello.dsk
 */

TEST(Zip, DirOnFileWithNoMatchingEntry)
{
  zip::t_zip_info file_infos;
  file_infos.filename = "test/zip/test1.zip";
  file_infos.extensions = ".zzz";
  file_infos.pchFileNames = nullptr;
  file_infos.pchSelection = nullptr;

  int rc = zip::dir(&file_infos);

  // Returns ERR_FILE_EMPTY_ZIP although zip file is not really empty,
  // it just has no matching file
  ASSERT_EQ(ERR_FILE_EMPTY_ZIP, rc);
  ASSERT_EQ(0, file_infos.iFiles);
}

TEST(Zip, DirOnFileWithOneExtensionAndMultipleEntries)
{
  zip::t_zip_info file_infos;
  file_infos.filename = "test/zip/test1.zip";
  file_infos.extensions = ".dsk";
  file_infos.pchFileNames = nullptr;
  file_infos.pchSelection = nullptr;

  int rc = zip::dir(&file_infos);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(2, file_infos.iFiles);
  ASSERT_STREQ("disk/empty.dsk", file_infos.pchFileNames);
  // TODO(cpitrat): Cleaner way to handle multiple files, offsets and filesizes
  // 19 because "disk/empty.dsk" requires 15 bytes and another 4 are used by offset concatenated after
  ASSERT_STREQ("disk/hello.dsk", file_infos.pchFileNames + 19);
}

TEST(Zip, DirOnFileWithMultipleExtensions)
{
  zip::t_zip_info file_infos;
  file_infos.filename = "test/zip/test1.zip";
  file_infos.extensions = ".dsk.txt";
  file_infos.pchFileNames = nullptr;
  file_infos.pchSelection = nullptr;

  int rc = zip::dir(&file_infos);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(3, file_infos.iFiles);
  ASSERT_STREQ("README.txt", file_infos.pchFileNames);
  ASSERT_STREQ("disk/empty.dsk", file_infos.pchFileNames + 15);
  ASSERT_STREQ("disk/hello.dsk", file_infos.pchFileNames + 34);
}

TEST(Zip, ExtractOnFileWithMultipleEntries)
{
  FILE *file;
  // Retrieve offset
  zip::t_zip_info file_infos;
  file_infos.filename = "test/zip/test1.zip";
  file_infos.extensions = ".txt";
  file_infos.pchFileNames = nullptr;
  file_infos.pchSelection = nullptr;
  int rc = zip::dir(&file_infos);
  ASSERT_EQ(0, rc);

  rc = zip::extract(file_infos, &file);

  ASSERT_EQ(0, rc);
  ASSERT_NE(nullptr, file);
  char buffer[256];
  size_t r = fread(buffer, 1, 256, file);
  buffer[r] = 0;
  ASSERT_STREQ("This file is a sample zip file used by Caprice32 tests.\n", buffer);
  ASSERT_EQ(0, fclose(file));
}
