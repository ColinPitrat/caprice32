#include <gtest/gtest.h>
#include "cartridge.h"
#include "types.h"
#include "errors.h"

extern byte* pbGPBuffer;
extern byte* pbCartridgeImage;

class Cartridge : public testing::Test {
   public:
      void SetUp() {
         // attempt to allocate the general purpose buffer
         pbGPBuffer = new byte [128*1024];
         ASSERT_NE(nullptr, pbGPBuffer);
      }

      void TearDown() {
         delete[] pbGPBuffer;
      }
};

TEST_F(Cartridge, CprLoadOnNonExistingFileReturnsError)
{
   int rc = cpr_load("test/cartridge/non_existent.cpr");

   ASSERT_EQ(ERR_FILE_NOT_FOUND, rc);
}

TEST_F(Cartridge, CprLoadOnEmptyCPRFileReturnsError)
{
   int rc = cpr_load("test/cartridge/invalid_empty.cpr");

   ASSERT_EQ(ERR_CPR_INVALID, rc);
}

TEST_F(Cartridge, CprLoadOnNonRIFFFileReturnsError)
{
   int rc = cpr_load("test/cartridge/invalid_noriff.cpr");

   ASSERT_EQ(ERR_CPR_INVALID, rc);
}

TEST_F(Cartridge, CprLoadOnNonAMSFileReturnsError)
{
   int rc = cpr_load("test/cartridge/invalid_noams.cpr");

   ASSERT_EQ(ERR_CPR_INVALID, rc);
}

TEST_F(Cartridge, CprLoadOnDummyCPRFile)
{
   int rc = cpr_load("test/cartridge/dummy.cpr");

   ASSERT_EQ(0, rc);
   ASSERT_EQ('t', pbCartridgeImage[0]);
   ASSERT_EQ('e', pbCartridgeImage[1]);
   ASSERT_EQ('s', pbCartridgeImage[2]);
   ASSERT_EQ('t', pbCartridgeImage[3]);
   for (int i = 4; i < 512*1024; i++) {
      ASSERT_EQ('\0', pbCartridgeImage[i]) << "with i = " << i;
   }
}

TEST_F(Cartridge, CprLoadOnSystemCPRFile)
{
   int rc = cpr_load("rom/system.cpr");

   ASSERT_EQ(0, rc);
   // Check the first byte of each page
   ASSERT_EQ('\x01', pbCartridgeImage[0]);
   //ASSERT_EQ('\x80', pbCartridgeImage[16*1024]);
   ASSERT_EQ('\x3a', pbCartridgeImage[32*1024]);
   ASSERT_EQ('\x01', pbCartridgeImage[48*1024]);
   //ASSERT_EQ('\xf3', pbCartridgeImage[64*1024]);
   ASSERT_EQ('\x21', pbCartridgeImage[80*1024]);
   ASSERT_EQ('\x60', pbCartridgeImage[96*1024]);
   //ASSERT_EQ('\x80', pbCartridgeImage[112*1024]);
   for (int i = 128*1024; i < 512*1024; i++) {
      ASSERT_EQ('\0', pbCartridgeImage[i]) << "with i = " << i;
   }
}
