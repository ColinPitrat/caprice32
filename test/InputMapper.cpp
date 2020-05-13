#include <gtest/gtest.h>

#include "cap32.h"
#include "keyboard.h"
#include <string>

extern t_CPC CPC;

class InputMapperTest : public testing::Test {
  public:
    void SetUp() {
      CPC.resources_path = "resources";
	  CPC.InputMapper = new InputMapper(&CPC);
	  CPC.InputMapper->init();
    }
};

TEST_F(InputMapperTest, StringToEventsSimpleString)
{
  CPC.kbd_layout ="keymap_us.map";
  CPC.keyboard = 0;
  CPC.InputMapper->init();

  std::string input = "cat";

  auto tmp = CPC.InputMapper->StringToEvents(input);
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

TEST_F(InputMapperTest, StringToEventsWithEscapedChar)
{
  CPC.kbd_layout ="keymap_us.map";
  CPC.keyboard = 0;
  CPC.InputMapper->init();

  std::string input = "run\"s\btest\n";

  auto tmp = CPC.InputMapper->StringToEvents(input);
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

TEST_F(InputMapperTest, StringToEventsWithSpecialChar)
{
  CPC.kbd_layout ="keymap_us.map";
  CPC.keyboard = 0;
  CPC.InputMapper->init();

  std::string input = "\a";
  input += CPC_ESC;

  auto tmp = CPC.InputMapper->StringToEvents(input);
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

TEST_F(InputMapperTest, Keymapping)
{
  SDL_keysym keysym;

  CPC.kbd_layout ="keymap_us.map";
  CPC.keyboard = 0;
  CPC.InputMapper->init();
  // Exclaim
  keysym.sym = SDLK_1;
  keysym.mod = KMOD_LSHIFT;
  ASSERT_EQ(0x80 | MOD_CPC_SHIFT, CPC.InputMapper->CPCkeyFromKeysym(keysym));

  CPC.kbd_layout ="keymap_uk_linux.map";
  CPC.keyboard = 0;
  CPC.InputMapper->init();
  // Pound
  keysym.sym = SDLK_3;
  keysym.mod = KMOD_RSHIFT;
  ASSERT_EQ(0x30 | MOD_CPC_SHIFT, CPC.InputMapper->CPCkeyFromKeysym(keysym));

  CPC.kbd_layout ="keymap_fr_win.map";
  CPC.keyboard = 1;
  CPC.InputMapper->init();
  // E acute
  keysym.sym = SDLK_2;
  keysym.mod = KMOD_NONE;
  ASSERT_EQ(0x81, CPC.InputMapper->CPCkeyFromKeysym(keysym));

  CPC.kbd_layout ="keymap_es_linux.map";
  CPC.keyboard = 2;
  CPC.InputMapper->init();
  // N Tilde
  keysym.sym = SDLK_WORLD_81;
  keysym.mod = KMOD_LSHIFT;
  ASSERT_EQ(0x35 | MOD_CPC_SHIFT, CPC.InputMapper->CPCkeyFromKeysym(keysym));


}
