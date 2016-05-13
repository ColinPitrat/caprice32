// wg_button.h
//
// CButton interface
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


#ifndef _WG_BUTTON_H_
#define _WG_BUTTON_H_

#include "wg_window.h"
#include "wg_painter.h"
#include "wg_renderedstring.h"
#include "wg_picture.h"
#include <memory>


namespace wGui
{

//! A simple pushbutton class

//! The button will generate CTRL_xCLICK messages when clicked with the mouse (where x is the button L,M,R)

class CButton : public CWindow
{
public:
	//! Constructs a new button
  //! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param sText The text on the button
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CButton(const CRect& WindowRect, CWindow* pParent, std::string sText, CFontEngine* pFontEngine = nullptr);

	//! Standard destructor
	virtual ~CButton(void);

	//! The button state
	enum EState {
		UP,  //!< The button is up
		DOWN,  //!< The button is down
		DISABLED  //!< The button is disabled
	};

	//! Gets the current state of the button
	//! \return The current button state
	EState GetButtonState(void) const { return m_eButtonState; }

	//! Set the button state
	//! \param eState The button state
	void SetButtonState(EState eState);


	// CWindow overrides
	//! Draws the button and renders the button label
	virtual void Draw(void) const;

	//! Set the WindowText of the button
	//! \param sWindowText The text to assign to the window
	virtual void SetWindowText(const std::string& sWindowText);

	//! This is called whenever the button is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the button
	virtual bool OnMouseButtonDown(CPoint Point, unsigned int Button);

	//! This is called whenever the a mouse button is released in the button
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the button
	virtual bool OnMouseButtonUp(CPoint Point, unsigned int Button);


	// CMessageClient overrides
	//! CButtons handle MOUSE_BUTTONDOWN and MOUSE_BUTTONUP messages
	//! \param pMessage A pointer to the message
	virtual bool HandleMessage(CMessage* pMessage);


protected:
	CFontEngine* m_pFontEngine;  //!< A pointer to the font engine to use to render the text
	std::unique_ptr<CRenderedString> m_pRenderedString;  //!< An autopointer to the rendered version of the string
	EState m_eButtonState;  //!< The button's state
	unsigned int m_MouseButton;  //!< The last mouse button to be pushed over the control, it's used internally

private:
	void operator=(CButton) { }  //!< The assignment operator is not allowed for CWindow derived objects
};


//! Picture Buttons are pushbuttons that display a bitmap in place of a text label

class CPictureButton : public CButton
{
public:
	//! Constructs a new picture button
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param sPictureFile The file to use as the button's picture
	CPictureButton(const CRect& WindowRect, CWindow* pParent, std::string sPictureFile);

	//! Constructs a new picture button
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param hBitmap A handle for the bitmap resource
	CPictureButton(const CRect& WindowRect, CWindow* pParent, const CBitmapResourceHandle& hBitmap);

	//! Standard destructor
	virtual ~CPictureButton(void);


	//! Change the picture the button displays
	//! \param sPictureFile The file to use as the button's picture
	void SetPicture(std::string sPictureFile);

	//! Change the picture the button displays
	//! \param hBitmap A handle for the bitmap resource
	void SetPicture(const CBitmapResourceHandle& hBitmap);


	// CWindow overrides
	//! Draws the button and renders the button label
	virtual void Draw(void) const;


private:
	std::unique_ptr<CBitmapResourceHandle> m_phBitmap;  //!< An auto pointer to a handle for the bitmap resource
	void operator=(CPictureButton) { }  //!< The assignment operator is not allowed for CWindow derived objects
};

}

#endif  // _WG_BUTTON_H_


