#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "SDL.h"

#include "CapriceDevTools.h"
#include "cap32.h"

extern byte *membank_read[4];
extern t_CPC CPC;

using namespace wGui;

class CapriceDevToolsTest : public testing::Test {
  public:
    CapriceDevToolsTest() : app(/*pWindow=*/nullptr) {}

    void SetUp() {
      byte membank0[1] = {0};
      membank_read[0] = membank0;
      membank_read[1] = membank0;
      membank_read[2] = membank0;
      membank_read[3] = membank0;
      CPC.resources_path = "resources";
      app.Init();
      CRect rect;
      surface = SDL_CreateRGBSurface(/*flags=*/0,/*width=*/10,/*height=*/10,/*depth=*/32,0,0,0,0);
      view = new CView(app, surface, surface, rect);
      cdt = new CapriceDevTools(rect, /*pParent=*/view, /*pFontEngine=*/nullptr, /*devtools=*/nullptr);
    }

    void TearDown() {
      delete cdt;
      delete view;
      SDL_FreeSurface(surface);
    }

  protected:
    CApplication app;
    SDL_Surface *surface;
    CView *view;
    CapriceDevTools *cdt;
};

namespace wGui
{
  bool operator==(const SListItem& l, const SListItem& r)
  {
    return l.sItemText == r.sItemText;
  }

  std::ostream& operator<<(std::ostream& os, const SListItem& item)
  {
    os << item.sItemText;
    return os;
  }
}

TEST_F(CapriceDevToolsTest, AsmSearchEmpty)
{
  std::vector<SListItem> items = {};
  cdt->SetDisassembly(items);

  cdt->SetAssemblySearch("text");

  cdt->AsmSearch(SearchFrom::Start, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());

  cdt->AsmSearch(SearchFrom::PositionIncluded, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());

  cdt->AsmSearch(SearchFrom::Start, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());

  cdt->AsmSearch(SearchFrom::PositionIncluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());
}

TEST_F(CapriceDevToolsTest, AsmSearchNoMatch)
{
  std::vector<SListItem> items = {
    SListItem("this doesn't match"),
    SListItem("neither does this"),
    SListItem("nope, still no match"),
    SListItem("don't count on it!"),
  };
  cdt->SetDisassembly(items);

  cdt->SetAssemblySearch("text");

  cdt->AsmSearch(SearchFrom::Start, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());

  cdt->AsmSearch(SearchFrom::PositionIncluded, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());

  cdt->AsmSearch(SearchFrom::Start, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());

  cdt->AsmSearch(SearchFrom::PositionIncluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre());
}

TEST_F(CapriceDevToolsTest, AsmSearchForwardFromPosIncluded)
{
  std::vector<SListItem> items = {
    SListItem("this doesn't match"),
    SListItem("neither does this"),
    SListItem("but this text does"),
    SListItem("this text too but we shouldn't reach it"),
    SListItem("another text"),
    SListItem("still more text"),
  };
  cdt->SetDisassembly(items);

  // We normally start with nothing selected.
  // We search text repeatedly. As we look with current position included, we
  // should always end-up at the same place.
  cdt->SetAssemblySearch("text");

  cdt->AsmSearch(SearchFrom::PositionIncluded, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("but this text does")));

  cdt->AsmSearch(SearchFrom::PositionIncluded, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("but this text does")));

  cdt->AsmSearch(SearchFrom::PositionIncluded, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("but this text does")));
}

TEST_F(CapriceDevToolsTest, AsmSearchForwardFromPosExcluded)
{
  std::vector<SListItem> items = {
    SListItem("this doesn't match"),
    SListItem("neither does this"),
    SListItem("but this text does"),
    SListItem("this text too"),
    SListItem("another text"),
    SListItem("still more text"),
  };
  cdt->SetDisassembly(items);

  // We normally start with nothing selected.
  // We search text repeatedly. As we look with current position included, we
  // should always end-up at the same place.
  cdt->SetAssemblySearch("text");

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("but this text does")));

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("this text too")));

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("another text")));
}


TEST_F(CapriceDevToolsTest, AsmSearchForwardFromStart)
{
  std::vector<SListItem> items = {
    SListItem("this doesn't match"),
    SListItem("neither does this"),
    SListItem("but this text does"),
    SListItem("this text too"),
    SListItem("another text"),
    SListItem("still more text"),
  };
  cdt->SetDisassembly(items);

  // We normally start with nothing selected.
  // We search text repeatedly. As we look with current position included, we
  // should always end-up at the same place.
  cdt->SetAssemblySearch("text");

  cdt->AsmSearch(SearchFrom::Start, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("but this text does")));

  // Search with position excluded to advance
  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("this text too")));

  // Check that "From start" actually restart from the start
  cdt->AsmSearch(SearchFrom::Start, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("but this text does")));

  cdt->AsmSearch(SearchFrom::Start, SearchDir::Forward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("but this text does")));
}

TEST_F(CapriceDevToolsTest, AsmSearchBackwardFromPosIncluded)
{
  std::vector<SListItem> items = {
    SListItem("this doesn't match"),
    SListItem("neither does this"),
    SListItem("but this text does"),
    SListItem("this text too but we shouldn't reach it"),
    SListItem("another text"),
    SListItem("still more text"),
  };
  cdt->SetDisassembly(items);

  // We normally start with nothing selected.
  // We search text repeatedly. As we look with current position included, we
  // should always end-up at the same place.
  cdt->SetAssemblySearch("text");

  cdt->AsmSearch(SearchFrom::PositionIncluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("still more text")));

  cdt->AsmSearch(SearchFrom::PositionIncluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("still more text")));

  cdt->AsmSearch(SearchFrom::PositionIncluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("still more text")));
}

TEST_F(CapriceDevToolsTest, AsmSearchBackwardFromPosExcluded)
{
  std::vector<SListItem> items = {
    SListItem("this doesn't match"),
    SListItem("neither does this"),
    SListItem("but this text does"),
    SListItem("this text too"),
    SListItem("another text"),
    SListItem("still more text"),
  };
  cdt->SetDisassembly(items);

  // We normally start with nothing selected.
  // We search text repeatedly. As we look with current position included, we
  // should always end-up at the same place.
  cdt->SetAssemblySearch("text");

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("still more text")));

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("another text")));

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("this text too")));

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("but this text does")));

  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("but this text does")));
}


TEST_F(CapriceDevToolsTest, AsmSearchBackwardFromStart)
{
  std::vector<SListItem> items = {
    SListItem("this doesn't match"),
    SListItem("neither does this"),
    SListItem("but this text does"),
    SListItem("this text too"),
    SListItem("another text"),
    SListItem("still more text"),
  };
  cdt->SetDisassembly(items);

  // We normally start with nothing selected.
  // We search text repeatedly. As we look with current position included, we
  // should always end-up at the same place.
  cdt->SetAssemblySearch("text");

  cdt->AsmSearch(SearchFrom::Start, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("still more text")));

  // Search with position excluded to advance
  cdt->AsmSearch(SearchFrom::PositionExcluded, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("another text")));

  // Check that "From start" actually restart from the start (well, the end here as we're searching backward)
  cdt->AsmSearch(SearchFrom::Start, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("still more text")));

  cdt->AsmSearch(SearchFrom::Start, SearchDir::Backward);
  EXPECT_THAT(cdt->GetSelectedAssembly(), testing::ElementsAre(SListItem("still more text")));
}
