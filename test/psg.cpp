#include <gtest/gtest.h>

#include "cap32.h"
#include "types.h"

// Validate that alignment is correct between bytes & words structs (cf issue #104)
TEST(PsgTest, RegisterAYAlignment)
{
   t_PSG psg;
   psg.RegisterAY.TonALo = 3;
   psg.RegisterAY.TonAHi = 5;
   EXPECT_EQ(0x0503, psg.RegisterAY.TonA);
   psg.RegisterAY.TonBLo = 3;
   psg.RegisterAY.TonBHi = 5;
   EXPECT_EQ(0x0503, psg.RegisterAY.TonB);
   psg.RegisterAY.TonCLo = 3;
   psg.RegisterAY.TonCHi = 5;
   EXPECT_EQ(0x0503, psg.RegisterAY.TonC);
   psg.RegisterAY.EnvelopeLo = 3;
   psg.RegisterAY.EnvelopeHi = 5;
   EXPECT_EQ(0x0503, psg.RegisterAY.Envelope);
}

TEST(PsgTest, CycleCountCombinedAccess)
{
  t_PSG psg;
  psg.cycle_count.high = 0x00112233;
  psg.cycle_count.low = 0x44556677;

  EXPECT_EQ(0x0011223344556677, psg.cycle_count.both);
}
