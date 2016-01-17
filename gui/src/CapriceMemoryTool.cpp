// 'Memory tool' window for Caprice32

#include "CapriceMemoryTool.h"
#include "cap32.h"
#include "z80.h"
#include <sstream>
#include <iomanip>

extern byte *pbRAM;

namespace wGui {

CapriceMemoryTool::CapriceMemoryTool(const CRect& WindowRect, CView* pParent, CFontEngine* pFontEngine) :
	CFrame(WindowRect, pParent, pFontEngine, "Memory Tool", false)
{
    // Make this window listen to incoming KEYBOARD_KEYDOWN messages to capture TAB pressed and change focused field
    CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);

    m_pPokeAdressLabel = new CLabel(        CPoint(25, 18),            this, "Adress: ");
    m_pPokeAdress      = new CEditBox(CRect(CPoint(65, 13),  30, 20),  this, NULL);
    m_pPokeValueLabel  = new CLabel(        CPoint(135, 18),           this, "Value: ");
    m_pPokeValue       = new CEditBox(CRect(CPoint(175, 13), 30, 20),  this, NULL);
    m_pButtonPoke      = new CButton( CRect(CPoint(245, 13), 30, 20),  this, "Poke");

    m_pFilterLabel     = new CLabel(        CPoint(25, 50),            this, "Byte: ");
    m_pFilterValue     = new CEditBox(CRect(CPoint(65, 45),  30, 20),  this, NULL);
    m_pButtonFilter    = new CButton( CRect(CPoint(135, 45), 30, 20),  this, "Filter");
    m_pButtonCopy      = new CButton( CRect(CPoint(205, 45), 70, 20),  this, "Dump to stdout");

    //m_pListMemContent  = new CListBox(CRect(CPoint(25, 75), 275, 100), this, true);
    m_pTextMemContent  = new CTextBox(CRect(CPoint(15, 75), 270, 100), this);
    m_pButtonClose     = new CButton( CRect(CPoint(25, 190), 250, 20),  this, "Close");

    m_pPokeAdress->SetContentType(CEditBox::HEXNUMBER);
    m_pPokeValue->SetContentType(CEditBox::HEXNUMBER);
    m_pFilterValue->SetContentType(CEditBox::HEXNUMBER);
    m_pTextMemContent->SetReadOnly(true);

    m_filterValue = 0;

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
              unsigned int pokeAdress = strtol(m_pPokeAdress->GetWindowText().c_str(), NULL, 16);
              int pokeValue           = strtol(m_pPokeValue->GetWindowText().c_str(),  NULL, 16);
              std::cout << "Poking " << pokeAdress << " with " << pokeValue << std::endl;
              pbRAM[pokeAdress] = pokeValue;
              UpdateTextMemory();
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pButtonFilter) {
              m_filterValue = strtol(m_pFilterValue->GetWindowText().c_str(), NULL, 16);
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
            // handle further buttons and events...
            bHandled = CFrame::HandleMessage(pMessage);
          }
        }
        break;

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
              CApplication::Instance()->SetKeyFocus(m_pPokeAdress);
              bHandled = true;
              break;
            }
          }
        }
        break;

      default :
        bHandled = CFrame::HandleMessage(pMessage);
        break;
    }
  }
	return bHandled;
}

void CapriceMemoryTool::UpdateTextMemory() {
  //std::cout << "Start creating listbox" << std::endl;
  std::ostringstream memText;
  for(unsigned int i = 0; i < 65536/16; i++) {
    std::ostringstream memLine;
    memLine << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << i*16 << " : ";
    //memText << std::setfill('0') << std::setw(4) << std::hex << i*16 << " : ";
    bool displayLine = false;
    for(unsigned int j = 0; j < 16; j++) {
      memLine << std::setw(2) << static_cast<unsigned int>(pbRAM[i*16+j]) << " ";
      //if(pbRAM[i*16+j] != 0) {
        if(m_filterValue == 0 || m_filterValue > 255 || static_cast<unsigned int>(pbRAM[i*16+j]) == m_filterValue) {
          displayLine = true;
        }
      //}
      //memText << std::setw(2) << static_cast<unsigned int>(pbRAM[i*16+j]) << " ";
    }
    if(displayLine) {
      memText << memLine.str() << "\n";
    }
    //std::cout << memLine.str() << std::endl;
    //m_pListMemContent->AddItem(SListItem(memLine.str()));
  }
  //std::cout << "End creating listbox" << std::endl;
  m_pTextMemContent->SetWindowText(memText.str().substr(0, memText.str().size()-1));
}

} // namespace wGui
