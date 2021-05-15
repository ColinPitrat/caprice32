// wg_register.h
//
// A widget combining a label and 3 fields for hexadecimal, decimal and char
// content of a register or memory location.


#ifndef _WG_REGISTER_H_
#define _WG_REGISTER_H_

#include <string>
#include "wg_window.h"
#include "wg_painter.h"
#include "wg_label.h"
#include "wg_editbox.h"

namespace wGui
{

//! A composite widget for register/memory location display, combining name, hexadecimal, decimal and char values.

class CRegister : public CWindow
{
public:
	//! Construct a new Register control
	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param name The name of the register (will constitute the label). 
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CRegister(const CRect& WindowRect, CWindow* pParent, std::string name, CFontEngine* pFontEngine = nullptr);

	//! Standard destructor
	~CRegister() override;

	//! Set the value of the register
	//! \param c The value to assign to the control
	void SetValue(const unsigned int c);

protected:

	CFontEngine* m_pFontEngine; //!< A pointer to the font engine to use to render the text
	unsigned int m_Value;      //!< The value of the register/memory location

	CLabel* m_pLabel;
	CEditBox* m_pHexValue;
	CEditBox* m_pDecValue;
	CEditBox* m_pCharValue;


private:
  CRegister(const CRegister&) = delete;
	CRegister& operator=(const CRegister&) = delete;
};

}

#endif // _WG_EDITBOX_H_
