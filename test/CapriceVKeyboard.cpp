#include <gtest/gtest.h>

#include "CapriceVKeyboard.h"
#include "cap32.h"
#include "keyboard.h"

using namespace wGui;

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

TEST_F(CapriceVKeyboardTest, StringToEventsSimpleString)
{
  std::string input = "cat";

  auto tmp = cvk->StringToEvents(input);
  std::vector<SDL_Event> result(tmp.begin(), tmp.end());

  ASSERT_EQ(6, result.size());

  // Result must be an alternance of key down / key up
  for(int i = 0; i < 3; ++i) {
    ASSERT_EQ(SDL_KEYDOWN,  result[2*i].key.type);
    ASSERT_EQ(SDL_PRESSED,  result[2*i].key.state);
    ASSERT_EQ(SDL_KEYUP,    result[2*i+1].key.type);
    ASSERT_EQ(SDL_RELEASED, result[2*i+1].key.state);
  }
  // Only keys without modifier
  for(int i = 0; i < 6; ++i) {
    ASSERT_EQ(KMOD_NONE, result[i].key.keysym.mod);
  }
  // Keys correspond to the input string
  ASSERT_EQ(SDLK_c, result[0].key.keysym.sym);
  ASSERT_EQ(SDLK_c, result[1].key.keysym.sym);
  ASSERT_EQ(SDLK_a, result[2].key.keysym.sym);
  ASSERT_EQ(SDLK_a, result[3].key.keysym.sym);
  ASSERT_EQ(SDLK_t, result[4].key.keysym.sym);
  ASSERT_EQ(SDLK_t, result[5].key.keysym.sym);
}

TEST_F(CapriceVKeyboardTest, StringToEventsWithEscapedChar)
{
  std::string input = "run\"s\btest\n";

  // Ensure we use US keyboard
  CPC.kbd_layout = 0;
  auto tmp = cvk->StringToEvents(input);
  std::vector<SDL_Event> result(tmp.begin(), tmp.end());

  ASSERT_EQ(22, result.size());

  ASSERT_EQ(SDLK_n, result[5].key.keysym.sym);
  // On US keyboard, " is on ' key with shift pressed
  ASSERT_EQ(SDLK_QUOTE, result[6].key.keysym.sym);
  ASSERT_EQ(SDLK_s, result[9].key.keysym.sym);
  ASSERT_EQ(SDLK_BACKSPACE, result[10].key.keysym.sym);
  ASSERT_EQ(SDLK_t, result[19].key.keysym.sym);
  ASSERT_EQ(SDLK_RETURN, result[20].key.keysym.sym);
}

TEST_F(CapriceVKeyboardTest, StringToEventsWithSpecialChar)
{
  std::string input = "\a";
  input += CPC_ESC;

  auto tmp = cvk->StringToEvents(input);
  std::vector<SDL_Event> result(tmp.begin(), tmp.end());

  ASSERT_EQ(2, result.size());

  // First key event is pressing ESCAPE
  ASSERT_EQ(SDLK_ESCAPE, result[0].key.keysym.sym);
  ASSERT_EQ(KMOD_NONE, result[0].key.keysym.mod);
  ASSERT_EQ(SDL_KEYDOWN, result[0].key.type);
  ASSERT_EQ(SDL_PRESSED, result[0].key.state);
  // Second key event is releasing ESCAPE
  ASSERT_EQ(SDLK_ESCAPE, result[0].key.keysym.sym);
  ASSERT_EQ(KMOD_NONE, result[1].key.keysym.mod);
  ASSERT_EQ(SDL_KEYUP, result[1].key.type);
  ASSERT_EQ(SDL_RELEASED, result[1].key.state);
}
