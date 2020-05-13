// wg_listbox.h
//
// CListBox interface
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


#ifndef _WG_LISTBOX_H_
#define _WG_LISTBOX_H_

#include "wg_window.h"
#include "wg_painter.h"
#include "wg_renderedstring.h"
#include "wg_scrollbar.h"
#include <string>
#include <vector>


namespace wGui
{

//! A listbox item

struct SListItem
{
public:
	//! Standard constructor
	SListItem(std::string sItemTextIn, void* pItemDataIn = nullptr, CRGBColor ItemColorIn = DEFAULT_TEXT_COLOR) :
		sItemText(std::move(sItemTextIn)), pItemData(pItemDataIn), ItemColor(ItemColorIn) { }

	std::string sItemText;  //!< The displayed text for the item
	void* pItemData;  //!< A pointer to void that can be used as a data pointer
	CRGBColor ItemColor;  //!< The color to display the item in
};


//! A simple listbox class

//! The button will generate CTRL_VALUECHANGE messages when a different listbox item is selected
//! The iNewValue of the message is the index of the item that was selected

class CListBox : public CWindow
{
public:
	//! Constructs a new listbox
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param bSingleSelection If true, only one item can be selected at a time, defaults to false
	//! \param iItemHeight The height of the items in the list, defaults to 15
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CListBox(const CRect& WindowRect, CWindow* pParent, bool bSingleSelection = false, unsigned int iItemHeight = 12, CFontEngine* pFontEngine = nullptr);

	//! Standard destructor
	~CListBox() override;

	//! Gets the height of the items
	//! \return The height of the items in the listbox
	unsigned int GetItemHeight() { return m_iItemHeight; }

	//! Sets the heigh of the items in the listbox
	//! \param iItemHeight The height of the items in the listbox
	void SetItemHeight(unsigned int iItemHeight);

	//! Adds a new item to the list
	//! \param ListItem A SListItem structure with the data for the item
	//! \return The index of the added item
	unsigned int AddItem(SListItem ListItem);

	//! Returns the desired item
	//! \param iItemIndex The index of the item to check (will throw an exception if the index is out of range)
	//! \return A reference to the SListItem struct
	SListItem& GetItem(unsigned int iItemIndex) { return m_Items.at(iItemIndex); }

	//! Remove an item from the list
	//! \param iItemIndex The index of the item to remove
	void RemoveItem(unsigned int iItemIndex);

	//! Remove all items from the list
	void ClearItems();

	//! Gets the number of items in the listbox
	//! \return The number of items in the list
	unsigned int Size() { return m_Items.size(); }

	//! \param iItemIndex The index of the item to check (will return false if the index is out of range)
	//! \return true if the item is selected
	bool IsSelected(unsigned int iItemIndex)
		{ return (iItemIndex < m_SelectedItems.size() && m_SelectedItems.at(iItemIndex)); }

    // Returns the index of the first selected item; returns -1 if there is no selection.
    int getFirstSelectedIndex();

	//! Set an item as selected
	//! \param iItemIndex The index of the item to change
	//! \param bSelected Will select the item if true, or unselect if false
	void SetSelection(unsigned int iItemIndex, bool bSelected, bool bNotify = true);

    //! Selects or deselects all items
    void SetAllSelections(bool bSelected);

    //! Sets the focus rectangle on the specified item:
    void SetFocus(unsigned int iItemIndex);

	//! Set the dropdown window this is a part of
	//! \param pDropDown A pointer to the dropdown window
	void SetDropDown(CWindow* pDropDown);


	// CWindow overrides
	//! Draws the button and renders the button label
	void Draw() const override;

	//! Giving a control a new WindowRect will move and resize the control
	//! \param WindowRect A CRect that defines the outer limits of the control
	void SetWindowRect(const CRect& WindowRect) override;

	//! Blit the window to the given surface, using m_WindowRect as the offset into the surface
	//! \param ScreenSurface A reference to the surface that the window will be copied to
	//! \param FloatingSurface A reference to the floating surface which is overlayed at the very end (used for tooltips, menus and such)
	//! \param Offset This is the current offset into the Surface that should be used as reference
	void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const override;

	//! This is called whenever the listbox is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the listbox
	bool OnMouseButtonDown(CPoint Point, unsigned int Button) override;

	//! This is called whenever the a mouse button is released in the listbox
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the listbox
	bool OnMouseButtonUp(CPoint Point, unsigned int Button) override;


	// CMessageClient overrides
	//! CListBoxes handle MOUSE_BUTTONDOWN, MOUSE_BUTTONUP and CTRL_VALUECHANGE messages
	//! \param pMessage A pointer to the message
	bool HandleMessage(CMessage* pMessage) override;


protected:
	CFontEngine* m_pFontEngine;  //!< A pointer to the font engine to use to render the text
	CScrollBar* m_pVScrollbar;  //!< A pointer to the vertical scrollbar
	unsigned int m_iItemHeight;  //!< The height of the items in the list
	unsigned int m_iFocusedItem;  //!< The currently focused item (rectangle)
	std::vector<SListItem> m_Items;  //!< The list of items
	std::vector<bool> m_SelectedItems;  //!< A vector of booleans indicating which items are selected
	std::vector<CRenderedString> m_RenderedStrings;  //!< A vector of the rendered strings
	const bool m_bSingleSelection;  //!< If true, only one item can be selected at a time
	CWindow* m_pDropDown;  //!< A pointer to the dropdown control if this a part of one


private:
  CListBox(const CListBox&) = delete;
	CListBox& operator=(const CListBox&) = delete;
};

}


#endif  // _WG_LISTBOX_H_

