// wg_fontengine.cpp
//
// CFontEngine implementation
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


#include "wg_fontengine.h"
#include "wg_error.h"
#include "wg_application.h"


namespace wGui {

// Static members
FT_Library CFontEngine::m_FTLibrary;
bool CFontEngine::m_bFTLibraryLoaded = false;


CFontEngine::CFontEngine(const std::string& sFontFileName, unsigned char FontSize)
{
	if (!m_bFTLibraryLoaded)
	{
		if (FT_Init_FreeType(&m_FTLibrary))
		{
			throw(Wg_Ex_FreeType("Unable to initialize FreeType library.", "CFontEngine::CFontEngine"));
		}
		m_bFTLibraryLoaded = true;
	}
	if (FT_New_Face(m_FTLibrary, sFontFileName.c_str(), 0, &m_FontFace))
	{
		throw(Wg_Ex_FreeType("Unable to create font face.", "CFontEngine::CFontEngine"));
	}
	if (FT_Set_Char_Size(m_FontFace, 0, FontSize * 64, 0, 0))
	{
		throw(Wg_Ex_FreeType("Unable to set character size.", "CFontEngine::CFontEngine"));
	}
	CApplication::Instance()->GetApplicationLog().
		AddLogEntry("CFontEngine - Loaded new font : " + stdex::itoa(FontSize) + " point, " + sFontFileName, APP_LOG_INFO);
}


CFontEngine::~CFontEngine()
{

	FT_Done_Face(m_FontFace);
}

FT_BitmapGlyphRec* CFontEngine::RenderGlyph(char Char)
{
	auto glyphIter = m_CachedGlyphMap.find(Char);
	if (glyphIter == m_CachedGlyphMap.end())
	{
		if (FT_Load_Char(m_FontFace, Char, FT_LOAD_DEFAULT))
		{
			throw(Wg_Ex_FreeType("Unable to render glyph.", "CFontEngine::RenderGlyph"));
		}
		FT_Glyph glyph;
		if (FT_Get_Glyph(m_FontFace->glyph, &glyph))
		{
			throw(Wg_Ex_FreeType("Unable to copy glyph.", "CFontEngine::RenderGlyph"));
		}
		if (FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, nullptr, 1))
		{
			throw(Wg_Ex_FreeType("Unable to render glyph.", "CFontEngine::RenderGlyph"));
		}
		glyphIter = m_CachedGlyphMap.insert(std::make_pair(Char, *reinterpret_cast<FT_BitmapGlyph>(glyph))).first;
	}
	return &(glyphIter->second);
}

FT_Glyph_Metrics* CFontEngine::GetMetrics(char Char)
{
	auto glyphIter = m_CachedMetricsMap.find(Char);
	if (glyphIter == m_CachedMetricsMap.end())
	{
		if (FT_Load_Char(m_FontFace, Char, FT_LOAD_DEFAULT))
		{
			throw(Wg_Ex_FreeType("Unable to render glyph.", "CFontEngine::RenderGlyph"));
		}
		glyphIter = m_CachedMetricsMap.insert(std::make_pair(Char, m_FontFace->glyph->metrics)).first;
	}
	return &(glyphIter->second);
}

}
