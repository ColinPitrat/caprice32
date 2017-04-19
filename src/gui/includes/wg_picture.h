// wg_picture.h
//
// CPicture interface
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


#ifndef _WG_PICTURE_H_
#define _WG_PICTURE_H_

#include "wg_window.h"
#include "wg_painter.h"
#include "wg_resource_handle.h"


namespace wGui
{

//! A picture control

//! Will take a picture file and display it
//! The CPicture control does not do any sort of resizing, but it will clip the picture to the client rect

class CPicture : public CWindow
{
public:
	//! Constructs a new picture control
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param sPictureFile The picture resource, must be a bitmap (.bmp)
	//! \param bDrawBorder If true, it will draw a border around the picture, defaults to false
	//! \param BorderColor The color to use when drawing the border
	CPicture(const CRect& WindowRect, CWindow* pParent, const std::string& sPictureFile,
		bool bDrawBorder = false, const CRGBColor& BorderColor = DEFAULT_LINE_COLOR);

	//! Constructs a new picture control
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param hBitmap A handle for the bitmap resource
	//! \param bDrawBorder If true, it will draw a border around the picture, defaults to false
	//! \param BorderColor The color to use when drawing the border
	CPicture(const CRect& WindowRect, CWindow* pParent, const CBitmapResourceHandle& hBitmap,
		bool bDrawBorder = false, const CRGBColor& BorderColor = DEFAULT_LINE_COLOR);

	//! Standard destructor
	virtual ~CPicture();


	// CWindow overrides
	//! Draws the button and renders the button label
	virtual void Draw() const override;

	//! Giving a control a new WindowRect will move and resize the control
	//! \param WindowRect A CRect that defines the outer limits of the control
	virtual void SetWindowRect(const CRect& WindowRect) override;


protected:
	bool m_bDrawBorder;  //!< The color to use when drawing the border
	CRGBColor m_BorderColor;  //!< The color to use when drawing the border
	CBitmapResourceHandle m_hBitmap;  //!< A handle for the bitmap resource


private:
  CPicture(const CPicture&) = delete;
	CPicture& operator=(const CPicture&) = delete;
};

}

#endif // _WG_PICTURE_H_

