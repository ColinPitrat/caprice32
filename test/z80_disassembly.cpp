#include <gtest/gtest.h>
#include "z80_disassembly.h"
#include "cap32.h"

#include "z80_macros.h"

extern byte *membank_read[4];
extern t_CPC CPC;

namespace
{

TEST(DisassembledLine, Comparisons)
{
  DisassembledLine l1(1, 0, "nop");
  DisassembledLine l1_identical(1, 0, "nop");
  DisassembledLine l1bis(1, 0xc9, "ret");
  DisassembledLine l2(2, 0x1802, "jr 2", 4);
  DisassembledLine l3(3, 0xc9, "ret");

  EXPECT_TRUE(l1 < l2);
  EXPECT_TRUE(l2 < l3);
  EXPECT_TRUE(l1 < l3);
  EXPECT_FALSE(l1 < l1bis);
  EXPECT_FALSE(l1 == l1bis);
  EXPECT_TRUE(l1 == l1_identical);
}

TEST(Z80DisassemblyTest, DisassembleNOPRET)
{
  byte membank0[2] = {0, 0xc9};

  CPC.resources_path = "resources";
  membank_read[0] = membank0;

  auto code = disassemble({0});

  DisassembledCode want;
  want.lines.emplace(0, 0, "nop");
  want.lines.emplace(1, 0xc9, "ret");

  EXPECT_EQ(want.lines, code.lines);
}

TEST(Z80DisassemblyTest, DisassembleDecALoop)
{
  byte membank0[10] = { 0x00, 0x00, 0x00, 0x3E, 0x10, 0x3D, 0xC2, 0x5, 0x0, 0xC9 };

  CPC.resources_path = "resources";
  membank_read[0] = membank0;

  auto code = disassemble({3});

  DisassembledCode want;
  want.lines.emplace(3, 0x3E10, "ld a,$10");
  want.lines.emplace(5, 0x3D, "dec a");
  want.lines.emplace(6, 0xC20500, "jp nz,$0005");
  want.lines.emplace(9, 0xC9, "ret");

  EXPECT_EQ(want.lines, code.lines);
}

TEST(Z80DisassemblyTest, SqrtRoutine)
{
  byte membank0[16] = { 0x21, 0x64, 0x00, 0x11, 0x01, 0x00, 0xAF, 0x3D, 0xED, 0x52, 0x13, 0x13, 0x3C, 0x30, 0xF9, 0xC9 };

  CPC.resources_path = "resources";
  membank_read[0] = membank0;

  auto code = disassemble({0});

  DisassembledCode want;
  want.lines.emplace(0, 0x216400, "ld hl,$0064");
  want.lines.emplace(3, 0x110100, "ld de,$0001");
  want.lines.emplace(6, 0xAF, "xor a");
  want.lines.emplace(7, 0x3D, "dec a");
  want.lines.emplace(8, 0xED52, "sbc hl,de");
  want.lines.emplace(10, 0x13, "inc de");
  want.lines.emplace(11, 0x13, "inc de");
  want.lines.emplace(12, 0x3C, "inc a");
  want.lines.emplace(13, 0x30F9, "jr nc,$f9  ; $0008");
  want.lines.emplace(15, 0xC9, "ret");

  EXPECT_EQ(want.lines, code.lines);
}

}
