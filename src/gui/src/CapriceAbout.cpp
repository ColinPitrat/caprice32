// 'About' box for Caprice32
// Inherited from CMessageBox

#include "CapriceAbout.h"
#include "cap32.h"
#include <string>
#include "log.h"
#include "wg_error.h"

// CPC emulation properties, defined in cap32.h:
extern t_CPC CPC;

namespace wGui {

static const std::string shortcuts = R"(F1 - Menu / Pause
F2 - Fullscreen
F3 - Save screenshot
F4 - Tape play
F5 - Reset
F6 - Multiface II Stop
F7 - Joystick emulation
F8 - Display FPS
F9 - Limit speed
F10 - Quit
Shift + F1 - Virtual keyboard
Shift + F2 - Developers' tools
Shift + F3 - Save machine snapshot
Shift + F4 - Load last snapshot
Shift + F5 - Next disk (drive A)
Shift + F7 - Activate phazer)";

CapriceAbout::CapriceAbout(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
	CMessageBox(WindowRect, pParent, pFontEngine, "About Caprice32", "", CMessageBox::BUTTON_OK)
{
    SetModal(true);
	// Override here: specify position of label ourselves:
#ifdef HASH
	std::string commit_hash = std::string(HASH);
#else
	std::string commit_hash;
#endif
	m_pMessageLabel = new CLabel(CPoint(5, 70), this, VERSION_STRING + (commit_hash.empty()?"":"-"+commit_hash.substr(0, 16)));
  m_pTextBox = new CTextBox(CRect(10, 90, 210, 200), this);
  m_pTextBox->SetWindowText(shortcuts);
  m_pTextBox->SetReadOnly(true);
	try	{
	    m_pPicture = new CPicture(CRect(CPoint(18, 5), 162, 62), this, CPC.resources_path + "/cap32logo.bmp", true);
	} catch (Wg_Ex_App &e) {
		// we don't want to stop the program if we can't load the picture, so just print the error and keep going
    LOG_ERROR("CapriceAbout::CapriceAbout: Couldn't load cap32logo.bmp: " << e.std_what());
	}
}

}

