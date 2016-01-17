// wg_renderedstring.cpp
//
// CRenderedString implementation
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


#include "wgui_include_config.h"
#include "wg_renderedstring.h"
#include "wg_painter.h"
#include "wg_error.h"
#include <algorithm>

namespace wGui
{

CRenderedString::CRenderedString(CFontEngine* pFontEngine, const std::string& sString, EVAlign eVertAlign, EHAlign eHorzAlign) :
	m_pFontEngine(pFontEngine),
	m_sString(sString),
	m_MaskChar(' '),
	m_eVertAlign(eVertAlign),
	m_eHorzAlign(eHorzAlign),
	m_bCachedMetricsValid(false),
	m_MaxFontHeight(-1),
  m_MaxFontWidth(-1)
{
	if (! m_pFontEngine)
	{
		throw(Wg_Ex_App("CRenderedString::CRenderedString : Bad pFontEngine pointer! (This is usually the result of the wgui.conf file missing or misconfigured.  See the Global Config section of the docs.)"));
	}
}


void CRenderedString::Draw(SDL_Surface* pSurface, const CRect& BoundingRect, const CPoint& OriginPoint, const CRGBColor& FontColor) const
{
	CPoint OriginOffset;
	std::vector<CRect> CharacterRects;
	GetMetrics(0, &OriginOffset, &CharacterRects);
	for (unsigned int i = 0; i < m_sString.size(); ++i)
	{
		FT_BitmapGlyphRec* pGlyph;
		if (m_MaskChar == ' ')
		{
			pGlyph = m_pFontEngine->RenderGlyph(m_sString[i]);
		}
		else
		{
			pGlyph = m_pFontEngine->RenderGlyph(m_MaskChar);
		}
		CPainter Painter(pSurface, CPainter::PAINT_NORMAL);
		for (unsigned int y = 0; y < static_cast<unsigned int>(pGlyph->bitmap.rows); ++y)
		{
			for (unsigned int x = 0; x < static_cast<unsigned int>(pGlyph->bitmap.width); ++x)
			{
				unsigned char* PixelOffset = pGlyph->bitmap.buffer + y * pGlyph->bitmap.width + x;
				if (*PixelOffset != 0x00)
				{
					CRGBColor PixelColor(FontColor.red, FontColor.green, FontColor.blue, *PixelOffset);
					CPoint PixelPoint(CPoint(x + pGlyph->left, y) + OriginPoint + OriginOffset + CharacterRects.at(i).TopLeft());
					if (BoundingRect.HitTest(PixelPoint) == CRect::RELPOS_INSIDE)
					{
						Painter.DrawPoint(PixelPoint, PixelColor);
					}
				}
			}
		}
	}
}

unsigned int CRenderedString::GetMaxFontHeight()
{
	if (m_MaxFontHeight < 0)
	{
		int maxHeight=0;
		FT_Glyph_Metrics* pMetrics;
		for(int i = 0; i < 256; i++)
		{
			pMetrics = m_pFontEngine->GetMetrics((char)i);
			if ((pMetrics->height >> 6) > maxHeight)
			{
				maxHeight = (pMetrics->height >> 6);
			}
		}
		m_MaxFontHeight = maxHeight;
	}
	return m_MaxFontHeight;
}

unsigned int CRenderedString::GetMaxFontWidth()
{
	if (m_MaxFontWidth < 0)
	{
		int maxWidth = 0;
		FT_Glyph_Metrics* pMetrics;
		for(int i = 0; i < 256; i++)
		{
			pMetrics = m_pFontEngine->GetMetrics((char)i);
			if ((pMetrics->width >> 6) > maxWidth)
			{
				maxWidth = (pMetrics->width >> 6);
			}
		}
		m_MaxFontWidth = maxWidth;
	}
	return m_MaxFontWidth;
}


unsigned int CRenderedString::GetWidth(std::string sText)
{
    int totalWidth = 0;
	FT_Glyph_Metrics* pMetrics;
	for(unsigned i = 0; i < sText.length(); i++)
	{
		pMetrics = m_pFontEngine->GetMetrics(sText[i]);
		totalWidth += (pMetrics->horiAdvance >> 6);
	}
	return totalWidth;
}


void CRenderedString::GetMetrics(CPoint* pBoundedDimensions, CPoint* pOriginOffset, std::vector<CRect>* pCharacterRects) const
{
	if (! m_bCachedMetricsValid)
	{
		m_CachedCharacterRects.clear();

		int iMinY = 0;
		int iMaxY = 0;
		int iLength = 0;
		for (unsigned int i = 0; i < m_sString.size(); ++i)
		{
			FT_Glyph_Metrics* pMetrics;
			if (m_MaskChar == ' ')
			{
				pMetrics = m_pFontEngine->GetMetrics(m_sString[i]);
			}
			else
			{
				pMetrics = m_pFontEngine->GetMetrics(m_MaskChar);
			}

			if ((pMetrics->horiBearingY - pMetrics->height) < iMinY)
			{
			    // judb I think this should always be 0 (when vertical-aligning, don't count the part of the character
                // below the 'baseline' for example in case of g, j, p ...
                iMinY = 0; //pMetrics->horiBearingY - pMetrics->height;
			}
			if (pMetrics->horiBearingY > iMaxY)
			{
				iMaxY = pMetrics->horiBearingY;
			}
			iLength += (pMetrics->horiAdvance);
			// The top and bottom values of the rect are not actually in rect coordinates at this point, since iMaxY and iMinY are not yet know
			m_CachedCharacterRects.push_back(
				CRect((iLength - pMetrics->horiAdvance) >> 6, pMetrics->horiBearingY >> 6, iLength >> 6, pMetrics->height >> 6));
		}

		iMinY = iMinY >> 6;
		iMaxY = iMaxY >> 6;
		iLength = iLength >> 6;

		// now fix the top and bottom values of the rects
		for(std::vector<CRect>::iterator iter = m_CachedCharacterRects.begin(); iter != m_CachedCharacterRects.end(); ++iter)
		{
			iter->SetTop(iMaxY - iter->Top());
			iter->SetBottom(iter->Top() + iter->Bottom());
		}

		// Tack an empty rect on the end
		m_CachedCharacterRects.push_back(CRect(iLength, iMaxY, iLength, iMinY));

		m_CachedBoundedDimensions = CPoint(iLength, iMaxY - iMinY);

		switch (m_eHorzAlign)
		{
		case HALIGN_CENTER:
			m_OriginOffset.SetX(-iLength / 2);
			break;
		case HALIGN_RIGHT:
			m_OriginOffset.SetX(-iLength);
			break;
		case HALIGN_LEFT:
		default:
			m_OriginOffset.SetX(0);
			break;
		}

		switch (m_eVertAlign)
		{
		case VALIGN_TOP:
			m_OriginOffset.SetY(0);
			break;
		case VALIGN_BOTTOM:
			m_OriginOffset.SetY(iMinY - iMaxY);
			break;
		case VALIGN_CENTER:
			m_OriginOffset.SetY((iMinY - iMaxY) / 2);
			break;
		case VALIGN_NORMAL:
		default:
			m_OriginOffset.SetY(-iMaxY);
			break;
		}

		m_bCachedMetricsValid = true;
	}

	if (pBoundedDimensions)
	{
		*pBoundedDimensions = m_CachedBoundedDimensions;
	}

	if (pOriginOffset)
	{
		*pOriginOffset = m_OriginOffset;
	}

	if (pCharacterRects)
	{
		*pCharacterRects = m_CachedCharacterRects;
	}
}

}

