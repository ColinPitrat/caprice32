#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "types.h"
#include "SDL.h"
#include <map>
#include <list>
#include <string>
#include "cap32.h"

#define MOD_CPC_SHIFT   (0x01 << 8)
#define MOD_CPC_CTRL    (0x02 << 8)
#define MOD_EMU_KEY     (0x10 << 8)

#define BITSHIFT_MOD 32
#define BITMASK_NOMOD ((static_cast<PCKey>(1)<<BITSHIFT_MOD) - 1)
#define MOD_PC_SHIFT    (static_cast<PCKey>(KMOD_SHIFT) << BITSHIFT_MOD)
#define MOD_PC_CTRL     (static_cast<PCKey>(KMOD_CTRL) << BITSHIFT_MOD)
#define MOD_PC_MODE     (static_cast<PCKey>(KMOD_MODE) << BITSHIFT_MOD)
// MOD_PC_ALT shouldn't be used: SDLK_LALT is mapped as a non-modifier key to CPC_COPY.
#define MOD_PC_ALT      (static_cast<PCKey>(KMOD_LALT) << BITSHIFT_MOD)
#define MOD_PC_NUM      (static_cast<PCKey>(KMOD_NUM) << BITSHIFT_MOD)
#define MOD_PC_CAPS     (static_cast<PCKey>(KMOD_CAPS) << BITSHIFT_MOD)

typedef enum {
   CAP32_EXIT = MOD_EMU_KEY,
   CAP32_FPS,
   CAP32_FULLSCRN,
   CAP32_GUI,
   CAP32_VKBD,
   CAP32_JOY,
   CAP32_PHAZER,
   CAP32_MF2STOP,
   CAP32_RESET,
   CAP32_SCRNSHOT,
   CAP32_SPEED,
   CAP32_TAPEPLAY,
   CAP32_DEBUG,
   CAP32_SNAPSHOT,
   CAP32_LD_SNAP,
   CAP32_WAITBREAK,
   CAP32_DELAY,
   CAP32_PASTE,
   CAP32_DEVTOOLS,
   CAP32_NEXTDISKA
} CAP32_KEYS;

typedef enum {
   CPC_0,
   CPC_1,
   CPC_2,
   CPC_3,
   CPC_4,
   CPC_5,
   CPC_6,
   CPC_7,
   CPC_8,
   CPC_9,
   CPC_A,
   CPC_B,
   CPC_C,
   CPC_D,
   CPC_E,
   CPC_F,
   CPC_G,
   CPC_H,
   CPC_I,
   CPC_J,
   CPC_K,
   CPC_L,
   CPC_M,
   CPC_N,
   CPC_O,
   CPC_P,
   CPC_Q,
   CPC_R,
   CPC_S,
   CPC_T,
   CPC_U,
   CPC_V,
   CPC_W,
   CPC_X,
   CPC_Y,
   CPC_Z,
   CPC_a,
   CPC_b,
   CPC_c,
   CPC_d,
   CPC_e,
   CPC_f,
   CPC_g,
   CPC_h,
   CPC_i,
   CPC_j,
   CPC_k,
   CPC_l,
   CPC_m,
   CPC_n,
   CPC_o,
   CPC_p,
   CPC_q,
   CPC_r,
   CPC_s,
   CPC_t,
   CPC_u,
   CPC_v,
   CPC_w,
   CPC_x,
   CPC_y,
   CPC_z,
   CPC_CTRL_a,
   CPC_CTRL_b,
   CPC_CTRL_c,
   CPC_CTRL_d,
   CPC_CTRL_e,
   CPC_CTRL_f,
   CPC_CTRL_g,
   CPC_CTRL_h,
   CPC_CTRL_i,
   CPC_CTRL_j,
   CPC_CTRL_k,
   CPC_CTRL_l,
   CPC_CTRL_m,
   CPC_CTRL_n,
   CPC_CTRL_o,
   CPC_CTRL_p,
   CPC_CTRL_q,
   CPC_CTRL_r,
   CPC_CTRL_s,
   CPC_CTRL_t,
   CPC_CTRL_u,
   CPC_CTRL_v,
   CPC_CTRL_w,
   CPC_CTRL_x,
   CPC_CTRL_y,
   CPC_CTRL_z,
   CPC_CTRL_0,
   CPC_CTRL_1,
   CPC_CTRL_2,
   CPC_CTRL_3,
   CPC_CTRL_4,
   CPC_CTRL_5,
   CPC_CTRL_6,
   CPC_CTRL_7,
   CPC_CTRL_8,
   CPC_CTRL_9,
   CPC_CTRL_UP,
   CPC_CTRL_DOWN,
   CPC_CTRL_LEFT,
   CPC_CTRL_RIGHT,
   CPC_AMPERSAND,
   CPC_ASTERISK,
   CPC_AT,
   CPC_BACKQUOTE,
   CPC_BACKSLASH,
   CPC_CAPSLOCK,
   CPC_CLR,
   CPC_COLON,
   CPC_COMMA,
   CPC_CONTROL,
   CPC_COPY,
   CPC_CPY_DOWN,
   CPC_CPY_LEFT,
   CPC_CPY_RIGHT,
   CPC_CPY_UP,
   CPC_CUR_DOWN,
   CPC_CUR_LEFT,
   CPC_CUR_RIGHT,
   CPC_CUR_UP,
   CPC_CUR_ENDBL,
   CPC_CUR_HOMELN,
   CPC_CUR_ENDLN,
   CPC_CUR_HOMEBL,
   CPC_DBLQUOTE,
   CPC_DEL,
   CPC_DOLLAR,
   CPC_ENTER,
   CPC_EQUAL,
   CPC_ESC,
   CPC_EXCLAMATN,
   CPC_F0,
   CPC_F1,
   CPC_F2,
   CPC_F3,
   CPC_F4,
   CPC_F5,
   CPC_F6,
   CPC_F7,
   CPC_F8,
   CPC_F9,
   CPC_CTRL_F0,
   CPC_CTRL_F1,
   CPC_CTRL_F2,
   CPC_CTRL_F3,
   CPC_CTRL_F4,
   CPC_CTRL_F5,
   CPC_CTRL_F6,
   CPC_CTRL_F7,
   CPC_CTRL_F8,
   CPC_CTRL_F9,
   CPC_SHIFT_F0,
   CPC_SHIFT_F1,
   CPC_SHIFT_F2,
   CPC_SHIFT_F3,
   CPC_SHIFT_F4,
   CPC_SHIFT_F5,
   CPC_SHIFT_F6,
   CPC_SHIFT_F7,
   CPC_SHIFT_F8,
   CPC_SHIFT_F9,
   CPC_FPERIOD,
   CPC_GREATER,
   CPC_HASH,
   CPC_LBRACKET,
   CPC_LCBRACE,
   CPC_LEFTPAREN,
   CPC_LESS,
   CPC_LSHIFT,
   CPC_MINUS,
   CPC_PERCENT,
   CPC_PERIOD,
   CPC_PIPE,
   CPC_PLUS,
   CPC_POUND,
   CPC_POWER,
   CPC_QUESTION,
   CPC_QUOTE,
   CPC_RBRACKET,
   CPC_RCBRACE,
   CPC_RETURN,
   CPC_RIGHTPAREN,
   CPC_RSHIFT,
   CPC_SEMICOLON,
   CPC_SLASH,
   CPC_SPACE,
   CPC_TAB,
   CPC_UNDERSCORE,
   CPC_J0_UP,
   CPC_J0_DOWN,
   CPC_J0_LEFT,
   CPC_J0_RIGHT,
   CPC_J0_FIRE1,
   CPC_J0_FIRE2,
   CPC_J1_UP,
   CPC_J1_DOWN,
   CPC_J1_LEFT,
   CPC_J1_RIGHT,
   CPC_J1_FIRE1,
   CPC_J1_FIRE2,
   CPC_ES_NTILDE,
   CPC_ES_nTILDE,
   CPC_ES_PESETA,
   CPC_FR_eACUTE,
   CPC_FR_eGRAVE,
   CPC_FR_cCEDIL,
   CPC_FR_aGRAVE,
   CPC_FR_uGRAVE
} CPC_KEYS;

#define CPC_KEY_NUM 209    // Number of different keys on a CPC keyboard
#define CPC_KEYBOARD_NUM 3 // Number of different keyboards supported.

// PCKey represents a key combination pressed on the host keyboard.
// This is a modifier (high dword) and a pressed key (low dword).
using PCKey = qword;

// CapriceKey represents a host-agnostic representation of a keyboard event.
// This can be either a CPC key combination (CPC_KEYS) or an emulator command (CAP32_KEYS).
using CapriceKey = unsigned int;

// CPCScancode is a hardware scancode.
// cf. https://www.cpcwiki.eu/index.php/Programming:Keyboard_scanning#Hardware_scancode_table
using CPCScancode = dword;

void applyKeypress(CPCScancode cpc_key, byte keyboard_matrix[], bool pressed);

class LineParsingResult {
  public:
    bool valid = true;
    bool contains_mapping = false;
    CapriceKey cpc_key = 0;
    PCKey sdl_key = 0;
    std::string cpc_key_name;
    std::string sdl_key_name;
};

// CPCScancode is a hardware scancode.
// cf. https://www.cpcwiki.eu/index.php/Programming:Keyboard_scanning#Hardware_scancode_table
using CPCScancode = dword;

class InputMapper {
  private:
    static const CPCScancode cpc_kbd[CPC_KEYBOARD_NUM][CPC_KEY_NUM];
    static const std::map<const std::string, const CapriceKey> CPCkeysFromStrings;
    static const std::map<const std::string, const PCKey> SDLkeysFromStrings;
    static const std::map<const char, const CPC_KEYS> CPCkeysFromChars;
    std::map<char, std::pair<SDL_Keycode, SDL_Keymod>> SDLkeysFromChars;
    static std::map<CapriceKey, PCKey> SDLkeysymFromCPCkeys_us;
    std::map<PCKey, CapriceKey> CPCkeysFromSDLkeysym;
    std::map<CapriceKey, PCKey> SDLkeysymFromCPCkeys;
    t_CPC *CPC;

    LineParsingResult process_cfg_line(char *line);

  public:
    InputMapper(t_CPC *CPC);
    bool load_layout(const std::string& filename);
    void init();
    CPCScancode CPCscancodeFromCPCkey(CPC_KEYS cpc_key);
    CPCScancode CPCscancodeFromKeysym(SDL_Keysym keysym);
    CapriceKey CPCkeyFromKeysym(SDL_Keysym keysym);
    std::string CPCkeyToString(const CapriceKey cpc_key);
    CPCScancode CPCscancodeFromJoystickButton(SDL_JoyButtonEvent jbutton);
    void CPCscancodeFromJoystickAxis(SDL_JoyAxisEvent jaxis, CPCScancode *cpc_key, bool &release);
    std::list<SDL_Event> StringToEvents(std::string toTranslate);
    void set_joystick_emulation();
};

#endif
