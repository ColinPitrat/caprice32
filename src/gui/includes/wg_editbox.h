// wg_editbox.h
//
// CEditBox interface
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


#ifndef _WG_EDITBOX_H_
#define _WG_EDITBOX_H_

#include "wg_window.h"
#include "wg_painter.h"
#include "wg_renderedstring.h"
#include "wg_timer.h"
#include "wg_resources.h"
#include <memory>

namespace wGui
{

//! A simple edit box control

//! The CEditBox will generate CTRL_VALUECHANGE messages every time the text changes

class CEditBox : public CWindow
{
public:
	//! Construct a new Edit control
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CEditBox(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine = nullptr);

	//! Standard destructor
	~CEditBox() override;

  //! The content allowed
  enum EContentType {
    ANY, //!< Any char allowed
    NUMBER, //!< Only numbers
    HEXNUMBER, //!< Numbers and char [A-Z]
    ALPHA, //!< Only letters
    ALPHANUM //!< Letters and digits
  };

	//! Set the Password mask state of the control
	//! \param bUseMask  If set to true, the control will act as a password mask box
	void SetUsePasswordMask(bool bUseMask) { m_bUseMask = bUseMask; }

	//! Indicates if the edit box is using a password mask
	//! \return true if the control is a password box
	bool UsingPasswordMask() const { return m_bUseMask; }

	//! Set the Read-only state of the control
	//! \param bReadOnly  If set to true, the control will not take any keyboard input
	void SetReadOnly(bool bReadOnly);

	//! Indicates if the edit box is operating in read-only mode
	//! \return true if the control is read-only
	bool IsReadOnly() const { return m_bReadOnly; }

	//! Gets the currently selected text
	//! \return The currently selected text in the edit box, if the edit box is in Password Mask mode, this will always return an empty string
	std::string GetSelText() const;

	//! Set the selection
	//! \param iSelStart The index of the start of the selection
	//! \param iSelLength The number of characters selected
	void SetSelection(std::string::size_type iSelStart, int iSelLength);

	//! Gets the starting index of the selection
	//! \return The index of the start of the selection
	virtual std::string::size_type GetSelectionStart() const { return m_SelStart; }

	//! Gets the length of the selection
	//! \return The length of the selection
	virtual int GetSelectionLength() const { return m_SelLength; }

	//! Gets a character index from a point
	//! \param Point The point (in window coordinates)
	//! \return The index (in characters) of the point in the string
	virtual std::string::size_type GetIndexFromPoint(const CPoint& Point) const; 


	// CWindow overrides
	//! Renders the text contents of a control, and the cursor
	void Draw() const override;

	//! Set the WindowText of the control
	//! \param sText The text to assign to the window
	void SetWindowText(const std::string& sText) override;

	//! This is called whenever the editbox is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the editbox
	bool OnMouseButtonDown(CPoint Point, unsigned int Button) override;

	// CMessageClient overrides
	//! CEditBox will handle MOUSE_BUTTONDOWN and KEYBOARD_KEYDOWN messages
	//! \param pMessage A pointer to the message that needs to be handled
	bool HandleMessage(CMessage* pMessage) override;

  void SetContentType(EContentType ctype) { m_contentType = ctype; };


protected:

	//! Deletes the selected portion of the string
	//! \internal
	//! \param psString A pointer to the buffer
	void SelDelete(std::string* psString);

	CFontEngine* m_pFontEngine; //!< A pointer to the font engine to use to render the text
	std::unique_ptr<CRenderedString> m_pRenderedString; //!< An autopointer to the rendered version of the string
	unsigned char m_FontSize;  //!< The font size (in points)
	std::string::size_type m_SelStart;  //!< Selection start point, in characters
	int m_SelLength;  //!< Selection length, in characters
	std::string::size_type m_DragStart;  //!< The position where the draw started
	mutable int m_ScrollOffset;  //!< The offset of the left side of the string, used for scrolling in the edit box
	bool m_bReadOnly;  //!< If true, the text of the control cannot be changed
	bool m_bMouseDown; //!< Set to true when the mouse button goes down
	bool m_bUseMask; //!< Set to true if you want the edit box to act as a password box and have a mask of asterikses
	bool m_bLastMouseMoveInside;  //!< True if the cursor was inside the control on the last MOUSE_MOVE message
  EContentType m_contentType;


private:
  CEditBox(const CEditBox&) = delete;
	CEditBox& operator=(const CEditBox&) = delete;
	bool m_bDrawCursor;
	CTimer* m_pDblClickTimer; //!< Timer to decide if we've double clicked or not.
	CTimer* m_pCursorTimer; //!< Timer to blink the cursor
};

}

#endif // _WG_EDITBOX_H_
