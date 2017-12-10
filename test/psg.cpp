#include <gtest/gtest.h>

#include "cap32.h"
#include "types.h"

// Validate that alignment is correct between bytes & words structs (cf issue #104)
TEST(PsgTest, RegisterAYAlignment)
{
   t_PSG psg;
   psg.RegisterAY.TonALo = 3;
   psg.RegisterAY.TonAHi = 5;
   EXPECT_EQ(*reinterpret_cast<word *>(&psg.RegisterAY.TonALo), psg.RegisterAY.TonA);
   psg.RegisterAY.TonBLo = 3;
   psg.RegisterAY.TonBHi = 5;
   EXPECT_EQ(*reinterpret_cast<word *>(&psg.RegisterAY.TonBLo), psg.RegisterAY.TonB);
   psg.RegisterAY.TonCLo = 3;
   psg.RegisterAY.TonCHi = 5;
   EXPECT_EQ(*reinterpret_cast<word *>(&psg.RegisterAY.TonCLo), psg.RegisterAY.TonC);
   psg.RegisterAY.EnvelopeLo = 3;
   psg.RegisterAY.EnvelopeHi = 5;
   EXPECT_EQ(*reinterpret_cast<word *>(&psg.RegisterAY.EnvelopeLo), psg.RegisterAY.Envelope);
}
