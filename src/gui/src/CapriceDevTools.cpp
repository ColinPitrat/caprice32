// 'DevTools' window for Caprice32

#include <algorithm>
#include <iomanip>
#include <map>
#include <string>
#include <sstream>
#include "std_ex.h"
#include "CapriceDevTools.h"
#include "devtools.h"
#include "cap32.h"
#include "log.h"
#include "z80.h"
#include "z80_macros.h"
#include "z80_disassembly.h"

extern t_z80regs z80;
extern t_CPC CPC;
extern byte *pbRAM;
extern std::vector<Breakpoint> breakpoints;
extern std::vector<Watchpoint> watchpoints;

namespace wGui {

CapriceDevTools::CapriceDevTools(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine, DevTools* devtools) :
  CFrame(WindowRect, pParent, pFontEngine, "DevTools", false), m_pDevTools(devtools)
{
    SetTitleBarHeight(0);
    SetModal(true);
    // Make this window listen to incoming CTRL_VALUECHANGE messages (used for updating scrollbar values)
    Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_VALUECHANGE);
    Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_VALUECHANGING);

    // Navigation bar
    m_pNavigationBar = new CNavigationBar(this, CPoint(10, 5), 6, 50, 50);
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

    m_pButtonPause   = new CButton(CRect(CPoint(m_ClientRect.Width() - 70, 10), 50, 15), this, (CPC.paused ? "Resume" : "Pause"));
    m_pButtonPause->SetIsFocusable(true);
    m_pButtonClose   = new CButton(CRect(CPoint(m_ClientRect.Width() - 70, 35), 50, 15), this, "Close");
    m_pButtonClose->SetIsFocusable(true);

    auto monoFontEngine = Application().GetFontEngine(CPC.resources_path + "/vera_mono.ttf", 10);
    // ---------------- 'z80' screen ----------------
    m_pZ80RegA = new CRegister(CRect(CPoint(10, 10), 110, 20), m_pGroupBoxTabZ80, "A");
    m_pZ80RegAp = new CRegister(CRect(CPoint(150, 10), 110, 20), m_pGroupBoxTabZ80, "A'");
    m_pZ80RegB = new CRegister(CRect(CPoint(10, 30), 110, 20), m_pGroupBoxTabZ80, "B");
    m_pZ80RegBp = new CRegister(CRect(CPoint(150, 30), 110, 20), m_pGroupBoxTabZ80, "B'");
    m_pZ80RegC = new CRegister(CRect(CPoint(10, 50), 110, 20), m_pGroupBoxTabZ80, "C");
    m_pZ80RegCp = new CRegister(CRect(CPoint(150, 50), 110, 20), m_pGroupBoxTabZ80, "C'");
    m_pZ80RegD = new CRegister(CRect(CPoint(10, 70), 110, 20), m_pGroupBoxTabZ80, "D");
    m_pZ80RegDp = new CRegister(CRect(CPoint(150, 70), 110, 20), m_pGroupBoxTabZ80, "D'");
    m_pZ80RegE = new CRegister(CRect(CPoint(10, 90), 110, 20), m_pGroupBoxTabZ80, "E");
    m_pZ80RegEp = new CRegister(CRect(CPoint(150, 90), 110, 20), m_pGroupBoxTabZ80, "E'");
    m_pZ80RegH = new CRegister(CRect(CPoint(10, 110), 110, 20), m_pGroupBoxTabZ80, "H");
    m_pZ80RegHp = new CRegister(CRect(CPoint(150, 110), 110, 20), m_pGroupBoxTabZ80, "H'");
    m_pZ80RegL = new CRegister(CRect(CPoint(10, 130), 110, 20), m_pGroupBoxTabZ80, "L");
    m_pZ80RegLp = new CRegister(CRect(CPoint(150, 130), 110, 20), m_pGroupBoxTabZ80, "L'");
    m_pZ80RegI = new CRegister(CRect(CPoint(10, 150), 110, 20), m_pGroupBoxTabZ80, "I");
    m_pZ80RegR = new CRegister(CRect(CPoint(150, 150), 110, 20), m_pGroupBoxTabZ80, "R");
    m_pZ80RegIXH = new CRegister(CRect(CPoint(10, 170), 110, 20), m_pGroupBoxTabZ80, "IXH");
    m_pZ80RegIXL = new CRegister(CRect(CPoint(150, 170), 110, 20), m_pGroupBoxTabZ80, "IXL'");
    m_pZ80RegIYH = new CRegister(CRect(CPoint(10, 190), 110, 20), m_pGroupBoxTabZ80, "IYH");
    m_pZ80RegIYL = new CRegister(CRect(CPoint(150, 190), 110, 20), m_pGroupBoxTabZ80, "IYL");
    m_pZ80RegAF = new CRegister(CRect(CPoint(10, 230), 110, 20), m_pGroupBoxTabZ80, "AF");
    m_pZ80RegAFp = new CRegister(CRect(CPoint(150, 230), 110, 20), m_pGroupBoxTabZ80, "AF'");
    m_pZ80RegBC = new CRegister(CRect(CPoint(10, 250), 110, 20), m_pGroupBoxTabZ80, "BC");
    m_pZ80RegBCp = new CRegister(CRect(CPoint(150, 250), 110, 20), m_pGroupBoxTabZ80, "BC'");
    m_pZ80RegDE = new CRegister(CRect(CPoint(10, 270), 110, 20), m_pGroupBoxTabZ80, "DE");
    m_pZ80RegDEp = new CRegister(CRect(CPoint(150, 270), 110, 20), m_pGroupBoxTabZ80, "DE'");
    m_pZ80RegHL = new CRegister(CRect(CPoint(10, 290), 110, 20), m_pGroupBoxTabZ80, "HL");
    m_pZ80RegHLp = new CRegister(CRect(CPoint(150, 290), 110, 20), m_pGroupBoxTabZ80, "HL'");
    m_pZ80RegIX = new CRegister(CRect(CPoint(10, 310), 110, 20), m_pGroupBoxTabZ80, "IX");
    m_pZ80RegIY = new CRegister(CRect(CPoint(150, 310), 110, 20), m_pGroupBoxTabZ80, "IY");
    m_pZ80RegSP = new CRegister(CRect(CPoint(10, 330), 110, 20), m_pGroupBoxTabZ80, "SP");
    m_pZ80RegPC = new CRegister(CRect(CPoint(150, 330), 110, 20), m_pGroupBoxTabZ80, "PC");

    m_pZ80StackLabel = new CLabel(CPoint(300, 10), m_pGroupBoxTabZ80, "Stack:");
    m_pZ80Stack = new CListBox(CRect(CPoint(300, 20), 100, 190), m_pGroupBoxTabZ80);

    m_pZ80RegF = new CRegister(CRect(CPoint(300, 230), 110, 20), m_pGroupBoxTabZ80, "F");
    m_pZ80RegFp = new CRegister(CRect(CPoint(440, 230), 110, 20), m_pGroupBoxTabZ80, "F'");
    m_pZ80FlagsLabel = new CLabel(CPoint(300, 260), m_pGroupBoxTabZ80, "Flags:");
    m_pZ80FlagSLbl = new CLabel(CPoint(300, 281), m_pGroupBoxTabZ80, "S");
    m_pZ80FlagS = new CEditBox(CRect(CPoint(310, 275), 20, 20), m_pGroupBoxTabZ80);
    m_pZ80FlagZLbl = new CLabel(CPoint(335, 281), m_pGroupBoxTabZ80, "Z");
    m_pZ80FlagZ = new CEditBox(CRect(CPoint(345, 275), 20, 20), m_pGroupBoxTabZ80);
    m_pZ80FlagHLbl = new CLabel(CPoint(370, 281), m_pGroupBoxTabZ80, "H");
    m_pZ80FlagH = new CEditBox(CRect(CPoint(380, 275), 20, 20), m_pGroupBoxTabZ80);
    m_pZ80FlagPVLbl = new CLabel(CPoint(405, 281), m_pGroupBoxTabZ80, "PV");
    m_pZ80FlagPV = new CEditBox(CRect(CPoint(420, 275), 20, 20), m_pGroupBoxTabZ80);
    m_pZ80FlagNLbl = new CLabel(CPoint(445, 281), m_pGroupBoxTabZ80, "N");
    m_pZ80FlagN = new CEditBox(CRect(CPoint(455, 275), 20, 20), m_pGroupBoxTabZ80);
    m_pZ80FlagCLbl = new CLabel(CPoint(480, 281), m_pGroupBoxTabZ80, "C");
    m_pZ80FlagC = new CEditBox(CRect(CPoint(490, 275), 20, 20), m_pGroupBoxTabZ80);

    // ---------------- 'Assembly' screen ----------------
    m_pAssemblyCode = new CListBox(
        CRect(10, 10, 320, m_pGroupBoxTabAsm->GetClientRect().Height() - 20),
        m_pGroupBoxTabAsm, /*bSingleSelection=*/true, /*iItemHeight=*/14, monoFontEngine);
    m_pAssemblyRefresh = new CButton(CRect(CPoint(340, 10), 50, 15), m_pGroupBoxTabAsm, "Refresh");
    m_pAssemblyStatusLabel = new CLabel(CPoint(400, 15), m_pGroupBoxTabAsm, "Status: ");
    // TODO: Allow editbox to have no border and 'transparent' background (i.e dynamic labels ...)
    m_pAssemblyStatus = new CEditBox(CRect(CPoint(440, 10), 170, 20), m_pGroupBoxTabAsm);
    m_pAssemblyStatus->SetReadOnly(true);

    m_pAssemblyEntryPointsGrp = new CGroupBox(CRect(CPoint(340, 40), 200, 120), m_pGroupBoxTabAsm, "Entry points");
    m_pAssemblyEntryPoints = new CListBox(CRect(CPoint(10, 5), 50, 80), m_pAssemblyEntryPointsGrp);
    m_pAssemblyRemoveEntryPoint = new CButton(CRect(CPoint(80, 5), 100, 20), m_pAssemblyEntryPointsGrp, "Remove selected");
    m_pAssemblyAddPCEntryPoint = new CButton(CRect(CPoint(80, 30), 100, 20), m_pAssemblyEntryPointsGrp, "Add PC");
    m_pAssemblyNewEntryPoint = new CEditBox(CRect(CPoint(80, 55), 50, 20), m_pAssemblyEntryPointsGrp);
    m_pAssemblyNewEntryPoint->SetContentType(CEditBox::HEXNUMBER);
    m_pAssemblyAddEntryPoint = new CButton(CRect(CPoint(140, 55), 50, 20), m_pAssemblyEntryPointsGrp, "Add");

    m_pAssemblyBreakPointsGrp = new CGroupBox(CRect(CPoint(340, 180), 200, 120), m_pGroupBoxTabAsm, "Break points");
    m_pAssemblyBreakPoints = new CListBox(CRect(CPoint(10, 5), 50, 80), m_pAssemblyBreakPointsGrp);
    m_pAssemblyRemoveBreakPoint = new CButton(CRect(CPoint(80, 5), 100, 20), m_pAssemblyBreakPointsGrp, "Remove selected");
    m_pAssemblyNewBreakPoint = new CEditBox(CRect(CPoint(80, 30), 50, 20), m_pAssemblyBreakPointsGrp);
    m_pAssemblyNewBreakPoint->SetContentType(CEditBox::HEXNUMBER);
    m_pAssemblyAddBreakPoint = new CButton(CRect(CPoint(140, 30), 50, 20), m_pAssemblyBreakPointsGrp, "Add");

    // ---------------- 'Memory' screen ----------------
    m_pMemPokeAdressLabel = new CLabel(        CPoint(15, 18),             m_pGroupBoxTabMemory, "Adress: ");
    m_pMemPokeAdress      = new CEditBox(CRect(CPoint(55, 13),  30, 20),   m_pGroupBoxTabMemory);
    m_pMemPokeAdress->SetIsFocusable(true);
    m_pMemPokeValueLabel  = new CLabel(        CPoint(95, 18),             m_pGroupBoxTabMemory, "Value: ");
    m_pMemPokeValue       = new CEditBox(CRect(CPoint(130, 13), 30, 20),   m_pGroupBoxTabMemory);
    m_pMemPokeValue->SetIsFocusable(true);
    m_pMemButtonPoke      = new CButton( CRect(CPoint(175, 13), 35, 20),   m_pGroupBoxTabMemory, "Poke");
    m_pMemButtonPoke->SetIsFocusable(true);

    m_pMemAdressLabel     = new CLabel(        CPoint(15, 50),             m_pGroupBoxTabMemory, "Adress: ");
    m_pMemAdressValue     = new CEditBox(CRect(CPoint(55, 45), 30, 20),    m_pGroupBoxTabMemory);
    m_pMemAdressValue->SetIsFocusable(true);
    m_pMemButtonDisplay   = new CButton( CRect(CPoint(95, 45), 45, 20),    m_pGroupBoxTabMemory, "Display");
    m_pMemButtonDisplay->SetIsFocusable(true);

    m_pMemBytesPerLineLbl = new CLabel(       CPoint(290, 35),             m_pGroupBoxTabMemory, "Bytes per line:");
    m_pMemBytesPerLine  = new CDropDown( CRect(CPoint(290, 45), 50, 20),   m_pGroupBoxTabMemory, false);
    m_pMemBytesPerLine->AddItem(SListItem("1"));
    m_pMemBytesPerLine->AddItem(SListItem("4"));
    m_pMemBytesPerLine->AddItem(SListItem("8"));
    m_pMemBytesPerLine->AddItem(SListItem("16"));
    m_pMemBytesPerLine->AddItem(SListItem("32"));
    m_pMemBytesPerLine->AddItem(SListItem("64"));
    m_pMemBytesPerLine->SetListboxHeight(4);
    m_MemBytesPerLine = 16;
    m_pMemBytesPerLine->SelectItem(3);
    m_pMemBytesPerLine->SetIsFocusable(true);

    m_pMemFilterLabel     = new CLabel(        CPoint(15, 80),             m_pGroupBoxTabMemory, "Byte: ");
    m_pMemFilterValue     = new CEditBox(CRect(CPoint(55, 75), 30, 20),    m_pGroupBoxTabMemory);
    m_pMemFilterValue->SetIsFocusable(true);
    m_pMemButtonFilter    = new CButton( CRect(CPoint(95, 75), 45, 20),    m_pGroupBoxTabMemory, "Filter");
    m_pMemButtonFilter->SetIsFocusable(true);
    m_pMemButtonCopy      = new CButton( CRect(CPoint(270, 75), 95, 20),   m_pGroupBoxTabMemory, "Dump to stdout");
    m_pMemButtonCopy->SetIsFocusable(true);

    m_pMemTextContent  = new CTextBox(CRect(CPoint(15, 105), 350, 240), m_pGroupBoxTabMemory, monoFontEngine);

    m_pMemPokeAdress->SetContentType(CEditBox::HEXNUMBER);
    m_pMemPokeValue->SetContentType(CEditBox::HEXNUMBER);
    m_pMemAdressValue->SetContentType(CEditBox::HEXNUMBER);
    m_pMemFilterValue->SetContentType(CEditBox::HEXNUMBER);
    m_pMemTextContent->SetReadOnly(true);

    m_MemFilterValue = -1;
    m_MemDisplayValue = -1;

    // TODO: Support read, write and R/W watch points
    m_pMemWatchPointsGrp = new CGroupBox(CRect(CPoint(380, 13), 200, 120), m_pGroupBoxTabMemory, "Watch points");
    m_pMemWatchPoints = new CListBox(CRect(CPoint(10, 5), 50, 80), m_pMemWatchPointsGrp);
    m_pMemRemoveWatchPoint = new CButton(CRect(CPoint(80, 5), 100, 20), m_pMemWatchPointsGrp, "Remove selected");
    m_pMemNewWatchPoint = new CEditBox(CRect(CPoint(80, 30), 50, 20), m_pMemWatchPointsGrp);
    m_pMemNewWatchPoint->SetContentType(CEditBox::HEXNUMBER);
    m_pMemAddWatchPoint = new CButton(CRect(CPoint(140, 30), 50, 20), m_pMemWatchPointsGrp, "Add");

    // ---------------- 'Video' screen ----------------
    m_pVidLabel = new CLabel(CPoint(10, 10), m_pGroupBoxTabVideo, "Work in progress ... Nothing to see here yet, but come back later for video (CRTC & PSG info).");
    // ---------------- 'Audio' screen ----------------
    m_pAudLabel = new CLabel(CPoint(10, 10), m_pGroupBoxTabAudio, "Work in progress ... Nothing to see here yet, but come back later for sound (tone and volume envelopes, etc ...).");
    // ---------------- 'Characters' screen ----------------
    m_pChrLabel = new CLabel(CPoint(10, 10), m_pGroupBoxTabChar, "Work in progress ... Nothing to see here yet, but come back later for charmap.");

    UpdateAll();
}

CapriceDevTools::~CapriceDevTools() = default;

void CapriceDevTools::UpdateDisassemblyPos()
{
  auto lines = m_pAssemblyCode->GetAllItems();
  SListItem toFind("ignored", reinterpret_cast<void*>(_PC));
  auto curpos = std::lower_bound(lines.begin(), lines.end(), toFind, [](auto x, auto y) {
    return x.pItemData < y.pItemData;
  });
  int idx = std::distance(lines.begin(), curpos);
  m_pAssemblyCode->SetPosition(idx, CListBox::CENTER);
  if (curpos != lines.begin()) {
    // TODO: Do not allow to select another line
    m_pAssemblyCode->SetSelection(idx, /*bSelected=*/true, /*bNotify=*/false);
  } else {
    m_pAssemblyCode->SetAllSelections(false);
    m_pAssemblyCode->Draw();
  }
}

void CapriceDevTools::RefreshDisassembly()
{
  m_pAssemblyCode->ClearItems();
  std::vector<SListItem> items;
  for (const auto& line : m_Disassembled.lines) {
    std::ostringstream oss;
    oss << std::hex << std::setw(5) << line.address_ << ": " << std::setw(10) << line.opcode_ << "     " << line.instruction_;
    // TODO: smart use of colors. Ideas:
    //   - labels, jumps & calls, ...
    //   - source of disassembling (from PC, from one entry point or another ...)
    if (std::any_of(breakpoints.begin(), breakpoints.end(), [&](const auto& b) {
          return b.address == line.address_;
          })) {
      items.emplace_back(oss.str(), reinterpret_cast<void*>(line.address_), COLOR_RED);
    } else {
      items.emplace_back(oss.str(), reinterpret_cast<void*>(line.address_));
    }
  }
  m_pAssemblyCode->AddItems(items);
  UpdateDisassemblyPos();
}

void CapriceDevTools::UpdateDisassembly()
{
  // TODO: Disassemble in a thread
  m_pAssemblyStatus->SetWindowText("Disassembling...");
  // We need to force the repaint for the status to be displayed.
  m_pParentWindow->HandleMessage(new CMessage(CMessage::APP_PAINT, GetAncestor(ROOT), this));
  m_Disassembled = disassemble(m_EntryPoints);
  RefreshDisassembly();
  // TODO: Report inconsistent disassembling
  m_pAssemblyStatus->SetWindowText("SUCCESS");
}

void CapriceDevTools::UpdateZ80()
{
  m_pZ80RegA->SetValue(z80.AF.b.h);
  m_pZ80RegAp->SetValue(z80.AFx.b.h);
  m_pZ80RegF->SetValue(z80.AF.b.l);
  m_pZ80RegFp->SetValue(z80.AFx.b.l);
  m_pZ80RegB->SetValue(z80.BC.b.h);
  m_pZ80RegBp->SetValue(z80.BCx.b.h);
  m_pZ80RegC->SetValue(z80.BC.b.l);
  m_pZ80RegCp->SetValue(z80.BCx.b.l);
  m_pZ80RegD->SetValue(z80.DE.b.h);
  m_pZ80RegDp->SetValue(z80.DEx.b.h);
  m_pZ80RegE->SetValue(z80.DE.b.l);
  m_pZ80RegEp->SetValue(z80.DEx.b.l);
  m_pZ80RegH->SetValue(z80.HL.b.h);
  m_pZ80RegHp->SetValue(z80.HLx.b.h);
  m_pZ80RegL->SetValue(z80.HL.b.l);
  m_pZ80RegLp->SetValue(z80.HLx.b.l);
  m_pZ80RegI->SetValue(z80.I);
  m_pZ80RegR->SetValue(z80.R);
  m_pZ80RegIXH->SetValue(z80.IX.b.h);
  m_pZ80RegIXL->SetValue(z80.IX.b.l);
  m_pZ80RegIYH->SetValue(z80.IY.b.h);
  m_pZ80RegIYL->SetValue(z80.IY.b.l);

  m_pZ80RegAF->SetValue(z80.AF.w.l);
  m_pZ80RegAFp->SetValue(z80.AFx.w.l);
  m_pZ80RegBC->SetValue(z80.BC.w.l);
  m_pZ80RegBCp->SetValue(z80.BCx.w.l);
  m_pZ80RegDE->SetValue(z80.DE.w.l);
  m_pZ80RegDEp->SetValue(z80.DEx.w.l);
  m_pZ80RegHL->SetValue(z80.HL.w.l);
  m_pZ80RegHLp->SetValue(z80.HLx.w.l);
  m_pZ80RegIX->SetValue(z80.IX.w.l);
  m_pZ80RegIY->SetValue(z80.IY.w.l);
  m_pZ80RegSP->SetValue(z80.SP.w.l);
  m_pZ80RegPC->SetValue(z80.PC.w.l);

  m_pZ80FlagS->SetWindowText((z80.AF.b.l & Sflag) ? "1" : "0");
  m_pZ80FlagZ->SetWindowText((z80.AF.b.l & Zflag) ? "1" : "0");
  m_pZ80FlagH->SetWindowText((z80.AF.b.l & Hflag) ? "1" : "0");
  m_pZ80FlagPV->SetWindowText((z80.AF.b.l & Pflag) ? "1" : "0");
  m_pZ80FlagN->SetWindowText((z80.AF.b.l & Nflag) ? "1" : "0");
  m_pZ80FlagC->SetWindowText((z80.AF.b.l & Cflag) ? "1" : "0");

  m_pZ80Stack->ClearItems();
  for (word addr = z80.SP.w.l; addr < 0xC000; addr += 2) {
    std::ostringstream oss;
    word val = (z80_read_mem(addr+1) << 8) + z80_read_mem(addr);
    oss << std::hex << std::setw(4) << std::setfill('0') << val
      << " (" << std::dec << val << ")";
    m_pZ80Stack->AddItem(SListItem(oss.str()));
  }
}

void CapriceDevTools::UpdateEntryPointsList()
{
  m_pAssemblyEntryPoints->ClearItems();
  for(word ep : m_EntryPoints) {
    std::ostringstream oss;
    oss << std::hex << std::setw(4) << std::setfill('0') << ep;
    m_pAssemblyEntryPoints->AddItem(SListItem(oss.str()));
  }
  UpdateDisassembly();
}

void CapriceDevTools::UpdateBreakPointsList()
{
  m_pAssemblyBreakPoints->ClearItems();
  for(const auto& bp : breakpoints) {
    std::ostringstream oss;
    oss << std::hex << std::setw(4) << std::setfill('0') << bp.address;
    m_pAssemblyBreakPoints->AddItem(SListItem(oss.str()));
  }
  // Ensure the lines corresponding to the breakpoints are colored
  RefreshDisassembly();
}

void CapriceDevTools::UpdateWatchPointsList()
{
  m_pMemWatchPoints->ClearItems();
  for(const auto& bp : watchpoints) {
    std::ostringstream oss;
    oss << std::hex << std::setw(4) << std::setfill('0') << bp.address;
    m_pMemWatchPoints->AddItem(SListItem(oss.str()));
  }
}

void CapriceDevTools::UpdateTextMemory() {
  std::ostringstream memText;
  for(unsigned int i = 0; i < 65536/m_MemBytesPerLine; i++) {
    std::ostringstream memLine;
    memLine << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << i*m_MemBytesPerLine << " : ";
    bool displayLine = false;
    bool filterAdress = (m_MemDisplayValue >= 0 && m_MemDisplayValue <= 65535);
    bool filterValue = (m_MemFilterValue >= 0 && m_MemFilterValue <= 255);
    for(unsigned int j = 0; j < m_MemBytesPerLine; j++) {
      memLine << std::setw(2) << static_cast<unsigned int>(pbRAM[i*m_MemBytesPerLine+j]) << " ";
      if(!filterAdress && !filterValue) {
        displayLine = true;
      }
      if(filterValue && static_cast<int>(pbRAM[i*m_MemBytesPerLine+j]) == m_MemFilterValue) {
        displayLine = true;
      }
      if(filterAdress && (i*m_MemBytesPerLine+j == static_cast<unsigned int>(m_MemDisplayValue))) {
        displayLine = true;
      }
    }
    if(displayLine) {
      memText << memLine.str() << "\n";
    }
  }
  m_pMemTextContent->SetWindowText(memText.str().substr(0, memText.str().size()-1));
}

void CapriceDevTools::PauseExecution()
{
  CPC.paused = true;
  m_pButtonPause->SetWindowText("Resume");
}

void CapriceDevTools::ResumeExecution()
{
  CPC.paused = false;
  m_pButtonPause->SetWindowText("Pause");
}

void CapriceDevTools::Update()
{
  if (CPC.paused) {
    m_pButtonPause->SetWindowText("Resume");
  } else {
    m_pButtonPause->SetWindowText("Pause");
    switch (m_pNavigationBar->getSelectedIndex()) {
      case 0 : { // 'z80'
                 UpdateZ80();
                 break;
               }
      case 1 : { // 'Assembly'
                 UpdateDisassemblyPos();
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
  // Pause on breakpoints and watchpoints. After update of screens as we don't
  // update them again after.
  if (!breakpoints.empty() || !watchpoints.empty()) {
    if (z80.watchpoint_reached ||
        std::any_of(breakpoints.begin(), breakpoints.end(), [&](const auto& b) {
          return b.address == _PC;
          })) {
      PauseExecution();
    };
  }
}

void CapriceDevTools::UpdateAll()
{
    UpdateZ80();
    UpdateBreakPointsList();
    //UpdateDisassembly();
    UpdateDisassemblyPos();
    UpdateTextMemory();
    UpdateWatchPointsList();
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
              // TODO: Unpause on closing, but only for the last window (so not here ...)
              CloseFrame();
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pButtonPause) {
              if (CPC.paused) {
                ResumeExecution();
              } else {
                PauseExecution();
              }
              break;
            }
          }
          if (pMessage->Destination() == m_pGroupBoxTabAsm) {
            if (pMessage->Source() == m_pAssemblyRefresh) {
              UpdateDisassembly();
              break;
            }
          }
          if (pMessage->Destination() == m_pAssemblyEntryPointsGrp) {
            if (pMessage->Source() == m_pAssemblyAddEntryPoint) {
              // stol can throw on empty string or invalid value
              try
              {
                m_EntryPoints.push_back(static_cast<word>(std::stol(m_pAssemblyNewEntryPoint->GetWindowText(), nullptr, 16)));
                UpdateEntryPointsList();
              } catch(...) {}
              break;
            }
            if (pMessage->Source() == m_pAssemblyAddPCEntryPoint) {
              m_EntryPoints.push_back(_PC);
              UpdateEntryPointsList();
              break;
            }
            if (pMessage->Source() == m_pAssemblyRemoveEntryPoint) {
              for (int i = static_cast<int>(m_pAssemblyEntryPoints->Size()) - 1; i >= 0; i--) {
                if (m_pAssemblyEntryPoints->IsSelected(i)) {
                  m_EntryPoints.erase(m_EntryPoints.begin() + i);
                }
              }
              UpdateEntryPointsList();
              break;
            }
          }
          if (pMessage->Destination() == m_pAssemblyBreakPointsGrp) {
            if (pMessage->Source() == m_pAssemblyAddBreakPoint) {
              // stol can throw on empty string or invalid value
              try
              {
                breakpoints.emplace_back(static_cast<word>(std::stol(m_pAssemblyNewBreakPoint->GetWindowText(), nullptr, 16)));
                UpdateBreakPointsList();
              } catch(...) {}
              break;
            }
            if (pMessage->Source() == m_pAssemblyRemoveBreakPoint) {
              for (int i = static_cast<int>(m_pAssemblyBreakPoints->Size()) - 1; i >= 0; i--) {
                if (m_pAssemblyBreakPoints->IsSelected(i)) {
                  breakpoints.erase(breakpoints.begin() + i);
                }
              }
              UpdateBreakPointsList();
              break;
            }
          }
          if (pMessage->Destination() == m_pGroupBoxTabMemory)
          {
            if (pMessage->Source() == m_pMemButtonPoke) {
              std::string adress = m_pMemPokeAdress->GetWindowText();
              std::string value  = m_pMemPokeValue->GetWindowText();
              unsigned int pokeAdress = strtol(adress.c_str(), nullptr, 16);
              int pokeValue           = strtol(value.c_str(),  nullptr, 16);
              if(!adress.empty() && !value.empty() && pokeAdress < 65536 && pokeValue >= -128 && pokeValue <= 255) {
                std::cout << "Poking " << pokeAdress << " with " << pokeValue << std::endl;
                pbRAM[pokeAdress] = pokeValue;
                UpdateTextMemory();
              } else {
                std::cout << "Cannot poke " << adress << "(" << pokeAdress << ") with " << value << "(" << pokeValue << ")" << std::endl;
              }
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pMemButtonDisplay) {
              std::string display = m_pMemAdressValue->GetWindowText();
              if(display.empty()) {
                m_MemDisplayValue = -1;
              } else {
                m_MemDisplayValue = strtol(display.c_str(), nullptr, 16);
              }
              m_MemFilterValue = -1;
              std::cout << "Displaying adress " << m_MemDisplayValue << " in memory." << std::endl;
              UpdateTextMemory();
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pMemButtonFilter) {
              m_MemDisplayValue = -1;
              std::string filter = m_pMemFilterValue->GetWindowText();
              if(filter.empty()) {
                m_MemFilterValue = -1;
              } else {
                m_MemFilterValue = strtol(filter.c_str(), nullptr, 16);
              }
              std::cout << "Filtering value " << m_MemFilterValue << " in memory." << std::endl;
              UpdateTextMemory();
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pMemButtonCopy) {
              std::cout << m_pMemTextContent->GetWindowText() << std::endl;
              if(SDL_SetClipboardText(m_pMemTextContent->GetWindowText().c_str()) < 0) {
                LOG_ERROR("Error while copying data to clipboard: " << SDL_GetError());
              }
              bHandled = true;
              break;
            }
          }
          if (pMessage->Destination() == m_pMemWatchPointsGrp) {
            if (pMessage->Source() == m_pMemAddWatchPoint) {
              // stol can throw on empty string or invalid value
              try
              {
                watchpoints.emplace_back(static_cast<word>(std::stol(m_pMemNewWatchPoint->GetWindowText(), nullptr, 16)));
                UpdateWatchPointsList();
              } catch(...) {}
              break;
            }
            if (pMessage->Source() == m_pMemRemoveWatchPoint) {
              for (int i = static_cast<int>(m_pMemWatchPoints->Size()) - 1; i >= 0; i--) {
                if (m_pMemWatchPoints->IsSelected(i)) {
                  watchpoints.erase(watchpoints.begin() + i);
                }
              }
              UpdateWatchPointsList();
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
                         UpdateZ80();
                         break;
                       }
              case 1 : { // 'Assembly'
                         EnableTab("asm");
                         UpdateDisassemblyPos();
                         break;
                       }
              case 2 : { // 'Memory'
                         EnableTab("memory");
                         UpdateTextMemory();
                         UpdateWatchPointsList();
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
        if (pMessage->Destination() == m_pMemBytesPerLine) {
          switch (m_pMemBytesPerLine->GetSelectedIndex()) {
            case 0:
              m_MemBytesPerLine = 1;
              break;
            case 1:
              m_MemBytesPerLine = 4;
              break;
            case 2:
              m_MemBytesPerLine = 8;
              break;
            case 3:
              m_MemBytesPerLine = 16;
              break;
            case 4:
              m_MemBytesPerLine = 32;
              break;
            case 5:
              m_MemBytesPerLine = 64;
              break;
          }
          UpdateTextMemory();
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
  Application().MessageServer()->QueueMessage(new CMessage(CMessage::APP_EXIT, GetAncestor(ROOT), this));
  m_pDevTools->Deactivate();
}

} // namespace wGui
