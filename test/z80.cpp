#include <gtest/gtest.h>
#include "z80.h"
#include "cap32.h"

#include "z80_macros.h"

extern byte *membank_read[4];
extern t_z80regs z80;

namespace
{

class Z80Test : public testing::Test {
  public:
    static void SetUpTestCase() {
      z80_init_tables();
    }
};

TEST_F(Z80Test, ExecuteNOP)
{
  byte membank0[1] = {0};

  // TODO: Move cycle count to z80
  //iCycleCount = 0;
  _R = 0;
  _PC = 0;
  membank_read[0] = membank0;

  z80_execute_instruction();

  //EXPECT_EQ(4, iCycleCount);
  EXPECT_EQ(1, _PC);
  EXPECT_EQ(1, _R);
}

TEST_F(Z80Test, DecALoop)
{
  //    3E 10 = ld a, 0x10
  //       3D = dec a (<= loop)
  // C2 02 00 = jp nz, 0x0002 (=> loop)
  byte membank0[6] = { 0x3E, 0x10, 0x3D, 0xC2, 0x2, 0x0 };

  // TODO: Move cycle count to z80
  //iCycleCount = 0;
  _R = 0;
  _PC = 0;
  membank_read[0] = membank0;

  // ld a, 0x10
  z80_execute_instruction();
  for(int i = 0x10; i > 0; i--) {
    EXPECT_EQ(2, _PC);
    EXPECT_EQ(i, _A);
    // dec a
    z80_execute_instruction();
    EXPECT_EQ(i-1, _A);
    EXPECT_EQ(3, _PC);
    // jp nz, 0x0002
    z80_execute_instruction();
  }
  EXPECT_EQ(0, _A);

  EXPECT_EQ(6, _PC);
  EXPECT_EQ(33, _R);
}

TEST_F(Z80Test, SqrtRoutine)
{
  // Routine to compute SQRT of HL in A
  // 21 64 00 = ld hl, 100
  // 11 01 00 = ld de, 1
  //       AF = xor a
  //       3D = dec a
  //    ED 52 = sbc hl, de (<= sqrt_loop)
  //       13 = inc de
  //       13 = inc de
  //       3C = inc a
  //    30 F9 = jr nc,-5 (=> sqrt_loop)
  byte membank0[15] = { 0x21, 0x64, 0x00, 0x11, 0x01, 0x00, 0xAF, 0x3D, 0xED, 0x52, 0x13, 0x13, 0x3C, 0x30, 0xF9 };

  // TODO: Move cycle count to z80
  //iCycleCount = 0;
  _R = 0;
  _PC = 0;
  membank_read[0] = membank0;

  z80_execute_instruction();
  EXPECT_EQ(0, _H);
  EXPECT_EQ(100, _L);
  z80_execute_instruction();
  EXPECT_EQ(0, _D);
  EXPECT_EQ(1, _E);
  z80_execute_instruction();
  EXPECT_EQ(0, _A);
  z80_execute_instruction();
  EXPECT_EQ(0xFF, _A);
  while(_PC != 15) {
    z80_execute_instruction();
  }

  EXPECT_EQ(10, _A);
}

}
