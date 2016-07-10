// wg_tooltip.h
//
// CToolTip interface
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


#ifndef _WG_TOOLTIP_H_
#define _WG_TOOLTIP_H_

#include "wg_window.h"
#include "wg_painter.h"
#include "wg_renderedstring.h"
#include "wg_application.h"
#include "wg_timer.h"
#include <string>
#include <memory>


namespace wGui
{

//! A tooltip that can pop up over windows
//! They are attached to an existing CWindow object, and watch for the mouse cursor to stop over the object.  When the cursor stops, the tooltip makes itself visible.

class CToolTip : public CWindow
{
public:
	//! Construct a new label
	//! \param pToolWindow A pointer to the CWindow based object the tooltip is for
	//! \param sText The label text
	//! \param FontColor The color of the tooltip text
	//! \param BackgroundColor The color of the tooltip's background
	//! \param pFontEngine A pointer to the font engine to use when drawing the tooltip
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CToolTip(CWindow* pToolWindow, std::string sText,
		CRGBColor& FontColor = DEFAULT_LINE_COLOR, CRGBColor& BackgroundColor = COLOR_WHITE, CFontEngine* pFontEngine = nullptr);

	//! Standard destructor
	virtual ~CToolTip(void);

	//! Displays the tooltip
	//! \param DrawPoint Where to poisition the top left corner of the tooltip, in view coordinates
	void ShowTip(const CPoint& DrawPoint);

	//! Hides the tooltip
	void HideTip(void);


	// CWindow overrides
	//! Renders the Window Text, and clips to the Window Rect
	virtual void Draw(void) const override;

	//! Move the window and any child windows
	//! \param MoveDistance The relative distance to move the window
	virtual void MoveWindow(const CPoint& MoveDistance) override;

	//! Blit the window to the given surface, using m_WindowRect as the offset into the surface
	//! \param ScreenSurface A reference to the surface that the window will be copied to
	//! \param FloatingSurface A reference to the floating surface which is overlayed at the very end (used for tooltips, menus and such)
	//! \param Offset This is the current offset into the Surface that should be used as reference
	virtual void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const override;


	// CMessageClient overrides
	//! CTooltips handle MOUSE_MOVE and CTRL_TIMER messages
	//! \param pMessage A pointer to the message
	virtual bool HandleMessage(CMessage* pMessage) override;


protected:
	CFontEngine* m_pFontEngine;  //!< A pointer to the font engine to use to render the text
	std::unique_ptr<CRenderedString> m_pRenderedString;  //!< An autopointer to the rendered version of the string
	CRGBColor m_FontColor;  //!< The font color
	CWindow* m_pToolWindow;  //!< A pointer to the CWindow based object the tooltip is for
	CTimer* m_pTimer;  //!< A pointer to a timer so that tooltips only appear after the mouse has been motionless for a bit
	CPoint m_LastMousePosition;  //!< The last mouse position
	CRect m_BoundingRect;  //!< A CRect that bounds the text


private:
  CToolTip(const CToolTip&) = delete;
	CToolTip& operator=(const CToolTip&) = delete;
};

}

#endif // _WG_TOOLTIP_H_

