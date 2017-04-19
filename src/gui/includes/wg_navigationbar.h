// wg_navigationbar.h
//
// A navigation bar like in the Firefox 1.x preferences window.
// It is like a horizontal or vertical variant of a listbox,
// in which only 1 item can be selected at once.
// Besides text, each item can also have an picture.
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

#ifndef _WG_NAVIGATION_H_
#define _WG_NAVIGATION_H_

#include "wg_painter.h"
#include "wg_renderedstring.h"
#include "wg_resource_handle.h"
#include "wg_window.h"
#include <vector>

namespace wGui
{

//! A navigation bar item
class SNavBarItem  {
public:
	//! Standard constructor
	SNavBarItem(std::string sItemTextIn, std::string PictureFilename ="" , CRGBColor ItemColorIn = DEFAULT_TEXT_COLOR) :
		sItemText(sItemTextIn), sPictureFilename(PictureFilename), ItemColor(ItemColorIn) {
    }
	std::string sItemText;  //!< The displayed text for the item
	std::string sPictureFilename;  //!< Name of the bmp file to display in the item.
	CRGBColor ItemColor;  //!< The text color to display the item in
};


//! Will generate CTRL_VALUECHANGE messages when a different item is selected.
//! The iNewValue of the message is the index of the item that was selected.

class CNavigationBar : public CWindow
{
public:
	//! Constructs a new navigation bar
	//! \param pParent A pointer to the parent window
    //! \param UpperLeft a CPoint that is the upper-left corner of the navigation bar.
	//! \param iMaxItems The (maximum) number of items that will fit in the navigation bar.
	//! \param iItemWidth The width of one item in the navigation bar. Default is 50.
	//! \param iItemHeight The height of one item in the navigation bar. Default is 50.
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CNavigationBar(CWindow* pParent, const CPoint& UpperLeft, unsigned int iMaxItems,
                   unsigned int iItemWidth = 50, unsigned int iItemHeight = 50, CFontEngine* pFontEngine = nullptr);

	//! Standard destructor
	virtual ~CNavigationBar();

	//! Gets the height of the items
	//! \return The height of the items in the navigation bar
	unsigned int GetItemHeight() { return m_iItemHeight; }

	//! Gets the width of the items
	//! \return The width of the items in the navigation bar
	unsigned int GetItemWidth() { return m_iItemWidth; }

	//! Sets the height of the items in the navigation bar
	//! \param iItemHeight The height of the items in the navigation bar
	void SetItemHeight(unsigned int iItemHeight);

	//! Sets the width of the items in the navigation bar
	//! \param iItemWidth The width of the items in the navigation bar
	void SetItemWidth(unsigned int iItemWidth);

	//! Adds a new item to the bar
	//! \param ListItem A SNavBarItem structure with the data for the item
	//! \return The index of the added item
	unsigned int AddItem(SNavBarItem ListItem);

	//! Returns the desired item
	//! \param iItemIndex The index of the item to check (will throw an exception if the index is out of range)
	//! \return A reference to the SNavBarItem struct
	SNavBarItem& GetItem(unsigned int iItemIndex) { return m_Items.at(iItemIndex); }

	//! Remove an item from the bar
	//! \param iItemIndex The index of the item to remove
	void RemoveItem(unsigned int iItemIndex);

	//! Remove all items from the bar
	void ClearItems();

	//! Gets the number of items in the navigation bar
	//! \return The number of items in the bar
	unsigned int Size() { return m_Items.size(); }

	//! \param iItemIndex The index of the item to check (will return false if the index is out of range)
	//! \return true if the item is selected
	bool IsSelected(unsigned int iItemIndex)
		{ return (iItemIndex < m_Items.size() && m_iSelectedItem == iItemIndex); }

  // Returns the index of the selected item; returns -1 if there is no selection.
  unsigned int getSelectedIndex();

	//! Selects an item in the navigation bar.
	//! \param iItemIndex The index of the item to select.
	void SelectItem(unsigned int iItemIndex);

  // Returns the index of the focused item; returns -1 if there is no selection.
  unsigned int getFocusedIndex();

	//! Focus an item in the navigation bar.
	//! \param iItemIndex The index of the item to focus.
  void FocusItem(unsigned int iItemIndex);

	// CWindow overrides
	//! Draws the navigation bar
	virtual void Draw() const override;

	//! Giving a control a new WindowRect will move and resize the control
	//! \param WindowRect A CRect that defines the outer limits of the control
	virtual void SetWindowRect(const CRect& WindowRect) override;

	//! Blit the window to the given surface, using m_WindowRect as the offset into the surface
	//! \param ScreenSurface A reference to the surface that the window will be copied to
	//! \param FloatingSurface A reference to the floating surface which is overlayed at the very end (used for tooltips, menus and such)
	//! \param Offset This is the current offset into the Surface that should be used as reference
	virtual void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const override;

	//! This is called whenever the navigation bar is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the navigation bar
	virtual bool OnMouseButtonDown(CPoint Point, unsigned int Button) override;

	// CMessageClient overrides
	//! CNavigationBars handle MOUSE_BUTTONDOWN, MOUSE_BUTTONUP and CTRL_VALUECHANGE messages
	//! \param pMessage A pointer to the message
	virtual bool HandleMessage(CMessage* pMessage) override;


protected:
	CFontEngine* m_pFontEngine;  //!< A pointer to the font engine to use to render the text
	unsigned int m_iItemHeight;  //!< The height of the items in the navigation bar
	unsigned int m_iItemWidth;  //!< The width of the items in the navigation bar
	unsigned int m_iSelectedItem;  //!< The currently selected item (selection color)
	unsigned int m_iFocusedItem;  //!< The currently focused item (rectangle)
	std::vector<SNavBarItem> m_Items;  //!< The list of items
	std::vector<CRenderedString> m_RenderedStrings;  //!< A vector of the rendered strings
	std::vector<CBitmapResourceHandle *> m_Bitmaps;  //!< A vector of the pictures (optional)

private:
  CNavigationBar(const CNavigationBar&) = delete;
	CNavigationBar& operator=(const CNavigationBar&) = delete;
};

}


#endif  // _WG_NAVIGATION_H_

