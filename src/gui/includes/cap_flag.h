// cap_flag.h
//
// A widget combining a label, a field and a tooltip for displaying the content
// of a z80 flag.

#ifndef _CAP_FLAG_H_
#define _CAP_FLAG_H_

#include <string>
#include "wg_window.h"
#include "wg_painter.h"
#include "wg_label.h"
#include "wg_editbox.h"
#include "wg_tooltip.h"

namespace wGui
{

//! A composite widget for flag display, combining name, value and tooltips.

class CFlag : public CWindow
{
public:
	//! Construct a new Flag control
	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param name The name of the flag (e.g. S, Z, ...). 
	//! \param description The long name of the flag (e.g. Sign flag, Zero flag, ...). 
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CFlag(const CRect& WindowRect, CWindow* pParent, const std::string& name, const std::string& description, CFontEngine* pFontEngine = nullptr);

	//! Standard destructor
	~CFlag() override;

	//! Set the value of the flag
	//! \param c The value to assign to the control
	void SetValue(const std::string& c);

	//! Get the value of the flag
	//! \return The value of the flag.
	bool GetValue() const;

	//! Set the Read-only state of the control
	//! \param bReadOnly  If set to true, the control will not take any input
	void SetReadOnly(bool bReadOnly);

protected:

	CFontEngine* m_pFontEngine; //!< A pointer to the font engine to use to render the text

	CLabel* m_pLabel;
	CEditBox* m_pValue;
	CToolTip* m_pTooltip;


private:
	CFlag(const CFlag&) = delete;
	CFlag& operator=(const CFlag&) = delete;
};

}

#endif // _CAP_FLAG_H_
