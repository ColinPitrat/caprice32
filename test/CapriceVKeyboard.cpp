#include <gtest/gtest.h>

#include "CapriceVKeyboard.h"
#include "cap32.h"

extern t_CPC CPC;

class CapriceVKeyboardTest : public testing::Test {
  public:
    void SetUp() {
      CPC.resources_path = "resources";
      app.Init();
      cvk = new CapriceVKeyboard(CRect(), nullptr, nullptr, nullptr);
    }

    void TearDown() {
      delete cvk;
    }

  protected:
    CApplication app;
    CapriceVKeyboard *cvk;
};

TEST_F(CapriceVKeyboardTest, StringToEventsSimple)
{
  std::string input = "cat\nrun\"s\btest";

  auto tmp = cvk->StringToEvents(input);
  std::vector<SDL_Event> result(tmp.begin(), tmp.end());

  ASSERT_EQ(28, result.size());

  // First key event is pressing 'c'
  ASSERT_EQ(SDLK_c, result[0].key.keysym.sym);
  ASSERT_EQ(KMOD_NONE, result[0].key.keysym.mod);
  ASSERT_EQ(SDL_KEYDOWN, result[0].key.type);
  ASSERT_EQ(SDL_PRESSED, result[0].key.state);
  // Second key event is releasing 'c'
  ASSERT_EQ(SDLK_c, result[1].key.keysym.sym);
  ASSERT_EQ(KMOD_NONE, result[1].key.keysym.mod);
  ASSERT_EQ(SDL_KEYUP, result[1].key.type);
  ASSERT_EQ(SDL_RELEASED, result[1].key.state);
  // 7th key event is return
  ASSERT_EQ(SDLK_RETURN, result[6].key.keysym.sym);
  ASSERT_EQ(KMOD_NONE, result[6].key.keysym.mod);
  ASSERT_EQ(SDL_KEYDOWN, result[6].key.type);
  ASSERT_EQ(SDL_PRESSED, result[6].key.state);
  // 19th key event is backspace
  ASSERT_EQ(SDLK_BACKSPACE, result[18].key.keysym.sym);
  ASSERT_EQ(KMOD_NONE, result[18].key.keysym.mod);
  ASSERT_EQ(SDL_KEYDOWN, result[18].key.type);
  ASSERT_EQ(SDL_PRESSED, result[18].key.state);
}
