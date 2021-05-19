// wg_view.h
//
// CView interface
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


#ifndef _WG_VIEW_H_
#define _WG_VIEW_H_

#include "wg_window.h"
#include "SDL.h"
#include "wg_menu.h"
#include <string>


namespace wGui
{

//! The directions in which focus can be toggled: forward or backward
enum class EFocusDirection {
  FORWARD,
  BACKWARD
};

//! A general view class

//! A CView creates itself as a root window (it has no parent)
//! and responds to APP_PAINT messages with itself or 0 as the destination, and will redraw all of it's children as well as itself
//! Because of a limitation in SDL, there can be only one View

class CView : public CWindow
{
public:
	// judb; surface is an existing SDL surface, WindowRect is the area in which we want to draw the gui:
	CView(CApplication& application, SDL_Surface* surface, SDL_Surface* backSurface, const CRect& WindowRect);

	//! Standard Destructor
	~CView() override;

	//! Attaches a standard menu to the view, if the view already has a menu, the old menu will be deleted
	//! \param pMenu A pointer to the menu, the CView is then responsible for cleaning it up, passing in 0 will delete the current menu
	void AttachMenu(CMenu* pMenu);

	//! Gets the menu for the view
	//! \return A pointer to the view's menu, 0 if the view doesn't have a menu
	CMenu* GetMenu() const { return m_pMenu; }

	//! Sets the current floating window, which will be drawn on top of all other controls
	//! \param pWindow A pointer to the window to set as the floating window
	void SetFloatingWindow(CWindow* pWindow) { m_pFloatingWindow = pWindow; }

	//! Gets teh current floating window
	//! \return a pointer to the current floating window
	CWindow* GetFloatingWindow() const { return m_pFloatingWindow; }

	// judb ; sometimes the surface is re-created in caprice32; in this case, we have
	// to pass it on here
	void SetSurface(SDL_Surface* surface) { m_pScreenSurface = surface; }

  SDL_Surface* GetSurface() { return m_pScreenSurface; }


	// CWindow Overrides
	//! Set the WindowText of the view, which is used as the window caption
	//! \param sText The text to assign to the view
	void SetWindowText(const std::string& sText) override;

	// CMessageClient overrides
	//! Handles APP_PAINT messages with itself or 0 as the destination, and will redraw all of it's children as well as itself
	bool HandleMessage(CMessage* pMessage) override;

  //! Display the view on the screen surface
  virtual void Flip() const;

  //! Callback to close this view when the application exit
  virtual void Close() {};

protected:
	CMenu* m_pMenu;  //!< A pointer to the view's menu
	CWindow* m_pFloatingWindow;  //!< A pointer to the current floating window.  This will be drawn overtop of everything else.
	SDL_Window* m_pWindow;  //!< A pointer to the window
	SDL_Surface* m_pScreenSurface;  //!< A pointer to the actual screen surface

	SDL_Surface* m_pBackSurface;  // Caprice32-specific; contains the current Caprice32 output surface
	                              // so we can draw the gui on top of it.

private:
  CView(const CView&) = delete;
	CView& operator=(const CView&) = delete;
};

}


#endif // _WG_VIEW_H_

