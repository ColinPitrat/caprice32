#include <benchmark/benchmark.h>
#include <cstring>

#include "crtc.h"
#include "cap32.h"

// Global variables defined in crtc.cpp/cap32.cpp
extern byte *RendWid, *RendOut;
extern dword *RendPos, *ModeMap;
extern byte *pbRAM;
extern t_GateArray GateArray;
extern t_CPC CPC;
extern t_CRTC CRTC;

// Mock data
static byte counts[256];
static byte indices[1024];
static byte screen[640 * 480 * 4];
static byte ram[64 * 1024];
static dword rend_pos_buff[1024];
static dword mode_map_buff[512];

static void setup_benchmark() {
    RendWid = counts;
    RendOut = indices;
    CPC.scr_pos = screen;
    CPC.scr_bps = 640 * 4;
    pbRAM = ram;
    RendPos = rend_pos_buff;
    ModeMap = mode_map_buff;
    CRTC.next_address = 0;

    memset(counts, 40, sizeof(counts));
    memset(indices, 1, sizeof(indices));
    memset(screen, 0, sizeof(screen));
    memset(ram, 0x55, sizeof(ram));
    memset(rend_pos_buff, 0, sizeof(rend_pos_buff));
    for (int i = 0; i < 512; ++i) mode_map_buff[i] = i;
    for (int i = 0; i < 34; ++i) GateArray.palette[i] = i * 0x01010101;
}

static void BM_PrerenderBorder(benchmark::State& state) {
    setup_benchmark();
    for (auto _ : state) {
        RendPos = rend_pos_buff;
        prerender_border();
        benchmark::DoNotOptimize(RendPos);
    }
}
BENCHMARK(BM_PrerenderBorder);

static void BM_PrerenderNormal(benchmark::State& state) {
    setup_benchmark();
    for (auto _ : state) {
        RendPos = rend_pos_buff;
        prerender_normal();
        benchmark::DoNotOptimize(RendPos);
    }
}
BENCHMARK(BM_PrerenderNormal);

static void BM_Render8bpp(benchmark::State& state) {
    setup_benchmark();
    for (auto _ : state) {
        RendWid = counts;
        RendOut = indices;
        CPC.scr_pos = screen;
        render8bpp();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Render8bpp);

static void BM_Render8bppDoubleY(benchmark::State& state) {
    setup_benchmark();
    for (auto _ : state) {
        RendWid = counts;
        RendOut = indices;
        CPC.scr_pos = screen;
        render8bpp_doubleY();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Render8bppDoubleY);

static void BM_Render16bpp(benchmark::State& state) {
    setup_benchmark();
    for (auto _ : state) {
        RendWid = counts;
        RendOut = indices;
        CPC.scr_pos = screen;
        render16bpp();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Render16bpp);

static void BM_Render16bppDoubleY(benchmark::State& state) {
    setup_benchmark();
    for (auto _ : state) {
        RendWid = counts;
        RendOut = indices;
        CPC.scr_pos = screen;
        render16bpp_doubleY();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Render16bppDoubleY);

static void BM_Render24bpp(benchmark::State& state) {
    setup_benchmark();
    for (auto _ : state) {
        RendWid = counts;
        RendOut = indices;
        CPC.scr_pos = screen;
        render24bpp();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Render24bpp);

static void BM_Render24bppDoubleY(benchmark::State& state) {
    setup_benchmark();
    for (auto _ : state) {
        RendWid = counts;
        RendOut = indices;
        CPC.scr_pos = screen;
        render24bpp_doubleY();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Render24bppDoubleY);

static void BM_Render32bpp(benchmark::State& state) {
    setup_benchmark();
    for (auto _ : state) {
        RendWid = counts;
        RendOut = indices;
        CPC.scr_pos = screen;
        render32bpp();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Render32bpp);

static void BM_Render32bppDoubleY(benchmark::State& state) {
    setup_benchmark();
    for (auto _ : state) {
        RendWid = counts;
        RendOut = indices;
        CPC.scr_pos = screen;
        render32bpp_doubleY();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Render32bppDoubleY);

