#include <gtest/gtest.h>

#include "asic.h"
#include "cap32.h"
#include <vector>

extern byte *membank_config[8][4];
extern byte *membank_write[4];
extern t_GateArray GateArray;

namespace 
{

class AsicTest : public testing::Test {
   public:
      AsicTest() : lockSeq({0x00, 0xff, 0x77, 0xb3, 0x51, 0xa8, 0xd4, 0x62, 0x39, 0x9c, 0x46, 0x2b, 0x15, 0x8a, 0xcd})
      {
         asic.locked = true;
         asic.lockSeqPos = 0;
      }

   protected:
      const std::vector<byte> lockSeq;
};

// About unlock sequence ...
//
// testplus.cpr unlock with:
// AA 00 FF 77 B3 51 A8 D4 62 39 9C 46 2B 15 8A CD EE FF
// and lock with:
// AA 00 FF 77 B3 51 A8 D4 62 39 9C 46 2B 15 8A CD EE A5
//
// But Arnold5 specs says the unlock sequence ends at CD
// http://www.cpcwiki.eu/index.php/Arnold_V_Specs_Revised#Locking_of_enhanced_features
// and older specs says the unlock sequence ends at EE
// http://www.cpcwiki.eu/index.php/Arnold_V_specs#Locking_of_enhanced_features
// In all cases, the lock sequence is the same with the last byte having any value except the one it has in the unlock sequence
// All cartridges tested unlock with:
// FF 00 FF 77 B3 51 A8 D4 62 39 9C 46 2B 15 8A CD EE
// Klax alternates between:
// FF 00 FF 77 B3 51 A8 D4 62 39 9C 46 2B 15 8A CD EE
// And:
// FF 00 FF 77 B3 51 A8 D4 62 39 9C 46 2B 15 8A CD FF
// But making the latter unlock the asic make the game bug (loop on welcome screen)

TEST_F(AsicTest, UnlockAsic)
{
   ASSERT_TRUE(asic.locked);

   // Sequence must start with a non null
   asic_poke_lock_sequence(0x01);
   for(const auto& val : lockSeq) {
      asic_poke_lock_sequence(val);
   }
   // Only unlock once sequence is complete
   ASSERT_TRUE(asic.locked);
   // And end with any value
   asic_poke_lock_sequence(0x01);

   ASSERT_FALSE(asic.locked);
}

TEST_F(AsicTest, UnlockAsicDoesntUnlockIfNoNonNullByteBeforeSeq)
{
   ASSERT_TRUE(asic.locked);

   asic_poke_lock_sequence(0x00);
   for(const auto& val : lockSeq) {
      asic_poke_lock_sequence(val);
   }
   asic_poke_lock_sequence(0x01);

   ASSERT_TRUE(asic.locked);
}

TEST_F(AsicTest, RelockAsicIfLockSeqReplayedExceptForLastByte)
{
   ASSERT_TRUE(asic.locked);

   asic_poke_lock_sequence(0x01);
   for(const auto& val : lockSeq) {
      asic_poke_lock_sequence(val);
   }
   asic_poke_lock_sequence(0x01);

   ASSERT_FALSE(asic.locked);

   asic_poke_lock_sequence(0x01);
   for(const auto& val : lockSeq) {
      if(val == 0xcd) {
         // Only unlock once sequence is complete
         ASSERT_FALSE(asic.locked);
         asic_poke_lock_sequence(0x0b);
      } else {
         asic_poke_lock_sequence(val);
      }
   }

   ASSERT_TRUE(asic.locked);
}

TEST_F(AsicTest, UnlockWithRepetitionOfFirstTwoBytesOfSequence)
{
   ASSERT_TRUE(asic.locked);

   // This is what Dick Tracy cartridge does
   // (Crazy Cars II also but it checks that the unlock is properly done and retries)
   asic_poke_lock_sequence(0x00);
   asic_poke_lock_sequence(0xff);
   for(const auto& val : lockSeq) {
      asic_poke_lock_sequence(val);
   }
   ASSERT_TRUE(asic.locked);
   asic_poke_lock_sequence(0xee);

   ASSERT_FALSE(asic.locked);
}

TEST_F(AsicTest, SetDMAControlAndStatusRegister)
{
  asic_register_page_write(0x6C0F, 1);

  EXPECT_TRUE(asic.dma.ch[0].enabled);
  EXPECT_FALSE(asic.dma.ch[1].enabled);
  EXPECT_FALSE(asic.dma.ch[2].enabled);

  asic_register_page_write(0x6C0F, 2);

  EXPECT_FALSE(asic.dma.ch[0].enabled);
  EXPECT_TRUE(asic.dma.ch[1].enabled);
  EXPECT_FALSE(asic.dma.ch[2].enabled);

  asic_register_page_write(0x6C0F, 3);

  EXPECT_TRUE(asic.dma.ch[0].enabled);
  EXPECT_TRUE(asic.dma.ch[1].enabled);
  EXPECT_FALSE(asic.dma.ch[2].enabled);

  asic_register_page_write(0x6C0F, 0x4);

  EXPECT_FALSE(asic.dma.ch[0].enabled);
  EXPECT_FALSE(asic.dma.ch[1].enabled);
  EXPECT_TRUE(asic.dma.ch[2].enabled);
}

TEST_F(AsicTest, AsicReset)
{
  asic.locked = false;
  asic.hscroll = 1;
  asic.vscroll = 2;
  asic.dma.ch[0].source_address = 0x1234;
  asic.dma.ch[0].loop_address = 0x1234;
  asic.dma.ch[0].prescaler = 42;
  asic.dma.ch[0].enabled = true;
  asic.dma.ch[0].interrupt = true;
  asic.dma.ch[0].pause_ticks = 42;
  asic.dma.ch[0].tick_cycles = 42;
  asic.dma.ch[0].loops = 42;

  asic_reset();

  ASSERT_TRUE(asic.locked);
  ASSERT_EQ(0, asic.hscroll);
  ASSERT_EQ(0, asic.vscroll);
  ASSERT_EQ(0, asic.dma.ch[0].source_address);
  ASSERT_EQ(0, asic.dma.ch[0].loop_address);
  ASSERT_EQ(0, asic.dma.ch[0].prescaler);
  ASSERT_FALSE(asic.dma.ch[0].enabled);
  ASSERT_FALSE(asic.dma.ch[0].interrupt);
  ASSERT_EQ(0, asic.dma.ch[0].pause_ticks);
  ASSERT_EQ(0, asic.dma.ch[0].tick_cycles);
  ASSERT_EQ(0, asic.dma.ch[0].loops);
  // D0 of IVR is set to 1 at reset says Arnold 5 specs
  ASSERT_TRUE((asic.interrupt_vector & 1) == 1);
}

TEST_F(AsicTest, AsicDMACycleWriteBackDCSR)
{
  std::vector<byte> membank0 = { 0x00, 0x00};
  std::vector<byte> membank1(0x3000, 0);
  membank_config[0][0] = &membank0[0];
  membank_write[1] = &membank1[0];

  GateArray.RAM_config = 0;
  asic_reset();
  asic.dma.ch[0].enabled = true;
  asic.dma.ch[0].source_address = 0x0;

  EXPECT_EQ(0, *(membank_write[1] + 0x2c00));
  EXPECT_EQ(0, *(membank_write[1] + 0x2c0f));

  asic_dma_cycle();

  EXPECT_EQ(2, *(membank_write[1] + 0x2c00));
  EXPECT_EQ(1, *(membank_write[1] + 0x2c0f));
}

}
