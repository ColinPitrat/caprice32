// wg_color.cpp
//
// CRGBColor class
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


#include "wg_color.h"
#include "std_ex.h"

namespace wGui
{

CRGBColor::CRGBColor(const Uint32* pColorValue, const SDL_PixelFormat* pFormat)
{
	red = stdex::safe_static_cast<unsigned char>((pFormat->Rmask & *pColorValue) >> pFormat->Rshift);
	green = stdex::safe_static_cast<unsigned char>((pFormat->Gmask & *pColorValue) >> pFormat->Gshift);
	blue = stdex::safe_static_cast<unsigned char>((pFormat->Bmask & *pColorValue) >> pFormat->Bshift);
	alpha = stdex::safe_static_cast<unsigned char>((pFormat->Amask & *pColorValue) >> pFormat->Ashift);
}


CRGBColor& CRGBColor::operator=(const CRGBColor& c)
{
	red = c.red;
	green = c.green;
	blue = c.blue;
	alpha = c.alpha;
	return *this;
}

CRGBColor::CRGBColor(std::string s)
{
	bool bInitted=false;
	std::string::size_type pos;
	std::string::size_type posOld;

	if (s == "WHITE")
	{
		red = 0xFF;
		green = 0xFF;
		blue = 0xFF;
		bInitted = true;
	}

	if (s == "LIGHTGRAY")
	{
		red = 0xC0;
		green = 0xC0;
		blue = 0xC0;
		bInitted = true;
	}

	// etc...


	//0,0,0 - 255,255,255
	//|-5-|   |---11----|

	//It's possible that we have a string with arbitrary values
	if (!bInitted && s.length() > 4 && s.length() < 12)
	{
		pos = s.find(",", 0);
		if (pos != std::string::npos)
		{
			std::string sub = s.substr(0, pos);

			red = stdex::safe_static_cast<unsigned char>((stdex::atoi(sub)));

			posOld = pos + 1;
			pos = s.find(",", posOld);
			if (pos != std::string::npos)
			{
				sub = s.substr(posOld, pos - posOld);

				green = stdex::safe_static_cast<unsigned char>((stdex::atoi(sub)));

				posOld = pos + 1;
				sub = s.substr(posOld, s.length() - posOld);
				if (sub.length() > 0)
				{
					blue = stdex::safe_static_cast<unsigned char>((stdex::atoi(sub)));

					bInitted = true;
				}
			}
		}

		//If we have yet to find a good value, we'll just use white... maybe this should throw an error...
		if (!bInitted)
		{
			red = 0xFF;
			green = 0xFF;
			blue = 0xFF;
		}
	}
}

CRGBColor CRGBColor::operator+(const CRGBColor& c) const
{
	double c1_ratio = stdex::safe_static_cast<double>(alpha) / 0xFF;
	double c2_ratio = stdex::safe_static_cast<double>(c.alpha) / 0xFF;
	double new_red = red * c1_ratio + c.red * c2_ratio;
	if (new_red > 255)
	{
		new_red = 255;
	}
	double new_green = green * c1_ratio + c.green * c2_ratio;
	if (new_green > 255)
	{
		new_green = 255;
	}
	double new_blue = blue * c1_ratio + c.blue * c2_ratio;
	if (new_blue > 255)
	{
		new_blue = 255;
	}
	double new_alpha = alpha + c.alpha;
	if (new_alpha > 255)
	{
		new_alpha = 255;
	}
	return CRGBColor(stdex::safe_static_cast<unsigned char>(new_red), stdex::safe_static_cast<unsigned char>(new_green),
		stdex::safe_static_cast<unsigned char>(new_blue), stdex::safe_static_cast<unsigned char>(new_alpha));
}

CRGBColor CRGBColor::operator*(float f) const
{
	double new_red = floor(red * f);
	if (new_red > 255)
	{
		new_red = 255;
	}
	double new_green = floor(green * f);
	if (new_green > 255)
	{
		new_green = 255;
	}
	double new_blue = floor(blue * f);
	if (new_blue > 255)
	{
		new_blue = 255;
	}
	return CRGBColor(stdex::safe_static_cast<unsigned char>(new_red), stdex::safe_static_cast<unsigned char>(new_green),
		stdex::safe_static_cast<unsigned char>(new_blue));
}


CRGBColor CRGBColor::operator|(const CRGBColor& c) const
{
	return CRGBColor(red | c.red, green | c.green, blue | c.blue, alpha | c.alpha);
}


CRGBColor CRGBColor::operator&(const CRGBColor& c) const
{
	return CRGBColor(red & c.red, green & c.green, blue & c.blue, alpha & c.alpha);
}


CRGBColor CRGBColor::operator^(const CRGBColor& c) const
{
	return CRGBColor(red ^ c.red, green ^ c.green, blue ^ c.blue, alpha ^ c.alpha);
}


CRGBColor CRGBColor::MixNormal(const CRGBColor& c) const
{
	double fg_ratio = stdex::safe_static_cast<double>(c.alpha) / 0xFF;
	double bg_ratio = stdex::safe_static_cast<double>(1.0 - fg_ratio);
	char new_red = stdex::safe_static_cast<unsigned char>(floor(red * bg_ratio + c.red * fg_ratio));
	char new_green = stdex::safe_static_cast<unsigned char>(floor(green * bg_ratio + c.green * fg_ratio));
	char new_blue = stdex::safe_static_cast<unsigned char>(floor(blue * bg_ratio + c.blue * fg_ratio));
	return CRGBColor(new_red, new_green, new_blue, alpha);
}

// standard
CRGBColor DEFAULT_BACKGROUND_COLOR = CRGBColor(0xC0, 0xC0, 0xCA);
CRGBColor DEFAULT_FOREGROUND_COLOR = CRGBColor(0x90, 0x90, 0x90);
CRGBColor DEFAULT_LINE_COLOR = CRGBColor(0xC0, 0xC0, 0xC0);
CRGBColor DEFAULT_DISABLED_LINE_COLOR = CRGBColor(0x80, 0x80, 0x80);
CRGBColor COLOR_TRANSPARENT = CRGBColor(0x00, 0x00, 0x00, 0x00);
CRGBColor COLOR_WHITE = CRGBColor(0xFF, 0xFF, 0xFF);
CRGBColor COLOR_LIGHTGRAY = CRGBColor(0xC0, 0xC0, 0xC0);
CRGBColor COLOR_GRAY = CRGBColor(0x80, 0x80, 0x80);
CRGBColor COLOR_DARKGRAY = CRGBColor(0x40, 0x40, 0x40);
CRGBColor COLOR_BLACK = CRGBColor(0x00, 0x00, 0x00);
CRGBColor COLOR_BLUE = CRGBColor(0x00, 0x00, 0xFF);
CRGBColor COLOR_BLUE_1 = CRGBColor(0x40, 0x48, 0x73);
CRGBColor COLOR_RED = CRGBColor(0xFF, 0x00, 0x00);
CRGBColor COLOR_GREEN = CRGBColor(0x00, 0xFF, 0x00);
CRGBColor COLOR_YELLOW = CRGBColor(0xFF, 0xFF, 0x00);
CRGBColor COLOR_CYAN = CRGBColor(0x00, 0xFF, 0xFF);
CRGBColor COLOR_MAGENTA = CRGBColor(0xFF, 0x00, 0xFF);

// judb 
CRGBColor DEFAULT_TITLEBAR_COLOR = CRGBColor(0xA6, 0xAD, 0xD0);  // caption bar for CFrames
CRGBColor DEFAULT_TITLEBAR_TEXT_COLOR = CRGBColor(0x30, 0x38, 0x63); // text in titlebar
CRGBColor DEFAULT_BUTTON_COLOR = CRGBColor(0xC0, 0xC0, 0xCA); // (picture) buttons, scrollbar and dropdown buttons.
CRGBColor DEFAULT_TEXT_COLOR = CRGBColor(0x00, 0x00, 0x00); // text in labels, buttons...
CRGBColor ALTERNATE_TEXT_COLOR = CRGBColor(0x30, 0x38, 0x63); // a 2nd color for example text on groupboxes...
CRGBColor DEFAULT_CHECKBOX_COLOR = CRGBColor(0x40, 0x48, 0x73); // checkbox and radiobutton color
CRGBColor DEFAULT_CHECKBOX_BACK_COLOR = CRGBColor(0xFF, 0xFF, 0xFF); // checkbox and radiobutton background color
CRGBColor DEFAULT_SELECTION_COLOR = CRGBColor(0xA6, 0xAD, 0xD0);


}

