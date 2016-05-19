// wg_window.h
//
// CWindow interface
// this serves as the base of all window derived classes
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


#ifndef _WG_WINDOW_H_
#define _WG_WINDOW_H_

#include "wg_rect.h"
#include "wg_color.h"
#include "wg_message_client.h"
#include <string>
#include <list>
#include <typeinfo>
#include "SDL.h"


namespace wGui
{

// forward declaration of the view class
class CView;


//! A base class with all the basic properties needed by a window

//! CWindow i inherits from the CMessageClient class so that any 'window' can recieve messages
//! CWindow provides the basic properties and methods needed to define a window
//! Almost all controls and views will be derived from this

class CWindow : public CMessageClient
{
public:
	//! The constructor will automatically register the new window with the specified parent as a child (if a parent is given)
	//! The parent is then responsible for destroying the window
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	CWindow(const CRect& WindowRect, CWindow* pParent);

    // judb constructor without CRect; don't forget to call SetWindowRect before using the CWindow!
	CWindow(CWindow* pParent);

	//! The CWindow destructor will automatically deregister itself with it's parent (if it had one)
	virtual ~CWindow(void);

	//! Return the classname for the object
	//! \return The classname of the object
	virtual std::string GetClassName(void) const { return typeid(*this).name(); }

	//! Giving a control a new WindowRect will move and resize the control
	//! \param WindowRect A CRect that defines the outer limits of the control
	virtual void SetWindowRect(const CRect& WindowRect);

	//! Gets the window's rectangle
	//! This is represented in the window's parent's client coordinates
	//! \return A copy of the CRect that the window represents
	virtual CRect GetWindowRect(void) const { return m_WindowRect; }

	//! Move the window and any child windows
	//! \param MoveDistance The relative distance to move the window
	virtual void MoveWindow(const CPoint& MoveDistance);

	//! The ClientRect describes the internal area of the control
	//! By default, this is initialized to the value of the WindowRect
	//! The ClientRect is useful for windows that will contain other windows
	//! Internally it's represented via the window's coordinates
	//! \return The client area CRect
	virtual CRect GetClientRect(void) const { return m_ClientRect; }

	//! Set the window's background color
	//! \param Color A CRGBColor that represents the background color
	virtual void SetBackgroundColor(const CRGBColor& Color) { m_BackgroundColor = Color; }

	//! Retrieve a window's background color
	//! \return A CRGBColor object that represents the background color
	virtual CRGBColor GetBackgroundColor(void) { return m_BackgroundColor; }

	//! Describes the ancestor that is desired
	enum EAncestor {
		PARENT, //!< return the direct parent of the window
		ROOT //!< climb the parent chain all the way until the root and return it
	};
	//! GetAncestor will return an ancestor of the window (using it's parent chain) based upon the requested ancestor
	//! \param eAncestor The desired ancestor of the window
	//! \return A pointer to the ancestor window, 0 is the window has no ancestors
	virtual CWindow* GetAncestor(EAncestor eAncestor) const;

	//! Gets the view the window is a part of
	//! \return A pointer to the view object for the window (if one exists), this assumes that the view is the root ancestor
	virtual CView* GetView(void) const;

	//! Find out if the window is a child of another specified window
	//! \param pWindow A pointer to the window that we're testing to see if this is a child of
	//! \return true if the window is a child of the specified window, this will return false if the specified window is the same as the current window
	virtual bool IsChildOf(CWindow* pWindow) const;

	//! Get the visibility of the control
	//! \return true if the control is visible
	virtual bool IsVisible(void) { return m_bVisible; }

	//! Set the visibility of the control, and all of it's children
	//! \param bVisible Set to false to hide the control
	virtual void SetVisible(bool bVisible);

	//! Get whether the control has the focus
	//! \return true if the control has the focus
  virtual bool HasFocus(void) const { return m_bHasFocus; }

	//! Set whether the control has the focus
	//! \param bHasFocus Set to true to tell the control it has the focus
  virtual void SetHasFocus(bool bHasFocus);

	//! Get whether the control is focusable or not
	//! \return true if the control can have the focus
  virtual bool IsFocusable(void) { return m_bIsFocusable; }

	//! Set whether the control has the focus
	//! \param bHasFocus Set to true to tell the control it has the focus
  virtual void SetIsFocusable(bool bIsFocusable);

	//! Gets the SDL surface the window draws to
	//! \return A pointer to the window's SDL surface
	virtual SDL_Surface* GetSDLSurface(void) { return m_pSDLSurface; }

	//! Translate the given CRect into view coordinates
	//! \param Rect A CRect in client coordinates
	virtual CRect ClientToView(const CRect& Rect) const;

	//! Translate the given CPoint into view coordinates
	//! \param Point A CPoint in client coordinates
	virtual CPoint ClientToView(const CPoint& Point) const;

	//! Translate the given CRect from view coordinates, to the window's client coordinates
	//! \param Rect A CRect in view coordinates
	virtual CRect ViewToClient(const CRect& Rect) const;

	//! Translate the given CPoint from view coordinates, to the window's client coordinates
	//! \param Point A CPoint in view coordinates
	virtual CPoint ViewToClient(const CPoint& Point) const;

	//! Translate the given CRect from view coordinates, to the window's coordinates
	//! \param Rect A CRect in view coordinates
	virtual CRect ViewToWindow(const CRect& Rect) const;

	//! Translate the given CPoint from view coordinates, to the window's coordinates
	//! \param Point A CPoint in view coordinates
	virtual CPoint ViewToWindow(const CPoint& Point) const;

	//! Set the WindowText of the control
	//! \param sText The text to assign to the window
	virtual void SetWindowText(const std::string& sText);

	//! Return the WindowText for the current window
	//! \return The WindowText
	virtual std::string GetWindowText(void) { return m_sWindowText; }

    // judb return list of children
	std::list<CWindow*> GetChildWindows() { return m_ChildWindows; }

	//! Check to see if a point lies within the window or any of it's children
	//! \param Point the point to check against in View coordinates
	//! \return true if the point lies within the window rect, or any of it's children's window rects
	virtual bool HitTest(const CPoint& Point) const;

	//! Render the window itself
	virtual void Draw(void) const;

	//! Blit the window to the given surface, using m_WindowRect as the offset into the surface
	//! \param ScreenSurface A reference to the surface that the window will be copied to
	//! \param FloatingSurface A reference to the floating surface which is overlayed at the very end (used for tooltips, menus and such)
	//! \param Offset This is the current offset into the Surface that should be used as reference
	virtual void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const;

	//! Transfer the ownership of the window, so it has a new parent
	//! \param pNewParent A pointer to a window that should be set as the parent
	virtual void SetNewParent(CWindow* pNewParent);

	//! This is called whenever the window is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return False by default, if an overridden control uses the click, then it should return true if it's in the bounds of the window
	virtual bool OnMouseButtonDown(CPoint Point, unsigned int Button);

	//! This is called whenever the a mouse button is released in a window
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return False by default, if an overridden control uses the click, then it should return true if it's in the bounds of the window
	virtual bool OnMouseButtonUp(CPoint Point, unsigned int Button);


	// CMessageClient overrides
	//! Attempt to handle the given message
	//! CWindows handle any APP_PAINT messages that have them marked as the destination
	//! \return true if the object handled the message (the message will not be given to any other handlers)
	virtual bool HandleMessage(CMessage* pMessage) override;

  virtual void AddFocusableWidget(CWindow *pWidget);

  virtual void RemoveFocusableWidget(CWindow *pWidget);


protected:
	// Registering and Deregistering child windows is automatically handled by the constructors and destructors

	//! Register pWindow as a child
	//! \param pWindow A pointer to the child window
	virtual void RegisterChildWindow(CWindow* pWindow);

	//! Deregister pWindow as a child
	//! \param pWindow A pointer to the child window
	virtual void DeregisterChildWindow(CWindow* pWindow);


	//! The Window Text (not directly used by CWindow)
	std::string m_sWindowText;

	//! The area the control occupies, these coordinates are in respect to the parent's client rect
	CRect m_WindowRect;

	//! Background color of the window
	CRGBColor m_BackgroundColor;

	//! The client area of the window, represented in the window's coordinates where (0,0) is the top left corner of the window
	CRect m_ClientRect;

	//! Pointer to the parent window
	CWindow* m_pParentWindow;

	//! A list of child windows
	std::list<CWindow*> m_ChildWindows;

	//! A pointer to the SDL surface buffer that the window draws to
	SDL_Surface* m_pSDLSurface;

	//! If this is false, the control will not paint itself
	bool m_bVisible;

  //! If this window currently has the focus
  bool m_bHasFocus;

  //! If this window can have the focus
  bool m_bIsFocusable;

private:
	void operator=(CWindow) { }  //!< The assignment operator is not allowed for CWindow objects
};

}

#endif // _WG_WINDOW_H_

