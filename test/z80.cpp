#include <gtest/gtest.h>
#include "z80.h"
#include "cap32.h"

extern byte *membank_read[4];
extern t_z80regs z80;

TEST(Z80, ExecuteNOP)
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
