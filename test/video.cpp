#include <gtest/gtest.h>
#include "video.h"
#include "cap32.h"
#include <algorithm>
#include <random>

extern SDL_Surface* pub;
extern SDL_Surface* scaled;

namespace
{

class ComputeRectsTest : public testing::Test {
  public:
    // Verifies that src corresponds to the whole screen
    void ExpectFullSrc(bool half_pixels) {
      EXPECT_EQ(src.x, 0);
      EXPECT_EQ(src.y, 0);
      if (half_pixels)
      {
        EXPECT_EQ(src.w, CPC_VISIBLE_SCR_WIDTH);
        // The -4 corresponds to the 'src->h-=2*2' in video.cpp when dh <= 0
        EXPECT_EQ(src.h, CPC_VISIBLE_SCR_HEIGHT-4);
      }
      else
      {
        EXPECT_EQ(src.w, CPC_VISIBLE_SCR_WIDTH*2);
        // The -4 corresponds to the 'src->h-=2*2' in video.cpp when dh <= 0
        EXPECT_EQ(src.h, CPC_VISIBLE_SCR_HEIGHT*2-4);
      }
    }

    // Verifies that a rectangle corresponds to the whole surface
    void ExpectRectMatchesSurface(SDL_Rect *r, SDL_Surface *s) {
      EXPECT_EQ(r->x, 0);
      EXPECT_EQ(r->y, 0);
      EXPECT_EQ(r->w, s->w);
      EXPECT_EQ(r->h, s->h);
    }

    // Verifies that a rectangle fits in the corresponding surface
    void ExpectRectInSurface(SDL_Rect *r, SDL_Surface *s) {
      EXPECT_LE(r->x + r->w, s->w);
      EXPECT_LE(r->y + r->h, s->h);
    }

    // Verifies that src scaled by a factor 2 fits in dst
    void ExpectSrcFitsInDst() {
      EXPECT_LE(src.w*2, dst.w);
      EXPECT_LE(src.h*2, dst.h);
    }

    // Some checks that should be valid for any call.
    void ExpectValid() {
      ExpectRectInSurface(&src, pub);
      ExpectRectInSurface(&dst, scaled);
      ExpectSrcFitsInDst();
    }

    SDL_Surface* CreateSurface(int width, int height) {
      // BPP shouldn't influence this method. Anyway, it is currently called only by filters that support only 16bpp.
      return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 16, 0, 0, 0, 0);
    }

    SDL_Rect src, dst;
};

TEST_F(ComputeRectsTest, DefaultSizeHalfPixels)
{
  pub = CreateSurface(CPC_VISIBLE_SCR_WIDTH, CPC_VISIBLE_SCR_HEIGHT);
  scaled = CreateSurface(2*CPC_VISIBLE_SCR_WIDTH, 2*CPC_VISIBLE_SCR_HEIGHT);
  uint8_t half_pixels = 1;

  compute_rects_for_tests(&src, &dst, half_pixels);

  ExpectFullSrc(half_pixels);
  ExpectRectMatchesSurface(&dst, scaled);
  ExpectValid();
}

TEST_F(ComputeRectsTest, BiggerVidHalfPixels)
{
  for (auto offset : { 1, 2, 3, 10, 17, 50, 100, 101 })
  {
    pub = CreateSurface(CPC_VISIBLE_SCR_WIDTH, CPC_VISIBLE_SCR_HEIGHT);
    scaled = CreateSurface(2*CPC_VISIBLE_SCR_WIDTH + offset, 2*CPC_VISIBLE_SCR_HEIGHT + offset);
    uint8_t half_pixels = 1;

    compute_rects_for_tests(&src, &dst, half_pixels);

    ExpectFullSrc(half_pixels);
    EXPECT_EQ(dst.x, offset/2);
    EXPECT_EQ(dst.y, offset/2);
    EXPECT_EQ(dst.w, 2*CPC_VISIBLE_SCR_WIDTH);
    EXPECT_EQ(dst.h, 2*CPC_VISIBLE_SCR_HEIGHT);
    ExpectValid();
  }
}

TEST_F(ComputeRectsTest, BiggerPubHalfPixels)
{
  for (auto offset : { 1, 2, 3, 10, 17, 50, 100, 101, CPC_VISIBLE_SCR_WIDTH, CPC_VISIBLE_SCR_WIDTH+10 })
  {
    pub = CreateSurface(CPC_VISIBLE_SCR_WIDTH, CPC_VISIBLE_SCR_HEIGHT);
    scaled = CreateSurface(2*CPC_VISIBLE_SCR_WIDTH - offset, 2*CPC_VISIBLE_SCR_HEIGHT - offset);
    uint8_t half_pixels = 1;

    compute_rects_for_tests(&src, &dst, half_pixels);

    EXPECT_EQ(src.x, (offset+1)/4);
    EXPECT_EQ(src.y, (offset+1)/4);
    EXPECT_EQ(src.w, CPC_VISIBLE_SCR_WIDTH - (offset+1)/2);
    // TODO: There is obviously a problem if offset/2 < 4 compared to when offset = 0 (where we have -4 here)
    EXPECT_EQ(src.h, CPC_VISIBLE_SCR_HEIGHT - (offset+1)/2);
    ExpectRectMatchesSurface(&dst, scaled);
    ExpectValid();
  }
}

TEST_F(ComputeRectsTest, DefaultSize)
{
  pub = CreateSurface(2*CPC_VISIBLE_SCR_WIDTH, 2*CPC_VISIBLE_SCR_HEIGHT);
  scaled = CreateSurface(4*CPC_VISIBLE_SCR_WIDTH, 4*CPC_VISIBLE_SCR_HEIGHT);
  uint8_t half_pixels = 0;

  compute_rects_for_tests(&src, &dst, half_pixels);

  ExpectFullSrc(half_pixels);
  ExpectRectMatchesSurface(&dst, scaled);
  ExpectValid();
}

TEST_F(ComputeRectsTest, BiggerVid)
{
  for (auto offset : { 1, 2, 3, 10, 17, 50, 100, 101 })
  {
    pub = CreateSurface(2*CPC_VISIBLE_SCR_WIDTH, 2*CPC_VISIBLE_SCR_HEIGHT);
    scaled = CreateSurface(4*CPC_VISIBLE_SCR_WIDTH + offset, 4*CPC_VISIBLE_SCR_HEIGHT + offset);
    uint8_t half_pixels = 0;

    compute_rects_for_tests(&src, &dst, half_pixels);

    ExpectFullSrc(half_pixels);
    EXPECT_EQ(dst.x, offset/2);
    EXPECT_EQ(dst.y, offset/2);
    EXPECT_EQ(dst.w, 4*CPC_VISIBLE_SCR_WIDTH);
    EXPECT_EQ(dst.h, 4*CPC_VISIBLE_SCR_HEIGHT);
    ExpectValid();
  }
}

TEST_F(ComputeRectsTest, BiggerPub)
{
  for (auto offset : { 1, 2, 3, 10, 17, 50, 100, 101, CPC_VISIBLE_SCR_WIDTH, CPC_VISIBLE_SCR_WIDTH+10 })
  {
    pub = CreateSurface(2*CPC_VISIBLE_SCR_WIDTH, 2*CPC_VISIBLE_SCR_HEIGHT);
    scaled = CreateSurface(4*CPC_VISIBLE_SCR_WIDTH - offset, 4*CPC_VISIBLE_SCR_HEIGHT - offset);
    uint8_t half_pixels = 1;

    compute_rects_for_tests(&src, &dst, half_pixels);

    EXPECT_EQ(src.x, (offset+1)/4);
    EXPECT_EQ(src.y, (offset+1)/4);
    EXPECT_EQ(src.w, 2*CPC_VISIBLE_SCR_WIDTH - (offset+1)/2);
    // TODO: There is obviously a problem if offset/2 < 4 compared to when offset = 0 (where we have -4 here)
    EXPECT_EQ(src.h, 2*CPC_VISIBLE_SCR_HEIGHT - (offset+1)/2);
    ExpectRectMatchesSurface(&dst, scaled);
    ExpectValid();
  }
}

TEST(FilterBicubicTest, UniformColors)
{
  const int W = 32;
  const int H = 16;
  const uint32_t srcPitch = (W + 8) * sizeof(uint16_t);
  const uint32_t dstPitch = (2 * W + 8) * sizeof(uint16_t);
  std::vector<uint16_t> srcBuf((srcPitch / sizeof(uint16_t)) * (H + 8));
  std::vector<uint16_t> dstBuf((dstPitch / sizeof(uint16_t)) * (2 * H + 8));

  uint8_t* srcPtr = reinterpret_cast<uint8_t*>(srcBuf.data() + 2 * (srcPitch / sizeof(uint16_t)) + 2);
  uint8_t* dstPtr = reinterpret_cast<uint8_t*>(dstBuf.data());

  for (uint16_t color : { static_cast<uint16_t>(0x0000), static_cast<uint16_t>(0xFFFF), static_cast<uint16_t>(0xA534), static_cast<uint16_t>(0x5AD6) })
  {
    std::fill(srcBuf.begin(), srcBuf.end(), color);
    std::fill(dstBuf.begin(), dstBuf.end(), 0x1234);

    filter_bicubic(srcPtr, srcPitch, dstPtr, dstPitch, W, H);

    for (int y = 0; y < 2 * H; ++y)
    {
      const uint16_t* row = reinterpret_cast<const uint16_t*>(dstPtr + y * dstPitch);
      for (int x = 0; x < 2 * W; ++x)
      {
        EXPECT_EQ(row[x], color) << "Mismatch at (" << x << ", " << y << ") for color " << color;
      }
    }
  }
}

// Compare filter_bicubic to a naive 4×4 convolution computed in 32-bit arithmetic.
TEST(FilterBicubicTest, MatchesReferenceRationalBicubic)
{
  const int W = 64;
  const int H = 32;
  const uint32_t nextlineSrc = W + 8;
  const uint32_t nextlineDst = 2 * W + 8;
  const uint32_t srcPitch = nextlineSrc * sizeof(uint16_t);
  const uint32_t dstPitch = nextlineDst * sizeof(uint16_t);
  std::vector<uint16_t> srcBuf(nextlineSrc * (H + 8));
  std::vector<uint16_t> dstBuf(nextlineDst * (2 * H + 8), 0);

  // Pseudo-random but deterministically seeded, so all runs do the same computation.
  std::mt19937 rng;
  std::generate(srcBuf.begin(), srcBuf.end(), rng);

  const uint16_t* p = srcBuf.data() + 2 * nextlineSrc + 2;
  uint8_t* srcPtr = reinterpret_cast<uint8_t*>(const_cast<uint16_t*>(p));
  uint8_t* dstPtr = reinterpret_cast<uint8_t*>(dstBuf.data());

  filter_bicubic(srcPtr, srcPitch, dstPtr, dstPitch, W, H);

  static constexpr int w_even[4] = { 1, 4, 1, 0 };
  static constexpr int w_odd[4] = { 1, 23, 23, 1 };

  for (int j = 0; j < 2 * H; ++j)
  {
    int iv = j >> 1;
    const int* wy = (j & 1) ? w_odd : w_even;
    int div_y = (j & 1) ? 48 : 6;
    const uint16_t* q = reinterpret_cast<const uint16_t*>(dstPtr + j * dstPitch);

    for (int i = 0; i < 2 * W; ++i)
    {
      int iu = i >> 1;
      const int* wx = (i & 1) ? w_odd : w_even;
      int div = div_y * ((i & 1) ? 48 : 6);

      int r = 0, g = 0, b = 0;
      for (int m = -1; m <= 2; ++m)
      {
        for (int n = -1; n <= 2; ++n)
        {
          int w = wy[m + 1] * wx[n + 1];
          uint16_t c = p[(iv + m) * static_cast<int>(nextlineSrc) + (iu + n)];
          r += w * ((c >> 11) & 0x1F);
          g += w * ((c >> 5) & 0x3F);
          b += w * (c & 0x1F);
        }
      }
      uint16_t expected = static_cast<uint16_t>(((r / div) << 11) | ((g / div) << 5) | (b / div));
      EXPECT_EQ(q[i], expected) << "Mismatch at (" << i << ", " << j << ")";
    }
  }
}
}
