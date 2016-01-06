// Caprice32 ROM slot selection window
// Inherited from CFrame

#include <dirent.h>
#include "CapriceRomSlots.h"
#include "cap32.h"
#include "fileutils.h"

// CPC emulation properties, defined in cap32.h:
extern t_CPC CPC;

namespace wGui {

CapriceRomSlots::CapriceRomSlots(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine, std::string sTitle, int selectedRomSlot, CButton* pSelectedRomButton) :
	CFrame(WindowRect, pParent, pFontEngine, sTitle, false)
{
    romSlot = selectedRomSlot;
    m_pSelectedRomButton =  pSelectedRomButton; // the button that was clicked to open this dialog
    SetWindowText("ROM slot " + stdex::itoa(romSlot));
    m_pButtonInsert = new CButton(CRect(CPoint( 40, m_ClientRect.Height() - 22), 50, 15), this, "Insert");
    m_pButtonClear  = new CButton(CRect(CPoint(100, m_ClientRect.Height() - 22), 50, 15), this, "Clear");
    m_pButtonCancel = new CButton(CRect(CPoint(160, m_ClientRect.Height() - 22), 50, 15), this, "Cancel");

	m_pListBoxRoms = new CListBox(CRect(CPoint(10, 10), m_ClientRect.Width() - 25, 140), this, true);

    std::vector<std::string> romFiles = getAvailableRoms();
    for (unsigned int i = 0; i < romFiles.size(); i ++) {
        m_pListBoxRoms->AddItem(SListItem(romFiles.at(i)));
        if (romFiles.at(i) == m_pSelectedRomButton->GetWindowText()) { // It's all based on the filename of the ROM,
                                                                    // maybe find a better way.
            m_pListBoxRoms->SetSelection(i, true);
            m_pListBoxRoms->SetFocus(i);
        }
    }
}

bool CapriceRomSlots::HandleMessage(CMessage* pMessage)
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
				if (pMessage->Source() == m_pButtonCancel) {
					CloseFrame();
					bHandled = true;
                    break;
                }
				if (pMessage->Source() == m_pButtonInsert) {
                    // Put selected ROM filename on button: (if there is a selection)
                    int selectedRomIndex = m_pListBoxRoms->getFirstSelectedIndex();
                    if (selectedRomIndex >= 0) {
                        m_pSelectedRomButton->SetWindowText((m_pListBoxRoms->GetItem(selectedRomIndex)).sItemText);
					    CloseFrame();
                        bHandled = true;
                    }
                    break;
                }
				if (pMessage->Source() == m_pButtonClear) {
                    // clear selected rom
                    m_pSelectedRomButton->SetWindowText("...");
					CloseFrame();
					bHandled = true;
                    break;
                }
           		bHandled = CFrame::HandleMessage(pMessage);
			}
			break;
		}

		default :
			bHandled = CFrame::HandleMessage(pMessage);
			break;
		}
	}
	return bHandled;
}

// Reads the existing ROM filenames (in roms subdirectory defined in cap32.cfg)
std::vector<std::string> CapriceRomSlots::getAvailableRoms() {
   // CPC.rom_path contains the ROM path, e.g. ./rom:
   return listDirectory(CPC.rom_path);
}


}
