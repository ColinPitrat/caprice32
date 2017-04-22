// wg_textbox.h
//
// CTextBox interface
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

#ifndef _WG_TEXTBOX_H_
#define _WG_TEXTBOX_H_

#include "wg_window.h"
#include "wg_painter.h"
#include "wg_renderedstring.h"
#include "wg_timer.h"
#include "wg_resources.h"
#include "wg_scrollbar.h"
#include <vector>
#include <map>


namespace wGui
{

//! A multiline text box control

//! The CTextBox will generate CTRL_VALUECHANGE messages every time the text changes

class CTextBox : public CWindow
{
public:
	//! Construct a new Edit control
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CTextBox(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine = nullptr);

	//! Standard destructor
	~CTextBox() override;

	//! Set the Read-only state of the control
	//! \param bReadOnly  If set to true, the control will not take any keyboard input
	virtual void SetReadOnly(bool bReadOnly);

	//! Indicates if the text box is in read-only mode
	//! \return true if the control is read-only
	virtual bool IsReadOnly() const { return m_bReadOnly; }

	//! Gets the currently selected text
	//! \return The currently selected text in the edit box, if the edit box is in Password Mask mode, this will always return an empty string
	virtual std::string GetSelText() const;

	//! Set the selection
	//! \param iSelStart The index of the start of the selection
	//! \param iSelLength The number of characters selected
	virtual void SetSelection(std::string::size_type iSelStart, int iSelLength);

	//! Gets the start of the selection
	//! \return The index of the start of the selection
	virtual std::string::size_type GetSelectionStart() const { return m_SelStart; }

	//! Gets the selection length
	//! \return The length of the selection
	virtual int GetSelectionLength() const { return m_SelLength; }

	//! The various states for the scrollbars
	enum EScrollBarVisibility
	{
		SCROLLBAR_VIS_AUTO,  //!< Display the scrollbar if the text extends beyond the right border of the client area
		SCROLLBAR_VIS_NEVER,  //!< Never display the scrollbar
		SCROLLBAR_VIS_ALWAYS  //!< Always show the scrollbar
	};

	//! Set the visibility mode for the scrollbars
	//! \param ScrollBarType Indicates the vertical or the horizontal scrollbar
	//! \param Visibility The visibility mode to set the scrollbar to
	virtual void SetScrollBarVisibility(CScrollBar::EScrollBarType ScrollBarType, EScrollBarVisibility Visibility);

	//! Gets the visibility mode for the indicated scrollbar
	//! \param ScrollBarType Indicates the vertical or the horizontal scrollbar
	//! \return An EScrollBarVisibility value indicating the mode for the bar
	virtual EScrollBarVisibility GetScrollBarVisibility(CScrollBar::EScrollBarType ScrollBarType) const
		{ return m_ScrollBarVisibilityMap.find(ScrollBarType)->second; }


	// CWindow overrides
	//! Renders the text contents of a control, and the cursor
	void Draw() const override;

	//! Giving a control a new WindowRect will move and resize the control
	//! \param WindowRect A CRect that defines the outer limits of the control
	void SetWindowRect(const CRect& WindowRect) override;

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
	//! CTextBox will handle MOUSE_BUTTONDOWN and KEYBOARD_KEYDOWN messages
	//! \param pMessage A pointer to the message that needs to be handled
	bool HandleMessage(CMessage* pMessage) override;


protected:

	//! Deletes the selected portion of the string
	//! \internal
	//! \param psString A pointer to the buffer
	void SelDelete(std::string* psString);

	//! Creates the rendered string objects and calculates some cached values for the string
	//! \param sText The text to assign to the window
	void PrepareWindowText(const std::string& sText);

	//! Updates the visibility of the scrollbars
	void UpdateScrollBars();

	//! Convert an index to a row, column pair (in a CPoint object)
	//! \param Index The index into the string
	//! \return A CPoint object with the row as YPos and the column as XPos
	CPoint RowColFromIndex(std::string::size_type Index) const;

	//! Convert a row, column pair to an index
	//! \param Row The row of the position to be converted
	//! \param Col The column of the position to be converted
	//! \return The index into the string of the specified position
	std::string::size_type IndexFromRowCol(std::string::size_type Row, std::string::size_type Col) const;

	CFontEngine* m_pFontEngine; //!< A pointer to the font engine to use to render the text
	std::vector<CRenderedString*> m_vpRenderedString; //!< A vector of pointers to the rendered lines of text
	unsigned char m_FontSize;  //!< The font size (in points)
	std::string::size_type m_SelStart;  //!< Selection start point, in characters
	int m_SelLength;  //!< Selection length, in characters
	std::string::size_type m_DragStart;  //!< The position where the draw started
	bool m_bReadOnly;  //!< If true, the text of the control cannot be changed
	bool m_bMouseDown; //!< Set to true when the mouse button goes down
	bool m_bLastMouseMoveInside;  //!< True if the cursor was inside the control on the last MOUSE_MOVE message
	CScrollBar* m_pVerticalScrollBar;  //!< A pointer to the vertical scrollbar for the control.
	CScrollBar* m_pHorizontalScrollBar;  //!< A pointer to the horizontal scrollbar for the control.
	std::map<CScrollBar::EScrollBarType, EScrollBarVisibility> m_ScrollBarVisibilityMap;  //!< The visibility mode for the scrollbars

	// cached values
	unsigned int m_iLineCount;  //!< The number of lines of the window text
	unsigned int m_iRowHeight;  //!< The row height
	unsigned int m_iMaxWidth;  //!< The width of the longest line (in pixels)


private:
  CTextBox(const CTextBox&) = delete;
	CTextBox& operator=(const CTextBox&) = delete;
	bool m_bDrawCursor;  //!< Used to indicate if the cursor should be drawn on the next draw pass (used for the cursor blinking)
	mutable bool m_bScrollToCursor;  //!< Will force the text area to scroll so the cursor is visible on the next draw pass
	CTimer* m_pDblClickTimer;  //!< Timer to decide if we've double clicked or not.
	CTimer* m_pCursorTimer;  //!< Timer to blink the cursor
};

}


#endif // _WG_TEXTBOX_H_
