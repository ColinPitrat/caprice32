// 'About' box for Caprice32
// Inherited from CMessageBox

#include "CapriceAbout.h"
#include "cap32.h"
#include <string>

// CPC emulation properties, defined in cap32.h:
extern t_CPC CPC;

namespace wGui {

CapriceAbout::CapriceAbout(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
	CMessageBox(WindowRect, pParent, pFontEngine, "About Caprice32", "", CMessageBox::BUTTON_OK)
{
    int y=80;
    SetModal(true);
	// Override here: specify position of label ourselves:
#ifdef HASH
	std::string commit_hash = std::string(HASH);
#else
	std::string commit_hash;
#endif
	m_pMessageLabel = new CLabel(CPoint(5, 70), this, VERSION_STRING + (commit_hash.empty()?"":"-"+commit_hash.substr(0, 16)));
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "F1 - Menu / Pause");
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "F2 - Fullscreen");
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "F3 - Save screenshot");
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "F4 - Tape play");
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "F5 - Reset");
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "F6 - Multiface II Stop");
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "F7 - Joystick emulation");
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "F8 - Display FPS");
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "F9 - Limit speed");
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "F10 - Quit");
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "Shift + F1 - Virtual keyboard");
	m_pMessageLabel = new CLabel(CPoint(5, y+=10), this, "Shift + F3 - save machine snapshot");
	try	{
	    m_pPicture = new CPicture(CRect(CPoint(5, 5), 162, 62), this, CPC.resources_path + "/cap32logo.bmp", true);
	} catch (Wg_Ex_App &e) {
		// we don't want to stop the program if we can't load the picture, so just print the error and keep going
		wUtil::Trace(e.std_what());
	}
}

}

