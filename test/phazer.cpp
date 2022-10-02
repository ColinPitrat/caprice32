#include <gtest/gtest.h>

#include "phazer.h"

TEST(PhazerTest, Next)
{
  PhazerType none(PhazerType::None);
  PhazerType trojan(PhazerType::TrojanLightPhazer);

  ASSERT_EQ(PhazerType::AmstradMagnumPhaser, none.Next());
  ASSERT_EQ(PhazerType::None, trojan.Next());
}

TEST(PhazerTest, operator_bool)
{
  ASSERT_FALSE(PhazerType(PhazerType::None));
  ASSERT_TRUE(PhazerType(PhazerType::AmstradMagnumPhaser));
  ASSERT_TRUE(PhazerType(PhazerType::TrojanLightPhazer));
}

TEST(PhazerTest, ToString)
{
  ASSERT_EQ("off", PhazerType(PhazerType::None).ToString());
  ASSERT_EQ("Amstrad Magnum Phaser", PhazerType(PhazerType::AmstradMagnumPhaser).ToString());
  ASSERT_EQ("Trojan Light Phazer", PhazerType(PhazerType::TrojanLightPhazer).ToString());
}
