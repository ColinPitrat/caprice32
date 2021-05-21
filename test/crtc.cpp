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

// Verifies that access to t_new_dt union works fine
TEST_F(CrtcTest, NewDtCombinedAccess)
{
  t_new_dt new_dt;

  new_dt.NewHDSPTIMG = 0x01;
  new_dt.NewDISPTIMG = 0x23;

  EXPECT_EQ(0x0123, new_dt.combined);
}

// Verifies that access to t_flags1 unions works fine
TEST_F(CrtcTest, Flags1CombinedAccess)
{
  t_flags1 flags1;

  flags1.dt.HDSPTIMG = 0x01;
  flags1.dt.DISPTIMG = 0x23;
  flags1.inHSYNC = 0x45;
  flags1.monVSYNC = 0x67;

  EXPECT_EQ(0x01234567, flags1.combined);
  EXPECT_EQ(0x0123, flags1.dt.combined);
}
