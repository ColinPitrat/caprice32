// 'Load/save' box for Caprice32
// Inherited from CMessageBox

#include "CapriceLoadSave.h"
#include "cap32.h"

// CPC emulation properties, defined in cap32.h:
extern t_CPC CPC;

namespace wGui {

CapriceLoadSave::CapriceLoadSave(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
	CFrame(WindowRect, pParent, pFontEngine, "Load / Save", false)
{
  // Make this window listen to incoming CTRL_VALUECHANGE messages (used for updating scrollbar values)
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGE);
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGING);

  // Navigation bar
  m_pNavigationBar = new CNavigationBar(this, CPoint(10, 5), 6, 50, 50);
  m_pNavigationBar->AddItem(SNavBarItem("Load Disk",     std::string(CPC.resources_path) + "/disk.bmp"));
  m_pNavigationBar->AddItem(SNavBarItem("Save Disk",     std::string(CPC.resources_path) + "/disk.bmp"));
  m_pNavigationBar->AddItem(SNavBarItem("Load Tape",     std::string(CPC.resources_path) + "/tape.bmp"));
  m_pNavigationBar->AddItem(SNavBarItem("Save Tape",     std::string(CPC.resources_path) + "/tape.bmp"));
  m_pNavigationBar->AddItem(SNavBarItem("Load Snap", std::string(CPC.resources_path) + "/snapshot.bmp"));
  m_pNavigationBar->AddItem(SNavBarItem("Save Snap", std::string(CPC.resources_path) + "/snapshot.bmp"));
  m_pNavigationBar->SelectItem(0);
}

}
