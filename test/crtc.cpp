#include <gtest/gtest.h>

#include "crtc.h"

class CrtcTest : public testing::Test {
   public:
   private:
};

TEST_F(CrtcTest, ShiftLittleEndianDwordTriplet)
{
   dword val;
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 0);
   EXPECT_EQ(0x01020304, val);
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 1);
   EXPECT_EQ(0x02030489, val);
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 2);
   EXPECT_EQ(0x030489AB, val);
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 3);
   EXPECT_EQ(0x0489ABCD, val);
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 4);
   EXPECT_EQ(0x89ABCDEF, val);
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 5);
   EXPECT_EQ(0xABCDEF01, val);
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 6);
   EXPECT_EQ(0xCDEF0123, val);
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 7);
   EXPECT_EQ(0xEF012345, val);
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 8);
   EXPECT_EQ(0x01234567, val);
}

// This test verifies that access to union as they are used extensively in the code works.
TEST_F(CrtcTest, NewDtCombinedAccess)
{
  t_new_dt new_dt;

  new_dt.NewHDSPTIMG = 0x01;
  new_dt.NewDISPTIMG = 0x23;

  EXPECT_EQ(0x0123, new_dt.combined);
}
