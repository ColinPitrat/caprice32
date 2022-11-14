#include <gtest/gtest.h>

#include "SDL.h"
#include "CapriceGuiView.h"
#include "CapriceLoadSave.h"
#include "cap32.h"
#include <string>

extern t_CPC CPC;

using namespace wGui;

class CapriceLoadSaveTest : public testing::Test {
  public:
    CapriceLoadSaveTest() : app(/*pWindow=*/nullptr) {}

    void SetUp() {
      CPC.resources_path = "resources";
      app.Init();
      CRect rect;
      surface = SDL_CreateRGBSurface(/*flags=*/0,/*width=*/10,/*height=*/10,/*depth=*/32,0,0,0,0);
      view = new CapriceGuiView(app, surface, surface, rect);
      cls = new CapriceLoadSave(CRect(), /*pParent=*/view, /*pFontEngine=*/nullptr);
    }

    void TearDown() {
      delete cls;
      delete view;
      SDL_FreeSurface(surface);
    }

    void SetFileSpec(const std::list<std::string> &fileSpec) {
      cls->m_fileSpec = fileSpec;
    }

  protected:
    CApplication app;
    CapriceLoadSave *cls;
    CapriceGuiView *view;
    SDL_Surface *surface;
};

TEST_F(CapriceLoadSaveTest, MatchCurrentFileSpecReturnsFalseIfExtensionListIsEmpty)
{
  std::list<std::string> fileSpec;
  SetFileSpec(fileSpec);

  ASSERT_FALSE(cls->MatchCurrentFileSpec("test.zip"));
  ASSERT_FALSE(cls->MatchCurrentFileSpec("test.dsk"));
  ASSERT_FALSE(cls->MatchCurrentFileSpec("test"));
  ASSERT_FALSE(cls->MatchCurrentFileSpec(""));
}

TEST_F(CapriceLoadSaveTest, MatchCurrentFileSpecReturnsTrueIffExtensionInExtensionList)
{
  std::list<std::string> fileSpec = { ".zip", ".dsk" };
  SetFileSpec(fileSpec);

  ASSERT_TRUE(cls->MatchCurrentFileSpec("test.zip"));
  ASSERT_TRUE(cls->MatchCurrentFileSpec("test.dsk"));
  ASSERT_FALSE(cls->MatchCurrentFileSpec("test.cpr"));
  ASSERT_FALSE(cls->MatchCurrentFileSpec("test.voc"));
  ASSERT_FALSE(cls->MatchCurrentFileSpec("test"));
  ASSERT_FALSE(cls->MatchCurrentFileSpec("zip"));
  ASSERT_FALSE(cls->MatchCurrentFileSpec(""));
}

TEST_F(CapriceLoadSaveTest, MatchCurrentFileSpecSupportsVariousExtensionSizes)
{
  std::list<std::string> fileSpec = { ".tar.gz", ".c", "nodot" };
  SetFileSpec(fileSpec);

  ASSERT_TRUE(cls->MatchCurrentFileSpec("test.tar.gz"));
  ASSERT_FALSE(cls->MatchCurrentFileSpec("test.gz"));
  ASSERT_TRUE(cls->MatchCurrentFileSpec("test.c"));
  ASSERT_FALSE(cls->MatchCurrentFileSpec("test.cpp"));
  ASSERT_TRUE(cls->MatchCurrentFileSpec("nodot"));
  ASSERT_TRUE(cls->MatchCurrentFileSpec("test.nodot"));
  ASSERT_FALSE(cls->MatchCurrentFileSpec("dot"));
}

TEST_F(CapriceLoadSaveTest, MatchCurrentFileSpecIsCaseInsensitive) {
  std::list<std::string> fileSpec = { ".dsk" };
  SetFileSpec(fileSpec);

  ASSERT_TRUE(cls->MatchCurrentFileSpec("test.dsk"));
  ASSERT_TRUE(cls->MatchCurrentFileSpec("TEST.DSK"));
  ASSERT_TRUE(cls->MatchCurrentFileSpec("test.DsK"));
}
