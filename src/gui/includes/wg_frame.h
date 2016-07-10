// wg_frame.h
//
// CFrame interface
// Frames are windows within a view that have their own window management controls
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


#ifndef _WG_FRAME_H_
#define _WG_FRAME_H_

#include "wg_window.h"
#include "wg_view.h"
#include "wg_button.h"


namespace wGui
{

//! Frames are windows within a view that have their own window management controls

//! The CFrame class allows the user to create multiple windows within the view.  Unfortunately they're still bounded by the view
//! but it's a slight workaround for the SDL limitation of only 1 SDL view per app

class CFrame : public CWindow
{
public:
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent view
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	//! \param sTitle The window title, which will appear in the title bar of the view
	//! \param bResizable If true, the window will be resizable
	CFrame(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine, const std::string& sTitle, bool bResizable = true);

	//! Standard destructor
	virtual ~CFrame(void);

	//! Set the color of the title bar
	//! \param TitleBarColor The new color for the title bar
	void SetTitleBarColor(CRGBColor& TitleBarColor) { m_TitleBarColor = TitleBarColor; }

	//! Set the color of the title bar text
	//! \param TitleBarTextColor The new color for the title bar text
	void SetTitleBarTextColor(CRGBColor& TitleBarTextColor) { m_TitleBarTextColor = TitleBarTextColor; }

	//! Set the height of the title bar
	//! \param iTitleBarHeight
	void SetTitleBarHeight(int iTitleBarHeight);

	//! Indicates if the frame is resizable (set in the object constructor)
	//! \return true if the frame is resizable
	bool IsResizable(void) const { return m_bResizable; }

	//! Attaches a standard menu to the frame, if the frame already has a menu, the old menu will be deleted
	//! \param pMenu A pointer to the menu, the CFrame is then responsible for cleaning it up, passing in 0 will delete the current menu
	void AttachMenu(CMenu* pMenu);

	//! Gets the menu for a frame
	//! \return A pointer to the frame's menu, 0 if the view doesn't have a menu
	CMenu* GetMenu(void) const { return m_pMenu; }

	//! Closes the frame and causes it to delete itself
	virtual void CloseFrame(void);

	//! Indicates if the frame is modal (doesn't allow input to any other windows)
	//! \return true if the frame is modal
	bool IsModal(void) const { return m_bModal; }

	//! Sets the frame's modal state
	//! param bModal the modal state to set (CFrames are non-modal by default)
	void SetModal(bool bModal);


	// CWindow overrides
	//! Draws the frame and renders the title bar
	virtual void Draw(void) const override;

	//! Blit the window to the given surface, using m_WindowRect as the offset into the surface
	//! \param ScreenSurface A reference to the surface that the window will be copied to
	//! \param FloatingSurface A reference to the floating surface which is overlayed at the very end (used for tooltips, menus and such)
	//! \param Offset This is the current offset into the Surface that should be used as reference
	virtual void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const override;

	//! Giving a control a new WindowRect will move and resize the control
	//! \param WindowRect A CRect that defines the outer limits of the control
	virtual void SetWindowRect(const CRect& WindowRect) override;

	//! Set the title bar text of the frame
	//! \param sText The text to assign to the view
	virtual void SetWindowText(const std::string& sText) override;

	//! This is called whenever the frame is clicked on by the mouse
	//! Only the topmost window that bounds the point will be called by the system
	//! \param Point The point where the mouse clicked
	//! \param Button A bitfield indicating which button the window was clicked with
	//! \return True if it's in the bounds of the frame
	virtual bool OnMouseButtonDown(CPoint Point, unsigned int Button) override;


	// CMessageClient overrides
	//! CFrame handles no messages at the moment
	//! \param pMessage A pointer to the message
	virtual bool HandleMessage(CMessage* pMessage) override;

  virtual void AddFocusableWidget(CWindow *pWidget) override;

  virtual void RemoveFocusableWidget(CWindow *pWidget) override;

  virtual CWindow *GetFocused();

  void FocusNext(EFocusDirection direction, bool loop = true);

protected:
	CPictureButton* m_pFrameCloseButton;  //!< The close button for the frame
	CRGBColor m_TitleBarColor;  //!< The title bar color, defaults to blue
	CRGBColor m_TitleBarTextColor;  //!< The title bar text color, defaults to the default line color
	int m_iTitleBarHeight;  //!< The height of the title bar, defaults to 12
	CFontEngine* m_pFontEngine;  //!< A pointer to the font engine to use to render the text
	std::unique_ptr<CRenderedString> m_pRenderedString;  //!< An autopointer to the rendered version of the string
	bool m_bResizable;  //!< Indicates if the frame is resizable
	bool m_bModal;  //!< Indicates if the frame is modal
	CMenu* m_pMenu;  //!< A pointer to the frame's menu
  std::list<CWindow*> m_FocusableWidgets; //!< A list of all focusable widgets in this frame

private:
	CRect m_TitleBarRect;  //!< A place to cache the title bar rect
	bool m_bDragMode;  //!< Indicates if the window is currently being dragged
	CPoint m_DragPointerStart;  //!< The location of the cursor when the drag was started
	CRect m_FrameGhostRect;  //!< The rect of the frame while being dragged in a semi-transparent state
  CFrame(const CFrame&) = delete;
	CFrame& operator=(const CFrame&) = delete;
};

}

#endif // _WG_FRAME_H_

