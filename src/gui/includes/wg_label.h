// wg_label.h
//
// CLabel interface
//
//
// Copyright (c) 2002-2004 Rob Wiskow
// rob-dev@boxedchaos.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//


#ifndef _WG_LABEL_H_
#define _WG_LABEL_H_

#include "wg_window.h"
#include "wg_painter.h"
#include "wg_renderedstring.h"
#include "wg_application.h"
#include <string>
#include <memory>


namespace wGui
{

//! A static label that renders it's WindowText to the screen

class CLabel : public CWindow
{
public:
	//! Construct a new label
 	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param sText The label text, defaults to an empty string
	//! \param FontColor The color of the label text, defaults to the DEFAULT_LINE_COLOR
	//! \param pFontEngine A pointer to the font engine to use when drawing the control, defaults to 0
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CLabel(const CRect& WindowRect, CWindow* pParent, std::string sText = "",
		CRGBColor& FontColor = DEFAULT_TEXT_COLOR, CFontEngine* pFontEngine = nullptr);

    // judb constructor using a reference point (upper-left corner)
	CLabel(const CPoint& point, CWindow* pParent, std::string sText = "",
		CRGBColor& FontColor = DEFAULT_TEXT_COLOR, CFontEngine* pFontEngine = nullptr);


	//! Standard destructor
	~CLabel() override;

	//! Sets the color of the font used to render the label
	//! \param FontColor The color of the label text
	void SetFontColor(CRGBColor& FontColor) { m_FontColor = FontColor; }

	//! Gets the font color for the label
	//! \return The color of the text in the label
	CRGBColor GetFontColor() { return m_FontColor; }


	// CWindow overrides
	//! Renders the Window Text, and clips to the Window Rect
	void Draw() const override;

	//! Set the WindowText of the label
	//! \param sWindowText The text to assign to the window
	void SetWindowText(const std::string& sWindowText) override;


protected:
	CFontEngine* m_pFontEngine;  //!< A pointer to the font engine to use to render the text
	std::unique_ptr<CRenderedString> m_pRenderedString;  //!< An autopointer to the rendered version of the string
	CRGBColor m_FontColor;  //!< The font color
	bool dynamic_size;


private:
	CLabel(const CLabel&) = delete;
	CLabel& operator=(const CLabel&) = delete;
};

}


#endif // _WG_LABEL_H_

