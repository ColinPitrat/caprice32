// wg_renderedstring.h
//
// CRenderedString interface
// CRenderedString uses the FreeType 2 library
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


#ifndef _WG_RENDEREDSTRING_H_
#define _WG_RENDEREDSTRING_H_

#include "wg_fontengine.h"
#include <vector>
#include "wg_rect.h"
#include "wg_color.h"
#include "SDL.h"
#include "std_ex.h"
#include <string>


namespace wGui
{

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable: 4512)
#endif // WIN32

//! For drawing a string to the screen
//! Optimized for strings that don't often change

class CRenderedString
{
public:
	//! The vertical alignment of the text
	enum EVAlign
	{
		VALIGN_NORMAL,  //!< Align the text so that the baseline is the bottom
		VALIGN_CENTER,  //!< Align the text in the center of the drawing area
		VALIGN_TOP,  //!< Align the top of the tallest character with the top of the drawing area
		VALIGN_BOTTOM  //!< Align the bottom of the lowest character with the bottom of the drawing area
	};

	//! The horizontal alignment of the text
	enum EHAlign
	{
		HALIGN_LEFT,  //!< Align the text to the left of the drawing area
		HALIGN_CENTER,  //!< Align the text with the center of the drawing area
		HALIGN_RIGHT  //!< Align the text with the right of the drawing area
	};

	//! Construct a new CRenderedString object
	//! \param pFontEngine A pointer to a CFontEngine object which is used to render the string
	//! \param sString The string to render
	//! \param eVertAlign The vertical alignment
	//! \param eHorzAlign The horizontal alignment
	CRenderedString(CFontEngine* pFontEngine, std::string sString, EVAlign eVertAlign = VALIGN_NORMAL, EHAlign eHorzAlign = HALIGN_LEFT);

	//! Render the string onto the given surface
	//! \param pSurface A pointer to the surface that will be drawn to
	//! \param BoundingRect The CRect to clip the rendered string to
	//! \param OriginPoint The origin of the string
	//! \param FontColor The color to draw the string in
	void Draw(SDL_Surface* pSurface, const CRect& BoundingRect, const CPoint& OriginPoint, const CRGBColor& FontColor = DEFAULT_LINE_COLOR) const;

	//! Get some metrics for the rendered string
	//! \param pBoundedDimensions A pointer to a CPoint object that will receive the width and height of the rendered string
	//! \param pOriginOffset A pointer to a CPoint object that will receive the offset of the top left corner of the rendered string from the origin of the string
	//! \param pCharacterRects A pointer to a CRect vector that will receive CRects that contain each character.  The corrdinates are in reference to the top left corner of the string as a whole
	void GetMetrics(CPoint* pBoundedDimensions, CPoint* pOriginOffset, std::vector<CRect>* pCharacterRects = nullptr) const;

	//! Get the length of the rendered string in characters
	//! \return The length of the string
	unsigned int GetLength() const { return stdex::safe_static_cast<unsigned int>(m_sString.size()); }

	//! Get the maximum height of the font, ASCII characters 0-255
	//! \return The max height of the font
	unsigned int GetMaxFontHeight();

    // judb
	unsigned int GetMaxFontWidth();
	// judb return the width of the given string when rendered in the engine's font
	unsigned int GetWidth(std::string sText);

	//! Set the mask character
	//! \param MaskChar Character to use as the mask
	void SetMaskChar(char MaskChar) { m_MaskChar = MaskChar; }


protected:
	CFontEngine* m_pFontEngine;  //!< A pointer to the font engine
	std::string m_sString;  //!< The string to be rendered
	char m_MaskChar;  //!< Character tp use as the mask, used for passwords and such


private:
	EVAlign m_eVertAlign;  //!< The vertical alignment
	EHAlign m_eHorzAlign;  //!< The horizontal alignment
	mutable CPoint m_CachedBoundedDimensions;  //!< The cached value of the rendered string's dimensions
	mutable CPoint m_OriginOffset;  //!< The cached value of the string's offset from the origin
	mutable std::vector<CRect> m_CachedCharacterRects;  //!< The cached value of the CRects for the various characters
	mutable bool m_bCachedMetricsValid;  //!< A boolean indicating if the cached values are valid
	int m_MaxFontHeight; //!< Maximum height of any character with ASCII value 0-255 for the current font
	int m_MaxFontWidth; //!< Maximum widht of any character with ASCII value 0-255 for the current font
};

#ifdef WIN32
#pragma warning(pop)
#endif // WIN32

}

#endif  // _WG_RENDEREDSTRING_H_
