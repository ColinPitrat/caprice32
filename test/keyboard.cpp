#include <gtest/gtest.h>
#include <filesystem>
#include "keyboard.h"

TEST(KeyboardTest, parseAllLayoutFiles)
{
  InputMapper mapper(/*CPC=*/nullptr);
  for (const auto & entry : std::filesystem::directory_iterator("./resources"))
  {
    auto path = entry.path().string();
    if (path.substr(path.length()-4, 4) == ".map") {
      EXPECT_TRUE(mapper.load_layout(path)) << "Error parsing " << path;
    }
  }
}
