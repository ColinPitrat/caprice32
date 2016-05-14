// 'Memory tool' window for Caprice32

#include "CapriceMemoryTool.h"
#include "cap32.h"
#include "z80.h"
#include <sstream>
#include <iomanip>

extern byte *pbRAM;
extern t_CPC CPC;

namespace wGui {

CapriceMemoryTool::CapriceMemoryTool(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
	CFrame(WindowRect, pParent, pFontEngine, "Memory Tool", false)
{
    SetModal(true);
    m_pMonoFontEngine = CApplication::Instance()->GetFontEngine(std::string(CPC.resources_path) + "/vera_mono.ttf", 8);

    // Make this window listen to incoming CTRL_VALUECHANGING messages for dropdown list update
    CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGE);
    CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGING);

    m_pPokeAdressLabel = new CLabel(        CPoint(15, 18),             this, "Adress: ");
    m_pPokeAdress      = new CEditBox(CRect(CPoint(55, 13),  30, 20),   this, nullptr);
    m_pPokeValueLabel  = new CLabel(        CPoint(95, 18),             this, "Value: ");
    m_pPokeValue       = new CEditBox(CRect(CPoint(130, 13), 30, 20),   this, nullptr);
    m_pButtonPoke      = new CButton( CRect(CPoint(175, 13), 30, 20),   this, "Poke", true);

    m_pAdressLabel     = new CLabel(        CPoint(15, 50),             this, "Adress: ");
    m_pAdressValue     = new CEditBox(CRect(CPoint(55, 45), 30, 20),    this, nullptr);
    m_pButtonDisplay   = new CButton( CRect(CPoint(95, 45), 40, 20),    this, "Display", true);
    m_pFilterLabel     = new CLabel(        CPoint(15, 80),             this, "Byte: ");
    m_pFilterValue     = new CEditBox(CRect(CPoint(55, 75), 30, 20),    this, nullptr);
    m_pButtonFilter    = new CButton( CRect(CPoint(95, 75), 40, 20),    this, "Filter", true);
    m_pButtonCopy      = new CButton( CRect(CPoint(240, 75), 75, 20),   this, "Dump to stdout", true);

    m_pBytesPerLineLbl = new CLabel(       CPoint(240, 35),             this, "Bytes per line:");
    m_pBytesPerLine  = new CDropDown( CRect(CPoint(240, 45), 50, 20),   this, false, 14);
    m_pBytesPerLine->AddItem(SListItem("1"));
    m_pBytesPerLine->AddItem(SListItem("4"));
    m_pBytesPerLine->AddItem(SListItem("8"));
    m_pBytesPerLine->AddItem(SListItem("16"));
    m_pBytesPerLine->AddItem(SListItem("32"));
    m_pBytesPerLine->AddItem(SListItem("64"));
    m_pBytesPerLine->SetListboxHeight(4);
    m_bytesPerLine = 16;
    m_pBytesPerLine->SelectItem(3);

    // The list box is way to slow to handle so much elements
    //m_pListMemContent  = new CListBox(CRect(CPoint(25, 75), 275, 100), this, true);
    m_pTextMemContent  = new CTextBox(CRect(CPoint(15, 105), 300, 102), this, m_pMonoFontEngine);
    m_pButtonClose     = new CButton( CRect(CPoint(15, 220), 300, 20),  this, "Close", true);

    m_pPokeAdress->SetContentType(CEditBox::HEXNUMBER);
    m_pPokeValue->SetContentType(CEditBox::HEXNUMBER);
    m_pAdressValue->SetContentType(CEditBox::HEXNUMBER);
    m_pFilterValue->SetContentType(CEditBox::HEXNUMBER);
    m_pTextMemContent->SetReadOnly(true);

    m_filterValue = -1;
    m_displayValue = -1;

    UpdateTextMemory();
}

CapriceMemoryTool::~CapriceMemoryTool(void) {
}

bool CapriceMemoryTool::HandleMessage(CMessage* pMessage)
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
            if (pMessage->Source() == m_pButtonPoke) {
              std::string adress = m_pPokeAdress->GetWindowText();
              std::string value  = m_pPokeValue->GetWindowText();
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
            if (pMessage->Source() == m_pButtonDisplay) {
              std::string display = m_pAdressValue->GetWindowText();
              if(display.empty()) {
                m_displayValue = -1;
              } else {
                m_displayValue = strtol(display.c_str(), nullptr, 16);
              }
              m_filterValue = -1;
              std::cout << "Displaying adress " << m_displayValue << " in memory." << std::endl;
              UpdateTextMemory();
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pButtonFilter) {
              m_displayValue = -1;
              std::string filter = m_pFilterValue->GetWindowText();
              if(filter.empty()) {
                m_filterValue = -1;
              } else {
                m_filterValue = strtol(filter.c_str(), nullptr, 16);
              }
              std::cout << "Filtering value " << m_filterValue << " in memory." << std::endl;
              UpdateTextMemory();
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pButtonCopy) {
              std::cout << m_pTextMemContent->GetWindowText() << std::endl;
            /* Requires SDL2
              if(SDL_SetClipboardText(m_pTextMemContent->GetWindowText().c_str()) < 0) {
                fprintf(stderr, "Error while copying data to clipboard: %s\n", SDL_GetError());
              }
              */
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pButtonClose) {
              CloseFrame();
              bHandled = true;
              break;
            }
          }
        }
        break;

/*
      case CMessage::KEYBOARD_KEYDOWN:
        if(m_bVisible)
        {
          CKeyboardMessage* pKeyboardMessage = dynamic_cast<CKeyboardMessage*>(pMessage);
          if (pKeyboardMessage && pKeyboardMessage->Key == SDLK_TAB) {
            if(CApplication::Instance()->GetKeyFocus() == m_pPokeAdress) {
              CApplication::Instance()->SetKeyFocus(m_pPokeValue);
              bHandled = true;
              break;
            }
            if(CApplication::Instance()->GetKeyFocus() == m_pPokeValue) {
              CApplication::Instance()->SetKeyFocus(m_pAdressValue);
              bHandled = true;
              break;
            }
            if(CApplication::Instance()->GetKeyFocus() == m_pAdressValue) {
              CApplication::Instance()->SetKeyFocus(m_pFilterValue);
              bHandled = true;
              break;
            }
            if(CApplication::Instance()->GetKeyFocus() == m_pFilterValue) {
              CApplication::Instance()->SetKeyFocus(m_pPokeAdress);
              bHandled = true;
              break;
            }
          }
        }
        break;
        */

      case CMessage::CTRL_VALUECHANGE:
        if (pMessage->Destination() == m_pBytesPerLine) {
          switch (m_pBytesPerLine->GetSelectedIndex()) {
            case 0:
              m_bytesPerLine = 1;
              break;
            case 1:
              m_bytesPerLine = 4;
              break;
            case 2:
              m_bytesPerLine = 8;
              break;
            case 3:
              m_bytesPerLine = 16;
              break;
            case 4:
              m_bytesPerLine = 32;
              break;
            case 5:
              m_bytesPerLine = 64;
              break;
          }
          UpdateTextMemory();
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

void CapriceMemoryTool::UpdateTextMemory() {
  std::ostringstream memText;
  for(unsigned int i = 0; i < 65536/m_bytesPerLine; i++) {
    std::ostringstream memLine;
    memLine << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << i*m_bytesPerLine << " : ";
    //memText << std::setfill('0') << std::setw(4) << std::hex << i*m_bytesPerLine << " : ";
    bool displayLine = false;
    bool filterAdress = (m_displayValue >= 0 && m_displayValue <= 65535);
    bool filterValue = (m_filterValue >= 0 && m_filterValue <= 255);
    for(unsigned int j = 0; j < m_bytesPerLine; j++) {
      memLine << std::setw(2) << static_cast<unsigned int>(pbRAM[i*m_bytesPerLine+j]) << " ";
      if(!filterAdress && !filterValue) {
        displayLine = true;
      }
      if(filterValue && static_cast<int>(pbRAM[i*m_bytesPerLine+j]) == m_filterValue) {
        displayLine = true;
      }
      if(filterAdress && (i*m_bytesPerLine+j == static_cast<unsigned int>(m_displayValue))) {
        displayLine = true;
      }
      //memText << std::setw(2) << static_cast<unsigned int>(pbRAM[i*m_bytesPerLine+j]) << " ";
    }
    if(displayLine) {
      memText << memLine.str() << "\n";
    }
    //m_pListMemContent->AddItem(SListItem(memLine.str()));
  }
  m_pTextMemContent->SetWindowText(memText.str().substr(0, memText.str().size()-1));
}

} // namespace wGui
