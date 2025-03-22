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
#include "stringutils.h"
#include "z80.h"
#include "z80_macros.h"
#include "z80_disassembly.h"

extern t_z80regs z80;
extern t_CPC CPC;
extern t_GateArray GateArray;
extern t_PSG PSG;
extern std::vector<Breakpoint> breakpoints;
extern std::vector<Watchpoint> watchpoints;
extern byte* pbROMlo;
extern byte* pbExpansionROM;
t_MemBankConfig memtool_membank_config;

namespace wGui {

int FormatSize(Format f)
{
  switch(f)
  {
    case Format::Hex:
    case Format::Char:
    case Format::U8:
    case Format::I8:
      return 1;
    case Format::U16:
    case Format::I16:
      return 2;
    case Format::U32:
    case Format::I32:
      return 4;
  };
  LOG_ERROR("Missing FormatSize for " << f);
  return 1;
}

std::ostream& operator<<(std::ostream& os, const Format& f)
{
  switch(f)
  {
    case Format::Hex:
      os << "hex";
      break;
    case Format::Char:
      os << "char";
      break;
    case Format::U8:
      os << "u8";
      break;
    case Format::U16:
      os << "u16";
      break;
    case Format::U32:
      os << "u32";
      break;
    case Format::I8:
      os << "i8";
      break;
    case Format::I16:
      os << "i16";
      break;
    case Format::I32:
      os << "i32";
      break;
    default:
      os << "!Unsupported!";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<Format>& fs)
{
  os << "[";
  bool first = true;
  for (auto f : fs)
  {
    if (!first) os << ",";
    first = false;
    os << f;
  }
  os << "]";
  return os;
}

std::string RAMConfig::RAMConfigText(int i)
{
  switch (i) {
    case 0:
      return "0 [0123]";
    case 1:
      return "1 [0127]";
    case 2:
      return "2 [4567]";
    case 3:
      return "3 [0327]";
    case 4:
      return "4 [0423]";
    case 5:
      return "5 [0523]";
    case 6:
      return "6 [0623]";
    case 7:
      return "7 [0723]";
    default:
      return "N/A";
  }
}

std::string RAMConfig::RAMConfigText()
{
  return RAMConfigText(RAMCfg);
}

RAMConfig RAMConfig::CurrentConfig()
{
  RAMConfig result;
  result.LoROMEnabled = !(GateArray.ROM_config & 4);
  result.HiROMEnabled = !(GateArray.ROM_config & 8);
  result.RAMBank = (GateArray.RAM_config & 0x38) >> 3;
  result.RAMCfg = GateArray.RAM_config & 0x7;
  return result;
}

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

    m_pButtonStepIn   = new CButton(CRect(CPoint(m_ClientRect.Width() - 150, 5), 70, 15), this, "Step in");
    m_pButtonStepIn->SetIsFocusable(true);
    m_pToolTipStepIn = new CToolTip(m_pButtonStepIn, "One instruction", COLOR_BLACK);
    m_pButtonStepOver   = new CButton(CRect(CPoint(m_ClientRect.Width() - 150, 25), 70, 15), this, "Step over");
    m_pButtonStepOver->SetIsFocusable(true);
    m_pToolTipStepOver = new CToolTip(m_pButtonStepOver, "One instruction or call", COLOR_BLACK);
    m_pButtonStepOut   = new CButton(CRect(CPoint(m_ClientRect.Width() - 150, 45), 70, 15), this, "Step out");
    m_pButtonStepOut->SetIsFocusable(true);
    m_pToolTipStepOut = new CToolTip(m_pButtonStepOut, "Exit current call/interrupt", COLOR_BLACK);

    m_pButtonPause   = new CButton(CRect(CPoint(m_ClientRect.Width() - 70, 25), 50, 15), this, (CPC.paused ? "Resume" : "Pause"));
    m_pButtonPause->SetIsFocusable(true);
    m_pButtonClose   = new CButton(CRect(CPoint(m_ClientRect.Width() - 70, 45), 50, 15), this, "Close");
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
    m_pZ80RegIXL = new CRegister(CRect(CPoint(150, 170), 110, 20), m_pGroupBoxTabZ80, "IXL");
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

    m_pZ80FlagsLabel = new CLabel(CPoint(290, 222), m_pGroupBoxTabZ80, "Flags:");
    m_pZ80RegF = new CRegister(CRect(CPoint(290, 235), 110, 20), m_pGroupBoxTabZ80, "F");
    m_pZ80RegFp = new CRegister(CRect(CPoint(290, 255), 110, 20), m_pGroupBoxTabZ80, "F'");

    m_pZ80FlagS  = new CFlag(CRect(CPoint(290, 280), 35, 20), m_pGroupBoxTabZ80, "S",  "Sign flag");
    m_pZ80FlagZ  = new CFlag(CRect(CPoint(290, 300), 35, 20), m_pGroupBoxTabZ80, "Z",  "Zero flag");
    m_pZ80FlagX1 = new CFlag(CRect(CPoint(290, 320), 35, 20), m_pGroupBoxTabZ80, "NU", "Unused flag");
    m_pZ80FlagH  = new CFlag(CRect(CPoint(290, 340), 35, 20), m_pGroupBoxTabZ80, "H",  "Half carry flag");
    m_pZ80FlagX2 = new CFlag(CRect(CPoint(340, 280), 35, 20), m_pGroupBoxTabZ80, "NU", "Unused flag");
    m_pZ80FlagPV = new CFlag(CRect(CPoint(340, 300), 35, 20), m_pGroupBoxTabZ80, "PV", "Parity/overflow flag");
    m_pZ80FlagN  = new CFlag(CRect(CPoint(340, 320), 35, 20), m_pGroupBoxTabZ80, "N",  "Add/substract flag");
    m_pZ80FlagC  = new CFlag(CRect(CPoint(340, 340), 35, 20), m_pGroupBoxTabZ80, "C",  "Carry flag");

    m_pZ80ModifyRegisters = new CButton(CRect(CPoint(510, 10), 100, 20), m_pGroupBoxTabZ80, "Unlock registers");

    LockRegisters();

    // TODO: Add information about interrupts (mode, IFF1, IFF2)

    // ---------------- 'Assembly' screen ----------------
    m_pAssemblyCode = new CListBox(
        CRect(10, 10, 320, m_pGroupBoxTabAsm->GetClientRect().Height() - 35),
        m_pGroupBoxTabAsm, /*bSingleSelection=*/true, /*iItemHeight=*/14, monoFontEngine);
    m_pAssemblySearchLbl = new CLabel(CPoint(10, m_pGroupBoxTabAsm->GetClientRect().Height() - 25), m_pGroupBoxTabAsm, "Search: ");
    m_pAssemblySearch = new CEditBox(CRect(CPoint(50, m_pGroupBoxTabAsm->GetClientRect().Height() - 30), 200, 20), m_pGroupBoxTabAsm);
    m_pAssemblySearchPrev = new CButton(CRect(CPoint(250, m_pGroupBoxTabAsm->GetClientRect().Height() - 30), 35, 20), m_pGroupBoxTabAsm, "Prev");
    m_pAssemblySearchNext = new CButton(CRect(CPoint(285, m_pGroupBoxTabAsm->GetClientRect().Height() - 30), 35, 20), m_pGroupBoxTabAsm, "Next");

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

    m_pAssemblyBreakPointsGrp = new CGroupBox(CRect(CPoint(340, 170), 200, 90), m_pGroupBoxTabAsm, "Break points");
    m_pAssemblyBreakPoints = new CListBox(CRect(CPoint(10, 5), 50, 50), m_pAssemblyBreakPointsGrp);
    m_pAssemblyRemoveBreakPoint = new CButton(CRect(CPoint(80, 5), 100, 20), m_pAssemblyBreakPointsGrp, "Remove selected");
    m_pAssemblyNewBreakPoint = new CEditBox(CRect(CPoint(80, 30), 50, 20), m_pAssemblyBreakPointsGrp);
    m_pAssemblyNewBreakPoint->SetContentType(CEditBox::HEXNUMBER);
    m_pAssemblyAddBreakPoint = new CButton(CRect(CPoint(140, 30), 50, 20), m_pAssemblyBreakPointsGrp, "Add");

    m_pAssemblyMemConfigGrp = new CGroupBox(CRect(CPoint(340, 270), 260, 100), m_pGroupBoxTabAsm, "RAM config");
    m_pAssemblyMemConfigAsmLbl = new CLabel(CPoint(10, 30), m_pAssemblyMemConfigGrp, "Asm:");
    m_pAssemblyMemConfigCurLbl = new CLabel(CPoint(10, 55), m_pAssemblyMemConfigGrp, "Cur:");
    m_pAssemblyMemConfigROMLbl = new CLabel(CPoint(70, 0), m_pAssemblyMemConfigGrp, "ROM");
    m_pAssemblyMemConfigLoLbl = new CLabel(CPoint(60, 10), m_pAssemblyMemConfigGrp, "Lo");
    m_pAssemblyMemConfigHiLbl = new CLabel(CPoint(90, 10), m_pAssemblyMemConfigGrp, "Hi");
    m_pAssemblyMemConfigRAMLbl = new CLabel(CPoint(170, 0), m_pAssemblyMemConfigGrp, "RAM");
    m_pAssemblyMemConfigBankLbl = new CLabel(CPoint(140, 10), m_pAssemblyMemConfigGrp, "Bank");
    m_pAssemblyMemConfigConfigLbl = new CLabel(CPoint(180, 10), m_pAssemblyMemConfigGrp, "Config");

    m_pAssemblyMemConfigAsmLoROM = new CCheckBox(CRect(CPoint(60, 30), 10, 10), m_pAssemblyMemConfigGrp);
    m_pAssemblyMemConfigAsmLoROM->SetReadOnly(true);
    m_pAssemblyMemConfigAsmHiROM = new CCheckBox(CRect(CPoint(90, 30), 10, 10), m_pAssemblyMemConfigGrp);
    m_pAssemblyMemConfigAsmHiROM->SetReadOnly(true);
    m_pAssemblyMemConfigAsmRAMBank = new CEditBox(CRect(CPoint(140, 25), 30, 20), m_pAssemblyMemConfigGrp);
    m_pAssemblyMemConfigAsmRAMBank->SetReadOnly(true);
    m_pAssemblyMemConfigAsmRAMConfig = new CEditBox(CRect(CPoint(180, 25), 60, 20), m_pAssemblyMemConfigGrp);
    m_pAssemblyMemConfigAsmRAMConfig->SetReadOnly(true);

    m_pAssemblyMemConfigCurLoROM = new CCheckBox(CRect(CPoint(60, 55), 10, 10), m_pAssemblyMemConfigGrp);
    m_pAssemblyMemConfigCurLoROM->SetReadOnly(true);
    m_pAssemblyMemConfigCurHiROM = new CCheckBox(CRect(CPoint(90, 55), 10, 10), m_pAssemblyMemConfigGrp);
    m_pAssemblyMemConfigCurHiROM->SetReadOnly(true);
    m_pAssemblyMemConfigCurRAMBank = new CEditBox(CRect(CPoint(140, 50), 30, 20), m_pAssemblyMemConfigGrp);
    m_pAssemblyMemConfigCurRAMBank->SetReadOnly(true);
    m_pAssemblyMemConfigCurRAMConfig = new CEditBox(CRect(CPoint(180, 50), 60, 20), m_pAssemblyMemConfigGrp);
    m_pAssemblyMemConfigCurRAMConfig->SetReadOnly(true);

    // ---------------- 'Memory' screen ----------------
    m_pMemPokeAdressLabel = new CLabel(        CPoint(15, 18),             m_pGroupBoxTabMemory, "Adress: ");
    m_pMemPokeAdress      = new CEditBox(CRect(CPoint(55, 13),  35, 20),   m_pGroupBoxTabMemory);
    m_pMemPokeAdress->SetIsFocusable(true);
    m_pMemPokeValueLabel  = new CLabel(        CPoint(95, 18),             m_pGroupBoxTabMemory, "Value: ");
    m_pMemPokeValue       = new CEditBox(CRect(CPoint(130, 13), 30, 20),   m_pGroupBoxTabMemory);
    m_pMemPokeValue->SetIsFocusable(true);
    m_pMemButtonPoke      = new CButton( CRect(CPoint(175, 13), 35, 20),   m_pGroupBoxTabMemory, "Poke");
    m_pMemButtonPoke->SetIsFocusable(true);

    m_pMemAdressLabel     = new CLabel(        CPoint(15, 50),             m_pGroupBoxTabMemory, "Adress: ");
    m_pMemAdressValue     = new CEditBox(CRect(CPoint(55, 45), 35, 20),    m_pGroupBoxTabMemory);
    m_pMemAdressValue->SetIsFocusable(true);
    m_pMemButtonDisplay   = new CButton( CRect(CPoint(95, 45), 45, 20),    m_pGroupBoxTabMemory, "Display");
    m_pMemButtonDisplay->SetIsFocusable(true);

    m_pMemBytesPerLineLbl = new CLabel(       CPoint(235, 18),             m_pGroupBoxTabMemory, "Bytes per line:");
    m_pMemBytesPerLine  = new CDropDown( CRect(CPoint(315, 13), 50, 20),   m_pGroupBoxTabMemory, false);
    m_pMemBytesPerLine->AddItem(SListItem("1"));
    m_pMemBytesPerLine->AddItem(SListItem("2"));
    m_pMemBytesPerLine->AddItem(SListItem("4"));
    m_pMemBytesPerLine->AddItem(SListItem("8"));
    m_pMemBytesPerLine->AddItem(SListItem("16"));
    m_pMemBytesPerLine->AddItem(SListItem("32"));
    m_pMemBytesPerLine->AddItem(SListItem("64"));
    m_pMemBytesPerLine->SetListboxHeight(4);
    m_MemBytesPerLine = 16;
    m_pMemBytesPerLine->SelectItem(4);
    m_pMemBytesPerLine->SetIsFocusable(true);

    m_pMemFormatLbl      = new CLabel(       CPoint(235, 38),             m_pGroupBoxTabMemory, "Format:");
    m_pMemFormat        = new CDropDown( CRect(CPoint(280, 33), 85, 20),   m_pGroupBoxTabMemory, true);
    m_pMemFormat->AddItem(SListItem("Hex"));
    m_pMemFormat->AddItem(SListItem("Hex & char"));
    m_pMemFormat->AddItem(SListItem("Hex & u8"));
    m_pMemFormat->AddItem(SListItem("Hex & u16"));
    m_pMemFormat->AddItem(SListItem("Hex & u32"));
    m_pMemFormat->AddItem(SListItem("Hex & i8"));
    m_pMemFormat->AddItem(SListItem("Hex & i16"));
    m_pMemFormat->AddItem(SListItem("Hex & i32"));
    m_pMemFormat->SetListboxHeight(4);
    m_MemFormat = {Format::Hex};
    m_pMemFormat->SelectItem(0);
    m_pMemFormat->SetIsFocusable(true);

    m_pMemFilterLabel     = new CLabel(        CPoint(15, 80),             m_pGroupBoxTabMemory, "Byte: ");
    m_pMemFilterValue     = new CEditBox(CRect(CPoint(55, 75), 30, 20),    m_pGroupBoxTabMemory);
    m_pMemFilterValue->SetIsFocusable(true);
    m_pMemButtonFilter    = new CButton( CRect(CPoint(95, 75), 45, 20),    m_pGroupBoxTabMemory, "Filter");
    m_pMemButtonFilter->SetIsFocusable(true);
    m_pMemButtonSaveFilter    = new CButton( CRect(CPoint(150, 75), 90, 20),    m_pGroupBoxTabMemory, "Save Filter");
    m_pMemButtonSaveFilter->SetIsFocusable(true);
    m_pMemButtonApplyFilter    = new CButton( CRect(CPoint(250, 75), 90, 20),    m_pGroupBoxTabMemory, "Apply saved");
    m_pMemButtonApplyFilter->SetIsFocusable(true);
    m_pMemButtonCopy      = new CButton( CRect(CPoint(380, 325), 95, 20),   m_pGroupBoxTabMemory, "Dump to stdout");
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
    m_pMemWatchPointsGrp = new CGroupBox(CRect(CPoint(380, 13), 240, 120), m_pGroupBoxTabMemory, "Watch points");
    m_pMemWatchPoints = new CListBox(CRect(CPoint(10, 5), 80, 80), m_pMemWatchPointsGrp,
        /*bSingleSelection=*/false, /*iItemHeight=*/15, monoFontEngine);
    m_pMemRemoveWatchPoint = new CButton(CRect(CPoint(110, 5), 100, 20), m_pMemWatchPointsGrp, "Remove selected");
    m_pMemNewWatchPoint = new CEditBox(CRect(CPoint(110, 40), 50, 20), m_pMemWatchPointsGrp);
    m_pMemNewWatchPoint->SetContentType(CEditBox::HEXNUMBER);
    m_pMemWatchPointType = new CDropDown(CRect(CPoint(170, 40), 50, 20), m_pMemWatchPointsGrp);
    m_pMemWatchPointType->AddItem(SListItem("R"));
    m_pMemWatchPointType->AddItem(SListItem("W"));
    m_pMemWatchPointType->AddItem(SListItem("RW"));
    m_pMemWatchPointType->SelectItem(2);
    m_pMemAddWatchPoint = new CButton(CRect(CPoint(110, 65), 50, 20), m_pMemWatchPointsGrp, "Add");

    m_pMemConfigGrp = new CGroupBox(CRect(CPoint(380, 143), 240, 100), m_pGroupBoxTabMemory, "RAM config");
    m_pMemConfigMemLbl = new CLabel(CPoint(10, 30), m_pMemConfigGrp, "Mem:");
    m_pMemConfigCurLbl = new CLabel(CPoint(10, 55), m_pMemConfigGrp, "Cur:");
    m_pMemConfigROMLbl = new CLabel(CPoint(60, 0), m_pMemConfigGrp, "ROM");
    m_pMemConfigLoLbl = new CLabel(CPoint(50, 10), m_pMemConfigGrp, "Lo");
    m_pMemConfigHiLbl = new CLabel(CPoint(80, 10), m_pMemConfigGrp, "Hi");
    m_pMemConfigRAMLbl = new CLabel(CPoint(150, 0), m_pMemConfigGrp, "RAM");
    m_pMemConfigBankLbl = new CLabel(CPoint(120, 10), m_pMemConfigGrp, "Bank");
    m_pMemConfigConfigLbl = new CLabel(CPoint(160, 10), m_pMemConfigGrp, "Config");

    // TODO: Distinct button to dump to stdout and copy to clipboard
    // TODO: Save memory dump to a file (same as dump to stdout but with a messagebox asking for a filename)

    m_pMemConfigMemLoROM = new CCheckBox(CRect(CPoint(50, 30), 10, 10), m_pMemConfigGrp);
    m_pMemConfigMemHiROM = new CCheckBox(CRect(CPoint(80, 30), 10, 10), m_pMemConfigGrp);
    m_pMemConfigMemRAMBank = new CDropDown(CRect(CPoint(110, 25), 40, 20), m_pMemConfigGrp);
    int nb_banks = (CPC.ram_size == 64) ? 1 : ((CPC.ram_size / 64)-1);
    for (int i = 0; i < nb_banks; i++) m_pMemConfigMemRAMBank->AddItem(SListItem(std::to_string(i)));
    // TODO: Default to RAMBank & RAMConfig that are currently used
    m_pMemConfigMemRAMBank->SelectItem(0);
    m_pMemConfigMemRAMConfig = new CDropDown(CRect(CPoint(160, 25), 70, 20), m_pMemConfigGrp);
    int nb_configs = (CPC.ram_size == 64) ? 1 : 8;
    for (int i = 0; i < nb_configs; i++) m_pMemConfigMemRAMConfig->AddItem(SListItem(RAMConfig::RAMConfigText(i)));
    m_pMemConfigMemRAMConfig->SelectItem(0);

    m_pMemConfigCurLoROM = new CCheckBox(CRect(CPoint(50, 55), 10, 10), m_pMemConfigGrp);
    m_pMemConfigCurLoROM->SetReadOnly(true);
    m_pMemConfigCurHiROM = new CCheckBox(CRect(CPoint(80, 55), 10, 10), m_pMemConfigGrp);
    m_pMemConfigCurHiROM->SetReadOnly(true);
    m_pMemConfigCurRAMBank = new CEditBox(CRect(CPoint(110, 50), 40, 20), m_pMemConfigGrp);
    m_pMemConfigCurRAMBank->SetReadOnly(true);
    m_pMemConfigCurRAMConfig = new CEditBox(CRect(CPoint(160, 50), 70, 20), m_pMemConfigGrp);
    m_pMemConfigCurRAMConfig->SetReadOnly(true);

    // ---------------- 'Video' screen ----------------
    m_pVidLabel = new CLabel(CPoint(10, 10), m_pGroupBoxTabVideo, "Work in progress ... Nothing to see here yet, but come back later for video (CRTC & GateArray info).");

    // ---------------- 'Audio' screen ----------------
    m_pAudPSGGrp = new CGroupBox(
        CRect(CPoint(5, 0), m_pGroupBoxTabAudio->GetClientRect().Width()-10, 120),
        m_pGroupBoxTabAudio, "PSG");
    m_pAudChannelALabel = new CLabel(CPoint(10, 30), m_pGroupBoxTabAudio, "Channel A:");
    m_pAudChannelBLabel = new CLabel(CPoint(10, 50), m_pGroupBoxTabAudio, "Channel B:");
    m_pAudChannelCLabel = new CLabel(CPoint(10, 70), m_pGroupBoxTabAudio, "Channel C:");
    m_pAudNoiseLabel = new CLabel(CPoint(10, 90), m_pGroupBoxTabAudio, "Noise:");

    m_pAudToneLabel = new CLabel(CPoint(100, 10), m_pGroupBoxTabAudio, "Tone");
    m_pAudVolLabel = new CLabel(CPoint(180, 10), m_pGroupBoxTabAudio, "Volume");
    m_pAudToneOnOffLabel = new CLabel(CPoint(230, 10), m_pGroupBoxTabAudio, "Tone");
    m_pAudNoiseOnOffLabel = new CLabel(CPoint(280, 10), m_pGroupBoxTabAudio, "Noise");

    m_pAudFreqA = new CLabel(CPoint(95, 30), m_pGroupBoxTabAudio);
    m_pAudFreqB = new CLabel(CPoint(95, 50), m_pGroupBoxTabAudio);
    m_pAudFreqC = new CLabel(CPoint(95, 70), m_pGroupBoxTabAudio);
    m_pAudFreqNoise = new CLabel(CPoint(95, 90), m_pGroupBoxTabAudio);
    m_pAudVolA = new CLabel(CPoint(180, 30), m_pGroupBoxTabAudio);
    m_pAudVolB = new CLabel(CPoint(180, 50), m_pGroupBoxTabAudio);
    m_pAudVolC = new CLabel(CPoint(180, 70), m_pGroupBoxTabAudio);

    m_pAudToneA = new CCheckBox(CRect(CPoint(230, 30), 10, 10), m_pGroupBoxTabAudio);
    m_pAudToneB = new CCheckBox(CRect(CPoint(230, 50), 10, 10), m_pGroupBoxTabAudio);
    m_pAudToneC = new CCheckBox(CRect(CPoint(230, 70), 10, 10), m_pGroupBoxTabAudio);
    m_pAudNoiseA = new CCheckBox(CRect(CPoint(280, 30), 10, 10), m_pGroupBoxTabAudio);
    m_pAudNoiseB = new CCheckBox(CRect(CPoint(280, 50), 10, 10), m_pGroupBoxTabAudio);
    m_pAudNoiseC = new CCheckBox(CRect(CPoint(280, 70), 10, 10), m_pGroupBoxTabAudio);

    m_pAudMixerControlLabel = new CLabel(CPoint(390, 30), m_pGroupBoxTabAudio, "Mixer control:");
    m_pAudVolEnvFreqLabel = new CLabel(CPoint(390, 50), m_pGroupBoxTabAudio, "Envelope frequence:");
    m_pAudVolEnvShapeLabel = new CLabel(CPoint(390, 70), m_pGroupBoxTabAudio, "Envelope shape:");

    m_pAudMixerControl = new CLabel(CPoint(520, 30), m_pGroupBoxTabAudio);
    m_pAudVolEnvFreq = new CLabel(CPoint(520, 50), m_pGroupBoxTabAudio);
    m_pAudVolEnvShape = new CLabel(CPoint(520, 70), m_pGroupBoxTabAudio);

    m_pAudToneA->SetReadOnly(true);
    m_pAudToneB->SetReadOnly(true);
    m_pAudToneC->SetReadOnly(true);
    m_pAudNoiseA->SetReadOnly(true);
    m_pAudNoiseB->SetReadOnly(true);
    m_pAudNoiseC->SetReadOnly(true);

    // ---------------- 'Characters' screen ----------------
    m_pChrLabel = new CLabel(CPoint(10, 10), m_pGroupBoxTabChar, "Work in progress ... Nothing to see here yet, but come back later for charmap.");
    // TODO: A 'Graphics' screen displaying memory as graphics (sprites) with choice of mode (0, 1 or 2), width and start address.

    // Moved to an explicit call by the caller to allow constructing
    // CapriceDevTools with less constraints in tests.
    //UpdateAll();
}

CapriceDevTools::~CapriceDevTools() = default;

void CapriceDevTools::UnlockRegisters()
{
  if (!CPC.paused) {
    return;
  }
  // Ensure registers are up-to-date before unlocking them.
  UpdateZ80();

  registersLocked = false;
  m_pZ80ModifyRegisters->SetWindowText("Save & Lock");

  // Only allow to modify registers for which saving is implemented.
  // The main problem is that a same register is displayed multiple times
  // (e.g. A in both A and AF) so allowing to modify both leads to the problem
  // that we don't know which one we should take.
  m_pZ80RegA->SetReadOnly(false);
  m_pZ80RegB->SetReadOnly(false);
  m_pZ80RegC->SetReadOnly(false);
  m_pZ80RegD->SetReadOnly(false);
  m_pZ80RegE->SetReadOnly(false);
  m_pZ80RegH->SetReadOnly(false);
  m_pZ80RegI->SetReadOnly(false);
  m_pZ80RegL->SetReadOnly(false);
  m_pZ80RegR->SetReadOnly(false);
  m_pZ80RegIX->SetReadOnly(false);
  m_pZ80RegIY->SetReadOnly(false);
  m_pZ80RegPC->SetReadOnly(false);
  m_pZ80RegSP->SetReadOnly(false);
  m_pZ80RegF->SetReadOnly(false);
  // TODO: Allow to modify prime registers too.
}

void CapriceDevTools::LockRegisters()
{
  registersLocked = true;
  m_pZ80ModifyRegisters->SetWindowText("Unlock registers");

  // Only allow to modify registers for which saving is implemented.
  // The main problem is that a same register is displayed multiple times
  // (e.g. A in both A and AF) so allowing to modify both leads to the problem
  // that we don't know which one we should take.
  m_pZ80RegA->SetReadOnly(true);
  m_pZ80RegB->SetReadOnly(true);
  m_pZ80RegC->SetReadOnly(true);
  m_pZ80RegD->SetReadOnly(true);
  m_pZ80RegE->SetReadOnly(true);
  m_pZ80RegH->SetReadOnly(true);
  m_pZ80RegI->SetReadOnly(true);
  m_pZ80RegL->SetReadOnly(true);
  m_pZ80RegR->SetReadOnly(true);
  m_pZ80RegIX->SetReadOnly(true);
  m_pZ80RegIY->SetReadOnly(true);
  m_pZ80RegPC->SetReadOnly(true);
  m_pZ80RegSP->SetReadOnly(true);
  m_pZ80RegF->SetReadOnly(true);

  // TODO: Allow to modify individual flags instead of F as a whole.
  m_pZ80FlagS->SetReadOnly(true);
  m_pZ80FlagZ->SetReadOnly(true);
  m_pZ80FlagX1->SetReadOnly(true);
  m_pZ80FlagH->SetReadOnly(true);
  m_pZ80FlagX2->SetReadOnly(true);
  m_pZ80FlagPV->SetReadOnly(true);
  m_pZ80FlagN->SetReadOnly(true);
  m_pZ80FlagC->SetReadOnly(true);

  // Update the values that depend on what may have been modified.
  UpdateZ80();
}

void CapriceDevTools::SaveRegisters()
{
  _A = m_pZ80RegA->GetValue();
  _B = m_pZ80RegB->GetValue();
  _C = m_pZ80RegC->GetValue();
  _D = m_pZ80RegD->GetValue();
  _E = m_pZ80RegE->GetValue();
  _F = m_pZ80RegF->GetValue();
  _H = m_pZ80RegH->GetValue();
  _I = m_pZ80RegI->GetValue();
  _L = m_pZ80RegL->GetValue();
  _R = m_pZ80RegR->GetValue();
  _IX = m_pZ80RegIXH->GetValue();
  _IY = m_pZ80RegIYH->GetValue();
  _PC = m_pZ80RegPC->GetValue();
  _SP = m_pZ80RegSP->GetValue();
}

void CapriceDevTools::UpdateDisassemblyPos()
{
  auto lines = m_pAssemblyCode->GetAllItems();
  SListItem toFind("ignored", reinterpret_cast<void*>(_PC));
  auto curpos = std::lower_bound(lines.begin(), lines.end(), toFind, [](auto x, auto y) {
    return x.pItemData < y.pItemData;
  });
  int idx = std::distance(lines.begin(), curpos);
  // TODO: Provide a way to jump to an address. Main problem is finding some space for the editbox and button
  // One option: Move "Disassembling / SUCCESS" down as some status bar (below the main group box).
  // TODO: Think about providing a way to trace execution (have all addresses through which the execution went).
  // Maybe only record call / jps / jrs / rst / ret destinations?
  // TODO: Would be nice to have a list of labels and be able to jump to code using them
  // TODO: Would be nice to be able to add / edit labels
  m_pAssemblyCode->SetPosition(idx, CListBox::CENTER);
  if (!lines.empty() && curpos->pItemData == toFind.pItemData) {
    // Skip over address labels
    while ((curpos = std::next(curpos))->pItemData == toFind.pItemData) idx++;
    // TODO: Do not allow to select another line
    m_pAssemblyCode->SetSelection(idx, /*bSelected=*/true, /*bNotify=*/false);
  } else {
    m_pAssemblyCode->SetAllSelections(false);
    m_pAssemblyCode->Draw();
  }

  m_pAssemblyMemConfigAsmLoROM->SetCheckBoxState(m_AsmRAMConfig.LoROMEnabled ? CCheckBox::CHECKED : CCheckBox::UNCHECKED);
  m_pAssemblyMemConfigAsmHiROM->SetCheckBoxState(m_AsmRAMConfig.HiROMEnabled ? CCheckBox::CHECKED : CCheckBox::UNCHECKED);
  m_pAssemblyMemConfigAsmRAMBank->SetWindowText(std::to_string(m_AsmRAMConfig.RAMBank));
  m_pAssemblyMemConfigAsmRAMConfig->SetWindowText(m_AsmRAMConfig.RAMConfigText());

  RAMConfig CurConfig = RAMConfig::CurrentConfig();
  m_pAssemblyMemConfigCurLoROM->SetCheckBoxState(CurConfig.LoROMEnabled ? CCheckBox::CHECKED : CCheckBox::UNCHECKED);
  m_pAssemblyMemConfigCurHiROM->SetCheckBoxState(CurConfig.HiROMEnabled ? CCheckBox::CHECKED : CCheckBox::UNCHECKED);
  m_pAssemblyMemConfigCurRAMBank->SetWindowText(std::to_string(CurConfig.RAMBank));
  m_pAssemblyMemConfigCurRAMConfig->SetWindowText(CurConfig.RAMConfigText());
}

std::string FormatSymbol(const std::map<word, std::string>::iterator& symbol_it)
{
  std::ostringstream oss;
  oss << symbol_it->second << ": [" << std::hex << std::setw(4) << std::setfill('0') << symbol_it->first << "]";
  return oss.str();
}

void CapriceDevTools::AsmSearch(SearchFrom from, SearchDir dir)
{
  const auto& lines = m_pAssemblyCode->GetAllItems();
  std::vector<SListItem>::const_iterator start_line, end_line;
  auto pos = m_pAssemblyCode->getFirstSelectedIndex();
  if (pos == -1) {
    from = SearchFrom::Start;
  }
  int delta = dir == SearchDir::Forward ? 1 : -1;
  switch (from)
  {
    case SearchFrom::PositionIncluded:
      break;
    case SearchFrom::PositionExcluded:
      pos += delta;
      break;
    case SearchFrom::Start:
      if (dir == SearchDir::Forward) {
        pos = 0;
      } else {
        pos = lines.size() - 1;
      }
      break;
  }

  start_line = lines.begin() + pos;
  if (dir == SearchDir::Forward) {
    end_line = lines.end();
  } else {
    end_line = lines.begin()-1;
  }

  std::string to_find = m_pAssemblySearch->GetWindowText();
  for (auto l = start_line; l != end_line; l += delta)
  {
    if (l->sItemText.find(to_find) != std::string::npos) {
      m_pAssemblyCode->SetAllSelections(false);
      m_pAssemblyCode->SetPosition(pos, CListBox::CENTER);
      m_pAssemblyCode->SetSelection(pos, /*bSelected=*/true, /*bNotify=*/true);
      break;
    }
    pos += delta;
  }
}

void CapriceDevTools::SetDisassembly(std::vector<SListItem> items)
{
  m_pAssemblyCode->ClearItems();
  m_pAssemblyCode->AddItems(items);
}

std::vector<SListItem> CapriceDevTools::GetSelectedAssembly()
{
  // TODO: This code should really be in CListBox!!
  std::vector<SListItem> result;
  for(unsigned int i = 0; i < m_pAssemblyCode->Size(); i++)
  {
    if (m_pAssemblyCode->IsSelected(i)) {
      result.push_back(m_pAssemblyCode->GetItem(i));
    }
  }
  return result;
}

void CapriceDevTools::SetAssemblySearch(const std::string& text)
{
  m_pAssemblySearch->SetWindowText(text);
}

void CapriceDevTools::RefreshDisassembly()
{
  std::vector<SListItem> items;
  std::map<word, std::string> symbols = m_Symfile.Symbols();
  std::map<word, std::string>::iterator symbols_it = symbols.begin();
  for (const auto& line : m_Disassembled.lines) {
    while (symbols_it != symbols.end() && symbols_it->first <= line.address_) {
      items.emplace_back(FormatSymbol(symbols_it), reinterpret_cast<void*>(symbols_it->first), COLOR_BLUE);
      symbols_it++;
    }
    std::ostringstream oss;
    oss << std::hex << " " << std::setw(4) << std::setfill('0') << line.address_ << ": ";
    if (line.opcode_ <= 0xFF) {
      oss << "        " << std::setw(2) << std::setfill('0') << line.opcode_;
    } else if (line.opcode_ <= 0xFFFF) {
      oss << "      " << std::setw(4) << std::setfill('0') << line.opcode_;
    } else if (line.opcode_ <= 0xFFFFFF) {
      oss << "    " << std::setw(6) << std::setfill('0') << line.opcode_;
    } else if (line.opcode_ <= 0xFFFFFFFF) {
      oss << "  " << std::setw(8) << std::setfill('0') << line.opcode_;
    } else {
      oss << std::setw(10) << std::setfill('0') << line.opcode_;
    }
    oss << "     " << line.instruction_;
    std::string instruction = oss.str();
    std::map<word, std::string>::iterator it;
    if (!line.ref_address_string_.empty() &&
        ((it = symbols.find(line.ref_address_)) != symbols.end())) {
      instruction = stringutils::replace(instruction, line.ref_address_string_, it->second);
    }
    // TODO: smart use of colors. Ideas:
    //   - labels, jumps & calls, ...
    //   - source of disassembling (from PC, from one entry point or another ...)
    auto color = COLOR_BLACK;
    if (std::any_of(breakpoints.begin(), breakpoints.end(), [&](const auto& b) {
          return (b.address == line.address_);
          })) {
      color = COLOR_RED;
    } else if (line.instruction_ == "ret") {
      color = COLOR_BLUE;
    }/* else if (!line.ref_address_string_.empty()) {
      color = COLOR_DARKGREEN;
    }*/
    items.emplace_back(instruction, reinterpret_cast<void*>(line.address_), color);
  }

  SetDisassembly(items);
  UpdateDisassemblyPos();
}

void CapriceDevTools::UpdateDisassembly()
{
  // TODO: Disassemble in a thread
  m_pAssemblyStatus->SetWindowText("Disassembling...");
  // We need to force the repaint for the status to be displayed.
  m_pParentWindow->HandleMessage(new CMessage(CMessage::APP_PAINT, GetAncestor(ROOT), this));
  m_Disassembled = disassemble(m_EntryPoints);
  m_AsmRAMConfig = RAMConfig::CurrentConfig();
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

  m_pZ80FlagS->SetValue((z80.AF.b.l & Sflag) ? "1" : "0");
  m_pZ80FlagZ->SetValue((z80.AF.b.l & Zflag) ? "1" : "0");
  m_pZ80FlagX1->SetValue((z80.AF.b.l & X1flag) ? "1" : "0");
  m_pZ80FlagH->SetValue((z80.AF.b.l & Hflag) ? "1" : "0");
  m_pZ80FlagX2->SetValue((z80.AF.b.l & X2flag) ? "1" : "0");
  m_pZ80FlagPV->SetValue((z80.AF.b.l & Pflag) ? "1" : "0");
  m_pZ80FlagN->SetValue((z80.AF.b.l & Nflag) ? "1" : "0");
  m_pZ80FlagC->SetValue((z80.AF.b.l & Cflag) ? "1" : "0");

  m_pZ80Stack->ClearItems();
  // Don't show more than 50 values in the stack if not paused as this slows
  // down everything.
  auto last_address = 0xC000;
  if (!CPC.paused) {
    last_address = std::min(0xC000, z80.SP.w.l + 100);
  }
  for (word addr = z80.SP.w.l; addr < last_address; addr += 2) {
    std::ostringstream oss;
    word val = (z80_read_mem(addr+1) << 8) + z80_read_mem(addr);
    oss << std::hex << std::setw(4) << std::setfill('0') << val
      << " (" << std::dec << val << ")";
    m_pZ80Stack->AddItem(SListItem(oss.str()));
    if (m_pZ80Stack->Size() >= CPC.devtools_max_stack_size) {
      m_pZ80Stack->AddItem(SListItem("(...)"));
      break;
    }
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
    oss << std::hex << std::setw(4) << std::setfill('0') << bp.address << "  "
      << ((bp.type & READ) ? "R" : "")
      << ((bp.type & WRITE) ? "W" : "");
    m_pMemWatchPoints->AddItem(SListItem(oss.str()));
  }
}

std::string toneString(unsigned short tone) {
  if (tone == 0) {
    return "0";
  }
  return std::to_string(tone) + " (" + std::to_string(62500/tone)+ " Hz)";
}

void CapriceDevTools::UpdateMemConfig()
{
  RAMConfig CurConfig = RAMConfig::CurrentConfig();
  m_pMemConfigCurLoROM->SetCheckBoxState(CurConfig.LoROMEnabled ? CCheckBox::CHECKED : CCheckBox::UNCHECKED);
  m_pMemConfigCurHiROM->SetCheckBoxState(CurConfig.HiROMEnabled ? CCheckBox::CHECKED : CCheckBox::UNCHECKED);
  m_pMemConfigCurRAMBank->SetWindowText(std::to_string(CurConfig.RAMBank));
  m_pMemConfigCurRAMConfig->SetWindowText(CurConfig.RAMConfigText());
}

void CapriceDevTools::UpdateAudio()
{
    // TODO(cpitrat): More user friendly display:
    //  - frequency in Hz on top of internal repr for frequency.
    // TODO(cpitrat): Make these fields read-only
    m_pAudFreqA->SetWindowText(toneString(PSG.RegisterAY.TonA));
    m_pAudFreqB->SetWindowText(toneString(PSG.RegisterAY.TonB));
    m_pAudFreqC->SetWindowText(toneString(PSG.RegisterAY.TonC));
    m_pAudFreqNoise->SetWindowText(toneString(PSG.RegisterAY.Noise));
    m_pAudToneA->SetCheckBoxState(PSG.RegisterAY.Mixer & 1 ? CCheckBox::UNCHECKED : CCheckBox::CHECKED);
    m_pAudToneB->SetCheckBoxState(PSG.RegisterAY.Mixer & 2 ? CCheckBox::UNCHECKED : CCheckBox::CHECKED);
    m_pAudToneC->SetCheckBoxState(PSG.RegisterAY.Mixer & 4 ? CCheckBox::UNCHECKED : CCheckBox::CHECKED);
    m_pAudNoiseA->SetCheckBoxState(PSG.RegisterAY.Mixer & 8 ? CCheckBox::UNCHECKED : CCheckBox::CHECKED);
    m_pAudNoiseB->SetCheckBoxState(PSG.RegisterAY.Mixer & 16 ? CCheckBox::UNCHECKED : CCheckBox::CHECKED);
    m_pAudNoiseC->SetCheckBoxState(PSG.RegisterAY.Mixer & 32 ? CCheckBox::UNCHECKED : CCheckBox::CHECKED);
    m_pAudMixerControl->SetWindowText(std::to_string(PSG.RegisterAY.Mixer));
    m_pAudVolA->SetWindowText(std::to_string(PSG.RegisterAY.AmplitudeA));
    m_pAudVolB->SetWindowText(std::to_string(PSG.RegisterAY.AmplitudeB));
    m_pAudVolC->SetWindowText(std::to_string(PSG.RegisterAY.AmplitudeC));
    m_pAudVolEnvFreq->SetWindowText(toneString(PSG.RegisterAY.Envelope));
    auto envtype = std::to_string(PSG.RegisterAY.EnvType);
    switch (PSG.RegisterAY.EnvType) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 9:
        envtype += " \\_________";
        break;
      case 4:
      case 5:
      case 6:
      case 7:
        envtype += " /_________";
        break;
      case 8:
        envtype += " \\\\\\\\\\\\\\\\\\\\";
        break;
      case 10:
        envtype += " \\/\\/\\/\\/\\/";
        break;
      case 11:
        envtype += " \\'''''''''";
        break;
      case 12:
        envtype += " //////////";
        break;
      case 13:
        envtype += " /'''''''''";
        break;
      case 14:
        envtype += " /\\/\\/\\/\\/\\";
        break;
      case 15:
        envtype += " /_________";
        break;
      default:
        envtype += " (unsupported)";
        break;
    }
    m_pAudVolEnvShape->SetWindowText(envtype);
}

void CapriceDevTools::PrepareMemBankConfig()
{
  ga_init_banking(memtool_membank_config, m_pMemConfigMemRAMBank->GetSelectedIndex());
  if (m_pMemConfigMemLoROM->GetCheckBoxState() == CCheckBox::CHECKED) {
    for (int i = 0; i < 8; i++) memtool_membank_config[i][GateArray.lower_ROM_bank] = pbROMlo;
  }
  // TODO: Provide option to have register page on if on the CPC+
  if (m_pMemConfigMemHiROM->GetCheckBoxState() == CCheckBox::CHECKED) {
    for (int i = 0; i < 8; i++) memtool_membank_config[i][3] = pbExpansionROM;
  }
}

byte CapriceDevTools::ReadMem(word address)
{
  return memtool_membank_config[m_pMemConfigMemRAMConfig->GetSelectedIndex()][address >> 14][address & 0x3fff];
}

void CapriceDevTools::WriteMem(word address, byte value)
{
  memtool_membank_config[m_pMemConfigMemRAMConfig->GetSelectedIndex()][address >> 14][address & 0x3fff] = value;
}

void CapriceDevTools::UpdateTextMemory()
{
  PrepareMemBankConfig();
  m_currentlyDisplayed.clear();
  std::ostringstream memText;
  for(unsigned int i = 0; i < 65536/m_MemBytesPerLine; i++) {
    if (!m_currentlyFiltered.empty() &&
        !std::binary_search(m_currentlyFiltered.begin(), m_currentlyFiltered.end(), i)) {
        continue;
    }
    std::ostringstream memLine;
    memLine << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << i*m_MemBytesPerLine << " : ";
    bool displayLine = false;
    bool filterAdress = (m_MemDisplayValue >= 0 && m_MemDisplayValue <= 65535);
    bool filterValue = (m_MemFilterValue >= 0 && m_MemFilterValue <= 255);
    bool first = true;
    for(auto f : m_MemFormat) {
      if (!first) {
        memLine << " | ";
      }
      first = false;
      for(unsigned int j = 0; j < m_MemBytesPerLine; j+=FormatSize(f)) {
        switch (f) {
          case Format::Hex:
            {
              unsigned int val = ReadMem(i*m_MemBytesPerLine+j);
              memLine << std::setfill('0') << std::setw(2) << std::hex << val << " ";
              break;
            }
          case Format::Char:
            {
              char val = ReadMem(i*m_MemBytesPerLine+j);
              if (val >= 32) {
                memLine << val;
              } else {
                memLine << ".";
              }
              break;
            }
          case Format::U8:
            {
              unsigned int val = ReadMem(i*m_MemBytesPerLine+j);
              memLine << std::setfill(' ') << std::setw(3) << std::dec << val << " ";
              break;
            }
          case Format::U16:
            {
              unsigned int val = static_cast<uint32_t>(ReadMem(i*m_MemBytesPerLine+j)) +
                (static_cast<uint32_t>(ReadMem(i*m_MemBytesPerLine+j+1) << 8));
              memLine << std::setfill(' ') << std::setw(5) << std::dec << val << " ";
              break;
            }
          case Format::U32:
            {
              unsigned int val = static_cast<uint32_t>(ReadMem(i*m_MemBytesPerLine+j)) +
                (static_cast<uint32_t>(ReadMem(i*m_MemBytesPerLine+j+1) << 8)) +
                (static_cast<uint32_t>(ReadMem(i*m_MemBytesPerLine+j+2) << 16)) +
                (static_cast<uint32_t>(ReadMem(i*m_MemBytesPerLine+j+3) << 24));
              memLine << std::setfill(' ') << std::setw(10) << std::dec << val << " ";
              break;
            }
          case Format::I8:
            {
              int8_t val = static_cast<int8_t>(ReadMem(i*m_MemBytesPerLine+j));
              memLine << std::setfill(' ') << std::setw(4) << std::dec << val << " ";
              break;
            }
          case Format::I16:
            {
              int16_t val = static_cast<uint16_t>(ReadMem(i*m_MemBytesPerLine+j)) +
                (static_cast<uint16_t>(ReadMem(i*m_MemBytesPerLine+j+1) << 8));
              memLine << std::setfill(' ') << std::setw(6) << std::dec << val << " ";
              break;
            }
          case Format::I32:
            {
              int32_t val = static_cast<int32_t>(ReadMem(i*m_MemBytesPerLine+j)) +
                (static_cast<int32_t>(ReadMem(i*m_MemBytesPerLine+j+1) << 8)) +
                (static_cast<int32_t>(ReadMem(i*m_MemBytesPerLine+j+2) << 16)) +
                (static_cast<int32_t>(ReadMem(i*m_MemBytesPerLine+j+3) << 24));
              memLine << std::setfill(' ') << std::setw(11) << std::dec << val << " ";
              break;
            }
        }
      }
      if(!filterAdress && !filterValue) {
        displayLine = true;
      } else {
        for(unsigned int j = 0; j < m_MemBytesPerLine; j++) {
          unsigned int val = ReadMem(i*m_MemBytesPerLine+j);
          if(filterValue && static_cast<int>(val) == m_MemFilterValue) {
            displayLine = true;
          }
          if(filterAdress && (i*m_MemBytesPerLine+j == static_cast<unsigned int>(m_MemDisplayValue))) {
            displayLine = true;
          }
        }
      }
    }
    if(displayLine) {
      m_currentlyDisplayed.push_back(i);
    }
    if(displayLine) {
      memText << memLine.str() << "\n";
    }
  }
  m_pMemTextContent->SetWindowText(memText.str().substr(0, memText.str().size()-1));
}

void CapriceDevTools::PauseExecution()
{
  if (!CPC.paused) {
    // Update all before pausing to be sure that the state is consistent.
    // This is particularly important for Z80 as the registers need to be
    // up-to-date otherwise we'll mess with the state when we resume.
    UpdateAll();
  }
  CPC.paused = true;
  m_pButtonPause->SetWindowText("Resume");
}

void CapriceDevTools::ResumeExecution()
{
  LockRegisters();
  CPC.paused = false;
  m_pButtonPause->SetWindowText("Pause");
}

void CapriceDevTools::LoadSymbols(const std::string& filename)
{
  m_Symfile = Symfile(filename);
  for (auto breakpoint : m_Symfile.Breakpoints()) {
    if (std::find_if(breakpoints.begin(), breakpoints.end(),
          [&](const auto& bp) { return bp.address == breakpoint; } ) != breakpoints.end()) continue;
    breakpoints.emplace_back(breakpoint);
  }
  for (auto entrypoint : m_Symfile.Entrypoints()) {
    if (std::find(m_EntryPoints.begin(), m_EntryPoints.end(), entrypoint) != m_EntryPoints.end()) continue;
    m_EntryPoints.push_back(entrypoint);
  }
  UpdateEntryPointsList();
  UpdateBreakPointsList();
  RefreshDisassembly();
}

void CapriceDevTools::RemoveEphemeralBreakpoints()
{
  breakpoints.erase(
      std::remove_if(breakpoints.begin(), breakpoints.end(),
                     [](const auto& x){ return x.type & EPHEMERAL; }),
      breakpoints.end());
}

void CapriceDevTools::PreUpdate()
{
  static bool wasRunning;
  // Pause on breakpoints and watchpoints.
  // Before updating display so that we can update differently: faster if not
  // paused, more details if paused.
  if (!breakpoints.empty() || !watchpoints.empty()) {
    if (z80.watchpoint_reached || z80.breakpoint_reached) {
      PauseExecution();
      RemoveEphemeralBreakpoints();
    };
  }
  if (CPC.paused) {
    m_pButtonPause->SetWindowText("Resume");
  } else {
    m_pButtonPause->SetWindowText("Pause");
  }
  // Do not update if we're paused.
  // This is particularly needed for disassembly pos as otherwise it's not
  // possible to scroll in the disassembled code.
  if (wasRunning) {
    wasRunning = !CPC.paused;
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
                 UpdateMemConfig();
                 break;
               }
      case 3 : { // 'Video'
                 break;
               }
      case 4 : { // 'Audio'
                 UpdateAudio();
                 break;
               }
      case 5 : { // 'Characters'
                 break;
               }
    }
  } else {
    wasRunning = !CPC.paused;
  }
}

void CapriceDevTools::PostUpdate()
{
  if (z80.step_in > 1) {
    PauseExecution();
    z80.step_in = 0;
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
    UpdateAudio();
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
            if (pMessage->Source() == m_pButtonStepIn) {
              z80.step_in = 1;
              ResumeExecution();
              break;
            }
            if (pMessage->Source() == m_pButtonStepOver) {
              // Like StepIn except if the instruction is a call
              std::vector<dword> unused_entrypoints;
              DisassembledCode unused_code;
              auto current_line = disassemble_one(z80.PC.d, unused_code, unused_entrypoints);
              if (current_line.instruction_.rfind("call", 0) == 0) {
                breakpoints.emplace_back(z80.PC.d + current_line.Size(), EPHEMERAL);
              } else {
                z80.step_in = 1;
              }
              ResumeExecution();
              break;
            }
            if (pMessage->Source() == m_pButtonStepOut) {
              z80.step_out = 1;
              z80.step_out_addresses.clear();
              ResumeExecution();
              break;
            }
          }
          if (pMessage->Source() == m_pZ80ModifyRegisters) {
            if (registersLocked) {
              UnlockRegisters();
            } else {
              SaveRegisters();
              LockRegisters();
            }
          }
          if (pMessage->Destination() == m_pGroupBoxTabAsm) {
            if (pMessage->Source() == m_pAssemblyRefresh) {
              UpdateDisassembly();
              break;
            }
            if (pMessage->Source() == m_pAssemblySearchPrev) {
              AsmSearch(SearchFrom::PositionExcluded, SearchDir::Backward);
            }
            if (pMessage->Source() == m_pAssemblySearchNext) {
              AsmSearch(SearchFrom::PositionExcluded, SearchDir::Forward);
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
              PrepareMemBankConfig();
              std::string adress = m_pMemPokeAdress->GetWindowText();
              std::string value  = m_pMemPokeValue->GetWindowText();
              unsigned int pokeAdress = strtol(adress.c_str(), nullptr, 16);
              int pokeValue           = strtol(value.c_str(),  nullptr, 16);
              if(!adress.empty() && !value.empty() && pokeAdress < 65536 && pokeValue >= -128 && pokeValue <= 255) {
                std::cout << "Poking " << pokeAdress << " with " << pokeValue << std::endl;
                WriteMem(pokeAdress, pokeValue);
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
              m_currentlyFiltered.clear();
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
            if (pMessage->Source() == m_pMemButtonSaveFilter) {
              m_savedFilter = m_currentlyDisplayed;
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pMemButtonApplyFilter) {
              m_currentlyFiltered = m_savedFilter;
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
                WatchpointType type = WatchpointType(m_pMemWatchPointType->GetSelectedIndex() + 1);
                watchpoints.emplace_back(static_cast<word>(std::stol(m_pMemNewWatchPoint->GetWindowText(), nullptr, 16)), type);
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
        if (pMessage->Destination() == m_pGroupBoxTabAsm) {
          if (pMessage->Source() == m_pAssemblySearch) {
            AsmSearch(SearchFrom::PositionIncluded, SearchDir::Forward);
          }
        }
        if (pMessage->Destination() == m_pGroupBoxTabMemory) {
          if (pMessage->Source() == m_pMemBytesPerLine) {
            m_MemBytesPerLine = 1 << m_pMemBytesPerLine->GetSelectedIndex();
            // Note: Any saved filter doesn't make sense anymore but we keep it
            // just in case the user wants to come back to the previous
            // BytesPerLine.
            // It would be nice to have a mechanism to warn the user if the
            // filter is not aligned with the BytesPerLine setting.
            UpdateTextMemory();
          }
          if (pMessage->Source() == m_pMemFormat) {
            m_MemFormat.clear();
            for (auto f : stringutils::split(m_pMemFormat->GetWindowText(), '&')) {
              f = stringutils::lower(stringutils::trim(f, ' '));
              if (f == "hex") {
                m_MemFormat.push_back(Format::Hex);
              } else if (f == "char") {
                m_MemFormat.push_back(Format::Char);
              } else if (f == "u8") {
                m_MemFormat.push_back(Format::U8);
              } else if (f == "u16") {
                m_MemFormat.push_back(Format::U16);
              } else if (f == "u32") {
                m_MemFormat.push_back(Format::U32);
              } else if (f == "i8") {
                m_MemFormat.push_back(Format::I8);
              } else if (f == "i16") {
                m_MemFormat.push_back(Format::I16);
              } else if (f == "i32") {
                m_MemFormat.push_back(Format::I32);
              } else {
                LOG_WARNING("Unknown format token '" << f << "', skipping.");
                continue;
              }
            }
            if (m_MemFormat.empty()) {
              LOG_ERROR("No valid format provided in '" << m_pMemFormat->GetWindowText() << "', defaulting to 'Hex'.");
              m_MemFormat.push_back(Format::Hex);
            }
            UpdateTextMemory();
          }
        }
#if __GNUC__ >= 7
        [[gnu::fallthrough]];
#endif
      case CMessage::CTRL_VALUECHANGING:
        if (pMessage->Destination() == m_pGroupBoxTabZ80) {
          // TODO: Handle scrollbars
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
