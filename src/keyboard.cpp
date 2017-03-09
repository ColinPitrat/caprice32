#include <keyboard.h>

dword cpc_kbd[3][149] = {
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

int kbd_layout[4][KBD_MAX_ENTRIES][2] = {
  { // US PC to CPC keyboard layout translation
    { CPC_0,          SDLK_0 },
    { CPC_1,          SDLK_1 },
    { CPC_2,          SDLK_2 },
    { CPC_3,          SDLK_3 },
    { CPC_4,          SDLK_4 },
    { CPC_5,          SDLK_5 },
    { CPC_6,          SDLK_6 },
    { CPC_7,          SDLK_7 },
    { CPC_8,          SDLK_8 },
    { CPC_9,          SDLK_9 },
    { CPC_A,          SDLK_a | MOD_PC_SHIFT },
    { CPC_B,          SDLK_b | MOD_PC_SHIFT },
    { CPC_C,          SDLK_c | MOD_PC_SHIFT },
    { CPC_D,          SDLK_d | MOD_PC_SHIFT },
    { CPC_E,          SDLK_e | MOD_PC_SHIFT },
    { CPC_F,          SDLK_f | MOD_PC_SHIFT },
    { CPC_G,          SDLK_g | MOD_PC_SHIFT },
    { CPC_H,          SDLK_h | MOD_PC_SHIFT },
    { CPC_I,          SDLK_i | MOD_PC_SHIFT },
    { CPC_J,          SDLK_j | MOD_PC_SHIFT },
    { CPC_K,          SDLK_k | MOD_PC_SHIFT },
    { CPC_L,          SDLK_l | MOD_PC_SHIFT },
    { CPC_M,          SDLK_m | MOD_PC_SHIFT },
    { CPC_N,          SDLK_n | MOD_PC_SHIFT },
    { CPC_O,          SDLK_o | MOD_PC_SHIFT },
    { CPC_P,          SDLK_p | MOD_PC_SHIFT },
    { CPC_Q,          SDLK_q | MOD_PC_SHIFT },
    { CPC_R,          SDLK_r | MOD_PC_SHIFT },
    { CPC_S,          SDLK_s | MOD_PC_SHIFT },
    { CPC_T,          SDLK_t | MOD_PC_SHIFT },
    { CPC_U,          SDLK_u | MOD_PC_SHIFT },
    { CPC_V,          SDLK_v | MOD_PC_SHIFT },
    { CPC_W,          SDLK_w | MOD_PC_SHIFT },
    { CPC_X,          SDLK_x | MOD_PC_SHIFT },
    { CPC_Y,          SDLK_y | MOD_PC_SHIFT },
    { CPC_Z,          SDLK_z | MOD_PC_SHIFT },
    { CPC_a,          SDLK_a },
    { CPC_b,          SDLK_b },
    { CPC_c,          SDLK_c },
    { CPC_d,          SDLK_d },
    { CPC_e,          SDLK_e },
    { CPC_f,          SDLK_f },
    { CPC_g,          SDLK_g },
    { CPC_h,          SDLK_h },
    { CPC_i,          SDLK_i },
    { CPC_j,          SDLK_j },
    { CPC_k,          SDLK_k },
    { CPC_l,          SDLK_l },
    { CPC_m,          SDLK_m },
    { CPC_n,          SDLK_n },
    { CPC_o,          SDLK_o },
    { CPC_p,          SDLK_p },
    { CPC_q,          SDLK_q },
    { CPC_r,          SDLK_r },
    { CPC_s,          SDLK_s },
    { CPC_t,          SDLK_t },
    { CPC_u,          SDLK_u },
    { CPC_v,          SDLK_v },
    { CPC_w,          SDLK_w },
    { CPC_x,          SDLK_x },
    { CPC_y,          SDLK_y },
    { CPC_z,          SDLK_z },
    { CPC_AMPERSAND,  SDLK_7 | MOD_PC_SHIFT },
    { CPC_ASTERISK,   SDLK_8 | MOD_PC_SHIFT },
    { CPC_AT,         SDLK_2 | MOD_PC_SHIFT },
    { CPC_BACKQUOTE,  SDLK_BACKQUOTE },
    { CPC_BACKSLASH,  SDLK_BACKSLASH },
    { CPC_CAPSLOCK,   SDLK_CAPSLOCK },
    { CPC_CLR,        SDLK_DELETE },
    { CPC_COLON,      SDLK_SEMICOLON | MOD_PC_SHIFT },
    { CPC_COMMA,      SDLK_COMMA },
    { CPC_CONTROL,    SDLK_LCTRL },
    { CPC_COPY,       SDLK_LALT },
    { CPC_CPY_DOWN,   SDLK_DOWN | MOD_PC_SHIFT },
    { CPC_CPY_LEFT,   SDLK_LEFT | MOD_PC_SHIFT },
    { CPC_CPY_RIGHT,  SDLK_RIGHT | MOD_PC_SHIFT },
    { CPC_CPY_UP,     SDLK_UP | MOD_PC_SHIFT },
    { CPC_CUR_DOWN,   SDLK_DOWN },
    { CPC_CUR_LEFT,   SDLK_LEFT },
    { CPC_CUR_RIGHT,  SDLK_RIGHT },
    { CPC_CUR_UP,     SDLK_UP },
    { CPC_CUR_ENDBL,  SDLK_END | MOD_PC_CTRL },
    { CPC_CUR_HOMELN, SDLK_HOME },
    { CPC_CUR_ENDLN,  SDLK_END },
    { CPC_CUR_HOMEBL, SDLK_HOME | MOD_PC_CTRL },
    { CPC_DBLQUOTE,   SDLK_QUOTE | MOD_PC_SHIFT },
    { CPC_DEL,        SDLK_BACKSPACE },
    { CPC_DOLLAR,     SDLK_4 | MOD_PC_SHIFT },
    { CPC_ENTER,      SDLK_KP_ENTER },
    { CPC_EQUAL,      SDLK_EQUALS },
    { CPC_ESC,        SDLK_ESCAPE },
    { CPC_EXCLAMATN,  SDLK_1 | MOD_PC_SHIFT },
    { CPC_F0,         SDLK_KP0 },
    { CPC_F1,         SDLK_KP1 },
    { CPC_F2,         SDLK_KP2 },
    { CPC_F3,         SDLK_KP3 },
    { CPC_F4,         SDLK_KP4 },
    { CPC_F5,         SDLK_KP5 },
    { CPC_F6,         SDLK_KP6 },
    { CPC_F7,         SDLK_KP7 },
    { CPC_F8,         SDLK_KP8 },
    { CPC_F9,         SDLK_KP9 },
    { CPC_FPERIOD,    SDLK_KP_PERIOD },
    { CPC_GREATER,    SDLK_PERIOD | MOD_PC_SHIFT },
    { CPC_HASH,       SDLK_3 | MOD_PC_SHIFT },
    { CPC_LBRACKET,   SDLK_LEFTBRACKET },
    { CPC_LCBRACE,    SDLK_LEFTBRACKET | MOD_PC_SHIFT },
    { CPC_LEFTPAREN,  SDLK_9 | MOD_PC_SHIFT },
    { CPC_LESS,       SDLK_COMMA | MOD_PC_SHIFT },
    { CPC_LSHIFT,     SDLK_LSHIFT },
    { CPC_MINUS,      SDLK_MINUS },
    { CPC_PERCENT,    SDLK_5 | MOD_PC_SHIFT },
    { CPC_PERIOD,     SDLK_PERIOD },
    { CPC_PIPE,       SDLK_BACKSLASH | MOD_PC_SHIFT },
    { CPC_PLUS,       SDLK_EQUALS | MOD_PC_SHIFT },
    { CPC_POUND,      0 },
    { CPC_POWER,      SDLK_6 | MOD_PC_SHIFT },
    { CPC_QUESTION,   SDLK_SLASH | MOD_PC_SHIFT },
    { CPC_QUOTE,      SDLK_QUOTE },
    { CPC_RBRACKET,   SDLK_RIGHTBRACKET },
    { CPC_RCBRACE,    SDLK_RIGHTBRACKET | MOD_PC_SHIFT },
    { CPC_RETURN,     SDLK_RETURN },
    { CPC_RIGHTPAREN, SDLK_0 | MOD_PC_SHIFT },
    { CPC_RSHIFT,     SDLK_RSHIFT },
    { CPC_SEMICOLON,  SDLK_SEMICOLON },
    { CPC_SLASH,      SDLK_SLASH },
    { CPC_SPACE,      SDLK_SPACE },
    { CPC_TAB,        SDLK_TAB },
    { CPC_UNDERSCORE, SDLK_MINUS | MOD_PC_SHIFT },
    { CAP32_EXIT,     SDLK_F10 },
    { CAP32_FPS,      SDLK_F8 },
    { CAP32_GUI,      SDLK_F1},
    { CAP32_VKBD,     SDLK_F1 | MOD_PC_SHIFT },
    { CAP32_FULLSCRN, SDLK_F2 },
    { CAP32_SCRNDUMP, SDLK_F3 },
    { CAP32_JOY,      SDLK_F7 },
    //{ CAP32_LOADDRVA, SDLK_F6 },
    //{ CAP32_LOADDRVB, SDLK_F7 },
    //{ CAP32_LOADSNAP, SDLK_F2 },
    //{ CAP32_LOADTAPE, SDLK_F3 },
    //{ CAP32_MF2RESET, SDLK_F6 | MOD_PC_CTRL },
    { CAP32_MF2STOP,  SDLK_F6 },
    //{ CAP32_OPTIONS,  SDLK_F8 },
    //{ CAP32_PAUSE,    SDLK_BREAK },
    { CAP32_RESET,    SDLK_F5 },
    //{ CAP32_SAVESNAP, SDLK_F4 },
    //{ CAP32_SCRNSHOT, SDLK_PRINT },
    { CAP32_SPEED,    SDLK_F9 },
    { CAP32_DEBUG,    SDLK_F12 },
    { CAP32_TAPEPLAY, SDLK_F4 }
  },
  { // French PC to CPC keyboard layout translation
    { CPC_0,          SDLK_WORLD_64 | MOD_PC_SHIFT },
    { CPC_1,          SDLK_AMPERSAND | MOD_PC_SHIFT },
    { CPC_2,          SDLK_WORLD_73 | MOD_PC_SHIFT},
    { CPC_3,          SDLK_QUOTEDBL | MOD_PC_SHIFT },
    { CPC_4,          SDLK_QUOTE | MOD_PC_SHIFT },
    { CPC_5,          SDLK_LEFTPAREN | MOD_PC_SHIFT },
    { CPC_6,          SDLK_MINUS | MOD_PC_SHIFT },
    { CPC_7,          SDLK_WORLD_72 | MOD_PC_SHIFT },
    { CPC_8,          SDLK_UNDERSCORE | MOD_PC_SHIFT },
    { CPC_9,          SDLK_WORLD_71 | MOD_PC_SHIFT },
    { CPC_A,          SDLK_a | MOD_PC_SHIFT },
    { CPC_B,          SDLK_b | MOD_PC_SHIFT },
    { CPC_C,          SDLK_c | MOD_PC_SHIFT },
    { CPC_D,          SDLK_d | MOD_PC_SHIFT },
    { CPC_E,          SDLK_e | MOD_PC_SHIFT },
    { CPC_F,          SDLK_f | MOD_PC_SHIFT },
    { CPC_G,          SDLK_g | MOD_PC_SHIFT },
    { CPC_H,          SDLK_h | MOD_PC_SHIFT },
    { CPC_I,          SDLK_i | MOD_PC_SHIFT },
    { CPC_J,          SDLK_j | MOD_PC_SHIFT },
    { CPC_K,          SDLK_k | MOD_PC_SHIFT },
    { CPC_L,          SDLK_l | MOD_PC_SHIFT },
    { CPC_M,          SDLK_m | MOD_PC_SHIFT },
    { CPC_N,          SDLK_n | MOD_PC_SHIFT },
    { CPC_O,          SDLK_o | MOD_PC_SHIFT },
    { CPC_P,          SDLK_p | MOD_PC_SHIFT },
    { CPC_Q,          SDLK_q | MOD_PC_SHIFT },
    { CPC_R,          SDLK_r | MOD_PC_SHIFT },
    { CPC_S,          SDLK_s | MOD_PC_SHIFT },
    { CPC_T,          SDLK_t | MOD_PC_SHIFT },
    { CPC_U,          SDLK_u | MOD_PC_SHIFT },
    { CPC_V,          SDLK_v | MOD_PC_SHIFT },
    { CPC_W,          SDLK_w | MOD_PC_SHIFT },
    { CPC_X,          SDLK_x | MOD_PC_SHIFT },
    { CPC_Y,          SDLK_y | MOD_PC_SHIFT },
    { CPC_Z,          SDLK_z | MOD_PC_SHIFT },
    { CPC_a,          SDLK_a },
    { CPC_b,          SDLK_b },
    { CPC_c,          SDLK_c },
    { CPC_d,          SDLK_d },
    { CPC_e,          SDLK_e },
    { CPC_f,          SDLK_f },
    { CPC_g,          SDLK_g },
    { CPC_h,          SDLK_h },
    { CPC_i,          SDLK_i },
    { CPC_j,          SDLK_j },
    { CPC_k,          SDLK_k },
    { CPC_l,          SDLK_l },
    { CPC_m,          SDLK_m },
    { CPC_n,          SDLK_n },
    { CPC_o,          SDLK_o },
    { CPC_p,          SDLK_p },
    { CPC_q,          SDLK_q },
    { CPC_r,          SDLK_r },
    { CPC_s,          SDLK_s },
    { CPC_t,          SDLK_t },
    { CPC_u,          SDLK_u },
    { CPC_v,          SDLK_v },
    { CPC_w,          SDLK_w },
    { CPC_x,          SDLK_x },
    { CPC_y,          SDLK_y },
    { CPC_z,          SDLK_z },
    { CPC_AMPERSAND,  SDLK_AMPERSAND },
    { CPC_ASTERISK,   SDLK_ASTERISK },
    { CPC_AT,         SDLK_WORLD_64 | MOD_PC_MODE },
    { CPC_BACKQUOTE,  SDLK_WORLD_73 | MOD_PC_MODE },
    { CPC_BACKSLASH,  SDLK_UNDERSCORE | MOD_PC_MODE },
    { CPC_CAPSLOCK,   SDLK_CAPSLOCK },
    { CPC_CLR,        SDLK_DELETE },
    { CPC_COLON,      SDLK_COLON },
    { CPC_COMMA,      SDLK_COMMA },
    { CPC_CONTROL,    SDLK_LCTRL },
    { CPC_COPY,       SDLK_LALT },
    { CPC_CPY_DOWN,   SDLK_DOWN | MOD_PC_SHIFT },
    { CPC_CPY_LEFT,   SDLK_LEFT | MOD_PC_SHIFT },
    { CPC_CPY_RIGHT,  SDLK_RIGHT | MOD_PC_SHIFT },
    { CPC_CPY_UP,     SDLK_UP | MOD_PC_SHIFT },
    { CPC_CUR_DOWN,   SDLK_DOWN },
    { CPC_CUR_LEFT,   SDLK_LEFT },
    { CPC_CUR_RIGHT,  SDLK_RIGHT },
    { CPC_CUR_UP,     SDLK_UP },
    { CPC_CUR_ENDBL,  SDLK_END | MOD_PC_CTRL },
    { CPC_CUR_HOMELN, SDLK_HOME },
    { CPC_CUR_ENDLN,  SDLK_END },
    { CPC_CUR_HOMEBL, SDLK_HOME | MOD_PC_CTRL },
    { CPC_DBLQUOTE,   SDLK_QUOTEDBL  },
    { CPC_DEL,        SDLK_BACKSPACE },
    { CPC_DOLLAR,     SDLK_DOLLAR },
    { CPC_ENTER,      SDLK_KP_ENTER },
    { CPC_EQUAL,      SDLK_EQUALS },
    { CPC_ESC,        SDLK_ESCAPE },
    { CPC_EXCLAMATN,  SDLK_EXCLAIM },
    { CPC_F0,         SDLK_KP0 },
    { CPC_F1,         SDLK_KP1 },
    { CPC_F2,         SDLK_KP2 },
    { CPC_F3,         SDLK_KP3 },
    { CPC_F4,         SDLK_KP4 },
    { CPC_F5,         SDLK_KP5 },
    { CPC_F6,         SDLK_KP6 },
    { CPC_F7,         SDLK_KP7 },
    { CPC_F8,         SDLK_KP8 },
    { CPC_F9,         SDLK_KP9 },
    { CPC_FR_aGRAVE,  SDLK_WORLD_64 },
    { CPC_FR_cCEDIL,  SDLK_WORLD_71 },
    { CPC_FR_eACUTE,  SDLK_WORLD_73 },
    { CPC_FR_eGRAVE,  SDLK_WORLD_72 },
    { CPC_FR_uGRAVE,  SDLK_WORLD_89 },
    { CPC_FPERIOD,    SDLK_KP_PERIOD },
    { CPC_GREATER,    SDLK_LESS | MOD_PC_SHIFT },
    { CPC_HASH,       SDLK_QUOTEDBL | MOD_PC_MODE },
    { CPC_LBRACKET,   SDLK_LEFTPAREN | MOD_PC_MODE },
    { CPC_LCBRACE,    SDLK_QUOTE | MOD_PC_MODE },
    { CPC_LEFTPAREN,  SDLK_LEFTPAREN },
    { CPC_LESS,       SDLK_LESS },
    { CPC_LSHIFT,     SDLK_LSHIFT },
    { CPC_MINUS,      SDLK_MINUS },
    { CPC_PERCENT,    SDLK_WORLD_89 | MOD_PC_SHIFT },
    { CPC_PERIOD,     SDLK_SEMICOLON | MOD_PC_SHIFT },
    { CPC_PIPE,       SDLK_MINUS | MOD_PC_MODE },
    { CPC_PLUS,       SDLK_EQUALS | MOD_PC_SHIFT },
    { CPC_POUND,      SDLK_DOLLAR | MOD_PC_SHIFT },
    { CPC_POWER,      SDLK_CARET },
    { CPC_QUESTION,   SDLK_COMMA | MOD_PC_SHIFT },
    { CPC_QUOTE,      SDLK_QUOTE },
    { CPC_RBRACKET,   SDLK_RIGHTPAREN | MOD_PC_MODE },
    { CPC_RCBRACE,    SDLK_EQUALS | MOD_PC_MODE },
    { CPC_RETURN,     SDLK_RETURN },
    { CPC_RIGHTPAREN, SDLK_RIGHTPAREN },
    { CPC_RSHIFT,     SDLK_RSHIFT },
    { CPC_SEMICOLON,  SDLK_SEMICOLON },
    { CPC_SLASH,      SDLK_COLON | MOD_PC_SHIFT },
    { CPC_SPACE,      SDLK_SPACE },
    { CPC_TAB,        SDLK_TAB },
    { CPC_UNDERSCORE, SDLK_UNDERSCORE },
    { CAP32_EXIT,     SDLK_F10 },
    { CAP32_FPS,      SDLK_F8 },
    { CAP32_GUI,      SDLK_F1},
    { CAP32_VKBD,     SDLK_F1 | MOD_PC_SHIFT },
    { CAP32_FULLSCRN, SDLK_F2 },
    { CAP32_SCRNDUMP, SDLK_F3 },
    { CAP32_JOY,      SDLK_F7 },
    //{ CAP32_LOADDRVA, SDLK_F6 },
    //{ CAP32_LOADDRVB, SDLK_F7 },
    //{ CAP32_LOADSNAP, SDLK_F2 },
    //{ CAP32_LOADTAPE, SDLK_F3 },
    //{ CAP32_MF2RESET, SDLK_F6 | MOD_PC_CTRL },
    { CAP32_MF2STOP,  SDLK_F6 },
    //{ CAP32_OPTIONS,  SDLK_F8 },
    //{ CAP32_PAUSE,    SDLK_BREAK },
    { CAP32_RESET,    SDLK_F5 },
    //{ CAP32_SAVESNAP, SDLK_F4 },
    //{ CAP32_SCRNSHOT, SDLK_PRINT },
    { CAP32_SPEED,    SDLK_F9 },
    { CAP32_DEBUG,    SDLK_F12 },
    { CAP32_TAPEPLAY, SDLK_F4 }
  },
  { // Spanish PC to CPC keyboard layout translation
    { CPC_0,          SDLK_0 },
    { CPC_1,          SDLK_1 },
    { CPC_2,          SDLK_2 },
    { CPC_3,          SDLK_3 },
    { CPC_4,          SDLK_4 },
    { CPC_5,          SDLK_5 },
    { CPC_6,          SDLK_6 },
    { CPC_7,          SDLK_7 },
    { CPC_8,          SDLK_8 },
    { CPC_9,          SDLK_9 },
    { CPC_A,          SDLK_a | MOD_PC_SHIFT },
    { CPC_B,          SDLK_b | MOD_PC_SHIFT },
    { CPC_C,          SDLK_c | MOD_PC_SHIFT },
    { CPC_D,          SDLK_d | MOD_PC_SHIFT },
    { CPC_E,          SDLK_e | MOD_PC_SHIFT },
    { CPC_F,          SDLK_f | MOD_PC_SHIFT },
    { CPC_G,          SDLK_g | MOD_PC_SHIFT },
    { CPC_H,          SDLK_h | MOD_PC_SHIFT },
    { CPC_I,          SDLK_i | MOD_PC_SHIFT },
    { CPC_J,          SDLK_j | MOD_PC_SHIFT },
    { CPC_K,          SDLK_k | MOD_PC_SHIFT },
    { CPC_L,          SDLK_l | MOD_PC_SHIFT },
    { CPC_M,          SDLK_m | MOD_PC_SHIFT },
    { CPC_N,          SDLK_n | MOD_PC_SHIFT },
    { CPC_O,          SDLK_o | MOD_PC_SHIFT },
    { CPC_P,          SDLK_p | MOD_PC_SHIFT },
    { CPC_Q,          SDLK_q | MOD_PC_SHIFT },
    { CPC_R,          SDLK_r | MOD_PC_SHIFT },
    { CPC_S,          SDLK_s | MOD_PC_SHIFT },
    { CPC_T,          SDLK_t | MOD_PC_SHIFT },
    { CPC_U,          SDLK_u | MOD_PC_SHIFT },
    { CPC_V,          SDLK_v | MOD_PC_SHIFT },
    { CPC_W,          SDLK_w | MOD_PC_SHIFT },
    { CPC_X,          SDLK_x | MOD_PC_SHIFT },
    { CPC_Y,          SDLK_y | MOD_PC_SHIFT },
    { CPC_Z,          SDLK_z | MOD_PC_SHIFT },
    { CPC_a,          SDLK_a },
    { CPC_b,          SDLK_b },
    { CPC_c,          SDLK_c },
    { CPC_d,          SDLK_d },
    { CPC_e,          SDLK_e },
    { CPC_f,          SDLK_f },
    { CPC_g,          SDLK_g },
    { CPC_h,          SDLK_h },
    { CPC_i,          SDLK_i },
    { CPC_j,          SDLK_j },
    { CPC_k,          SDLK_k },
    { CPC_l,          SDLK_l },
    { CPC_m,          SDLK_m },
    { CPC_n,          SDLK_n },
    { CPC_o,          SDLK_o },
    { CPC_p,          SDLK_p },
    { CPC_q,          SDLK_q },
    { CPC_r,          SDLK_r },
    { CPC_s,          SDLK_s },
    { CPC_t,          SDLK_t },
    { CPC_u,          SDLK_u },
    { CPC_v,          SDLK_v },
    { CPC_w,          SDLK_w },
    { CPC_x,          SDLK_x },
    { CPC_y,          SDLK_y },
    { CPC_z,          SDLK_z },
    { CPC_AMPERSAND,  SDLK_7 | MOD_PC_SHIFT },
    { CPC_ASTERISK,   SDLK_8 | MOD_PC_SHIFT },
    { CPC_AT,         SDLK_2 | MOD_PC_SHIFT },
    { CPC_BACKQUOTE,  SDLK_BACKQUOTE },
    { CPC_BACKSLASH,  SDLK_BACKSLASH },
    { CPC_CAPSLOCK,   SDLK_CAPSLOCK },
    { CPC_CLR,        SDLK_DELETE },
    { CPC_COLON,      SDLK_SEMICOLON | MOD_PC_SHIFT },
    { CPC_COMMA,      SDLK_COMMA },
    { CPC_CONTROL,    SDLK_LCTRL },
    { CPC_COPY,       SDLK_LALT },
    { CPC_CPY_DOWN,   SDLK_DOWN | MOD_PC_SHIFT },
    { CPC_CPY_LEFT,   SDLK_LEFT | MOD_PC_SHIFT },
    { CPC_CPY_RIGHT,  SDLK_RIGHT | MOD_PC_SHIFT },
    { CPC_CPY_UP,     SDLK_UP | MOD_PC_SHIFT },
    { CPC_CUR_DOWN,   SDLK_DOWN },
    { CPC_CUR_LEFT,   SDLK_LEFT },
    { CPC_CUR_RIGHT,  SDLK_RIGHT },
    { CPC_CUR_UP,     SDLK_UP },
    { CPC_CUR_ENDBL,  SDLK_END | MOD_PC_CTRL },
    { CPC_CUR_HOMELN, SDLK_HOME },
    { CPC_CUR_ENDLN,  SDLK_END },
    { CPC_CUR_HOMEBL, SDLK_HOME | MOD_PC_CTRL },
    { CPC_DBLQUOTE,   SDLK_QUOTE | MOD_PC_SHIFT },
    { CPC_DEL,        SDLK_BACKSPACE },
    { CPC_DOLLAR,     SDLK_4 | MOD_PC_SHIFT },
    { CPC_ENTER,      SDLK_KP_ENTER },
    { CPC_EQUAL,      SDLK_EQUALS },
    { CPC_ESC,        SDLK_ESCAPE },
    { CPC_EXCLAMATN,  SDLK_1 | MOD_PC_SHIFT },
    { CPC_F0,         SDLK_KP0 },
    { CPC_F1,         SDLK_KP1 },
    { CPC_F2,         SDLK_KP2 },
    { CPC_F3,         SDLK_KP3 },
    { CPC_F4,         SDLK_KP4 },
    { CPC_F5,         SDLK_KP5 },
    { CPC_F6,         SDLK_KP6 },
    { CPC_F7,         SDLK_KP7 },
    { CPC_F8,         SDLK_KP8 },
    { CPC_F9,         SDLK_KP9 },
    { CPC_FPERIOD,    SDLK_KP_PERIOD },
    { CPC_GREATER,    SDLK_PERIOD | MOD_PC_SHIFT },
    { CPC_HASH,       SDLK_3 | MOD_PC_SHIFT },
    { CPC_LBRACKET,   SDLK_LEFTBRACKET },
    { CPC_LCBRACE,    SDLK_LEFTBRACKET | MOD_PC_SHIFT },
    { CPC_LEFTPAREN,  SDLK_9 | MOD_PC_SHIFT },
    { CPC_LESS,       SDLK_COMMA | MOD_PC_SHIFT },
    { CPC_LSHIFT,     SDLK_LSHIFT },
    { CPC_MINUS,      SDLK_MINUS },
    { CPC_PERCENT,    SDLK_5 | MOD_PC_SHIFT },
    { CPC_PERIOD,     SDLK_PERIOD },
    { CPC_PIPE,       SDLK_BACKSLASH | MOD_PC_SHIFT },
    { CPC_PLUS,       SDLK_EQUALS | MOD_PC_SHIFT },
    { CPC_POUND,      0 },
    { CPC_POWER,      SDLK_6 | MOD_PC_SHIFT },
    { CPC_QUESTION,   SDLK_SLASH | MOD_PC_SHIFT },
    { CPC_QUOTE,      SDLK_QUOTE },
    { CPC_RBRACKET,   SDLK_RIGHTBRACKET },
    { CPC_RCBRACE,    SDLK_RIGHTBRACKET | MOD_PC_SHIFT },
    { CPC_RETURN,     SDLK_RETURN },
    { CPC_RIGHTPAREN, SDLK_0 | MOD_PC_SHIFT },
    { CPC_RSHIFT,     SDLK_RSHIFT },
    { CPC_SEMICOLON,  SDLK_SEMICOLON },
    { CPC_SLASH,      SDLK_SLASH },
    { CPC_SPACE,      SDLK_SPACE },
    { CPC_TAB,        SDLK_TAB },
    { CPC_UNDERSCORE, SDLK_MINUS | MOD_PC_SHIFT },
    { CAP32_EXIT,     SDLK_F10 },
    { CAP32_FPS,      SDLK_F8 },
    { CAP32_GUI,      SDLK_F1},
    { CAP32_VKBD,     SDLK_F1 | MOD_PC_SHIFT },
    { CAP32_FULLSCRN, SDLK_F2 },
    { CAP32_SCRNDUMP, SDLK_F3 },
    { CAP32_JOY,      SDLK_F7 },
    //{ CAP32_LOADDRVA, SDLK_F6 },
    //{ CAP32_LOADDRVB, SDLK_F7 },
    //{ CAP32_LOADSNAP, SDLK_F2 },
    //{ CAP32_LOADTAPE, SDLK_F3 },
    //{ CAP32_MF2RESET, SDLK_F6 | MOD_PC_CTRL },
    { CAP32_MF2STOP,  SDLK_F6 },
    //{ CAP32_OPTIONS,  SDLK_F8 },
    //{ CAP32_PAUSE,    SDLK_BREAK },
    { CAP32_RESET,    SDLK_F5 },
    //{ CAP32_SAVESNAP, SDLK_F4 },
    //{ CAP32_SCRNSHOT, SDLK_PRINT },
    { CAP32_SPEED,    SDLK_F9 },
    { CAP32_DEBUG,    SDLK_F12 },
    { CAP32_TAPEPLAY, SDLK_F4 }
  }
};

std::map<char, std::pair<SDLKey, SDLMod>> keysFromChars[3] = {
  { // Char to US PC keyboard translation
    { '&', {SDLK_7, KMOD_RSHIFT} },
    { '#', {SDLK_3, KMOD_RSHIFT} },
    { '"', {SDLK_QUOTE, KMOD_RSHIFT} },
    { '\'', {SDLK_QUOTE, KMOD_NONE} },
    { '(', {SDLK_9, KMOD_RSHIFT} },
    { '-', {SDLK_MINUS, KMOD_NONE} },
    { '_', {SDLK_MINUS, KMOD_RSHIFT} },
    { ')', {SDLK_0, KMOD_RSHIFT} },
    { '=', {SDLK_EQUALS, KMOD_NONE} },
    { '*', {SDLK_8, KMOD_RSHIFT} },
    { ',', {SDLK_COMMA, KMOD_NONE} },
    { ';', {SDLK_SEMICOLON, KMOD_NONE} },
    { ':', {SDLK_SEMICOLON, KMOD_RSHIFT} },
    { '!', {SDLK_1, KMOD_RSHIFT} },
    { '$', {SDLK_4, KMOD_RSHIFT} },
    { 'a', {SDLK_a, KMOD_NONE} },
    { 'b', {SDLK_b, KMOD_NONE} },
    { 'c', {SDLK_c, KMOD_NONE} },
    { 'd', {SDLK_d, KMOD_NONE} },
    { 'e', {SDLK_e, KMOD_NONE} },
    { 'f', {SDLK_f, KMOD_NONE} },
    { 'g', {SDLK_g, KMOD_NONE} },
    { 'h', {SDLK_h, KMOD_NONE} },
    { 'i', {SDLK_i, KMOD_NONE} },
    { 'j', {SDLK_j, KMOD_NONE} },
    { 'k', {SDLK_k, KMOD_NONE} },
    { 'l', {SDLK_l, KMOD_NONE} },
    { 'm', {SDLK_m, KMOD_NONE} },
    { 'n', {SDLK_n, KMOD_NONE} },
    { 'o', {SDLK_o, KMOD_NONE} },
    { 'p', {SDLK_p, KMOD_NONE} },
    { 'q', {SDLK_q, KMOD_NONE} },
    { 'r', {SDLK_r, KMOD_NONE} },
    { 's', {SDLK_s, KMOD_NONE} },
    { 't', {SDLK_t, KMOD_NONE} },
    { 'u', {SDLK_u, KMOD_NONE} },
    { 'v', {SDLK_v, KMOD_NONE} },
    { 'w', {SDLK_w, KMOD_NONE} },
    { 'x', {SDLK_x, KMOD_NONE} },
    { 'y', {SDLK_y, KMOD_NONE} },
    { 'z', {SDLK_z, KMOD_NONE} },
    { 'A', {SDLK_a, KMOD_RSHIFT} },
    { 'B', {SDLK_b, KMOD_RSHIFT} },
    { 'C', {SDLK_c, KMOD_RSHIFT} },
    { 'D', {SDLK_d, KMOD_RSHIFT} },
    { 'E', {SDLK_e, KMOD_RSHIFT} },
    { 'F', {SDLK_f, KMOD_RSHIFT} },
    { 'G', {SDLK_g, KMOD_RSHIFT} },
    { 'H', {SDLK_h, KMOD_RSHIFT} },
    { 'I', {SDLK_i, KMOD_RSHIFT} },
    { 'J', {SDLK_j, KMOD_RSHIFT} },
    { 'K', {SDLK_k, KMOD_RSHIFT} },
    { 'L', {SDLK_l, KMOD_RSHIFT} },
    { 'M', {SDLK_m, KMOD_RSHIFT} },
    { 'N', {SDLK_n, KMOD_RSHIFT} },
    { 'O', {SDLK_o, KMOD_RSHIFT} },
    { 'P', {SDLK_p, KMOD_RSHIFT} },
    { 'Q', {SDLK_q, KMOD_RSHIFT} },
    { 'R', {SDLK_r, KMOD_RSHIFT} },
    { 'S', {SDLK_s, KMOD_RSHIFT} },
    { 'T', {SDLK_t, KMOD_RSHIFT} },
    { 'U', {SDLK_u, KMOD_RSHIFT} },
    { 'V', {SDLK_v, KMOD_RSHIFT} },
    { 'W', {SDLK_w, KMOD_RSHIFT} },
    { 'X', {SDLK_x, KMOD_RSHIFT} },
    { 'Y', {SDLK_y, KMOD_RSHIFT} },
    { 'Z', {SDLK_z, KMOD_RSHIFT} },
    { '0', {SDLK_0, KMOD_NONE} },
    { '1', {SDLK_1, KMOD_NONE} },
    { '2', {SDLK_2, KMOD_NONE} },
    { '3', {SDLK_3, KMOD_NONE} },
    { '4', {SDLK_4, KMOD_NONE} },
    { '5', {SDLK_5, KMOD_NONE} },
    { '6', {SDLK_6, KMOD_NONE} },
    { '7', {SDLK_7, KMOD_NONE} },
    { '8', {SDLK_8, KMOD_NONE} },
    { '9', {SDLK_9, KMOD_NONE} },
    { '|', {SDLK_BACKSLASH, KMOD_RSHIFT} },
    { '?', {SDLK_SLASH, KMOD_RSHIFT}},
    { '.', {SDLK_PERIOD, KMOD_NONE} },
    { '/', {SDLK_SLASH, KMOD_NONE} },
    { ' ', {SDLK_SPACE, KMOD_NONE} },
    { '\n', {SDLK_RETURN, KMOD_NONE} },
    { '\b', {SDLK_BACKSPACE, KMOD_NONE} },
    { '+', {SDLK_EQUALS, KMOD_RSHIFT} },
    { '%', {SDLK_5, KMOD_RSHIFT} },
    { '<', {SDLK_COMMA, KMOD_RSHIFT} },
    { '>', {SDLK_PERIOD, KMOD_RSHIFT} },
    { '[', {SDLK_LEFTBRACKET, KMOD_NONE} },
    { ']', {SDLK_RIGHTBRACKET, KMOD_NONE} },
    { '{', {SDLK_LEFTBRACKET, KMOD_RSHIFT} },
    { '}', {SDLK_RIGHTBRACKET, KMOD_RSHIFT} },
    { '\\', {SDLK_BACKSLASH, KMOD_NONE} },
    { '`', {SDLK_BACKQUOTE, KMOD_NONE} },
    // Not (yet?) on virtual keyboard
    { '@', {SDLK_2, KMOD_RSHIFT} },
    { '^', {SDLK_6, KMOD_RSHIFT} },
    //{ '~', {0, KMOD_NONE} } // should be pound but it's not part of base ascii (it's in extended ASCII)
  },
  { // Char to French PC keyboard translation
    { '&', {SDLK_AMPERSAND, KMOD_NONE} },
    { '#', {SDLK_QUOTEDBL, KMOD_MODE} },
    { '"', {SDLK_QUOTEDBL, KMOD_NONE} },
    { '\'', {SDLK_QUOTE, KMOD_NONE} },
    { '(', {SDLK_LEFTPAREN, KMOD_NONE} },
    { '-', {SDLK_MINUS, KMOD_NONE} },
    { '_', {SDLK_UNDERSCORE, KMOD_NONE} },
    { ')', {SDLK_RIGHTPAREN, KMOD_NONE} },
    { '=', {SDLK_EQUALS, KMOD_NONE} },
    { '*', {SDLK_ASTERISK, KMOD_NONE} },
    { ',', {SDLK_COMMA, KMOD_NONE} },
    { ';', {SDLK_SEMICOLON, KMOD_NONE} },
    { ':', {SDLK_COLON, KMOD_NONE} },
    { '!', {SDLK_EXCLAIM, KMOD_NONE} },
    { '$', {SDLK_DOLLAR, KMOD_NONE} },
    { 'a', {SDLK_a, KMOD_NONE} },
    { 'b', {SDLK_b, KMOD_NONE} },
    { 'c', {SDLK_c, KMOD_NONE} },
    { 'd', {SDLK_d, KMOD_NONE} },
    { 'e', {SDLK_e, KMOD_NONE} },
    { 'f', {SDLK_f, KMOD_NONE} },
    { 'g', {SDLK_g, KMOD_NONE} },
    { 'h', {SDLK_h, KMOD_NONE} },
    { 'i', {SDLK_i, KMOD_NONE} },
    { 'j', {SDLK_j, KMOD_NONE} },
    { 'k', {SDLK_k, KMOD_NONE} },
    { 'l', {SDLK_l, KMOD_NONE} },
    { 'm', {SDLK_m, KMOD_NONE} },
    { 'n', {SDLK_n, KMOD_NONE} },
    { 'o', {SDLK_o, KMOD_NONE} },
    { 'p', {SDLK_p, KMOD_NONE} },
    { 'q', {SDLK_q, KMOD_NONE} },
    { 'r', {SDLK_r, KMOD_NONE} },
    { 's', {SDLK_s, KMOD_NONE} },
    { 't', {SDLK_t, KMOD_NONE} },
    { 'u', {SDLK_u, KMOD_NONE} },
    { 'v', {SDLK_v, KMOD_NONE} },
    { 'w', {SDLK_w, KMOD_NONE} },
    { 'x', {SDLK_x, KMOD_NONE} },
    { 'y', {SDLK_y, KMOD_NONE} },
    { 'z', {SDLK_z, KMOD_NONE} },
    { 'A', {SDLK_a, KMOD_RSHIFT} },
    { 'B', {SDLK_b, KMOD_RSHIFT} },
    { 'C', {SDLK_c, KMOD_RSHIFT} },
    { 'D', {SDLK_d, KMOD_RSHIFT} },
    { 'E', {SDLK_e, KMOD_RSHIFT} },
    { 'F', {SDLK_f, KMOD_RSHIFT} },
    { 'G', {SDLK_g, KMOD_RSHIFT} },
    { 'H', {SDLK_h, KMOD_RSHIFT} },
    { 'I', {SDLK_i, KMOD_RSHIFT} },
    { 'J', {SDLK_j, KMOD_RSHIFT} },
    { 'K', {SDLK_k, KMOD_RSHIFT} },
    { 'L', {SDLK_l, KMOD_RSHIFT} },
    { 'M', {SDLK_m, KMOD_RSHIFT} },
    { 'N', {SDLK_n, KMOD_RSHIFT} },
    { 'O', {SDLK_o, KMOD_RSHIFT} },
    { 'P', {SDLK_p, KMOD_RSHIFT} },
    { 'Q', {SDLK_q, KMOD_RSHIFT} },
    { 'R', {SDLK_r, KMOD_RSHIFT} },
    { 'S', {SDLK_s, KMOD_RSHIFT} },
    { 'T', {SDLK_t, KMOD_RSHIFT} },
    { 'U', {SDLK_u, KMOD_RSHIFT} },
    { 'V', {SDLK_v, KMOD_RSHIFT} },
    { 'W', {SDLK_w, KMOD_RSHIFT} },
    { 'X', {SDLK_x, KMOD_RSHIFT} },
    { 'Y', {SDLK_y, KMOD_RSHIFT} },
    { 'Z', {SDLK_z, KMOD_RSHIFT} },
    { '0', {SDLK_WORLD_64, KMOD_RSHIFT} },
    { '1', {SDLK_AMPERSAND, KMOD_RSHIFT} },
    { '2', {SDLK_WORLD_73, KMOD_RSHIFT} },
    { '3', {SDLK_QUOTEDBL, KMOD_RSHIFT} },
    { '4', {SDLK_QUOTE, KMOD_RSHIFT} },
    { '5', {SDLK_LEFTPAREN, KMOD_RSHIFT} },
    { '6', {SDLK_MINUS, KMOD_RSHIFT} },
    { '7', {SDLK_WORLD_72, KMOD_RSHIFT} },
    { '8', {SDLK_UNDERSCORE, KMOD_RSHIFT} },
    { '9', {SDLK_WORLD_71, KMOD_RSHIFT} },
    { '|', {SDLK_MINUS, KMOD_MODE} },
    { '?', {SDLK_COMMA, KMOD_RSHIFT}},
    { '.', {SDLK_SEMICOLON, KMOD_RSHIFT} },
    { '/', {SDLK_COLON, KMOD_RSHIFT} },
    { ' ', {SDLK_SPACE, KMOD_NONE} },
    { '\n', {SDLK_RETURN, KMOD_NONE} },
    { '\b', {SDLK_BACKSPACE, KMOD_NONE} },
    { '+', {SDLK_EQUALS, KMOD_RSHIFT} },
    { '%', {SDLK_WORLD_89, KMOD_RSHIFT} },
    { '<', {SDLK_LESS, KMOD_NONE} },
    { '>', {SDLK_LESS, KMOD_RSHIFT} },
    { '[', {SDLK_LEFTPAREN, KMOD_MODE} },
    { ']', {SDLK_RIGHTPAREN, KMOD_MODE} },
    { '{', {SDLK_QUOTE, KMOD_MODE} },
    { '}', {SDLK_EQUALS, KMOD_MODE} },
    { '\\', {SDLK_UNDERSCORE, KMOD_MODE} },
    { '`', {SDLK_WORLD_72, KMOD_MODE} },
    // Not (yet?) on virtual keyboard
    { '@', {SDLK_WORLD_64, KMOD_MODE} },
    { '^', {SDLK_CARET, KMOD_NONE} },
    //{ '~', {0, KMOD_NONE} } // should be pound but it's not part of base ascii (it's in extended ASCII)
  },
  { // Char to Spanish PC keyboard translation
    { '&', {SDLK_7, KMOD_RSHIFT} },
    { '#', {SDLK_3, KMOD_RSHIFT} },
    { '"', {SDLK_QUOTE, KMOD_RSHIFT} },
    { '\'', {SDLK_QUOTE, KMOD_NONE} },
    { '(', {SDLK_9, KMOD_RSHIFT} },
    { '-', {SDLK_MINUS, KMOD_NONE} },
    { '_', {SDLK_MINUS, KMOD_RSHIFT} },
    { ')', {SDLK_0, KMOD_RSHIFT} },
    { '=', {SDLK_EQUALS, KMOD_NONE} },
    { '*', {SDLK_8, KMOD_RSHIFT} },
    { ',', {SDLK_COMMA, KMOD_NONE} },
    { ';', {SDLK_SEMICOLON, KMOD_NONE} },
    { ':', {SDLK_SEMICOLON, KMOD_RSHIFT} },
    { '!', {SDLK_1, KMOD_RSHIFT} },
    { '$', {SDLK_4, KMOD_RSHIFT} },
    { 'a', {SDLK_a, KMOD_NONE} },
    { 'b', {SDLK_b, KMOD_NONE} },
    { 'c', {SDLK_c, KMOD_NONE} },
    { 'd', {SDLK_d, KMOD_NONE} },
    { 'e', {SDLK_e, KMOD_NONE} },
    { 'f', {SDLK_f, KMOD_NONE} },
    { 'g', {SDLK_g, KMOD_NONE} },
    { 'h', {SDLK_h, KMOD_NONE} },
    { 'i', {SDLK_i, KMOD_NONE} },
    { 'j', {SDLK_j, KMOD_NONE} },
    { 'k', {SDLK_k, KMOD_NONE} },
    { 'l', {SDLK_l, KMOD_NONE} },
    { 'm', {SDLK_m, KMOD_NONE} },
    { 'n', {SDLK_n, KMOD_NONE} },
    { 'o', {SDLK_o, KMOD_NONE} },
    { 'p', {SDLK_p, KMOD_NONE} },
    { 'q', {SDLK_q, KMOD_NONE} },
    { 'r', {SDLK_r, KMOD_NONE} },
    { 's', {SDLK_s, KMOD_NONE} },
    { 't', {SDLK_t, KMOD_NONE} },
    { 'u', {SDLK_u, KMOD_NONE} },
    { 'v', {SDLK_v, KMOD_NONE} },
    { 'w', {SDLK_w, KMOD_NONE} },
    { 'x', {SDLK_x, KMOD_NONE} },
    { 'y', {SDLK_y, KMOD_NONE} },
    { 'z', {SDLK_z, KMOD_NONE} },
    { 'A', {SDLK_a, KMOD_RSHIFT} },
    { 'B', {SDLK_b, KMOD_RSHIFT} },
    { 'C', {SDLK_c, KMOD_RSHIFT} },
    { 'D', {SDLK_d, KMOD_RSHIFT} },
    { 'E', {SDLK_e, KMOD_RSHIFT} },
    { 'F', {SDLK_f, KMOD_RSHIFT} },
    { 'G', {SDLK_g, KMOD_RSHIFT} },
    { 'H', {SDLK_h, KMOD_RSHIFT} },
    { 'I', {SDLK_i, KMOD_RSHIFT} },
    { 'J', {SDLK_j, KMOD_RSHIFT} },
    { 'K', {SDLK_k, KMOD_RSHIFT} },
    { 'L', {SDLK_l, KMOD_RSHIFT} },
    { 'M', {SDLK_m, KMOD_RSHIFT} },
    { 'N', {SDLK_n, KMOD_RSHIFT} },
    { 'O', {SDLK_o, KMOD_RSHIFT} },
    { 'P', {SDLK_p, KMOD_RSHIFT} },
    { 'Q', {SDLK_q, KMOD_RSHIFT} },
    { 'R', {SDLK_r, KMOD_RSHIFT} },
    { 'S', {SDLK_s, KMOD_RSHIFT} },
    { 'T', {SDLK_t, KMOD_RSHIFT} },
    { 'U', {SDLK_u, KMOD_RSHIFT} },
    { 'V', {SDLK_v, KMOD_RSHIFT} },
    { 'W', {SDLK_w, KMOD_RSHIFT} },
    { 'X', {SDLK_x, KMOD_RSHIFT} },
    { 'Y', {SDLK_y, KMOD_RSHIFT} },
    { 'Z', {SDLK_z, KMOD_RSHIFT} },
    { '0', {SDLK_0, KMOD_NONE} },
    { '1', {SDLK_1, KMOD_NONE} },
    { '2', {SDLK_2, KMOD_NONE} },
    { '3', {SDLK_3, KMOD_NONE} },
    { '4', {SDLK_4, KMOD_NONE} },
    { '5', {SDLK_5, KMOD_NONE} },
    { '6', {SDLK_6, KMOD_NONE} },
    { '7', {SDLK_7, KMOD_NONE} },
    { '8', {SDLK_8, KMOD_NONE} },
    { '9', {SDLK_9, KMOD_NONE} },
    { '|', {SDLK_BACKSLASH, KMOD_RSHIFT} },
    { '?', {SDLK_SLASH, KMOD_RSHIFT}},
    { '.', {SDLK_PERIOD, KMOD_NONE} },
    { '/', {SDLK_SLASH, KMOD_NONE} },
    { ' ', {SDLK_SPACE, KMOD_NONE} },
    { '\n', {SDLK_RETURN, KMOD_NONE} },
    { '\b', {SDLK_BACKSPACE, KMOD_NONE} },
    { '+', {SDLK_EQUALS, KMOD_RSHIFT} },
    { '%', {SDLK_5, KMOD_RSHIFT} },
    { '<', {SDLK_COMMA, KMOD_RSHIFT} },
    { '>', {SDLK_PERIOD, KMOD_RSHIFT} },
    { '[', {SDLK_LEFTBRACKET, KMOD_NONE} },
    { ']', {SDLK_RIGHTBRACKET, KMOD_NONE} },
    { '{', {SDLK_LEFTBRACKET, KMOD_RSHIFT} },
    { '}', {SDLK_RIGHTBRACKET, KMOD_RSHIFT} },
    { '\\', {SDLK_BACKSLASH, KMOD_NONE} },
    { '`', {SDLK_BACKQUOTE, KMOD_NONE} },
    // Not (yet?) on virtual keyboard
    { '@', {SDLK_2, KMOD_RSHIFT} },
    { '^', {SDLK_6, KMOD_RSHIFT} },
    //{ '~', {0, KMOD_NONE} } // should be pound but it's not part of base ascii (it's in extended ASCII)
  }
};
