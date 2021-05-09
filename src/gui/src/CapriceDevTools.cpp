// 'DevTools' window for Caprice32

#include <map>
#include <string>
#include "std_ex.h"
#include "CapriceDevTools.h"
#include "devtools.h"
#include "cap32.h"
#include "z80.h"

extern t_z80regs z80;
extern t_CPC CPC;

namespace wGui {

CapriceDevTools::CapriceDevTools(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine, DevTools* devtools) :
  CFrame(WindowRect, pParent, pFontEngine, "DevTools", false), m_pDevTools(devtools)
{
    SetTitleBarHeight(0);
    SetModal(true);
    // Make this window listen to incoming CTRL_VALUECHANGE messages (used for updating scrollbar values)
    CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGE);
    CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGING);

    // Navigation bar
    m_pNavigationBar = new CNavigationBar(this, CPoint(10, 5), 6, 50, 50);
    // TODO: Better icon for chars, memory and z80
    m_pNavigationBar->AddItem(SNavBarItem("z80",    CPC.resources_path + "/rom.bmp"));
    m_pNavigationBar->AddItem(SNavBarItem("Asm", CPC.resources_path + "/asm.bmp"));
    m_pNavigationBar->AddItem(SNavBarItem("Memory", CPC.resources_path + "/memory.bmp"));
    m_pNavigationBar->AddItem(SNavBarItem("Video",  CPC.resources_path + "/video.bmp"));
    m_pNavigationBar->AddItem(SNavBarItem("Audio",  CPC.resources_path + "/audio.bmp"));
    m_pNavigationBar->AddItem(SNavBarItem("Char",  CPC.resources_path + "/char.bmp"));
    m_pNavigationBar->SelectItem(0);
    m_pNavigationBar->SetIsFocusable(true);

    // Groupboxes containing controls for each 'tab' (easier to make a 'tab page' visible or invisible)
    m_pGroupBoxTabZ80 = new CGroupBox(CRect(CPoint(5, 60), m_ClientRect.Width() - 12, m_ClientRect.Height() - 80), this, "z80");
    m_pGroupBoxTabAsm = new CGroupBox(CRect(CPoint(5, 60), m_ClientRect.Width() - 12, m_ClientRect.Height() - 80), this, "Assembly");
    m_pGroupBoxTabMemory = new CGroupBox(CRect(CPoint(5, 60), m_ClientRect.Width() - 12, m_ClientRect.Height() - 80), this, "Memory");
    m_pGroupBoxTabVideo   = new CGroupBox(CRect(CPoint(5, 60), m_ClientRect.Width() - 12, m_ClientRect.Height() - 80), this, "Video");
    m_pGroupBoxTabAudio   = new CGroupBox(CRect(CPoint(5, 60), m_ClientRect.Width() - 12, m_ClientRect.Height() - 80), this, "Audio");
    m_pGroupBoxTabChar   = new CGroupBox(CRect(CPoint(5, 60), m_ClientRect.Width() - 12, m_ClientRect.Height() - 80), this, "Characters");

    // Store associations, see EnableTab method:
    TabMap["z80"] = m_pGroupBoxTabZ80;
    TabMap["asm"] = m_pGroupBoxTabAsm;
    TabMap["memory"] = m_pGroupBoxTabMemory;
    TabMap["video"] = m_pGroupBoxTabVideo;
    TabMap["audio"] = m_pGroupBoxTabAudio;
    TabMap["char"] = m_pGroupBoxTabChar;

    EnableTab("z80");

    m_pButtonPause   = new CButton(CRect(CPoint(m_ClientRect.Width() - 70, 10), 50, 15), this, "Pause");
    m_pButtonPause->SetIsFocusable(true);
    m_pButtonClose   = new CButton(CRect(CPoint(m_ClientRect.Width() - 70, 35), 50, 15), this, "Close");
    m_pButtonClose->SetIsFocusable(true);

    // ---------------- 'z80' screen ----------------
    m_pRegA = new CRegister(CRect(CPoint(10, 10), 110, 20), m_pGroupBoxTabZ80, "A");
    m_pRegAp = new CRegister(CRect(CPoint(140, 10), 110, 20), m_pGroupBoxTabZ80, "A'");
    m_pRegB = new CRegister(CRect(CPoint(10, 30), 110, 20), m_pGroupBoxTabZ80, "B");
    m_pRegBp = new CRegister(CRect(CPoint(140, 30), 110, 20), m_pGroupBoxTabZ80, "B'");
    m_pRegC = new CRegister(CRect(CPoint(10, 50), 110, 20), m_pGroupBoxTabZ80, "C");
    m_pRegCp = new CRegister(CRect(CPoint(140, 50), 110, 20), m_pGroupBoxTabZ80, "C'");
    m_pRegD = new CRegister(CRect(CPoint(10, 70), 110, 20), m_pGroupBoxTabZ80, "D");
    m_pRegDp = new CRegister(CRect(CPoint(140, 70), 110, 20), m_pGroupBoxTabZ80, "D'");
    m_pRegE = new CRegister(CRect(CPoint(10, 90), 110, 20), m_pGroupBoxTabZ80, "E");
    m_pRegEp = new CRegister(CRect(CPoint(140, 90), 110, 20), m_pGroupBoxTabZ80, "E'");
    m_pRegH = new CRegister(CRect(CPoint(10, 110), 110, 20), m_pGroupBoxTabZ80, "H");
    m_pRegHp = new CRegister(CRect(CPoint(140, 110), 110, 20), m_pGroupBoxTabZ80, "H'");
    m_pRegL = new CRegister(CRect(CPoint(10, 130), 110, 20), m_pGroupBoxTabZ80, "L");
    m_pRegLp = new CRegister(CRect(CPoint(140, 130), 110, 20), m_pGroupBoxTabZ80, "L'");
    m_pRegI = new CRegister(CRect(CPoint(10, 150), 110, 20), m_pGroupBoxTabZ80, "I");
    m_pRegR = new CRegister(CRect(CPoint(140, 150), 110, 20), m_pGroupBoxTabZ80, "R");
    m_pRegIXH = new CRegister(CRect(CPoint(10, 170), 110, 20), m_pGroupBoxTabZ80, "IXH");
    m_pRegIXL = new CRegister(CRect(CPoint(140, 170), 110, 20), m_pGroupBoxTabZ80, "IXL'");
    m_pRegIYH = new CRegister(CRect(CPoint(10, 190), 110, 20), m_pGroupBoxTabZ80, "IYH");
    m_pRegIYL = new CRegister(CRect(CPoint(140, 190), 110, 20), m_pGroupBoxTabZ80, "IYL");
    m_pRegAF = new CRegister(CRect(CPoint(10, 230), 110, 20), m_pGroupBoxTabZ80, "AF");
    m_pRegAFp = new CRegister(CRect(CPoint(140, 230), 110, 20), m_pGroupBoxTabZ80, "AF'");
    m_pRegBC = new CRegister(CRect(CPoint(10, 250), 110, 20), m_pGroupBoxTabZ80, "BC");
    m_pRegBCp = new CRegister(CRect(CPoint(140, 250), 110, 20), m_pGroupBoxTabZ80, "BC'");
    m_pRegDE = new CRegister(CRect(CPoint(10, 270), 110, 20), m_pGroupBoxTabZ80, "DE");
    m_pRegDEp = new CRegister(CRect(CPoint(140, 270), 110, 20), m_pGroupBoxTabZ80, "DE'");
    m_pRegHL = new CRegister(CRect(CPoint(10, 290), 110, 20), m_pGroupBoxTabZ80, "HL");
    m_pRegHLp = new CRegister(CRect(CPoint(140, 290), 110, 20), m_pGroupBoxTabZ80, "HL'");
    m_pRegIX = new CRegister(CRect(CPoint(10, 310), 110, 20), m_pGroupBoxTabZ80, "IX");
    m_pRegIY = new CRegister(CRect(CPoint(140, 310), 110, 20), m_pGroupBoxTabZ80, "IY");
    m_pRegSP = new CRegister(CRect(CPoint(10, 330), 110, 20), m_pGroupBoxTabZ80, "SP");
    m_pRegPC = new CRegister(CRect(CPoint(140, 330), 110, 20), m_pGroupBoxTabZ80, "PC");

    // ---------------- 'Assembly' screen ----------------
    m_pAsmLabel = new CLabel(CPoint(10, 10), m_pGroupBoxTabAsm, "Work in progress ... Nothing to see here yet, but come back later for disassembly.");
    // ---------------- 'Memory' screen ----------------
    m_pMemLabel = new CLabel(CPoint(10, 10), m_pGroupBoxTabMemory, "Work in progress ... Nothing to see here yet, but come back later for memory tool.");
    // ---------------- 'Video' screen ----------------
    m_pVidLabel = new CLabel(CPoint(10, 10), m_pGroupBoxTabVideo, "Work in progress ... Nothing to see here yet, but come back later for video (CRTC & PSG info).");
    // ---------------- 'Audio' screen ----------------
    m_pAudLabel = new CLabel(CPoint(10, 10), m_pGroupBoxTabAudio, "Work in progress ... Nothing to see here yet, but come back later for sound (tone and volume envelopes, etc ...).");
    // ---------------- 'Characters' screen ----------------
    m_pChrLabel = new CLabel(CPoint(10, 10), m_pGroupBoxTabChar, "Work in progress ... Nothing to see here yet, but come back later for charmap.");

    Update();
}

CapriceDevTools::~CapriceDevTools() = default;

void CapriceDevTools::Update()
{
  switch (m_pNavigationBar->getSelectedIndex()) {
    case 0 : { // 'z80'
               m_pRegA->SetValue(z80.AF.b.h);
               m_pRegAp->SetValue(z80.AFx.b.h);
               m_pRegB->SetValue(z80.BC.b.h);
               m_pRegBp->SetValue(z80.BCx.b.h);
               m_pRegC->SetValue(z80.BC.b.l);
               m_pRegCp->SetValue(z80.BCx.b.l);
               m_pRegD->SetValue(z80.DE.b.h);
               m_pRegDp->SetValue(z80.DEx.b.h);
               m_pRegE->SetValue(z80.DE.b.l);
               m_pRegEp->SetValue(z80.DEx.b.l);
               m_pRegH->SetValue(z80.HL.b.h);
               m_pRegHp->SetValue(z80.HLx.b.h);
               m_pRegL->SetValue(z80.HL.b.l);
               m_pRegLp->SetValue(z80.HLx.b.l);
               m_pRegI->SetValue(z80.I);
               m_pRegR->SetValue(z80.R);
               m_pRegIXH->SetValue(z80.IX.b.h);
               m_pRegIXL->SetValue(z80.IX.b.l);
               m_pRegIYH->SetValue(z80.IY.b.h);
               m_pRegIYL->SetValue(z80.IY.b.l);

               m_pRegAF->SetValue(z80.AF.w.l);
               m_pRegAFp->SetValue(z80.AFx.w.l);
               m_pRegBC->SetValue(z80.BC.w.l);
               m_pRegBCp->SetValue(z80.BCx.w.l);
               m_pRegDE->SetValue(z80.DE.w.l);
               m_pRegDEp->SetValue(z80.DEx.w.l);
               m_pRegHL->SetValue(z80.HL.w.l);
               m_pRegHLp->SetValue(z80.HLx.w.l);
               m_pRegIX->SetValue(z80.IX.w.l);
               m_pRegIY->SetValue(z80.IY.w.l);
               m_pRegSP->SetValue(z80.SP.w.l);
               m_pRegPC->SetValue(z80.PC.w.l);
               break;
             }
    case 1 : { // 'Assembly'
               break;
             }
    case 2 : { // 'Memory'
               break;
             }
    case 3 : { // 'Video'
               break;
             }
    case 4 : { // 'Audio'
               break;
             }
    case 5 : { // 'Characters'
               break;
             }
  }
}

bool CapriceDevTools::HandleMessage(CMessage* pMessage)
{
  bool bHandled = false;

  if (pMessage)
  {
    switch(pMessage->MessageType())
    {
      case CMessage::CTRL_SINGLELCLICK:
        {
          if (pMessage->Destination() == this)
          {
            if (pMessage->Source() == m_pButtonClose) {
              std::cout << "cpitrat: Closing frame" << std::endl;
              CloseFrame();
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pButtonPause) {
              if (CPC.paused) {
                std::cout << "cpitrat: Unpausing emulation" << std::endl;
                m_pButtonPause->SetWindowText("Pause");
              } else {
                std::cout << "cpitrat: Pausing emulation" << std::endl;
                m_pButtonPause->SetWindowText("Unpause");
              }
              CPC.paused = !CPC.paused;
              break;
            }
          }
          break;
        }

      case CMessage::CTRL_VALUECHANGE:
        if (pMessage->Destination() == this) {
          if (pMessage->Source() == m_pNavigationBar) {
            switch (m_pNavigationBar->getSelectedIndex()) {
              case 0 : { // 'z80'
                         EnableTab("z80");
                         break;
                       }
              case 1 : { // 'Assembly'
                         EnableTab("asm");
                         break;
                       }
              case 2 : { // 'Memory'
                         EnableTab("memory");
                         break;
                       }
              case 3 : { // 'Video'
                         EnableTab("video");
                         break;
                       }
              case 4 : { // 'Audio'
                         EnableTab("audio");
                         break;
                       }
              case 5 : { // 'Characters'
                         EnableTab("char");
                         break;
                       }
            }
          }
        }
#if __GNUC__ >= 7
        [[gnu::fallthrough]];
#endif
      case CMessage::CTRL_VALUECHANGING:
        if (pMessage->Destination() == m_pGroupBoxTabZ80) {
          // Handle scrollbars
        }
        break;

      default :
        break;
    }
  }
  if (!bHandled) {
    bHandled = CFrame::HandleMessage(pMessage);
  }
  return bHandled;
}

// Enable a 'tab', i.e. make the corresponding CGroupBox (and its content) visible.
void CapriceDevTools::EnableTab(std::string sTabName) {
    std::map<std::string, CGroupBox*>::const_iterator iter;
    for (iter=TabMap.begin(); iter != TabMap.end(); ++iter) {
           iter->second->SetVisible(iter->first == sTabName);
    }
}

void CapriceDevTools::CloseFrame() {
  // Exit gui
  CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_EXIT, nullptr, this));
  m_pDevTools->Deactivate();
}

} // namespace wGui
