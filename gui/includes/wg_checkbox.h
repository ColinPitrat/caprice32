// wg_checkbox.h
//
// CCheckBox interface
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


#ifndef _WG_CHECKBOX_H_
#define _WG_CHECKBOX_H_

#include "wg_window.h"
#include "wg_painter.h"
#include "wg_resources.h"


namespace wGui
{

//! A checkbox control

//! The checkbox will generate CTRL_xCLICK messages when clicked with the mouse (where x is the button L,M,R)
//! It will also generate a CTRL_VALUECHANGE message whenever the checkbox is toggled
//! Checkboxes do not display their own labels

class CCheckBox : public CWindow
{
public:
	//! Constructs a new checkbox
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	CCheckBox(const CRect& WindowRect, CWindow* pParent);

	//! Standard destructor
	virtual ~CCheckBox(void);

	//! The checkbox state
	enum EState {
		UNCHECKED,  //!< The checkbox is unchecked
		CHECKED,  //!< The checkbox is checked
		DISABLED  //!< The checkbox is disabled
	};

	//! Gets the current state of the checkbox
	//! \return The current checkbox state
	EState GetCheckBoxState(void) const { return m_eCheckBoxState; }

	//! Set the checkbox state
	//! \param eState The checkbox state
	void SetCheckBoxState(EState eState);

	//! Toggle the checkbox state
  void ToggleCheckBoxState();

	// CWindow overrides
	//! Draws the checkbox
	virtual void Draw(void) const;

	//! This is called whenever the checkbox is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the checkbox
	virtual bool OnMouseButtonDown(CPoint Point, unsigned int Button);

	//! This is called whenever the a mouse button is released in the checkbox
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the checkbox
	virtual bool OnMouseButtonUp(CPoint Point, unsigned int Button);


	// CMessageClient overrides
	//! CCheckBoxes handle MOUSE_BUTTONDOWN, MOUSE_BUTTONUP, and it's own CTRL_SINGLELCLICK messages
	//! \param pMessage A pointer to the message
	virtual bool HandleMessage(CMessage* pMessage);


protected:
	EState m_eCheckBoxState;  //!< The checkbox's state
	unsigned int m_MouseButton;  //!< The last mouse button to be pushed over the control, it's used internally
    CBitmapResourceHandle m_hBitmapCheck; // CheckMark defined as a bitmap resource.


private:
	void operator=(CCheckBox) { }  //!< The assignment operator is not allowed for CWindow derived objects
};

}

#endif  // _WG_CHECKBOX_H_

