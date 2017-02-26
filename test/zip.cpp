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

  int rc = zip::dir(&file_infos);

  // Returns ERR_FILE_EMPTY_ZIP although zip file is not really empty,
  // it just has no matching file
  ASSERT_EQ(ERR_FILE_EMPTY_ZIP, rc);
  ASSERT_EQ(0, file_infos.filesOffsets.size());
}

TEST(Zip, DirOnFileWithOneExtensionAndMultipleEntries)
{
  zip::t_zip_info file_infos;
  file_infos.filename = "test/zip/test1.zip";
  file_infos.extensions = ".dsk";

  int rc = zip::dir(&file_infos);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(2, file_infos.filesOffsets.size());
  ASSERT_EQ("disk/empty.dsk", file_infos.filesOffsets[0].first);
  ASSERT_EQ(119, file_infos.filesOffsets[0].second);
  ASSERT_EQ("disk/hello.dsk", file_infos.filesOffsets[1].first);
  ASSERT_EQ(1918, file_infos.filesOffsets[1].second);
}

TEST(Zip, DirOnFileWithMultipleExtensions)
{
  zip::t_zip_info file_infos;
  file_infos.filename = "test/zip/test1.zip";
  file_infos.extensions = ".dsk.txt";

  int rc = zip::dir(&file_infos);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(3, file_infos.filesOffsets.size());
  ASSERT_EQ("README.txt", file_infos.filesOffsets[0].first);
  ASSERT_EQ("disk/empty.dsk", file_infos.filesOffsets[1].first);
  ASSERT_EQ("disk/hello.dsk", file_infos.filesOffsets[2].first);
}

TEST(Zip, ExtractOnFileWithMultipleEntries)
{
  FILE *file;
  // Retrieve offset
  zip::t_zip_info file_infos;
  file_infos.filename = "test/zip/test1.zip";
  file_infos.extensions = ".txt";
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
