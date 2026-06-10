#include <gtest/gtest.h>
#include <cstring>

#include "crtc.h"
#include "cap32.h"
#include "asic.h"

extern byte *RendWid, *RendOut;
extern dword *RendPos, *ModeMap;
extern byte *pbRAM;
extern t_GateArray GateArray;
extern t_CPC CPC;
extern t_CRTC CRTC;
extern asic_t asic;

class CrtcTest : public testing::Test {
   public:
   protected:
    byte counts[256];
    byte indices[1024];
    byte screen[8192];
    byte ram[1024];
    dword rend_pos_buff[1024];
    dword mode_map_buff[512];

    void SetUp() override {
        RendWid = counts;
        RendOut = indices;
        CPC.scr_pos = screen;
        CPC.scr_bps = 128;
        pbRAM = ram;
        RendPos = rend_pos_buff;
        ModeMap = mode_map_buff;
        CRTC.next_address = 0;
        asic.hscroll = 0;
        asic.vscroll = 0;
        memset(counts, 0, sizeof(counts));
        memset(indices, 0, sizeof(indices));
        memset(screen, 0, sizeof(screen));
        memset(ram, 0, sizeof(ram));
        memset(rend_pos_buff, 0, sizeof(rend_pos_buff));
        memset(mode_map_buff, 0, sizeof(mode_map_buff));
        memset(GateArray.palette, 0, sizeof(GateArray.palette));
    }
};

TEST_F(CrtcTest, ShiftLittleEndianDwordTriplet)
{
   dword val;
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 0);
   EXPECT_EQ(0x01020304, val);
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 4);
   EXPECT_EQ(0x89ABCDEF, val);
   val = shiftLittleEndianDwordTriplet(0x01234567, 0x89ABCDEF, 0x01020304, 8);
   EXPECT_EQ(0x01234567, val);
}

TEST_F(CrtcTest, PrerenderNormal) {
    CRTC.next_address = 10;
    ram[10] = 0xAA;
    ram[11] = 0xBB;
    mode_map_buff[0xAA * 2] = 0x11111111;
    mode_map_buff[0xAA * 2 + 1] = 0x22222222;
    mode_map_buff[0xBB * 2] = 0x33333333;
    mode_map_buff[0xBB * 2 + 1] = 0x44444444;

    prerender_normal();

    EXPECT_EQ(rend_pos_buff[0], 0x11111111);
    EXPECT_EQ(rend_pos_buff[1], 0x22222222);
    EXPECT_EQ(rend_pos_buff[2], 0x33333333);
    EXPECT_EQ(rend_pos_buff[3], 0x44444444);
    EXPECT_EQ(RendPos, rend_pos_buff + 4);
}

TEST_F(CrtcTest, PrerenderBorder) {
    prerender_border();
    EXPECT_EQ(rend_pos_buff[0], 0x10101010);
    EXPECT_EQ(rend_pos_buff[1], 0x10101010);
    EXPECT_EQ(rend_pos_buff[2], 0x10101010);
    EXPECT_EQ(rend_pos_buff[3], 0x10101010);
    EXPECT_EQ(RendPos, rend_pos_buff + 4);
}

TEST_F(CrtcTest, PrerenderBorderHalf) {
    prerender_border_half();
    EXPECT_EQ(rend_pos_buff[0], 0x10101010);
    EXPECT_EQ(rend_pos_buff[1], 0x10101010);
    EXPECT_EQ(RendPos, rend_pos_buff + 2);
}

TEST_F(CrtcTest, PrerenderSync) {
    prerender_sync();
    EXPECT_EQ(rend_pos_buff[0], 0x11111111);
    EXPECT_EQ(rend_pos_buff[1], 0x11111111);
    EXPECT_EQ(rend_pos_buff[2], 0x11111111);
    EXPECT_EQ(rend_pos_buff[3], 0x11111111);
    EXPECT_EQ(RendPos, rend_pos_buff + 4);
}

TEST_F(CrtcTest, PrerenderSyncHalf) {
    prerender_sync_half();
    EXPECT_EQ(rend_pos_buff[0], 0x11111111);
    EXPECT_EQ(rend_pos_buff[1], 0x11111111);
    EXPECT_EQ(RendPos, rend_pos_buff + 2);
}

TEST_F(CrtcTest, PrerenderNormalHalf) {
    CRTC.next_address = 10;
    ram[10] = 0xAA;
    ram[11] = 0xBB;
    mode_map_buff[0xAA] = 0x11111111;
    mode_map_buff[0xBB] = 0x22222222;

    prerender_normal_half();

    EXPECT_EQ(rend_pos_buff[0], 0x11111111);
    EXPECT_EQ(rend_pos_buff[1], 0x22222222);
    EXPECT_EQ(RendPos, rend_pos_buff + 2);
}

TEST_F(CrtcTest, PrerenderNormalPlus) {
    asic.hscroll = 0;
    CRTC.next_address = 10;
    ram[10] = 0xAA;
    ram[11] = 0xBB;
    mode_map_buff[0xAA * 2] = 0x11111111;
    mode_map_buff[0xAA * 2 + 1] = 0x22222222;
    mode_map_buff[0xBB * 2] = 0x33333333;
    mode_map_buff[0xBB * 2 + 1] = 0x44444444;

    prerender_normal_plus();

    EXPECT_EQ(rend_pos_buff[0], 0x11111111);
    EXPECT_EQ(rend_pos_buff[1], 0x22222222);
    EXPECT_EQ(rend_pos_buff[2], 0x33333333);
    EXPECT_EQ(rend_pos_buff[3], 0x44444444);
}

TEST_F(CrtcTest, PrerenderNormalHalfPlus) {
    asic.hscroll = 0;
    CRTC.next_address = 10;
    ram[10] = 0xAA;
    ram[11] = 0xBB;
    mode_map_buff[0xAA] = 0x11111111;
    mode_map_buff[0xBB] = 0x22222222;

    prerender_normal_half_plus();

    EXPECT_EQ(rend_pos_buff[0], 0x11111111);
    EXPECT_EQ(rend_pos_buff[1], 0x22222222);
}


TEST_F(CrtcTest, Render8bpp) {
    counts[0] = 2;
    indices[0] = 1;
    indices[1] = 2;
    GateArray.palette[1] = 0x11;
    GateArray.palette[2] = 0x22;

    render8bpp();

    EXPECT_EQ(screen[0], 0x11);
    EXPECT_EQ(screen[1], 0x22);
    EXPECT_EQ(CPC.scr_pos, screen + 2);
}

TEST_F(CrtcTest, Render8bpp_DoubleY) {
    counts[0] = 2;
    indices[0] = 5;
    indices[1] = 6;
    GateArray.palette[5] = 0x55;
    GateArray.palette[6] = 0x66;
    CPC.scr_bps = 10;

    render8bpp_doubleY();

    EXPECT_EQ(screen[0], 0x55);
    EXPECT_EQ(screen[1], 0x66);
    EXPECT_EQ(screen[10], 0x55);
    EXPECT_EQ(screen[11], 0x66);
    EXPECT_EQ(CPC.scr_pos, screen + 2);
}

TEST_F(CrtcTest, Render16bpp) {
    counts[0] = 2;
    indices[0] = 1;
    indices[1] = 2;
    GateArray.palette[1] = 0x1122;
    GateArray.palette[2] = 0x3344;

    render16bpp();

    word *s = reinterpret_cast<word*>(screen);
    EXPECT_EQ(s[0], 0x1122);
    EXPECT_EQ(s[1], 0x3344);
    EXPECT_EQ(CPC.scr_pos, screen + 4);
}

TEST_F(CrtcTest, Render16bpp_DoubleY) {
    counts[0] = 2;
    indices[0] = 1;
    indices[1] = 2;
    GateArray.palette[1] = 0x1122;
    GateArray.palette[2] = 0x3344;
    CPC.scr_bps = 10;

    render16bpp_doubleY();

    word *s1 = reinterpret_cast<word*>(screen);
    word *s2 = reinterpret_cast<word*>(screen + 10);
    EXPECT_EQ(s1[0], 0x1122);
    EXPECT_EQ(s1[1], 0x3344);
    EXPECT_EQ(s2[0], 0x1122);
    EXPECT_EQ(s2[1], 0x3344);
    EXPECT_EQ(CPC.scr_pos, screen + 4);
}

TEST_F(CrtcTest, Render24bpp) {
    counts[0] = 2;
    indices[0] = 1;
    indices[1] = 2;
    GateArray.palette[1] = 0x112233;
    GateArray.palette[2] = 0x445566;

    render24bpp();

    EXPECT_EQ(screen[0], 0x33);
    EXPECT_EQ(screen[1], 0x22);
    EXPECT_EQ(screen[2], 0x11);
    EXPECT_EQ(screen[3], 0x66);
    EXPECT_EQ(screen[4], 0x55);
    EXPECT_EQ(screen[5], 0x44);
    EXPECT_EQ(CPC.scr_pos, screen + 6);
}

TEST_F(CrtcTest, Render24bpp_DoubleY) {
    counts[0] = 1;
    indices[0] = 1;
    GateArray.palette[1] = 0x112233;
    CPC.scr_bps = 10;

    render24bpp_doubleY();

    EXPECT_EQ(screen[0], 0x33);
    EXPECT_EQ(screen[1], 0x22);
    EXPECT_EQ(screen[2], 0x11);
    EXPECT_EQ(screen[10], 0x33);
    EXPECT_EQ(screen[11], 0x22);
    EXPECT_EQ(screen[12], 0x11);
    EXPECT_EQ(CPC.scr_pos, screen + 3);
}

TEST_F(CrtcTest, Render32bpp) {
    counts[0] = 2;
    indices[0] = 1;
    indices[1] = 2;
    GateArray.palette[1] = 0x11223344;
    GateArray.palette[2] = 0x55667788;

    render32bpp();

    dword *s = reinterpret_cast<dword*>(screen);
    EXPECT_EQ(s[0], 0x11223344);
    EXPECT_EQ(s[1], 0x55667788);
    EXPECT_EQ(CPC.scr_pos, screen + 8);
}

TEST_F(CrtcTest, Render32bpp_DoubleY) {
    counts[0] = 2;
    indices[0] = 1;
    indices[1] = 2;
    GateArray.palette[1] = 0x11223344;
    GateArray.palette[2] = 0x55667788;
    CPC.scr_bps = 16;

    render32bpp_doubleY();

    dword *s1 = reinterpret_cast<dword*>(screen);
    dword *s2 = reinterpret_cast<dword*>(screen + 16);
    EXPECT_EQ(s1[0], 0x11223344);
    EXPECT_EQ(s1[1], 0x55667788);
    EXPECT_EQ(s2[0], 0x11223344);
    EXPECT_EQ(s2[1], 0x55667788);
    EXPECT_EQ(CPC.scr_pos, screen + 8);
}
