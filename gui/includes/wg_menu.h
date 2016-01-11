// wg_menu.h
//
// CMenu interface
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


#ifndef _WG_MENU_H_
#define _WG_MENU_H_

#include "wg_window.h"
#include "wg_painter.h"
#include "wg_renderedstring.h"
#include "wg_resources.h"
#include "wg_timer.h"
#include <vector>
#include <string>


namespace wGui
{

class CPopupMenu;


//! The structure used to represent menu items

struct SMenuItem
{
public:
	//! Constructs a new Menu Item
	//! \param sItemText The text to display for the menu item
	//! \param iItemId An identifier for the menu item, which gets returned in the CTRL_SINGLELCLICK message
	//! \param pPopup A pointer to a popup menu, if the menu item is actually a submenu, this should be 0 if the item isn't a submenu (defaults to 0)
	SMenuItem(std::string sItemText, long int iItemId = 0, CPopupMenu* pPopup = 0) :
		sItemText(sItemText), iItemId(iItemId), pPopup(pPopup), bSpacer(false) { }

	//! Constructs a new Spacer Menu Item
	SMenuItem(void) : sItemText(""), iItemId(0), pPopup(0), bSpacer(true) { }

	std::string sItemText;  //!< The caption to display for the menu item
	long int iItemId;  //!< An identifier for the menu item, which gets returned in the CTRL_SINGLELCLICK message
	CPopupMenu* pPopup;  //!< A pointer to a popup menu, if the menu item is actually a submenu
	bool bSpacer;  //!< Indicates if this is a spacer.  If true, pPopup, iItemId and sItemText are ignored.
};


//! The menu base class

//! The CMenuBase is the base class for CMenus and CPopupMenus, and shouldn't be instantiated itself
//! Menus will generate CTRL_SINGLELCLICK messages when a menu item is selected

class CMenuBase : public CWindow
{
public:
	//! Constructs a new MenuBase
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CMenuBase(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine = 0);

	//! Standard destructor
	virtual ~CMenuBase(void);

	//! Insert a menu item into the menu
	//! \param MenuItem An SMenuItem struct that defines the menu item to add
	//! \param iPosition The position to insert it at, -1 will insert it at the end, defaults to -1
	virtual void InsertMenuItem(const SMenuItem& MenuItem, int iPosition = -1);

	//! Removes a menu item or a submenu
	//! \param iPosition The item to be removed
	void RemoveMenuItem(int iPosition);

	//! Gets the number of items in a menu
	//! \return The number of items in the menu
	unsigned int GetMenuItemCount(void) const { return m_MenuItems.size(); }

	//! Hides the active popup window
	void HideActivePopup(void);

	//! Set the highlight color for the menu
	//! \param HighlightColor The new color to use for highlighting
	void SetHighlightColor(const CRGBColor& HighlightColor) { m_HighlightColor = HighlightColor; }


	//! CWindow overrides
	//! Draws the menu
	virtual void Draw(void) const = 0;

	//! This is called whenever the menu is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the menu
	virtual bool OnMouseButtonDown(CPoint Point, unsigned int Button);


	// CMessageClient overrides
	//! CMenus handle MOUSE_BUTTONDOWN and MOUSE_BUTTONUP messages
	//! \param pMessage A pointer to the message
	virtual bool HandleMessage(CMessage* pMessage);


protected:
	//! This updates the cached item rects if they are marked as invalid
	virtual void UpdateCachedRects(void) const = 0;

	//! Check to see where it will fit, then show the popup menu
	//! \param ParentRect A CRect that defines the dimensions of the item that is spawning the popup
	//! \param BoundingRect A CRect that defines the boundaries the popup has to fit in
	virtual void ShowActivePopup(const CRect& ParentRect, const CRect& BoundingRect) = 0;


	CFontEngine* m_pFontEngine;  //!< A pointer to the font engine to use to render the text
	//! A struct containing the menu item with some cached data
	struct s_MenuItemInfo
	{
		//! struct constructor
		//! \param MI the menu item
		//! \param RS the rendered string for the menu item
		//! \param R a CRect describing the boundaries of the menu item
		s_MenuItemInfo(const SMenuItem& MI, const CRenderedString& RS, const CRect& R)
			: MenuItem(MI), RenderedString(RS), Rect(R)
		{ }
		
		SMenuItem MenuItem;  //!< The actual menu item
		CRenderedString RenderedString;  //!< A cached rendered string for the text of the menu item
		CRect Rect;  //!< The bounds rect for the menu item		
	};
	typedef std::vector<s_MenuItemInfo> t_MenuItemVector;  //!< The type for menu items
	mutable t_MenuItemVector m_MenuItems;  //!< The vector of menu items
	SMenuItem* m_pHighlightedItem;  //!< The item that should be highlighted
	mutable bool m_bCachedRectsValid;  //!< True if the cached item rects are valid
	CPopupMenu* m_pActivePopup;  //!< A pointer to the active popup
	CwgBitmapResourceHandle m_hRightArrowBitmap;  //!< A handle to the bitmap for the right arrow
	CRGBColor m_HighlightColor;  //!< Sets the highlight color to use, defaults to Dark Gray
	CTimer* m_pPopupTimer;  //!< A timer to be used for opening sub menus when the mouse hovers over an item


private:
	//void operator=(CMenuBase) { }  //!< The assignment operator is not allowed for CWindow derived objects
};


//! A standard application level menu

class CMenu : public CMenuBase
{
public:
	//! Constructs a new Menu
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CMenu(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine = 0);

	//! Standard destructor
	virtual ~CMenu(void);

	//! Insert a menu item into the menu
	//! \param MenuItem An SMenuItem struct that defines the menu item to add
	//! \param iPosition The position to insert it at, -1 will insert it at the end, defaults to -1
	virtual void InsertMenuItem(const SMenuItem& MenuItem, int iPosition = -1);


	//! CWindow overrides
	//! Draws the menu
	virtual void Draw(void) const;

	//! This is called whenever the menu is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the menu
	virtual bool OnMouseButtonDown(CPoint Point, unsigned int Button);


	// CMessageClient overrides
	//! CMenus handle MOUSE_BUTTONDOWN and MOUSE_BUTTONUP messages
	//! \param pMessage A pointer to the message
	virtual bool HandleMessage(CMessage* pMessage);


protected:
	//! This updates the cached item rects if they are marked as invalid
	virtual void UpdateCachedRects(void) const;

	//! Check to see where it will fit, then show the popup menu
	//! \param ParentRect A CRect that defines the dimensions of the item that is spawning the popup
	//! \param BoundingRect A CRect that defines the boundaries the popup has to fit in
	virtual void ShowActivePopup(const CRect& ParentRect, const CRect& BoundingRect);


private:
	void operator=(CMenu) { }  //!< The assignment operator is not allowed for CWindow derived objects
};


//! Popup menus are used for both context menus, and as the popups when a CMenu item is clicked that has a submenu

class CPopupMenu : public CMenuBase
{
public:
	//! Constructs a new Popup Menu
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CPopupMenu(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine = 0);

	//! Standard destructor
	virtual ~CPopupMenu(void);

	//! Show the popup at the given point
	//! \param Position The point to use for the top left corner of the popup, in view coordinates
	void Show(CPoint Position);

	//! Hide the popup and any popups hanging off of it
	void Hide(void);

	//! Hide the popup, along with it's popup parents and children
	//! This method just searches for the root popup, and then calls Hide() on it
	void HideAll(void);

	//! Tests to see if any children are hit by the point
	bool IsInsideChild(const CPoint& Point) const;

	//! Indicates if the popup menu has any popup parents
	//! \return true is the Popup menu doesn't have any popup parents
	bool IsRootPopup(void) { return !(dynamic_cast<CPopupMenu*>(m_pParentWindow)); }

	//! This is only for root popup menus that are dropped by a CMenu
	//! This doesn't set the actual parent of the control since the root window (probably a CView) should be the real parent
	//! This is called automatically when a popup menu is inserted into a CMenu (via InsertMenuItem)
	//! \param pParentMenu A pointer to the CMenu object that acts as the Popup's parent
	void SetParentMenu(CMenu* pParentMenu) { m_pParentMenu = pParentMenu; }


	//! CWindow overrides
	//! Draws the menu
	virtual void Draw(void) const;

	//! Blit the window to the given surface, using m_WindowRect as the offset into the surface
	//! \param ScreenSurface A reference to the surface that the window will be copied to
	//! \param FloatingSurface A reference to the floating surface which is overlayed at the very end (used for tooltips, menus and such)
	//! \param Offset This is the current offset into the Surface that should be used as reference
	virtual void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const;

	//! This is called whenever the popup is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the popup
	virtual bool OnMouseButtonDown(CPoint Point, unsigned int Button);


	// CMessageClient overrides
	//! CMenus handle MOUSE_BUTTONDOWN and MOUSE_BUTTONUP messages
	//! \param pMessage A pointer to the message
	virtual bool HandleMessage(CMessage* pMessage);


protected:
	//! This updates the cached item rects if they are marked as invalid
	virtual void UpdateCachedRects(void) const;

	//! Check to see where it will fit, then show the popup menu
	//! \param ParentRect A CRect that defines the dimensions of the item that is spawning the popup
	//! \param BoundingRect A CRect that defines the boundaries the popup has to fit in, this is in view coordinates
	virtual void ShowActivePopup(const CRect& ParentRect, const CRect& BoundingRect);

	//! This is a pointer to the CMenu that acts as parent for the popup,
	//! though it's not actually the parent, because the parent for root popups should be the CView
	CMenu* m_pParentMenu;


private:
	void operator=(CPopupMenu) { }  //!< The assignment operator is not allowed for CWindow derived objects
};

}

#endif  // _WG_MENU_H_

