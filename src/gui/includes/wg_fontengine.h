// wg_font.h
//
// CFontEngine interface
// CFontEngine uses the FreeType 2 library
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


#ifndef _WG_FONTENGINE_H_
#define _WG_FONTENGINE_H_

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <string>
#include <map>


namespace wGui
{

//! The CFont class is wGui's interface to FreeType2 and is used to render strings
//! This is used by the CRenderedString class, and shouldn't need to ever be called directly
//! \sa CRenderedString

class CFontEngine
{
public:
	//! Construct a new CFont object, using the specified font
	//! For most cases, there is no need to directly instantiate a CFontEngine object.
	//! CApplication provides a GetFontEngine() method which should be used
	//! \param sFontFileName The file that contains a file
	//! \param FontSize The size of the font (in points)
	CFontEngine(const std::string& sFontFileName, unsigned char FontSize);

	//! Standard destructor
	virtual ~CFontEngine();

	//! Renders the specified character
	//! \param Char The character to render
	//! \return A pointer to a FreeType glyph
	FT_BitmapGlyphRec* RenderGlyph(char Char);

	//! Returns the metrics for a specified character
	//! \param Char The character to render
	//! \return A pointer to a FreeType metrics structure
	FT_Glyph_Metrics* GetMetrics(char Char);


protected:
	static FT_Library m_FTLibrary;  //!< The FreeType library
	static bool m_bFTLibraryLoaded;  //!< Indicates if the FreeType library has been loaded
	FT_Face m_FontFace;  //!< The FreeType font face
	std::map<char, FT_BitmapGlyphRec> m_CachedGlyphMap;  //!< A cached map of the rendered glyphs
	std::map<char, FT_Glyph_Metrics> m_CachedMetricsMap;  //!< A cached map of the glyph metrics
};

}

#endif  // _WG_FONTENGINE_
