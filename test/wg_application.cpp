#include <gtest/gtest.h>

#include "wg_error.h"
#include "wg_application.h"
#include "cap32.h"
#include <string>

extern t_CPC CPC;

TEST(CApplicationTest, InitThrowExceptionWhenFailToFindResources)
{
  wGui::CApplication app;
  CPC.resources_path = "does_not_exist";

  try {
    app.Init();
    FAIL() << "CApplication::Init() with unexisting resource path didn't throw an exception";
  } catch(wGui::Wg_Ex_App& e) {
    // We don't care which file is not found, the important part is that the
    // message mention the failing path
    std::string expected("File not found: does_not_exist");
    ASSERT_EQ(expected, e.std_what().substr(0, expected.size()));
  } catch(...) {
    FAIL() << "CApplication::Init() with unexisting resource path threw an unexpected exception";
  }
}

