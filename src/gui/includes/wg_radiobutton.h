// wg_radiobutton.h
//
// CRadioButton interface
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
#ifndef _WG_RADIOBUTTON_H_
#define _WG_RADIOBUTTON_H_

#include "wg_window.h"
#include "wg_painter.h"
#include "wg_resources.h"

namespace wGui
{

//! A radiobutton control

//! Radiobuttons should share a common parent (preferably a CGroupBox :-)) so they work as expected (only one can be 
//! checked at a time)
//! The radiobutton will generate CTRL_xCLICK messages when clicked with the mouse (where x is the button L,M,R)
//! It will also generate a CTRL_VALUECHANGE message whenever the radiobutton is toggled (todo adopt for radiobutton)
//! Radiobuttons do not display their own labels

class CRadioButton : public CWindow
{
public:
	//! Constructs a new radiobutton
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	CRadioButton(const CPoint& p, int size, CWindow* pParent);

	//! Standard destructor
	virtual ~CRadioButton();

	//! The radiobutton state(s)
	enum EState {
		UNCHECKED,  //!< The radiobutton is unchecked
		CHECKED,  //!< The radiobutton is checked
		DISABLED  //!< The radiobutton is disabled
	};

	//! Gets the current state of the radiobutton
	//! \return The current radiobutton state
	EState GetState() const { return m_eRadioButtonState; }

	//! Set the radiobutton state
	//! \param eState The radiobutton state
	void SetState(EState eState);

	//! Check this radiobutton and uncheck all its siblings
  void Select();

	// CWindow overrides
	//! Draws the radiobutton
	virtual void Draw() const override;

	//! This is called whenever the radiobutton is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the radiobutton
	virtual bool OnMouseButtonDown(CPoint Point, unsigned int Button) override;

	//! This is called whenever the a mouse button is released in the radiobutton
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the radiobutton
	virtual bool OnMouseButtonUp(CPoint Point, unsigned int Button) override;


	// CMessageClient overrides
	//! CRadioButtons handle MOUSE_BUTTONDOWN, MOUSE_BUTTONUP, and it's own CTRL_SINGLELCLICK messages
	//! \param pMessage A pointer to the message
	virtual bool HandleMessage(CMessage* pMessage) override;


protected:
	EState m_eRadioButtonState;  //!< The radiobutton's state
	unsigned int m_MouseButton;  //!< The last mouse button to be pushed over the control, it's used internally
    CBitmapResourceHandle m_hBitmapRadioButton; // radiobutton (black dot if selected) defined as a bitmap resource.


private:
  CRadioButton(const CRadioButton&) = delete;
	CRadioButton& operator=(const CRadioButton&) = delete;
};

}

#endif  // _WG_RADIOBUTTON_H_

