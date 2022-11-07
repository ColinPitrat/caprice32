#include <gtest/gtest.h>
#include "video.h"
#include "cap32.h"

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
  Uint8 half_pixels = 1;

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
    Uint8 half_pixels = 1;

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
    Uint8 half_pixels = 1;

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
  Uint8 half_pixels = 0;

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
    Uint8 half_pixels = 0;

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
    Uint8 half_pixels = 1;

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
}
