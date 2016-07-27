#include <gtest/gtest.h>

#include "CapriceLoadSave.h"
#include "cap32.h"

extern t_CPC CPC;

using namespace wGui;

class CapriceLoadSaveTest : public testing::Test {
  public:
    void SetUp() {
      CPC.resources_path = "resources";
      app.Init();
      cls = new CapriceLoadSave(CRect(), nullptr, nullptr);
    }

    void TearDown() {
      delete cls;
    }

    void SetFileSpec(const std::list<std::string> &fileSpec) {
      cls->m_fileSpec = fileSpec;
    }

  protected:
    CApplication app;
    CapriceLoadSave *cls;
};

TEST_F(CapriceLoadSaveTest, MatchCurrentFileSpecReturnsFalseIfExtensionListIsEmpty)
{
  std::list<std::string> fileSpec;
  SetFileSpec(fileSpec);

  ASSERT_EQ(false, cls->MatchCurrentFileSpec("test.zip"));
  ASSERT_EQ(false, cls->MatchCurrentFileSpec("test.dsk"));
  ASSERT_EQ(false, cls->MatchCurrentFileSpec("test"));
  ASSERT_EQ(false, cls->MatchCurrentFileSpec(""));
}

TEST_F(CapriceLoadSaveTest, MatchCurrentFileSpecReturnsTrueIffExtensionInExtensionList)
{
  std::list<std::string> fileSpec = { ".zip", ".dsk" };
  SetFileSpec(fileSpec);

  ASSERT_EQ(true, cls->MatchCurrentFileSpec("test.zip"));
  ASSERT_EQ(true, cls->MatchCurrentFileSpec("test.dsk"));
  ASSERT_EQ(false, cls->MatchCurrentFileSpec("test.cpr"));
  ASSERT_EQ(false, cls->MatchCurrentFileSpec("test.voc"));
  ASSERT_EQ(false, cls->MatchCurrentFileSpec("test"));
  ASSERT_EQ(false, cls->MatchCurrentFileSpec("zip"));
  ASSERT_EQ(false, cls->MatchCurrentFileSpec(""));
}

TEST_F(CapriceLoadSaveTest, MatchCurrentFileSpecSupportsVariousExtensionSizes)
{
  std::list<std::string> fileSpec = { ".tar.gz", ".c", "nodot" };
  SetFileSpec(fileSpec);

  ASSERT_EQ(true, cls->MatchCurrentFileSpec("test.tar.gz"));
  ASSERT_EQ(false, cls->MatchCurrentFileSpec("test.gz"));
  ASSERT_EQ(true, cls->MatchCurrentFileSpec("test.c"));
  ASSERT_EQ(false, cls->MatchCurrentFileSpec("test.cpp"));
  ASSERT_EQ(true, cls->MatchCurrentFileSpec("nodot"));
  ASSERT_EQ(true, cls->MatchCurrentFileSpec("test.nodot"));
  ASSERT_EQ(false, cls->MatchCurrentFileSpec("dot"));
}
