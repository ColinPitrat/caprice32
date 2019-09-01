// wg_color.h
//
// CRGBColor class interface
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


#ifndef _WG_COLOR_H_
#define _WG_COLOR_H_

#include "SDL.h"
#include <cmath>
#include <string>


namespace wGui
{

//! The CRGBColor class is used for all wGui representations of color

class CRGBColor
{
public:
	unsigned char red;  //!< Red component of the color
	unsigned char green;  //!< Green component of the color
	unsigned char blue;  //!< Blue component of the color
	unsigned char alpha;  //!< Alpha component (or opacity) of the color

  //! Default copy constructor.
  CRGBColor(const CRGBColor& other) = default;
  
	//! Construct a new color object
	//! \param r Red component value
	//! \param b Blue component value
	//! \param g Green component value
	//! \param a Alpha value, default value of 0xFF (fully opaque)
	CRGBColor(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a = 0xFF) :
		red(r), green(g), blue(b), alpha(a) { }

	//! Construct a CRGBColor object from an SDL Color
	//! \param pColorValue A pointer to the SDL Color
	//! \param pFormat A pointer to the SDL Pixel Format
	CRGBColor(const Uint32* pColorValue, const SDL_PixelFormat* pFormat);

	//! Construct a CRGBColor object from a text string
	//!  The string can be passed one of 2 ways.
	//!  1.) A color name - "RED"
	//!  2.) An RGB comma separated value - "255,0,0"
	//! \param s The string to get the color code of
	CRGBColor(std::string s);

	//! Convert the color so an SDL Color
	//! \param pFormat A pointer to the SDL Pixel Format
	unsigned long int SDLColor(SDL_PixelFormat* pFormat) const
		{ return SDL_MapRGBA(pFormat, red, green, blue, alpha); }

	//! Comparison operator does not take into account alpha values
	//! \return true if the Red, Green, and Blue color components are the same
	bool operator==(const CRGBColor& c) const
	{
		return (red == c.red && green == c.green && blue == c.blue);
	}

	//! Inequality operator does not take into accoutn alpha values
	//! \return true if any of the Red, Green, or Blue color components differ
	bool operator!=(const CRGBColor& c) const
	{
		return (red != c.red || green != c.green || blue != c.blue);
	}

	//! Assignment operator
	CRGBColor& operator=(const CRGBColor& c);

	//! Does additive color mixing
	CRGBColor operator+(const CRGBColor& c) const;

	//! Multiplies the r, g and b components by f.
	CRGBColor operator*(float f) const;

	//! Does OR color mixing
	CRGBColor operator|(const CRGBColor& c) const;

	//! Does AND color mixing
	CRGBColor operator&(const CRGBColor& c) const;

	//! Does XOR color mixing
	CRGBColor operator^(const CRGBColor& c) const;

	//! Does Normal color mixing
	//! \param c The color to be applied as the foreground color
	CRGBColor MixNormal(const CRGBColor& c) const;

};


// Predefined colors
extern CRGBColor DEFAULT_BUTTON_COLOR;
extern CRGBColor DEFAULT_TEXT_COLOR;
extern CRGBColor ALTERNATE_TEXT_COLOR;
extern CRGBColor DEFAULT_TITLEBAR_COLOR;
extern CRGBColor DEFAULT_TITLEBAR_TEXT_COLOR;

extern CRGBColor DEFAULT_BACKGROUND_COLOR;
extern CRGBColor DEFAULT_FOREGROUND_COLOR;
extern CRGBColor DEFAULT_LINE_COLOR;
extern CRGBColor DEFAULT_DISABLED_LINE_COLOR;
extern CRGBColor DEFAULT_CHECKBOX_COLOR;
extern CRGBColor DEFAULT_CHECKBOX_BACK_COLOR;
extern CRGBColor DEFAULT_SELECTION_COLOR;

extern CRGBColor COLOR_TRANSPARENT;

extern CRGBColor COLOR_WHITE;
extern CRGBColor COLOR_LIGHTGRAY;
extern CRGBColor COLOR_GRAY;
extern CRGBColor COLOR_DARKGRAY;
extern CRGBColor COLOR_BLACK;
extern CRGBColor COLOR_BLUE;
extern CRGBColor COLOR_BLUE_1;
extern CRGBColor COLOR_RED;
extern CRGBColor COLOR_GREEN;
extern CRGBColor COLOR_YELLOW;
extern CRGBColor COLOR_CYAN;
extern CRGBColor COLOR_MAGENTA;

}


#endif // _WG_COLOR_H_

