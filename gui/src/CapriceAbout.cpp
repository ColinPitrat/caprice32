// 'About' box for Caprice32
// Inherited from CMessageBox

#include "CapriceAbout.h"

namespace wGui {

CapriceAbout::CapriceAbout(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
	CMessageBox(WindowRect, pParent, pFontEngine, "About Caprice32", "", CMessageBox::BUTTON_OK)
{
	// Override here: specify position of label ourselves:
	m_pMessageLabel = new CLabel(CPoint(5, 70), this, "Version 4.2.0");
	try	{
	    m_pPicture = new CPicture(CRect(CPoint(5, 5), 158, 58), this, "resource/cap32logo.bmp", true);
	} catch (Wg_Ex_App e) {
		// we don't want to stop the program if we can't load the picture, so just print the error and keep going
		wUtil::Trace(e.std_what());
	}
}

}

