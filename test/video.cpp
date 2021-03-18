#include <gtest/gtest.h>
#include "video.h"
#include "cap32.h"

extern SDL_Surface* pub;
extern SDL_Surface* vid;

namespace
{

class ComputeRectsTest : public testing::Test {
  public:
    void ExpectFullSrc() {
      EXPECT_EQ(src.x, 0);
      EXPECT_EQ(src.y, 0);
      EXPECT_EQ(src.w, CPC_VISIBLE_SCR_WIDTH);
      // The -4 corresponds to the 'src->h-=2*2' in video.cpp when dh <= 0
      EXPECT_EQ(src.h, CPC_VISIBLE_SCR_HEIGHT-4);
    }

    void ExpectRectMatchesSurface(SDL_Rect *r, SDL_Surface *s) {
      EXPECT_EQ(r->x, 0);
      EXPECT_EQ(r->y, 0);
      EXPECT_EQ(r->w, s->w);
      EXPECT_EQ(r->h, s->h);
    }

    void ExpectRectInSurface(SDL_Rect *r, SDL_Surface *s) {
      EXPECT_LE(r->x + r->w, s->w);
      EXPECT_LE(r->y + r->h, s->h);
    }

    SDL_Surface* CreateSurface(int width, int height) {
      // BPP shouldn't influence this method. Anyway, it is currently called only by filters that support only 16bpp.
      return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 16, 0, 0, 0, 0);
    }

    SDL_Rect src, dst;
};

TEST_F(ComputeRectsTest, DefaultSize)
{
  pub = CreateSurface(CPC_VISIBLE_SCR_WIDTH, CPC_VISIBLE_SCR_HEIGHT);
  vid = CreateSurface(2*CPC_VISIBLE_SCR_WIDTH, 2*CPC_VISIBLE_SCR_HEIGHT);

  compute_rects_for_tests(&src, &dst);

  ExpectFullSrc();
  ExpectRectMatchesSurface(&dst, vid);
  ExpectRectInSurface(&src, pub);
  ExpectRectInSurface(&dst, vid);
}

TEST_F(ComputeRectsTest, BiggerVid)
{
  for (auto offset : { 1, 2, 3, 10, 17, 50, 100, 101 }) 
  {
    pub = CreateSurface(CPC_VISIBLE_SCR_WIDTH, CPC_VISIBLE_SCR_HEIGHT);
    vid = CreateSurface(2*CPC_VISIBLE_SCR_WIDTH + offset, 2*CPC_VISIBLE_SCR_HEIGHT + offset);

    compute_rects_for_tests(&src, &dst);

    ExpectFullSrc();
    EXPECT_EQ(dst.x, offset/2);
    EXPECT_EQ(dst.y, offset/2);
    EXPECT_EQ(dst.w, 2*CPC_VISIBLE_SCR_WIDTH);
    EXPECT_EQ(dst.h, 2*CPC_VISIBLE_SCR_HEIGHT);
    ExpectRectInSurface(&src, pub);
    ExpectRectInSurface(&dst, vid);
  }
}

TEST_F(ComputeRectsTest, BiggerPub)
{
  for (auto offset : { 1, 2, 3, 10, 17, 50, 100, 101 }) 
  {
    pub = CreateSurface(CPC_VISIBLE_SCR_WIDTH, CPC_VISIBLE_SCR_HEIGHT);
    vid = CreateSurface(2*CPC_VISIBLE_SCR_WIDTH - offset, 2*CPC_VISIBLE_SCR_HEIGHT - offset);

    compute_rects_for_tests(&src, &dst);

    EXPECT_EQ(src.x, offset/4);
    EXPECT_EQ(src.y, offset/4);
    EXPECT_EQ(src.w, CPC_VISIBLE_SCR_WIDTH - offset/2);
    // TODO: There is obviously a problem if offset/2 < 4 compared to when offset = 0 (where we have -4 here)
    EXPECT_EQ(src.h, CPC_VISIBLE_SCR_HEIGHT - offset/2);
    ExpectRectMatchesSurface(&dst, vid);
    ExpectRectInSurface(&src, pub);
    ExpectRectInSurface(&dst, vid);
  }
}

}
