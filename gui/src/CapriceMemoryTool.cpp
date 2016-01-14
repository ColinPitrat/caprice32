// 'Memory tool' window for Caprice32

#include "CapriceMemoryTool.h"
#include "cap32.h"
#include "z80.h"

extern byte *pbRAM;

namespace wGui {

CapriceMemoryTool::CapriceMemoryTool(const CRect& WindowRect, CView* pParent, CFontEngine* pFontEngine) :
	CFrame(WindowRect, pParent, pFontEngine, "Memory Tool", false)
{
    // Make this window listen to incoming KEYBOARD_KEYDOWN messages to capture TAB pressed and change focused field
    CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);

    m_pPokeAdressLabel = new CLabel(        CPoint(25, 18),           this, "Adress: ");
    m_pPokeAdress      = new CEditBox(CRect(CPoint(65, 13),  30, 20), this, NULL);
    m_pPokeValueLabel  = new CLabel(        CPoint(135, 18),          this, "Value: ");
    m_pPokeValue       = new CEditBox(CRect(CPoint(175, 13), 30, 20), this, NULL);
    m_pButtonPoke      = new CButton( CRect(CPoint(245, 13), 30, 20), this, "Poke");
    m_pButtonClose     = new CButton( CRect(CPoint(25, 45), 250, 20), this, "Close");
    m_pPokeAdress->SetContentType(CEditBox::HEXNUMBER);
    m_pPokeValue->SetContentType(CEditBox::HEXNUMBER);
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

} // namespace wGui
