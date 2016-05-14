// 'About' box for Caprice32
// Inherited from CMessageBox

#include "CapriceAbout.h"
#include "cap32.h"

// CPC emulation properties, defined in cap32.h:
extern t_CPC CPC;

namespace wGui {

CapriceAbout::CapriceAbout(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
	CMessageBox(WindowRect, pParent, pFontEngine, "About Caprice32", "", CMessageBox::BUTTON_OK)
{
  SetModal(true);
	// Override here: specify position of label ourselves:
	m_pMessageLabel = new CLabel(CPoint(5, 70), this, "Version 4.2.0");
	m_pMessageLabel = new CLabel(CPoint(5, 90), this, "F1 - Menu / Pause");
	m_pMessageLabel = new CLabel(CPoint(5, 100), this, "F2 - Fullscreen");
	m_pMessageLabel = new CLabel(CPoint(5, 110), this, "F3 - ");
	//m_pMessageLabel = new CLabel(CPoint(5, 120), this, "F3 - Load/Save disk/tape/snapshot/screenshot");
	m_pMessageLabel = new CLabel(CPoint(5, 120), this, "F4 - Tape play");
	m_pMessageLabel = new CLabel(CPoint(5, 130), this, "F5 - Reset");
	m_pMessageLabel = new CLabel(CPoint(5, 140), this, "F6 - Multiface II Stop");
	m_pMessageLabel = new CLabel(CPoint(5, 150), this, "F7 - Joystick emulation");
	m_pMessageLabel = new CLabel(CPoint(5, 160), this, "F8 - Display FPS");
	m_pMessageLabel = new CLabel(CPoint(5, 170), this, "F9 - Limit speed");
	m_pMessageLabel = new CLabel(CPoint(5, 180), this, "F10 - Quit");
	try	{
	    m_pPicture = new CPicture(CRect(CPoint(5, 5), 162, 62), this, std::string(CPC.resources_path) + "/cap32logo.bmp", true);
	} catch (Wg_Ex_App e) {
		// we don't want to stop the program if we can't load the picture, so just print the error and keep going
		wUtil::Trace(e.std_what());
	}
}

}

