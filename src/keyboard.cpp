#include "keyboard.h"
#include <iostream>
#include <fstream>
#include "cap32.h"
#include "fileutils.h"
#include "log.h"

extern byte bit_values[8];
extern t_CPC CPC;

const dword InputMapper::cpc_kbd[CPC_KEYBOARD_NUM][CPC_KEY_NUM] = {
  { // original CPC keyboard
    0x40,                   // CPC_0
    0x80,                   // CPC_1
    0x81,                   // CPC_2
    0x71,                   // CPC_3
    0x70,                   // CPC_4
    0x61,                   // CPC_5
    0x60,                   // CPC_6
    0x51,                   // CPC_7
    0x50,                   // CPC_8
    0x41,                   // CPC_9
    0x85 | MOD_CPC_SHIFT,   // CPC_A
    0x66 | MOD_CPC_SHIFT,   // CPC_B
    0x76 | MOD_CPC_SHIFT,   // CPC_C
    0x75 | MOD_CPC_SHIFT,   // CPC_D
    0x72 | MOD_CPC_SHIFT,   // CPC_E
    0x65 | MOD_CPC_SHIFT,   // CPC_F
    0x64 | MOD_CPC_SHIFT,   // CPC_G
    0x54 | MOD_CPC_SHIFT,   // CPC_H
    0x43 | MOD_CPC_SHIFT,   // CPC_I
    0x55 | MOD_CPC_SHIFT,   // CPC_J
    0x45 | MOD_CPC_SHIFT,   // CPC_K
    0x44 | MOD_CPC_SHIFT,   // CPC_L
    0x46 | MOD_CPC_SHIFT,   // CPC_M
    0x56 | MOD_CPC_SHIFT,   // CPC_N
    0x42 | MOD_CPC_SHIFT,   // CPC_O
    0x33 | MOD_CPC_SHIFT,   // CPC_P
    0x83 | MOD_CPC_SHIFT,   // CPC_Q
    0x62 | MOD_CPC_SHIFT,   // CPC_R
    0x74 | MOD_CPC_SHIFT,   // CPC_S
    0x63 | MOD_CPC_SHIFT,   // CPC_T
    0x52 | MOD_CPC_SHIFT,   // CPC_U
    0x67 | MOD_CPC_SHIFT,   // CPC_V
    0x73 | MOD_CPC_SHIFT,   // CPC_W
    0x77 | MOD_CPC_SHIFT,   // CPC_X
    0x53 | MOD_CPC_SHIFT,   // CPC_Y
    0x87 | MOD_CPC_SHIFT,   // CPC_Z
    0x85,                   // CPC_a
    0x66,                   // CPC_b
    0x76,                   // CPC_c
    0x75,                   // CPC_d
    0x72,                   // CPC_e
    0x65,                   // CPC_f
    0x64,                   // CPC_g
    0x54,                   // CPC_h
    0x43,                   // CPC_i
    0x55,                   // CPC_j
    0x45,                   // CPC_k
    0x44,                   // CPC_l
    0x46,                   // CPC_m
    0x56,                   // CPC_n
    0x42,                   // CPC_o
    0x33,                   // CPC_p
    0x83,                   // CPC_q
    0x62,                   // CPC_r
    0x74,                   // CPC_s
    0x63,                   // CPC_t
    0x52,                   // CPC_u
    0x67,                   // CPC_v
    0x73,                   // CPC_w
    0x77,                   // CPC_x
    0x53,                   // CPC_y
    0x87,                   // CPC_z
    0x60 | MOD_CPC_SHIFT,   // CPC_AMPERSAND
    0x35 | MOD_CPC_SHIFT,   // CPC_ASTERISK
    0x32,                   // CPC_AT
    0x26 | MOD_CPC_SHIFT,   // CPC_BACKQUOTE
    0x26,                   // CPC_BACKSLASH
    0x86,                   // CPC_CAPSLOCK
    0x20,                   // CPC_CLR
    0x35,                   // CPC_COLON
    0x47,                   // CPC_COMMA
    0x27,                   // CPC_CONTROL
    0x11,                   // CPC_COPY
    0x02 | MOD_CPC_SHIFT,   // CPC_CPY_DOWN
    0x10 | MOD_CPC_SHIFT,   // CPC_CPY_LEFT
    0x01 | MOD_CPC_SHIFT,   // CPC_CPY_RIGHT
    0x00 | MOD_CPC_SHIFT,   // CPC_CPY_UP
    0x02,                   // CPC_CUR_DOWN
    0x10,                   // CPC_CUR_LEFT
    0x01,                   // CPC_CUR_RIGHT
    0x00,                   // CPC_CUR_UP
    0x02 | MOD_CPC_CTRL,    // CPC_CUR_ENDBL
    0x10 | MOD_CPC_CTRL,    // CPC_CUR_HOMELN
    0x01 | MOD_CPC_CTRL,    // CPC_CUR_ENDLN
    0x00 | MOD_CPC_CTRL,    // CPC_CUR_HOMEBL
    0x81 | MOD_CPC_SHIFT,   // CPC_DBLQUOTE
    0x97,                   // CPC_DEL
    0x70 | MOD_CPC_SHIFT,   // CPC_DOLLAR
    0x06,                   // CPC_ENTER
    0x31 | MOD_CPC_SHIFT,   // CPC_EQUAL
    0x82,                   // CPC_ESC
    0x80 | MOD_CPC_SHIFT,   // CPC_EXCLAMATN
    0x17,                   // CPC_F0
    0x15,                   // CPC_F1
    0x16,                   // CPC_F2
    0x05,                   // CPC_F3
    0x24,                   // CPC_F4
    0x14,                   // CPC_F5
    0x04,                   // CPC_F6
    0x12,                   // CPC_F7
    0x13,                   // CPC_F8
    0x03,                   // CPC_F9
    0x07,                   // CPC_FPERIOD
    0x37 | MOD_CPC_SHIFT,   // CPC_GREATER
    0x71 | MOD_CPC_SHIFT,   // CPC_HASH
    0x21,                   // CPC_LBRACKET
    0x21 | MOD_CPC_SHIFT,   // CPC_LCBRACE
    0x50 | MOD_CPC_SHIFT,   // CPC_LEFTPAREN
    0x47 | MOD_CPC_SHIFT,   // CPC_LESS
    0x25,                   // CPC_LSHIFT
    0x31,                   // CPC_MINUS
    0x61 | MOD_CPC_SHIFT,   // CPC_PERCENT
    0x37,                   // CPC_PERIOD
    0x32 | MOD_CPC_SHIFT,   // CPC_PIPE
    0x34 | MOD_CPC_SHIFT,   // CPC_PLUS
    0x30 | MOD_CPC_SHIFT,   // CPC_POUND
    0x30,                   // CPC_POWER
    0x36 | MOD_CPC_SHIFT,   // CPC_QUESTION
    0x51 | MOD_CPC_SHIFT,   // CPC_QUOTE
    0x23,                   // CPC_RBRACKET
    0x23 | MOD_CPC_SHIFT,   // CPC_RCBRACE
    0x22,                   // CPC_RETURN
    0x41 | MOD_CPC_SHIFT,   // CPC_RIGHTPAREN
    0x25,                   // CPC_RSHIFT
    0x34,                   // CPC_SEMICOLON
    0x36,                   // CPC_SLASH
    0x57,                   // CPC_SPACE
    0x84,                   // CPC_TAB
    0x40 | MOD_CPC_SHIFT,   // CPC_UNDERSCORE
    0x90,                   // CPC_J0_UP
    0x91,                   // CPC_J0_DOWN
    0x92,                   // CPC_J0_LEFT
    0x93,                   // CPC_J0_RIGHT
    0x94,                   // CPC_J0_FIRE1
    0x95,                   // CPC_J0_FIRE2
    0x60,                   // CPC_J1_UP
    0x61,                   // CPC_J1_DOWN
    0x62,                   // CPC_J1_LEFT
    0x63,                   // CPC_J1_RIGHT
    0x64,                   // CPC_J1_FIRE1
    0x65,                   // CPC_J1_FIRE2
    0xff,                   // CPC_ES_NTILDE
    0xff,                   // CPC_ES_nTILDE
    0xff,                   // CPC_ES_PESETA
    0xff,                   // CPC_FR_eACUTE
    0xff,                   // CPC_FR_eGRAVE
    0xff,                   // CPC_FR_cCEDIL
    0xff,                   // CPC_FR_aGRAVE
    0xff,                   // CPC_FR_uGRAVE
  },
  { // French CPC keyboard
    0x40 | MOD_CPC_SHIFT,   // CPC_0
    0x80 | MOD_CPC_SHIFT,   // CPC_1
    0x81 | MOD_CPC_SHIFT,   // CPC_2
    0x71 | MOD_CPC_SHIFT,   // CPC_3
    0x70 | MOD_CPC_SHIFT,   // CPC_4
    0x61 | MOD_CPC_SHIFT,   // CPC_5
    0x60 | MOD_CPC_SHIFT,   // CPC_6
    0x51 | MOD_CPC_SHIFT,   // CPC_7
    0x50 | MOD_CPC_SHIFT,   // CPC_8
    0x41 | MOD_CPC_SHIFT,   // CPC_9
    0x83 | MOD_CPC_SHIFT,   // CPC_A
    0x66 | MOD_CPC_SHIFT,   // CPC_B
    0x76 | MOD_CPC_SHIFT,   // CPC_C
    0x75 | MOD_CPC_SHIFT,   // CPC_D
    0x72 | MOD_CPC_SHIFT,   // CPC_E
    0x65 | MOD_CPC_SHIFT,   // CPC_F
    0x64 | MOD_CPC_SHIFT,   // CPC_G
    0x54 | MOD_CPC_SHIFT,   // CPC_H
    0x43 | MOD_CPC_SHIFT,   // CPC_I
    0x55 | MOD_CPC_SHIFT,   // CPC_J
    0x45 | MOD_CPC_SHIFT,   // CPC_K
    0x44 | MOD_CPC_SHIFT,   // CPC_L
    0x35 | MOD_CPC_SHIFT,   // CPC_M
    0x56 | MOD_CPC_SHIFT,   // CPC_N
    0x42 | MOD_CPC_SHIFT,   // CPC_O
    0x33 | MOD_CPC_SHIFT,   // CPC_P
    0x85 | MOD_CPC_SHIFT,   // CPC_Q
    0x62 | MOD_CPC_SHIFT,   // CPC_R
    0x74 | MOD_CPC_SHIFT,   // CPC_S
    0x63 | MOD_CPC_SHIFT,   // CPC_T
    0x52 | MOD_CPC_SHIFT,   // CPC_U
    0x67 | MOD_CPC_SHIFT,   // CPC_V
    0x87 | MOD_CPC_SHIFT,   // CPC_W
    0x77 | MOD_CPC_SHIFT,   // CPC_X
    0x53 | MOD_CPC_SHIFT,   // CPC_Y
    0x73 | MOD_CPC_SHIFT,   // CPC_Z
    0x83,                   // CPC_a
    0x66,                   // CPC_b
    0x76,                   // CPC_c
    0x75,                   // CPC_d
    0x72,                   // CPC_e
    0x65,                   // CPC_f
    0x64,                   // CPC_g
    0x54,                   // CPC_h
    0x43,                   // CPC_i
    0x55,                   // CPC_j
    0x45,                   // CPC_k
    0x44,                   // CPC_l
    0x35,                   // CPC_m
    0x56,                   // CPC_n
    0x42,                   // CPC_o
    0x33,                   // CPC_p
    0x85,                   // CPC_q
    0x62,                   // CPC_r
    0x74,                   // CPC_s
    0x63,                   // CPC_t
    0x52,                   // CPC_u
    0x67,                   // CPC_v
    0x87,                   // CPC_w
    0x77,                   // CPC_x
    0x53,                   // CPC_y
    0x73,                   // CPC_z
    0x80,                   // CPC_AMPERSAND
    0x21,                   // CPC_ASTERISK
    0x26 | MOD_CPC_SHIFT,   // CPC_AT
    0xff,                   // CPC_BACKQUOTE
    0x26 | MOD_CPC_CTRL,    // CPC_BACKSLASH
    0x86,                   // CPC_CAPSLOCK
    0x20,                   // CPC_CLR
    0x37,                   // CPC_COLON
    0x46,                   // CPC_COMMA
    0x27,                   // CPC_CONTROL
    0x11,                   // CPC_COPY
    0x02 | MOD_CPC_SHIFT,   // CPC_CPY_DOWN
    0x10 | MOD_CPC_SHIFT,   // CPC_CPY_LEFT
    0x01 | MOD_CPC_SHIFT,   // CPC_CPY_RIGHT
    0x00 | MOD_CPC_SHIFT,   // CPC_CPY_UP
    0x02,                   // CPC_CUR_DOWN
    0x10,                   // CPC_CUR_LEFT
    0x01,                   // CPC_CUR_RIGHT
    0x00,                   // CPC_CUR_UP
    0x02 | MOD_CPC_CTRL,    // CPC_CUR_ENDBL
    0x10 | MOD_CPC_CTRL,    // CPC_CUR_HOMELN
    0x01 | MOD_CPC_CTRL,    // CPC_CUR_ENDLN
    0x00 | MOD_CPC_CTRL,    // CPC_CUR_HOMEBL
    0x71,                   // CPC_DBLQUOTE
    0x97,                   // CPC_DEL
    0x26,                   // CPC_DOLLAR
    0x06,                   // CPC_ENTER
    0x36,                   // CPC_EQUAL
    0x82,                   // CPC_ESC
    0x50,                   // CPC_EXCLAMATN
    0x17,                   // CPC_F0
    0x15,                   // CPC_F1
    0x16,                   // CPC_F2
    0x05,                   // CPC_F3
    0x24,                   // CPC_F4
    0x14,                   // CPC_F5
    0x04,                   // CPC_F6
    0x12,                   // CPC_F7
    0x13,                   // CPC_F8
    0x03,                   // CPC_F9
    0x07,                   // CPC_FPERIOD
    0x23 | MOD_CPC_SHIFT,   // CPC_GREATER
    0x23,                   // CPC_HASH
    0x31 | MOD_CPC_SHIFT,   // CPC_LBRACKET
    0xff,                   // CPC_LCBRACE
    0x61,                   // CPC_LEFTPAREN
    0x21 | MOD_CPC_SHIFT,   // CPC_LESS
    0x25,                   // CPC_LSHIFT
    0x30,                   // CPC_MINUS
    0x34 | MOD_CPC_SHIFT,   // CPC_PERCENT
    0x47 | MOD_CPC_SHIFT,   // CPC_PERIOD
    0x32 | MOD_CPC_SHIFT,   // CPC_PIPE
    0x36 | MOD_CPC_SHIFT,   // CPC_PLUS
    0xff,                   // CPC_POUND
    0x32,                   // CPC_POWER
    0x46 | MOD_CPC_SHIFT,   // CPC_QUESTION
    0x70,                   // CPC_QUOTE
    0x60,                   // CPC_RBRACKET
    0xff,                   // CPC_RCBRACE
    0x22,                   // CPC_RETURN
    0x31,                   // CPC_RIGHTPAREN
    0x25,                   // CPC_RSHIFT
    0x47,                   // CPC_SEMICOLON
    0x37 | MOD_CPC_SHIFT,   // CPC_SLASH
    0x57,                   // CPC_SPACE
    0x84,                   // CPC_TAB
    0x30 | MOD_CPC_SHIFT,   // CPC_UNDERSCORE
    0x90,                   // CPC_J0_UP
    0x91,                   // CPC_J0_DOWN
    0x92,                   // CPC_J0_LEFT
    0x93,                   // CPC_J0_RIGHT
    0x94,                   // CPC_J0_FIRE1
    0x95,                   // CPC_J0_FIRE2
    0x60,                   // CPC_J1_UP
    0x61,                   // CPC_J1_DOWN
    0x62,                   // CPC_J1_LEFT
    0x63,                   // CPC_J1_RIGHT
    0x64,                   // CPC_J1_FIRE1
    0x65,                   // CPC_J1_FIRE2
    0xff,                   // CPC_ES_NTILDE
    0xff,                   // CPC_ES_nTILDE
    0xff,                   // CPC_ES_PESETA
    0x81,                   // CPC_FR_eACUTE
    0x51,                   // CPC_FR_eGRAVE
    0x41,                   // CPC_FR_cCEDIL
    0x40,                   // CPC_FR_aGRAVE
    0x34,                   // CPC_FR_uGRAVE
  },
  { // Spanish CPC keyboard
    0x40,                   // CPC_0
    0x80,                   // CPC_1
    0x81,                   // CPC_2
    0x71,                   // CPC_3
    0x70,                   // CPC_4
    0x61,                   // CPC_5
    0x60,                   // CPC_6
    0x51,                   // CPC_7
    0x50,                   // CPC_8
    0x41,                   // CPC_9
    0x85 | MOD_CPC_SHIFT,   // CPC_A
    0x66 | MOD_CPC_SHIFT,   // CPC_B
    0x76 | MOD_CPC_SHIFT,   // CPC_C
    0x75 | MOD_CPC_SHIFT,   // CPC_D
    0x72 | MOD_CPC_SHIFT,   // CPC_E
    0x65 | MOD_CPC_SHIFT,   // CPC_F
    0x64 | MOD_CPC_SHIFT,   // CPC_G
    0x54 | MOD_CPC_SHIFT,   // CPC_H
    0x43 | MOD_CPC_SHIFT,   // CPC_I
    0x55 | MOD_CPC_SHIFT,   // CPC_J
    0x45 | MOD_CPC_SHIFT,   // CPC_K
    0x44 | MOD_CPC_SHIFT,   // CPC_L
    0x46 | MOD_CPC_SHIFT,   // CPC_M
    0x56 | MOD_CPC_SHIFT,   // CPC_N
    0x42 | MOD_CPC_SHIFT,   // CPC_O
    0x33 | MOD_CPC_SHIFT,   // CPC_P
    0x83 | MOD_CPC_SHIFT,   // CPC_Q
    0x62 | MOD_CPC_SHIFT,   // CPC_R
    0x74 | MOD_CPC_SHIFT,   // CPC_S
    0x63 | MOD_CPC_SHIFT,   // CPC_T
    0x52 | MOD_CPC_SHIFT,   // CPC_U
    0x67 | MOD_CPC_SHIFT,   // CPC_V
    0x73 | MOD_CPC_SHIFT,   // CPC_W
    0x77 | MOD_CPC_SHIFT,   // CPC_X
    0x53 | MOD_CPC_SHIFT,   // CPC_Y
    0x87 | MOD_CPC_SHIFT,   // CPC_Z
    0x85,                   // CPC_a
    0x66,                   // CPC_b
    0x76,                   // CPC_c
    0x75,                   // CPC_d
    0x72,                   // CPC_e
    0x65,                   // CPC_f
    0x64,                   // CPC_g
    0x54,                   // CPC_h
    0x43,                   // CPC_i
    0x55,                   // CPC_j
    0x45,                   // CPC_k
    0x44,                   // CPC_l
    0x46,                   // CPC_m
    0x56,                   // CPC_n
    0x42,                   // CPC_o
    0x33,                   // CPC_p
    0x83,                   // CPC_q
    0x62,                   // CPC_r
    0x74,                   // CPC_s
    0x63,                   // CPC_t
    0x52,                   // CPC_u
    0x67,                   // CPC_v
    0x73,                   // CPC_w
    0x77,                   // CPC_x
    0x53,                   // CPC_y
    0x87,                   // CPC_z
    0x60 | MOD_CPC_SHIFT,   // CPC_AMPERSAND
    0x21 | MOD_CPC_SHIFT,   // CPC_ASTERISK
    0x32,                   // CPC_AT
    0x26 | MOD_CPC_SHIFT,   // CPC_BACKQUOTE
    0x26,                   // CPC_BACKSLASH
    0x86,                   // CPC_CAPSLOCK
    0x20,                   // CPC_CLR
    0x34 | MOD_CPC_SHIFT,   // CPC_COLON
    0x47,                   // CPC_COMMA
    0x27,                   // CPC_CONTROL
    0x11,                   // CPC_COPY
    0x02 | MOD_CPC_SHIFT,   // CPC_CPY_DOWN
    0x10 | MOD_CPC_SHIFT,   // CPC_CPY_LEFT
    0x01 | MOD_CPC_SHIFT,   // CPC_CPY_RIGHT
    0x00 | MOD_CPC_SHIFT,   // CPC_CPY_UP
    0x02,                   // CPC_CUR_DOWN
    0x10,                   // CPC_CUR_LEFT
    0x01,                   // CPC_CUR_RIGHT
    0x00,                   // CPC_CUR_UP
    0x02 | MOD_CPC_CTRL,    // CPC_CUR_ENDBL
    0x10 | MOD_CPC_CTRL,    // CPC_CUR_HOMELN
    0x01 | MOD_CPC_CTRL,    // CPC_CUR_ENDLN
    0x00 | MOD_CPC_CTRL,    // CPC_CUR_HOMEBL
    0x81 | MOD_CPC_SHIFT,   // CPC_DBLQUOTE
    0x97,                   // CPC_DEL
    0x70 | MOD_CPC_SHIFT,   // CPC_DOLLAR
    0x06,                   // CPC_ENTER
    0x31 | MOD_CPC_SHIFT,   // CPC_EQUAL
    0x82,                   // CPC_ESC
    0x80 | MOD_CPC_SHIFT,   // CPC_EXCLAMATN
    0x17,                   // CPC_F0
    0x15,                   // CPC_F1
    0x16,                   // CPC_F2
    0x05,                   // CPC_F3
    0x24,                   // CPC_F4
    0x14,                   // CPC_F5
    0x04,                   // CPC_F6
    0x12,                   // CPC_F7
    0x13,                   // CPC_F8
    0x03,                   // CPC_F9
    0x07,                   // CPC_FPERIOD
    0x37 | MOD_CPC_SHIFT,   // CPC_GREATER
    0x71 | MOD_CPC_SHIFT,   // CPC_HASH
    0x21,                   // CPC_LBRACKET
    0xff,                   // CPC_LCBRACE
    0x50 | MOD_CPC_SHIFT,   // CPC_LEFTPAREN
    0x47 | MOD_CPC_SHIFT,   // CPC_LESS
    0x25,                   // CPC_LSHIFT
    0x31,                   // CPC_MINUS
    0x61 | MOD_CPC_SHIFT,   // CPC_PERCENT
    0x37,                   // CPC_PERIOD
    0x32 | MOD_CPC_SHIFT,   // CPC_PIPE
    0x23 | MOD_CPC_SHIFT,   // CPC_PLUS
    0xff,                   // CPC_POUND
    0x30,                   // CPC_POWER
    0x36 | MOD_CPC_SHIFT,   // CPC_QUESTION
    0x51 | MOD_CPC_SHIFT,   // CPC_QUOTE
    0x23,                   // CPC_RBRACKET
    0xff,                   // CPC_RCBRACE
    0x22,                   // CPC_RETURN
    0x41 | MOD_CPC_SHIFT,   // CPC_RIGHTPAREN
    0x25,                   // CPC_RSHIFT
    0x34,                   // CPC_SEMICOLON
    0x36,                   // CPC_SLASH
    0x57,                   // CPC_SPACE
    0x84,                   // CPC_TAB
    0x40 | MOD_CPC_SHIFT,   // CPC_UNDERSCORE
    0x90,                   // CPC_J0_UP
    0x91,                   // CPC_J0_DOWN
    0x92,                   // CPC_J0_LEFT
    0x93,                   // CPC_J0_RIGHT
    0x94,                   // CPC_J0_FIRE1
    0x95,                   // CPC_J0_FIRE2
    0x60,                   // CPC_J1_UP
    0x61,                   // CPC_J1_DOWN
    0x62,                   // CPC_J1_LEFT
    0x63,                   // CPC_J1_RIGHT
    0x64,                   // CPC_J1_FIRE1
    0x65,                   // CPC_J1_FIRE2
    0x35 | MOD_CPC_SHIFT,   // CPC_ES_NTILDE
    0x35,                   // CPC_ES_nTILDE
    0x30 | MOD_CPC_SHIFT,   // CPC_ES_PESETA
    0xff,                   // CPC_FR_eACUTE
    0xff,                   // CPC_FR_eGRAVE
    0xff,                   // CPC_FR_cCEDIL
    0xff,                   // CPC_FR_aGRAVE
    0xff,                   // CPC_FR_uGRAVE
  }
};


const std::map<const char, const CPC_KEYS> InputMapper::CPCkeysFromChars = {
    // Char to CPC keyboard translation
    // TODO(sebhz): Need to map non ASCII chars present on the CPC keyboard - maybe by using their ISO-8859-1 code
    { '&', CPC_AMPERSAND },
    { '#', CPC_HASH },
    { '"', CPC_DBLQUOTE },
    { '\'', CPC_BACKSLASH },
    { '(', CPC_LEFTPAREN },
    { '-', CPC_MINUS },
    { '_', CPC_UNDERSCORE },
    { ')', CPC_RIGHTPAREN },
    { '=', CPC_EQUAL },
    { '*', CPC_ASTERISK },
    { ',', CPC_COMMA },
    { ';', CPC_SEMICOLON },
    { ':', CPC_COLON },
    { '!', CPC_EXCLAMATN },
    { '$', CPC_DOLLAR },
    { 'a', CPC_a },
    { 'b', CPC_b },
    { 'c', CPC_c },
    { 'd', CPC_d },
    { 'e', CPC_e },
    { 'f', CPC_f },
    { 'g', CPC_g },
    { 'h', CPC_h },
    { 'i', CPC_i },
    { 'j', CPC_j },
    { 'k', CPC_k },
    { 'l', CPC_l },
    { 'm', CPC_m },
    { 'n', CPC_n },
    { 'o', CPC_o },
    { 'p', CPC_p },
    { 'q', CPC_q },
    { 'r', CPC_r },
    { 's', CPC_s },
    { 't', CPC_t },
    { 'u', CPC_u },
    { 'v', CPC_v },
    { 'w', CPC_w },
    { 'x', CPC_x },
    { 'y', CPC_y },
    { 'z', CPC_z },
    { 'A', CPC_A },
    { 'B', CPC_B },
    { 'C', CPC_C },
    { 'D', CPC_D },
    { 'E', CPC_E },
    { 'F', CPC_F },
    { 'G', CPC_G },
    { 'H', CPC_H },
    { 'I', CPC_I },
    { 'J', CPC_J },
    { 'K', CPC_K },
    { 'L', CPC_L },
    { 'M', CPC_M },
    { 'N', CPC_N },
    { 'O', CPC_O },
    { 'P', CPC_P },
    { 'Q', CPC_Q },
    { 'R', CPC_R },
    { 'S', CPC_S },
    { 'T', CPC_T },
    { 'U', CPC_U },
    { 'V', CPC_V },
    { 'W', CPC_W },
    { 'X', CPC_X },
    { 'Y', CPC_Y },
    { 'Z', CPC_Z },
    { '0', CPC_0 },
    { '1', CPC_1 },
    { '2', CPC_2 },
    { '3', CPC_3 },
    { '4', CPC_4 },
    { '5', CPC_5 },
    { '6', CPC_6 },
    { '7', CPC_7 },
    { '8', CPC_8 },
    { '9', CPC_9 },
    { '|', CPC_PIPE },
    { '?', CPC_QUESTION},
    { '.', CPC_PERIOD },
    { '/', CPC_SLASH },
    { ' ', CPC_SPACE },
    { '\n', CPC_RETURN },
    { '+', CPC_PLUS },
    { '%', CPC_PERCENT },
    { '<', CPC_LESS },
    { '>', CPC_GREATER },
    { '[', CPC_LBRACKET },
    { ']', CPC_RBRACKET },
    { '{', CPC_LCBRACE },
    { '}', CPC_RCBRACE },
    { '\\', CPC_BACKSLASH },
    { '\b', CPC_DEL },
    { '`', CPC_BACKQUOTE },
    // Not (yet?) on virtual keyboard
    { '@', CPC_AT },
    { '^', CPC_POWER },
    //{ '~', {0, KMOD_NONE} } // should be pound but it's not part of base ascii (it's in extended ASCII)
};

std::map<unsigned int, unsigned int> InputMapper::SDLkeysymFromCPCkeys_us = {
  { CPC_0,           SDLK_0 },
  { CPC_1,           SDLK_1 },
  { CPC_2,           SDLK_2 },
  { CPC_3,           SDLK_3 },
  { CPC_4,           SDLK_4 },
  { CPC_5,           SDLK_5 },
  { CPC_6,           SDLK_6 },
  { CPC_7,           SDLK_7 },
  { CPC_8,           SDLK_8 },
  { CPC_9,           SDLK_9 },
  { CPC_A,           SDLK_a | MOD_PC_SHIFT },
  { CPC_B,           SDLK_b | MOD_PC_SHIFT },
  { CPC_C,           SDLK_c | MOD_PC_SHIFT },
  { CPC_D,           SDLK_d | MOD_PC_SHIFT },
  { CPC_E,           SDLK_e | MOD_PC_SHIFT },
  { CPC_F,           SDLK_f | MOD_PC_SHIFT },
  { CPC_G,           SDLK_g | MOD_PC_SHIFT },
  { CPC_H,           SDLK_h | MOD_PC_SHIFT },
  { CPC_I,           SDLK_i | MOD_PC_SHIFT },
  { CPC_J,           SDLK_j | MOD_PC_SHIFT },
  { CPC_K,           SDLK_k | MOD_PC_SHIFT },
  { CPC_L,           SDLK_l | MOD_PC_SHIFT },
  { CPC_M,           SDLK_m | MOD_PC_SHIFT },
  { CPC_N,           SDLK_n | MOD_PC_SHIFT },
  { CPC_O,           SDLK_o | MOD_PC_SHIFT },
  { CPC_P,           SDLK_p | MOD_PC_SHIFT },
  { CPC_Q,           SDLK_q | MOD_PC_SHIFT },
  { CPC_R,           SDLK_r | MOD_PC_SHIFT },
  { CPC_S,           SDLK_s | MOD_PC_SHIFT },
  { CPC_T,           SDLK_t | MOD_PC_SHIFT },
  { CPC_U,           SDLK_u | MOD_PC_SHIFT },
  { CPC_V,           SDLK_v | MOD_PC_SHIFT },
  { CPC_W,           SDLK_w | MOD_PC_SHIFT },
  { CPC_X,           SDLK_x | MOD_PC_SHIFT },
  { CPC_Y,           SDLK_y | MOD_PC_SHIFT },
  { CPC_Z,           SDLK_z | MOD_PC_SHIFT },
  { CPC_a,           SDLK_a },
  { CPC_b,           SDLK_b },
  { CPC_c,           SDLK_c },
  { CPC_d,           SDLK_d },
  { CPC_e,           SDLK_e },
  { CPC_f,           SDLK_f },
  { CPC_g,           SDLK_g },
  { CPC_h,           SDLK_h },
  { CPC_i,           SDLK_i },
  { CPC_j,           SDLK_j },
  { CPC_k,           SDLK_k },
  { CPC_l,           SDLK_l },
  { CPC_m,           SDLK_m },
  { CPC_n,           SDLK_n },
  { CPC_o,           SDLK_o },
  { CPC_p,           SDLK_p },
  { CPC_q,           SDLK_q },
  { CPC_r,           SDLK_r },
  { CPC_s,           SDLK_s },
  { CPC_t,           SDLK_t },
  { CPC_u,           SDLK_u },
  { CPC_v,           SDLK_v },
  { CPC_w,           SDLK_w },
  { CPC_x,           SDLK_x },
  { CPC_y,           SDLK_y },
  { CPC_z,           SDLK_z },
  { CPC_AMPERSAND,   SDLK_7 | MOD_PC_SHIFT },
  { CPC_ASTERISK,    SDLK_8 | MOD_PC_SHIFT },
  { CPC_AT,          SDLK_2 | MOD_PC_SHIFT },
  { CPC_BACKQUOTE,   SDLK_BACKQUOTE },
  { CPC_BACKSLASH,   SDLK_BACKSLASH },
  { CPC_CAPSLOCK,    SDLK_CAPSLOCK },
  { CPC_CLR,         SDLK_DELETE },
  { CPC_COLON,       SDLK_SEMICOLON | MOD_PC_SHIFT },
  { CPC_COMMA,       SDLK_COMMA },
  { CPC_CONTROL,     SDLK_LCTRL },
  { CPC_COPY,        SDLK_LALT },
  { CPC_CPY_DOWN,    SDLK_DOWN | MOD_PC_SHIFT },
  { CPC_CPY_LEFT,    SDLK_LEFT | MOD_PC_SHIFT },
  { CPC_CPY_RIGHT,   SDLK_RIGHT | MOD_PC_SHIFT },
  { CPC_CPY_UP,      SDLK_UP | MOD_PC_SHIFT },
  { CPC_CUR_DOWN,    SDLK_DOWN },
  { CPC_CUR_LEFT,    SDLK_LEFT },
  { CPC_CUR_RIGHT,   SDLK_RIGHT },
  { CPC_CUR_UP,      SDLK_UP },
  { CPC_CUR_HOMELN,  SDLK_HOME },
  { CPC_CUR_ENDLN,   SDLK_END },
  { CPC_CUR_HOMEBL,  SDLK_HOME | MOD_PC_CTRL },
  { CPC_CUR_ENDBL,   SDLK_END | MOD_PC_CTRL },
  { CPC_DBLQUOTE,    SDLK_QUOTE | MOD_PC_SHIFT },
  { CPC_DEL,         SDLK_BACKSPACE },
  { CPC_DOLLAR,      SDLK_4 | MOD_PC_SHIFT },
  { CPC_ENTER,       SDLK_KP_ENTER },
  { CPC_EQUAL,       SDLK_EQUALS },
  { CPC_ESC,         SDLK_ESCAPE },
  { CPC_EXCLAMATN,   SDLK_1 | MOD_PC_SHIFT },
  { CPC_F0,          SDLK_KP0 },
  { CPC_F1,          SDLK_KP1 },
  { CPC_F2,          SDLK_KP2 },
  { CPC_F3,          SDLK_KP3 },
  { CPC_F4,          SDLK_KP4 },
  { CPC_F5,          SDLK_KP5 },
  { CPC_F6,          SDLK_KP6 },
  { CPC_F7,          SDLK_KP7 },
  { CPC_F8,          SDLK_KP8 },
  { CPC_F9,          SDLK_KP9 },
  { CPC_FPERIOD,     SDLK_KP_PERIOD },
  { CPC_GREATER,     SDLK_PERIOD | MOD_PC_SHIFT },
  { CPC_HASH,        SDLK_3 | MOD_PC_SHIFT },
  { CPC_LBRACKET,    SDLK_LEFTBRACKET },
  { CPC_LCBRACE,     SDLK_LEFTBRACKET | MOD_PC_SHIFT },
  { CPC_LEFTPAREN,   SDLK_9 | MOD_PC_SHIFT },
  { CPC_LESS,        SDLK_COMMA | MOD_PC_SHIFT },
  { CPC_LSHIFT,      SDLK_LSHIFT },
  { CPC_MINUS,       SDLK_MINUS },
  { CPC_PERCENT,     SDLK_5 | MOD_PC_SHIFT },
  { CPC_PERIOD,      SDLK_PERIOD },
  { CPC_PIPE,        SDLK_BACKSLASH | MOD_PC_SHIFT },
  { CPC_PLUS,        SDLK_EQUALS | MOD_PC_SHIFT },
  { CPC_POUND,       0 },
  { CPC_POWER,       SDLK_6 | MOD_PC_SHIFT },
  { CPC_QUESTION,    SDLK_SLASH | MOD_PC_SHIFT },
  { CPC_QUOTE,       SDLK_QUOTE },
  { CPC_RBRACKET,    SDLK_RIGHTBRACKET },
  { CPC_RCBRACE,     SDLK_RIGHTBRACKET | MOD_PC_SHIFT },
  { CPC_RETURN,      SDLK_RETURN },
  { CPC_RIGHTPAREN,  SDLK_0 | MOD_PC_SHIFT },
  { CPC_RSHIFT,      SDLK_RSHIFT },
  { CPC_SEMICOLON,   SDLK_SEMICOLON },
  { CPC_SLASH,       SDLK_SLASH },
  { CPC_SPACE,       SDLK_SPACE },
  { CPC_TAB,         SDLK_TAB },
  { CPC_UNDERSCORE,  SDLK_MINUS | MOD_PC_SHIFT },
  { CAP32_EXIT,      SDLK_F10 },
  { CAP32_FPS,       SDLK_F8 },
  { CAP32_GUI,       SDLK_F1},
  { CAP32_VKBD,      SDLK_F1 | MOD_PC_SHIFT },
  { CAP32_FULLSCRN,  SDLK_F2 },
  { CAP32_SCRNSHOT,  SDLK_F3 },
  { CAP32_SNAPSHOT,  SDLK_F3 | MOD_PC_SHIFT },
  { CAP32_JOY,       SDLK_F7 },
  { CAP32_MF2STOP,   SDLK_F6 },
  { CAP32_RESET,     SDLK_F5 },
  { CAP32_SPEED,     SDLK_F9 },
  { CAP32_DEBUG,     SDLK_F12 },
  { CAP32_TAPEPLAY,  SDLK_F4 },
  { CAP32_DELAY,     SDLK_BREAK },
  { CAP32_WAITBREAK, SDLK_BREAK | MOD_PC_SHIFT }
};

const std::map<const std::string, const unsigned int> InputMapper::CPCkeysFromStrings = {
   {"CPC_0",           CPC_0},
   {"CPC_1",           CPC_1},
   {"CPC_2",           CPC_2},
   {"CPC_3",           CPC_3},
   {"CPC_4",           CPC_4},
   {"CPC_5",           CPC_5},
   {"CPC_6",           CPC_6},
   {"CPC_7",           CPC_7},
   {"CPC_8",           CPC_8},
   {"CPC_9",           CPC_9},
   {"CPC_A",           CPC_A},
   {"CPC_B",           CPC_B},
   {"CPC_C",           CPC_C},
   {"CPC_D",           CPC_D},
   {"CPC_E",           CPC_E},
   {"CPC_F",           CPC_F},
   {"CPC_G",           CPC_G},
   {"CPC_H",           CPC_H},
   {"CPC_I",           CPC_I},
   {"CPC_J",           CPC_J},
   {"CPC_K",           CPC_K},
   {"CPC_L",           CPC_L},
   {"CPC_M",           CPC_M},
   {"CPC_N",           CPC_N},
   {"CPC_O",           CPC_O},
   {"CPC_P",           CPC_P},
   {"CPC_Q",           CPC_Q},
   {"CPC_R",           CPC_R},
   {"CPC_S",           CPC_S},
   {"CPC_T",           CPC_T},
   {"CPC_U",           CPC_U},
   {"CPC_V",           CPC_V},
   {"CPC_W",           CPC_W},
   {"CPC_X",           CPC_X},
   {"CPC_Y",           CPC_Y},
   {"CPC_Z",           CPC_Z},
   {"CPC_a",           CPC_a},
   {"CPC_b",           CPC_b},
   {"CPC_c",           CPC_c},
   {"CPC_d",           CPC_d},
   {"CPC_e",           CPC_e},
   {"CPC_f",           CPC_f},
   {"CPC_g",           CPC_g},
   {"CPC_h",           CPC_h},
   {"CPC_i",           CPC_i},
   {"CPC_j",           CPC_j},
   {"CPC_k",           CPC_k},
   {"CPC_l",           CPC_l},
   {"CPC_m",           CPC_m},
   {"CPC_n",           CPC_n},
   {"CPC_o",           CPC_o},
   {"CPC_p",           CPC_p},
   {"CPC_q",           CPC_q},
   {"CPC_r",           CPC_r},
   {"CPC_s",           CPC_s},
   {"CPC_t",           CPC_t},
   {"CPC_u",           CPC_u},
   {"CPC_v",           CPC_v},
   {"CPC_w",           CPC_w},
   {"CPC_x",           CPC_x},
   {"CPC_y",           CPC_y},
   {"CPC_z",           CPC_z},
   {"CPC_AMPERSAND",   CPC_AMPERSAND},
   {"CPC_ASTERISK",    CPC_ASTERISK},
   {"CPC_AT",          CPC_AT},
   {"CPC_BACKQUOTE",   CPC_BACKQUOTE},
   {"CPC_BACKSLASH",   CPC_BACKSLASH},
   {"CPC_CAPSLOCK",    CPC_CAPSLOCK},
   {"CPC_CLR",         CPC_CLR},
   {"CPC_COLON",       CPC_COLON},
   {"CPC_COMMA",       CPC_COMMA},
   {"CPC_CONTROL",     CPC_CONTROL},
   {"CPC_COPY",        CPC_COPY},
   {"CPC_CPY_DOWN",    CPC_CPY_DOWN},
   {"CPC_CPY_LEFT",    CPC_CPY_LEFT},
   {"CPC_CPY_RIGHT",   CPC_CPY_RIGHT},
   {"CPC_CPY_UP",      CPC_CPY_UP},
   {"CPC_CUR_DOWN",    CPC_CUR_DOWN},
   {"CPC_CUR_LEFT",    CPC_CUR_LEFT},
   {"CPC_CUR_RIGHT",   CPC_CUR_RIGHT},
   {"CPC_CUR_UP",      CPC_CUR_UP},
   {"CPC_CUR_ENDBL",   CPC_CUR_ENDBL},
   {"CPC_CUR_HOMELN",  CPC_CUR_HOMELN},
   {"CPC_CUR_ENDLN",   CPC_CUR_ENDLN},
   {"CPC_CUR_HOMEBL",  CPC_CUR_HOMEBL},
   {"CPC_DBLQUOTE",    CPC_DBLQUOTE},
   {"CPC_DEL",         CPC_DEL},
   {"CPC_DOLLAR",      CPC_DOLLAR},
   {"CPC_ENTER",       CPC_ENTER},
   {"CPC_EQUAL",       CPC_EQUAL},
   {"CPC_ESC",         CPC_ESC},
   {"CPC_EXCLAMATN",   CPC_EXCLAMATN},
   {"CPC_F0",          CPC_F0},
   {"CPC_F1",          CPC_F1},
   {"CPC_F2",          CPC_F2},
   {"CPC_F3",          CPC_F3},
   {"CPC_F4",          CPC_F4},
   {"CPC_F5",          CPC_F5},
   {"CPC_F6",          CPC_F6},
   {"CPC_F7",          CPC_F7},
   {"CPC_F8",          CPC_F8},
   {"CPC_F9",          CPC_F9},
   {"CPC_FPERIOD",     CPC_FPERIOD},
   {"CPC_GREATER",     CPC_GREATER},
   {"CPC_HASH",        CPC_HASH},
   {"CPC_LBRACKET",    CPC_LBRACKET},
   {"CPC_LCBRACE",     CPC_LCBRACE},
   {"CPC_LEFTPAREN",   CPC_LEFTPAREN},
   {"CPC_LESS",        CPC_LESS},
   {"CPC_LSHIFT",      CPC_LSHIFT},
   {"CPC_MINUS",       CPC_MINUS},
   {"CPC_PERCENT",     CPC_PERCENT},
   {"CPC_PERIOD",      CPC_PERIOD},
   {"CPC_PIPE",        CPC_PIPE},
   {"CPC_PLUS",        CPC_PLUS},
   {"CPC_POUND",       CPC_POUND},
   {"CPC_POWER",       CPC_POWER},
   {"CPC_QUESTION",    CPC_QUESTION},
   {"CPC_QUOTE",       CPC_QUOTE},
   {"CPC_RBRACKET",    CPC_RBRACKET},
   {"CPC_RCBRACE",     CPC_RCBRACE},
   {"CPC_RETURN",      CPC_RETURN},
   {"CPC_RIGHTPAREN",  CPC_RIGHTPAREN},
   {"CPC_RSHIFT",      CPC_RSHIFT},
   {"CPC_SEMICOLON",   CPC_SEMICOLON},
   {"CPC_SLASH",       CPC_SLASH},
   {"CPC_SPACE",       CPC_SPACE},
   {"CPC_TAB",         CPC_TAB},
   {"CPC_UNDERSCORE",  CPC_UNDERSCORE},
   {"CPC_J0_UP",       CPC_J0_UP},
   {"CPC_J0_DOWN",     CPC_J0_DOWN},
   {"CPC_J0_LEFT",     CPC_J0_LEFT},
   {"CPC_J0_RIGHT",    CPC_J0_RIGHT},
   {"CPC_J0_FIRE1",    CPC_J0_FIRE1},
   {"CPC_J0_FIRE2",    CPC_J0_FIRE2},
   {"CPC_J1_UP",       CPC_J1_UP},
   {"CPC_J1_DOWN",     CPC_J1_DOWN},
   {"CPC_J1_LEFT",     CPC_J1_LEFT},
   {"CPC_J1_RIGHT",    CPC_J1_RIGHT},
   {"CPC_J1_FIRE1",    CPC_J1_FIRE1},
   {"CPC_J1_FIRE2",    CPC_J1_FIRE2},
   {"CPC_ES_NTILDE",   CPC_ES_NTILDE},
   {"CPC_ES_nTILDE",   CPC_ES_nTILDE},
   {"CPC_ES_PESETA",   CPC_ES_PESETA},
   {"CPC_FR_eACUTE",   CPC_FR_eACUTE},
   {"CPC_FR_eGRAVE",   CPC_FR_eGRAVE},
   {"CPC_FR_cCEDIL",   CPC_FR_cCEDIL},
   {"CPC_FR_aGRAVE",   CPC_FR_aGRAVE},
   {"CPC_FR_uGRAVE",   CPC_FR_uGRAVE},
   {"CAP32_EXIT",      CAP32_EXIT},
   {"CAP32_FPS",       CAP32_FPS},
   {"CAP32_FULLSCRN",  CAP32_FULLSCRN},
   {"CAP32_GUI",       CAP32_GUI},
   {"CAP32_VKBD",      CAP32_VKBD},
   {"CAP32_JOY",       CAP32_JOY},
   {"CAP32_MF2STOP",   CAP32_MF2STOP},
   {"CAP32_RESET",     CAP32_RESET},
   {"CAP32_SCRNSHOT",  CAP32_SCRNSHOT},
   {"CAP32_SNAPSHOT",  CAP32_SNAPSHOT},
   {"CAP32_SPEED",     CAP32_SPEED},
   {"CAP32_TAPEPLAY",  CAP32_TAPEPLAY},
   {"CAP32_DEBUG",     CAP32_DEBUG},  
   {"CAP32_DELAY",     CAP32_DELAY},
   {"CAP32_WAITBREAK", CAP32_WAITBREAK},
};

const std::map<const std::string, const unsigned int> InputMapper::SDLkeysFromStrings = {
	{ "SDLK_BACKSPACE", SDLK_BACKSPACE},
	{ "SDLK_TAB", SDLK_TAB},
	{ "SDLK_CLEAR", SDLK_CLEAR},
	{ "SDLK_RETURN", SDLK_RETURN},
	{ "SDLK_PAUSE", SDLK_PAUSE},
	{ "SDLK_ESCAPE", SDLK_ESCAPE},
	{ "SDLK_SPACE", SDLK_SPACE},
	{ "SDLK_EXCLAIM", SDLK_EXCLAIM},
	{ "SDLK_QUOTEDBL", SDLK_QUOTEDBL},
	{ "SDLK_HASH", SDLK_HASH},
	{ "SDLK_DOLLAR", SDLK_DOLLAR},
	{ "SDLK_AMPERSAND", SDLK_AMPERSAND},
	{ "SDLK_QUOTE", SDLK_QUOTE},
	{ "SDLK_LEFTPAREN", SDLK_LEFTPAREN},
	{ "SDLK_RIGHTPAREN", SDLK_RIGHTPAREN},
	{ "SDLK_ASTERISK", SDLK_ASTERISK},
	{ "SDLK_PLUS", SDLK_PLUS},
	{ "SDLK_COMMA", SDLK_COMMA},
	{ "SDLK_MINUS", SDLK_MINUS},
	{ "SDLK_PERIOD", SDLK_PERIOD},
	{ "SDLK_SLASH", SDLK_SLASH},
	{ "SDLK_0", SDLK_0},
	{ "SDLK_1", SDLK_1},
	{ "SDLK_2", SDLK_2},
	{ "SDLK_3", SDLK_3},
	{ "SDLK_4", SDLK_4},
	{ "SDLK_5", SDLK_5},
	{ "SDLK_6", SDLK_6},
	{ "SDLK_7", SDLK_7},
	{ "SDLK_8", SDLK_8},
	{ "SDLK_9", SDLK_9},
	{ "SDLK_COLON", SDLK_COLON},
	{ "SDLK_SEMICOLON", SDLK_SEMICOLON},
	{ "SDLK_LESS", SDLK_LESS},
	{ "SDLK_EQUALS", SDLK_EQUALS},
	{ "SDLK_GREATER", SDLK_GREATER},
	{ "SDLK_QUESTION", SDLK_QUESTION},
	{ "SDLK_AT", SDLK_AT},
	{ "SDLK_LEFTBRACKET", SDLK_LEFTBRACKET},
	{ "SDLK_BACKSLASH", SDLK_BACKSLASH},
	{ "SDLK_RIGHTBRACKET", SDLK_RIGHTBRACKET},
	{ "SDLK_CARET", SDLK_CARET},
	{ "SDLK_UNDERSCORE", SDLK_UNDERSCORE},
	{ "SDLK_BACKQUOTE", SDLK_BACKQUOTE},
	{ "SDLK_a", SDLK_a},
	{ "SDLK_b", SDLK_b},
	{ "SDLK_c", SDLK_c},
	{ "SDLK_d", SDLK_d},
	{ "SDLK_e", SDLK_e},
	{ "SDLK_f", SDLK_f},
	{ "SDLK_g", SDLK_g},
	{ "SDLK_h", SDLK_h},
	{ "SDLK_i", SDLK_i},
	{ "SDLK_j", SDLK_j},
	{ "SDLK_k", SDLK_k},
	{ "SDLK_l", SDLK_l},
	{ "SDLK_m", SDLK_m},
	{ "SDLK_n", SDLK_n},
	{ "SDLK_o", SDLK_o},
	{ "SDLK_p", SDLK_p},
	{ "SDLK_q", SDLK_q},
	{ "SDLK_r", SDLK_r},
	{ "SDLK_s", SDLK_s},
	{ "SDLK_t", SDLK_t},
	{ "SDLK_u", SDLK_u},
	{ "SDLK_v", SDLK_v},
	{ "SDLK_w", SDLK_w},
	{ "SDLK_x", SDLK_x},
	{ "SDLK_y", SDLK_y},
	{ "SDLK_z", SDLK_z},
	{ "SDLK_DELETE", SDLK_DELETE},
	/* End of ASCII mapped keysyms */
	/*@}*/
		/** @name International keyboard syms */
	/*@{*/
	{ "SDLK_WORLD_0", SDLK_WORLD_0},		/* 0xA0 */
	{ "SDLK_WORLD_1", SDLK_WORLD_1},
	{ "SDLK_WORLD_2", SDLK_WORLD_2},
	{ "SDLK_WORLD_3", SDLK_WORLD_3},
	{ "SDLK_WORLD_4", SDLK_WORLD_4},
	{ "SDLK_WORLD_5", SDLK_WORLD_5},
	{ "SDLK_WORLD_6", SDLK_WORLD_6},
	{ "SDLK_WORLD_7", SDLK_WORLD_7},
	{ "SDLK_WORLD_8", SDLK_WORLD_8},
	{ "SDLK_WORLD_9", SDLK_WORLD_9},
	{ "SDLK_WORLD_10", SDLK_WORLD_10},
	{ "SDLK_WORLD_11", SDLK_WORLD_11},
	{ "SDLK_WORLD_12", SDLK_WORLD_12},
	{ "SDLK_WORLD_13", SDLK_WORLD_13},
	{ "SDLK_WORLD_14", SDLK_WORLD_14},
	{ "SDLK_WORLD_15", SDLK_WORLD_15},
	{ "SDLK_WORLD_16", SDLK_WORLD_16},
	{ "SDLK_WORLD_17", SDLK_WORLD_17},
	{ "SDLK_WORLD_18", SDLK_WORLD_18},
	{ "SDLK_WORLD_19", SDLK_WORLD_19},
	{ "SDLK_WORLD_20", SDLK_WORLD_20},
	{ "SDLK_WORLD_21", SDLK_WORLD_21},
	{ "SDLK_WORLD_22", SDLK_WORLD_22},
	{ "SDLK_WORLD_23", SDLK_WORLD_23},
	{ "SDLK_WORLD_24", SDLK_WORLD_24},
	{ "SDLK_WORLD_25", SDLK_WORLD_25},
	{ "SDLK_WORLD_26", SDLK_WORLD_26},
	{ "SDLK_WORLD_27", SDLK_WORLD_27},
	{ "SDLK_WORLD_28", SDLK_WORLD_28},
	{ "SDLK_WORLD_29", SDLK_WORLD_29},
	{ "SDLK_WORLD_30", SDLK_WORLD_30},
	{ "SDLK_WORLD_31", SDLK_WORLD_31},
	{ "SDLK_WORLD_32", SDLK_WORLD_32},
	{ "SDLK_WORLD_33", SDLK_WORLD_33},
	{ "SDLK_WORLD_34", SDLK_WORLD_34},
	{ "SDLK_WORLD_35", SDLK_WORLD_35},
	{ "SDLK_WORLD_36", SDLK_WORLD_36},
	{ "SDLK_WORLD_37", SDLK_WORLD_37},
	{ "SDLK_WORLD_38", SDLK_WORLD_38},
	{ "SDLK_WORLD_39", SDLK_WORLD_39},
	{ "SDLK_WORLD_40", SDLK_WORLD_40},
	{ "SDLK_WORLD_41", SDLK_WORLD_41},
	{ "SDLK_WORLD_42", SDLK_WORLD_42},
	{ "SDLK_WORLD_43", SDLK_WORLD_43},
	{ "SDLK_WORLD_44", SDLK_WORLD_44},
	{ "SDLK_WORLD_45", SDLK_WORLD_45},
	{ "SDLK_WORLD_46", SDLK_WORLD_46},
	{ "SDLK_WORLD_47", SDLK_WORLD_47},
	{ "SDLK_WORLD_48", SDLK_WORLD_48},
	{ "SDLK_WORLD_49", SDLK_WORLD_49},
	{ "SDLK_WORLD_50", SDLK_WORLD_50},
	{ "SDLK_WORLD_51", SDLK_WORLD_51},
	{ "SDLK_WORLD_52", SDLK_WORLD_52},
	{ "SDLK_WORLD_53", SDLK_WORLD_53},
	{ "SDLK_WORLD_54", SDLK_WORLD_54},
	{ "SDLK_WORLD_55", SDLK_WORLD_55},
	{ "SDLK_WORLD_56", SDLK_WORLD_56},
	{ "SDLK_WORLD_57", SDLK_WORLD_57},
	{ "SDLK_WORLD_58", SDLK_WORLD_58},
	{ "SDLK_WORLD_59", SDLK_WORLD_59},
	{ "SDLK_WORLD_60", SDLK_WORLD_60},
	{ "SDLK_WORLD_61", SDLK_WORLD_61},
	{ "SDLK_WORLD_62", SDLK_WORLD_62},
	{ "SDLK_WORLD_63", SDLK_WORLD_63},
	{ "SDLK_WORLD_64", SDLK_WORLD_64},
	{ "SDLK_WORLD_65", SDLK_WORLD_65},
	{ "SDLK_WORLD_66", SDLK_WORLD_66},
	{ "SDLK_WORLD_67", SDLK_WORLD_67},
	{ "SDLK_WORLD_68", SDLK_WORLD_68},
	{ "SDLK_WORLD_69", SDLK_WORLD_69},
	{ "SDLK_WORLD_70", SDLK_WORLD_70},
	{ "SDLK_WORLD_71", SDLK_WORLD_71},
	{ "SDLK_WORLD_72", SDLK_WORLD_72},
	{ "SDLK_WORLD_73", SDLK_WORLD_73},
	{ "SDLK_WORLD_74", SDLK_WORLD_74},
	{ "SDLK_WORLD_75", SDLK_WORLD_75},
	{ "SDLK_WORLD_76", SDLK_WORLD_76},
	{ "SDLK_WORLD_77", SDLK_WORLD_77},
	{ "SDLK_WORLD_78", SDLK_WORLD_78},
	{ "SDLK_WORLD_79", SDLK_WORLD_79},
	{ "SDLK_WORLD_80", SDLK_WORLD_80},
	{ "SDLK_WORLD_81", SDLK_WORLD_81},
	{ "SDLK_WORLD_82", SDLK_WORLD_82},
	{ "SDLK_WORLD_83", SDLK_WORLD_83},
	{ "SDLK_WORLD_84", SDLK_WORLD_84},
	{ "SDLK_WORLD_85", SDLK_WORLD_85},
	{ "SDLK_WORLD_86", SDLK_WORLD_86},
	{ "SDLK_WORLD_87", SDLK_WORLD_87},
	{ "SDLK_WORLD_88", SDLK_WORLD_88},
	{ "SDLK_WORLD_89", SDLK_WORLD_89},
	{ "SDLK_WORLD_90", SDLK_WORLD_90},
	{ "SDLK_WORLD_91", SDLK_WORLD_91},
	{ "SDLK_WORLD_92", SDLK_WORLD_92},
	{ "SDLK_WORLD_93", SDLK_WORLD_93},
	{ "SDLK_WORLD_94", SDLK_WORLD_94},
	{ "SDLK_WORLD_95", SDLK_WORLD_95},		/* 0xFF */
	/*@}*/
		/** @name Numeric keypad */
	/*@{*/
	{ "SDLK_KP0", SDLK_KP0},
	{ "SDLK_KP1", SDLK_KP1},
	{ "SDLK_KP2", SDLK_KP2},
	{ "SDLK_KP3", SDLK_KP3},
	{ "SDLK_KP4", SDLK_KP4},
	{ "SDLK_KP5", SDLK_KP5},
	{ "SDLK_KP6", SDLK_KP6},
	{ "SDLK_KP7", SDLK_KP7},
	{ "SDLK_KP8", SDLK_KP8},
	{ "SDLK_KP9", SDLK_KP9},
	{ "SDLK_KP_PERIOD", SDLK_KP_PERIOD},
	{ "SDLK_KP_DIVIDE", SDLK_KP_DIVIDE},
	{ "SDLK_KP_MULTIPLY", SDLK_KP_MULTIPLY},
	{ "SDLK_KP_MINUS", SDLK_KP_MINUS},
	{ "SDLK_KP_PLUS", SDLK_KP_PLUS},
	{ "SDLK_KP_ENTER", SDLK_KP_ENTER},
	{ "SDLK_KP_EQUALS", SDLK_KP_EQUALS},
	/*@}*/
		/** @name Arrows + Home/End pad */
	/*@{*/
	{ "SDLK_UP", SDLK_UP},
	{ "SDLK_DOWN", SDLK_DOWN},
	{ "SDLK_RIGHT", SDLK_RIGHT},
	{ "SDLK_LEFT", SDLK_LEFT},
	{ "SDLK_INSERT", SDLK_INSERT},
	{ "SDLK_HOME", SDLK_HOME},
	{ "SDLK_END", SDLK_END},
	{ "SDLK_PAGEUP", SDLK_PAGEUP},
	{ "SDLK_PAGEDOWN", SDLK_PAGEDOWN},
	/*@}*/
		/** @name Function keys */
	/*@{*/
	{ "SDLK_F1", SDLK_F1},
	{ "SDLK_F2", SDLK_F2},
	{ "SDLK_F3", SDLK_F3},
	{ "SDLK_F4", SDLK_F4},
	{ "SDLK_F5", SDLK_F5},
	{ "SDLK_F6", SDLK_F6},
	{ "SDLK_F7", SDLK_F7},
	{ "SDLK_F8", SDLK_F8},
	{ "SDLK_F9", SDLK_F9},
	{ "SDLK_F10", SDLK_F10},
	{ "SDLK_F11", SDLK_F11},
	{ "SDLK_F12", SDLK_F12},
	{ "SDLK_F13", SDLK_F13},
	{ "SDLK_F14", SDLK_F14},
	{ "SDLK_F15", SDLK_F15},
	/*@}*/
		/** @name Key state modifier keys */
	/*@{*/
	{ "SDLK_NUMLOCK", SDLK_NUMLOCK},
	{ "SDLK_CAPSLOCK", SDLK_CAPSLOCK},
	{ "SDLK_SCROLLOCK", SDLK_SCROLLOCK},
	{ "SDLK_RSHIFT", SDLK_RSHIFT},
	{ "SDLK_LSHIFT", SDLK_LSHIFT},
	{ "SDLK_RCTRL", SDLK_RCTRL},
	{ "SDLK_LCTRL", SDLK_LCTRL},
	{ "SDLK_RALT", SDLK_RALT},
	{ "SDLK_LALT", SDLK_LALT},
	{ "SDLK_RMETA", SDLK_RMETA},
	{ "SDLK_LMETA", SDLK_LMETA},
	{ "SDLK_LSUPER", SDLK_LSUPER},		/**< Left "Windows" key */
	{ "SDLK_RSUPER", SDLK_RSUPER},		/**< Right "Windows" key */
	{ "SDLK_MODE", SDLK_MODE},		/**< "Alt Gr" key */
	{ "SDLK_COMPOSE", SDLK_COMPOSE},		/**< Multi-key compose key */
	/*@}*/
		/** @name Miscellaneous function keys */
	/*@{*/
	{ "SDLK_HELP", SDLK_HELP},
	{ "SDLK_PRINT", SDLK_PRINT},
	{ "SDLK_SYSREQ", SDLK_SYSREQ},
	{ "SDLK_BREAK", SDLK_BREAK},
	{ "SDLK_MENU", SDLK_MENU},
	{ "SDLK_POWER", SDLK_POWER},		/**< Power Macintosh power key */
	{ "SDLK_EURO", SDLK_EURO},		/**< Some european keyboards */
	{ "SDLK_UNDO", SDLK_UNDO},		/**< Atari keyboard has Undo */
       /*@}*/
	{ "MOD_PC_SHIFT", MOD_PC_SHIFT},
	{ "MOD_PC_CTRL", MOD_PC_CTRL},
	{ "MOD_PC_MODE", MOD_PC_MODE},
	{ "MOD_PC_META", MOD_PC_META},
	{ "MOD_PC_ALT", MOD_PC_ALT}
};


// Format of a line: CPC_xxx\tSDLK_xxx\tMODIFIER
// Last field is optional
void InputMapper::process_cfg_line(char *line)
{
  unsigned int cpc_key = 0, sdl_key = 0;

  char *pch = strtok(line, "\t");
  if (pch == nullptr || pch[0] == '#')
    return;

  if (CPCkeysFromStrings.count(pch) == 0) {
    LOG_ERROR("Unknown CPC key " << pch << " found in mapping file. Ignoring it.");
    return;
  }

  for (unsigned int field=0; field < 3; field++) {
    switch (field) {
      case 0:
        cpc_key = CPCkeysFromStrings.at(pch);
        break;
      case 1:
      case 2:
        if (SDLkeysFromStrings.count(pch) == 0) {
          LOG_ERROR("Unknown SDL key or modifier " << pch << " found in mapping file. Ignoring it.");
          return;
        }
        sdl_key |= SDLkeysFromStrings.at(pch);
        break;
      default:
        break;
    }
    pch = strtok(nullptr, "\t");
    if (pch == nullptr)
      break;
  }
  SDLkeysymFromCPCkeys[cpc_key] = sdl_key;
  return;
}

#define MAX_LINE_LENGTH 80
void InputMapper::init()
{
  std::string layout_file = CPC->resources_path + "/" + CPC->kbd_layout;
  std::filebuf fb;
  unsigned int sdl_moddedkey;
  char line[MAX_LINE_LENGTH]; // sufficient for now ! TODO(sebhz): proper malloc'ing etc...

  if (is_directory(layout_file) || (fb.open(layout_file, std::ios::in) == nullptr)) {
    SDLkeysymFromCPCkeys = SDLkeysymFromCPCkeys_us;
  }
  else {
    std::istream is(&fb);
    while (is.good()) {
      is.getline(line, MAX_LINE_LENGTH);
      process_cfg_line(line);
    }
    fb.close();
  }

  for (const auto &mapping : SDLkeysymFromCPCkeys) {
    CPCkeysFromSDLkeysym[mapping.second] = mapping.first;
  }

  for (const auto &mapping : CPCkeysFromChars) {
    if (SDLkeysymFromCPCkeys.count(mapping.second) != 0) {
      sdl_moddedkey = SDLkeysymFromCPCkeys[mapping.second];
      SDLkeysFromChars[mapping.first] = std::make_pair(static_cast<SDLKey>(sdl_moddedkey & 0xffff), static_cast<SDLMod>(sdl_moddedkey >> 16));
    }
  }
}

dword InputMapper::CPCkeyFromKeysym(SDL_keysym keysym) {
    dword sdl_key = keysym.sym;

    if (keysym.mod & KMOD_SHIFT)  sdl_key |= MOD_PC_SHIFT;
    if (keysym.mod & KMOD_CTRL)   sdl_key |= MOD_PC_CTRL;
    if (keysym.mod & KMOD_MODE)   sdl_key |= MOD_PC_MODE;
    if (keysym.mod & KMOD_META)   sdl_key |= MOD_PC_META;
    if (keysym.mod & KMOD_ALT)    sdl_key |= MOD_PC_ALT;
    // Ignore sticky modifiers (MOD_PC_NUM and MOD_PC_CAPS)

    auto cpc_key = CPCkeysFromSDLkeysym.find(sdl_key);
    // TODO(sebhz) magic numbers are bad. Get rid of the 0xff.
    if (cpc_key == CPCkeysFromSDLkeysym.end()) return 0xff;

    if (cpc_key->second & MOD_EMU_KEY)
        return cpc_key->second;
    return cpc_kbd[CPC->keyboard][cpc_key->second];
}

std::list<SDL_Event> InputMapper::StringToEvents(std::string toTranslate) {
    std::list<SDL_Event> result;
    bool escaped = false;
    bool cap32_cmd = false;
    std::map<unsigned int, unsigned int>::iterator sdl_keysym;

    for (auto c : toTranslate) {
      if (c == '\a') {
        // Escape prefix: next char is a special one
        escaped = true;
        continue;
      }
      if (c == '\f') {
        // Emulator special command
        cap32_cmd = true;
        continue;
      }
      SDL_Event key;
      if (escaped || cap32_cmd) {
        int keycode = c;
        if (cap32_cmd) {
          keycode += MOD_EMU_KEY;
        }
        // Lookup the SDL key corresponding to this emulator command
        sdl_keysym = SDLkeysymFromCPCkeys.find(keycode);
        if (sdl_keysym != SDLkeysymFromCPCkeys.end()) {
          key.key.keysym.sym = static_cast<SDLKey>(sdl_keysym->second & 0xffff);
          key.key.keysym.mod = static_cast<SDLMod>(sdl_keysym->second >> 16);
        }
        escaped = false;
        cap32_cmd = false;
      } else {
        // key.key.keysym.scancode = ;
        key.key.keysym.sym = SDLkeysFromChars[c].first;
        key.key.keysym.mod = SDLkeysFromChars[c].second;
        // key.key.keysym.unicode = c;
      }
      key.key.type = SDL_KEYDOWN;
      key.key.state = SDL_PRESSED;
      result.push_back(key);

      key.key.type = SDL_KEYUP;
      key.key.state = SDL_RELEASED;
      result.push_back(key);
    }
    return result;
}

void InputMapper::set_joystick_emulation()
{
  // CPC joy key, CPC original key
  static int joy_layout[12][2] = {
    { CPC_J0_UP,      CPC_CUR_UP },
    { CPC_J0_DOWN,    CPC_CUR_DOWN },
    { CPC_J0_LEFT,    CPC_CUR_LEFT },
    { CPC_J0_RIGHT,   CPC_CUR_RIGHT },
    { CPC_J0_FIRE1,   CPC_z },
    { CPC_J0_FIRE2,   CPC_x },
    { CPC_J1_UP,      0 },
    { CPC_J1_DOWN,    0 },
    { CPC_J1_LEFT,    0 },
    { CPC_J1_RIGHT,   0 },
    { CPC_J1_FIRE1,   0 },
    { CPC_J1_FIRE2,   0 }
  };

  for (dword n = 0; n < 6; n++) {
    int cpc_idx = joy_layout[n][1]; // get the CPC key to change the assignment for
    if (cpc_idx) {
      dword pc_idx = SDLkeysymFromCPCkeys[cpc_idx]; // SDL key corresponding to the CPC key to remap
      if (CPC->joystick_emulation) {
        CPCkeysFromSDLkeysym[pc_idx] = joy_layout[n][0];
      }
      else {
        CPCkeysFromSDLkeysym[pc_idx] = cpc_idx;
      }
    }
  }
}

dword InputMapper::CPCkeyFromJoystickButton(SDL_JoyButtonEvent jbutton)
{
    dword cpc_key(0xff);
    switch(jbutton.button) {
        case 0:
            switch(jbutton.which) {
                case 0:
                   cpc_key = cpc_kbd[CPC->keyboard][CPC_J0_FIRE1];
                   break;
                case 1:
                   cpc_key = cpc_kbd[CPC->keyboard][CPC_J1_FIRE1];
                   break;
            }
            break;
        case 1:
            switch(jbutton.which) {
                case 0:
                    cpc_key = cpc_kbd[CPC->keyboard][CPC_J0_FIRE2];
                    break;
                case 1:
                    cpc_key = cpc_kbd[CPC->keyboard][CPC_J1_FIRE2];
                    break;
            }
            break;
        default:
            break;
    }
    return cpc_key;
}

void InputMapper::CPCkeyFromJoystickAxis(SDL_JoyAxisEvent jaxis, dword *cpc_key, bool &release)
{
   switch(jaxis.axis) {
     case 0:
     case 2:
       switch(jaxis.which) {
         case 0:
           if(jaxis.value < -JOYSTICK_AXIS_THRESHOLD) {
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J0_LEFT];
           } else if(jaxis.value > JOYSTICK_AXIS_THRESHOLD) {
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J0_RIGHT];
           } else {
             // release both LEFT and RIGHT
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J0_LEFT];
             cpc_key[1] = cpc_kbd[CPC->keyboard][CPC_J0_RIGHT];
             release = true;
           }
           break;
         case 1:
           if(jaxis.value < -JOYSTICK_AXIS_THRESHOLD) {
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J1_LEFT];
           } else if(jaxis.value > JOYSTICK_AXIS_THRESHOLD) {
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J1_RIGHT];
           } else {
             // release both LEFT and RIGHT
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J1_LEFT];
             cpc_key[1] = cpc_kbd[CPC->keyboard][CPC_J1_RIGHT];
             release = true;
           }
           break;
       }
       break;
     case 1:
     case 3:
       switch(jaxis.which) {
         case 0:
           if(jaxis.value < -JOYSTICK_AXIS_THRESHOLD) {
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J0_UP];
           } else if(jaxis.value > JOYSTICK_AXIS_THRESHOLD) {
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J0_DOWN];
           } else {
             // release both UP and DOWN
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J0_UP];
             cpc_key[1] = cpc_kbd[CPC->keyboard][CPC_J0_DOWN];
             release = true;
           }
           break;
         case 1:
           if(jaxis.value < -JOYSTICK_AXIS_THRESHOLD) {
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J1_UP];
           } else if(jaxis.value > JOYSTICK_AXIS_THRESHOLD) {
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J1_DOWN];
           } else {
             // release both UP and DOWN
             cpc_key[0] = cpc_kbd[CPC->keyboard][CPC_J1_UP];
             cpc_key[1] = cpc_kbd[CPC->keyboard][CPC_J1_DOWN];
             release = true;
           }
           break;
       }
       break;
   }
}

InputMapper::InputMapper(t_CPC *CPC): CPC(CPC) { }

void applyKeypress(dword cpc_key, byte keyboard_matrix[], bool pressed) {
    if ((!CPC.paused) && (static_cast<byte>(cpc_key) != 0xff)) {
        if (pressed) {
            keyboard_matrix[static_cast<byte>(cpc_key) >> 4] &= ~bit_values[static_cast<byte>(cpc_key) & 7]; // key is being held down
            if (cpc_key & MOD_CPC_SHIFT) { // CPC SHIFT key required?
                keyboard_matrix[0x25 >> 4] &= ~bit_values[0x25 & 7]; // key needs to be SHIFTed
            } else {
                keyboard_matrix[0x25 >> 4] |= bit_values[0x25 & 7]; // make sure key is unSHIFTed
            }
            if (cpc_key & MOD_CPC_CTRL) { // CPC CONTROL key required?
                keyboard_matrix[0x27 >> 4] &= ~bit_values[0x27 & 7]; // CONTROL key is held down
            } else {
                keyboard_matrix[0x27 >> 4] |= bit_values[0x27 & 7]; // make sure CONTROL key is released
            }
        } else {
            keyboard_matrix[static_cast<byte>(cpc_key) >> 4] |= bit_values[static_cast<byte>(cpc_key) & 7]; // key has been released
            keyboard_matrix[0x25 >> 4] |= bit_values[0x25 & 7]; // make sure key is unSHIFTed
            keyboard_matrix[0x27 >> 4] |= bit_values[0x27 & 7]; // make sure CONTROL key is not held down
        }
    }
}
