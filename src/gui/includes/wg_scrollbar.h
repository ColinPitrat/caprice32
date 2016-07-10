// wg_scrollbar.h
//
// CScrollBar interface
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


#ifndef _WG_SCROLLBAR_H_
#define _WG_SCROLLBAR_H_

#include "wg_range_control.h"
#include "wg_painter.h"
#include "wg_button.h"


namespace wGui
{

//! A scroll bar

class CScrollBar : public CRangeControl<int>
{
public:
	//! The types of scrollbars possible
	enum EScrollBarType
	{
		VERTICAL,  //!< A standard vertical scrollbar
		HORIZONTAL  //!< A standard horizontal scrollbar
	};

	//! Constructs a scroll bar, initilizes the limits to 0, and 100 with the position at 0
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param ScrollBarType Indicates if this is to be a vertical or horizontal scrollbar
	CScrollBar(const CRect& WindowRect, CWindow* pParent, EScrollBarType ScrollBarType);

	//! Standard destructor
	virtual ~CScrollBar(void);

	//! Set the amount to jump by when the area below or above the thumb is clicked (this is 5 by default)
	//! \param iJumpAmount The amount to step by
	virtual void SetJumpAmount(int iJumpAmount) { m_iJumpAmount = iJumpAmount; }

	//! Get the amount that the scrollbar will jump by when the arrow buttons are clicked
	//! \return The amount the scrollbar jumps by when clicked above or below the thumb
	virtual int GetJumpAmount(void) const { return m_iJumpAmount; }


	// CRangeControl overrides
	//! Set the current value.
	//! \param iValue The new value for the control
	//! \param bRedraw indicates if the control should be redrawn (defaults to true)
	virtual void SetValue(int iValue, bool bRedraw = true) override;


	// CWindow overrides
	//! Draws the scroll bar
	virtual void Draw(void) const override;

	//! Giving a control a new WindowRect will move and resize the control
	//! \param WindowRect A CRect that defines the outer limits of the control
	virtual void SetWindowRect(const CRect& WindowRect) override;

	//! Move the window and any child windows
	//! \param MoveDistance The relative distance to move the window
	virtual void MoveWindow(const CPoint& MoveDistance) override;

	//! This is called whenever the scrollbar is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the scrollbar
	virtual bool OnMouseButtonDown(CPoint Point, unsigned int Button) override;

  virtual void SetIsFocusable(bool bFocusable) override;

	// CMessageClient overrides
	//! CScrollBars handle MOUSE_BUTTONDOWN and MOUSE_BUTTONUP messages
	//! \param pMessage A pointer to the message
	virtual bool HandleMessage(CMessage* pMessage) override;


protected:
	//! Repositions the thumb according to the value
	virtual void RepositionThumb(void);

	EScrollBarType m_ScrollBarType;  //!< The type of scroll bar
	int m_iJumpAmount;  //!< The amount to jump when the area below or above the thumb is clicked
	CPictureButton* m_pBtnUpLeft;  //!< A pointer to the Up or Left button
	CPictureButton* m_pBtnDownRight;  //!< A pointer to the Down or Right button
	CRect m_ThumbRect;  //!< The thumb rect
	bool m_bDragging;  //!< Indicates if the thumb is currently being dragged


private:
	CScrollBar& operator=(CScrollBar) = delete;
};

}


#endif  // _WG_SCROLLBAR_H_

