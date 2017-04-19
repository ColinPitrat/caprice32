// wg_painter.h
//
// CPainter class which provides useful graphics routines
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


#ifndef _WG_GRAPHICS_H_
#define _WG_GRAPHICS_H_

#include "wg_rect.h"
#include "wg_color.h"
#include "wg_point.h"
#include "wg_window.h"
#include "SDL.h"


namespace wGui
{

//! Painter objects take care of all the actual drawing functions needed to draw to an SDL surface

class CPainter
{
public:
	//! A mode for painting the surface can be specified
	//! currently this does not work for the DrawHLine, DrawVLine, or DrawRect methods
	enum EPaintMode {
		PAINT_IGNORE = 0,  //!< Don't actually draw anything
		PAINT_REPLACE,  //!< Replace the pixel with the new pixel
		PAINT_NORMAL,  //!< Do normal color mixing (uses the alpha channel)
		PAINT_OR,  //!< OR the new color in
		PAINT_AND,  //!< AND the new color in
		PAINT_XOR,  //!< XOR the new color in
		PAINT_ADDITIVE  //!< Use Additive color mixing
	};

	//! Construct a new Painter object
	//! \param pSurface A pointer to the SDL surface to draw to
	//! \param ePaintMode The painting mode to use, defaults to PAINT_NORMAL
	CPainter(SDL_Surface* pSurface, EPaintMode ePaintMode = PAINT_NORMAL);

	//! Construct a new Painter object
	//! \param pWindow A pointer to the CWindow to draw to, this will only allow drawing to the window's client area
	//! \param ePaintMode The painting mode to use, defaults to PAINT_NORMAL
	CPainter(CWindow* pWindow, EPaintMode ePaintMode = PAINT_NORMAL);

	//! Standard destructor
	virtual ~CPainter() { }


	//! Draw a horizontal line
	//! \param xStart The start position of the line
	//! \param xEnd The end position of the line
	//! \param y The vertical location of the line
	//! \param LineColor The color of the line
	void DrawHLine(int xStart, int xEnd, int y, const CRGBColor& LineColor = DEFAULT_LINE_COLOR);

	//! Draw a vertical line
	//! \param yStart The start position of the line
	//! \param yEnd The end position of the line
	//! \param x The horizontal location of the line
	//! \param LineColor The color of the line
	void DrawVLine(int yStart, int yEnd, int x, const CRGBColor& LineColor = DEFAULT_LINE_COLOR);

	//! Draw a rectangle (this has been optimized to work much faster for filled rects in PAINT_REPLACE mode)
	//! \param Rect A CRect that describes the rectangle
	//! \param bFilled If true, rectangle will be filled with the FillColor, otherwise only the border is drawn
	//! \param BorderColor The color for the border
	//! \param FillColor The color to fill the rectangle with
	void DrawRect(const CRect& Rect, bool bFilled, const CRGBColor& BorderColor = DEFAULT_LINE_COLOR,
		const CRGBColor& FillColor = DEFAULT_FOREGROUND_COLOR);

    // judb draw a 'raised button' border based on the given color
    void Draw3DRaisedRect(const CRect& Rect, const CRGBColor& Color);

    // judb draw a 'lowered button' border based on the given color
    void Draw3DLoweredRect(const CRect& Rect, const CRGBColor& Color);

	//! Draw a line between two points
	//! \param StartPoint The beginning point of the line
	//! \param EndPoint The end point of the line
	//! \param LineColor The color to use for drawing the line
	void DrawLine(const CPoint& StartPoint, const CPoint& EndPoint, const CRGBColor& LineColor = DEFAULT_LINE_COLOR);
    // judb draw a box (filled)
    void DrawBox(CPoint UpperLeftPoint, int width, int height, const CRGBColor& LineColor);

	//! Draw a pixel
	//! \param Point The location of the pixel to set
	//! \param PointColor The color to set the pixel to
	void DrawPoint(const CPoint& Point, const CRGBColor& PointColor = DEFAULT_LINE_COLOR);

	//! Get the color of the pixel at the given point
	//! \param Point The location of the pixel to read
	//! \return A CRGBColor object representing the color of the pixel
	CRGBColor ReadPoint(const CPoint& Point);

	//! Replace all pixels of a certain color with a new color
	//! \param NewColor The color value to replace the pixels with
	//! \param OldColor The color of the pixels to be replaced
	void ReplaceColor(const CRGBColor& NewColor, const CRGBColor& OldColor);

	//! Sets the transparent pixel
	//! \param TransparentColor The pixel color to be treated as transparent
	void TransparentColor(const CRGBColor& TransparentColor);


protected:
	//! Locks the SDL surface
	void LockSurface();

	//! Unlocks the SDL surface
	void UnlockSurface();

	//! Mixes thje two colors based on the painting mode
	//! \param ColorBase The color to use as the base
	//! \param ColorAdd The color to add to the base
	//! \return A CRGBColor object representing the mixed colors
	CRGBColor MixColor(const CRGBColor& ColorBase, const CRGBColor& ColorAdd);

	SDL_Surface* m_pSurface;  //!< A pointer to the SDL Surface to draw on
	CWindow* m_pWindow;  //!< A pointer to the CWindow to draw to
	EPaintMode m_PaintMode;  //!< The painting mode to use
};


}

#endif // _WG_GRAPHICS_H_

