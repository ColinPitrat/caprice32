// wg_painter.cpp
//
// CPainter class
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
#include "wg_painter.h"
#include "wg_error.h"
#include "std_ex.h"
#include <algorithm>
#include <math.h>

namespace wGui
{

CPainter::CPainter(SDL_Surface* pSurface, EPaintMode ePaintMode) :
	m_pSurface(pSurface),
	m_pWindow(nullptr),
	m_PaintMode(ePaintMode)
{
	if (!m_pSurface)
	{
		throw Wg_Ex_App("CPainter::CPainter : Invalid pointer to surface.");
	}
}


CPainter::CPainter(CWindow* pWindow, EPaintMode ePaintMode) :
	m_pSurface(nullptr),
	m_pWindow(pWindow),
	m_PaintMode(ePaintMode)
{
	if (!m_pWindow)
	{
		throw Wg_Ex_App("CPainter::CPainter : Invalid pointer to window.");
	}
	m_pSurface = pWindow->GetSDLSurface();
	if (!m_pSurface)
	{
		throw Wg_Ex_App("CPainter::CPainter : Invalid pointer to surface.");
	}
}


void CPainter::DrawHLine(int xStart, int xEnd, int y, const CRGBColor& LineColor)
{
	if (m_pWindow)
	{
		CPoint Offset = m_pWindow->GetClientRect().TopLeft();
		xStart += Offset.XPos();
		xEnd += Offset.XPos();
		y += Offset.YPos();
	}
	SDL_Rect Rect;
	Rect.x = stdex::safe_static_cast<short int>(std::min(xStart, xEnd));
	Rect.y = stdex::safe_static_cast<short int>(y);
	Rect.w = stdex::safe_static_cast<short int>(std::max(xEnd - xStart + 1, xStart - xEnd + 1));
	Rect.h = 1;
	SDL_FillRect(m_pSurface, &Rect, LineColor.SDLColor(m_pSurface->format));
}


void CPainter::DrawVLine(int yStart, int yEnd, int x, const CRGBColor& LineColor)
{
	if (m_pWindow)
	{
		CPoint Offset = m_pWindow->GetClientRect().TopLeft();
		yStart += Offset.YPos();
		yEnd += Offset.YPos();
		x += Offset.XPos();
	}
	SDL_Rect Rect;
	Rect.x = stdex::safe_static_cast<short int>(x);
	Rect.y = stdex::safe_static_cast<short int>(std::min(yStart, yEnd));
	Rect.w = 1;
	Rect.h = stdex::safe_static_cast<short int>(std::max(yEnd - yStart + 1, yStart - yEnd + 1));
	SDL_FillRect(m_pSurface, &Rect, LineColor.SDLColor(m_pSurface->format));
}

void CPainter::DrawRect(const CRect& Rect, bool bFilled, const CRGBColor& BorderColor, const CRGBColor& FillColor)
{
	CRect RealRect(Rect);
	if (m_pWindow)
	{
		RealRect = Rect + m_pWindow->GetClientRect().TopLeft();
		RealRect.ClipTo(m_pWindow->GetClientRect());
	}
	if (!bFilled || (BorderColor != FillColor))
	{
		DrawHLine(RealRect.Left(), RealRect.Right(), RealRect.Top(), BorderColor);
		DrawHLine(RealRect.Left(), RealRect.Right(), RealRect.Bottom(), BorderColor);
		DrawVLine(RealRect.Top(), RealRect.Bottom(), RealRect.Left(), BorderColor);
		DrawVLine(RealRect.Top(), RealRect.Bottom(), RealRect.Right(), BorderColor);
		RealRect.Grow(-1); //In case we have to fill the rect, then it's ready to go.
	}

	if (bFilled)
	{
		if (m_PaintMode == PAINT_REPLACE)
		{
			SDL_Rect FillRect = RealRect.SDLRect();
			SDL_FillRect(m_pSurface, &FillRect, FillColor.SDLColor(m_pSurface->format));
		}
		else
		{
			for (int iY = RealRect.Top(); iY <= RealRect.Bottom(); ++iY)
			{
				for (int iX = RealRect.Left(); iX <= RealRect.Right(); ++iX)
				{
					DrawPoint(CPoint(iX, iY), FillColor);
				}
			}
		}
	}
}

// judb draw a 'raised button' border based on the given color
void CPainter::Draw3DRaisedRect(const CRect& Rect, const CRGBColor& Color)
{
	CRect RealRect(Rect);
	if (m_pWindow)	{
		RealRect = Rect + m_pWindow->GetClientRect().TopLeft();
		RealRect.ClipTo(m_pWindow->GetClientRect());
	}
	DrawHLine(RealRect.Left(), RealRect.Right(), RealRect.Top(), Color * 1.6);
	DrawVLine(RealRect.Top(), RealRect.Bottom(), RealRect.Left(), Color * 1.6);
	DrawVLine(RealRect.Top(), RealRect.Bottom(), RealRect.Right(), Color * 0.3);
	DrawHLine(RealRect.Left(), RealRect.Right(), RealRect.Bottom(), Color * 0.3);
}

// judb draw a 'lowered button' border based on the given color
void CPainter::Draw3DLoweredRect(const CRect& Rect, const CRGBColor& Color)
{
	CRect RealRect(Rect);
	if (m_pWindow)	{
		RealRect = Rect + m_pWindow->GetClientRect().TopLeft();
		RealRect.ClipTo(m_pWindow->GetClientRect());
	}
	DrawHLine(RealRect.Left(), RealRect.Right(), RealRect.Top(), Color * 0.3);
	DrawHLine(RealRect.Left(), RealRect.Right(), RealRect.Bottom(), Color * 1.6);
	DrawVLine(RealRect.Top(), RealRect.Bottom(), RealRect.Left(), Color * 0.3);
	DrawVLine(RealRect.Top(), RealRect.Bottom(), RealRect.Right(), Color * 1.6);
}


void CPainter::DrawLine(const CPoint& Point1, const CPoint& Point2, const CRGBColor& LineColor)
{
	if (Point1.XPos() == Point2.XPos())
	{
		DrawVLine(Point1.YPos(), Point2.YPos(), Point1.XPos(), LineColor);
	}
	else
	{
		double iSlope = double(Point2.YPos() - Point1.YPos()) / (Point2.XPos() - Point1.XPos());
		if (iSlope <= 1 && iSlope >= -1)
		{
			CPoint StartPoint = (Point1.XPos() < Point2.XPos()) ? Point1 : Point2;
			CPoint EndPoint = (Point1.XPos() < Point2.XPos()) ? Point2 : Point1;
			for (int x = StartPoint.XPos(); x <= EndPoint.XPos(); ++x)
			{
				DrawPoint(CPoint(x, stdex::safe_static_cast<int>(StartPoint.YPos() + (x - StartPoint.XPos()) * iSlope)), LineColor);
			}
		}
		else
		{
			CPoint StartPoint = (Point1.YPos() < Point2.YPos()) ? Point1 : Point2;
			CPoint EndPoint = (Point1.YPos() < Point2.YPos()) ? Point2 : Point1;
			for (int y = StartPoint.YPos(); y <= EndPoint.YPos(); ++y)
			{
				DrawPoint(CPoint(stdex::safe_static_cast<int>(StartPoint.XPos() + (y - StartPoint.YPos()) / iSlope), y), LineColor);
			}
		}
	}
}

// judb draw box (filled)
void CPainter::DrawBox(CPoint UpperLeftPoint, int width, int height, const CRGBColor& LineColor)
{
	if (m_pWindow)
	{
		CPoint Offset = m_pWindow->GetClientRect().TopLeft();
        UpperLeftPoint = UpperLeftPoint + Offset;
	}
	SDL_Rect Rect = CRect(UpperLeftPoint, width, height).SDLRect();
	SDL_FillRect(m_pSurface, &Rect, LineColor.SDLColor(m_pSurface->format));
}


void CPainter::DrawPoint(const CPoint& Point, const CRGBColor& PointColor)
{
	CPoint RealPoint = (m_pWindow != nullptr) ? Point + m_pWindow->GetClientRect().TopLeft() : Point;
	if (CRect(0, 0, m_pSurface->w, m_pSurface->h).HitTest(RealPoint) == CRect::RELPOS_INSIDE)
	{
		LockSurface();
		Uint8* PixelOffset = static_cast<Uint8*>(m_pSurface->pixels) +
			m_pSurface->format->BytesPerPixel * RealPoint.XPos() + m_pSurface->pitch * RealPoint.YPos();
		switch (m_pSurface->format->BytesPerPixel)
		{
		case 1: // 8 bpp
			*reinterpret_cast<Uint8*>(PixelOffset) = static_cast<Uint8>(MixColor(ReadPoint(Point), PointColor).SDLColor(m_pSurface->format));
			break;
		case 2: // 16 bpp
			*reinterpret_cast<Uint16*>(PixelOffset) = static_cast<Uint16>(MixColor(ReadPoint(Point), PointColor).SDLColor(m_pSurface->format));
			break;
		case 3:  // 24 bpp
		{
			Uint32 PixelColor = MixColor(ReadPoint(Point), PointColor).SDLColor(m_pSurface->format);
			Uint8* pPixelSource = reinterpret_cast<Uint8*>(&PixelColor);
			Uint8* pPixelDest = reinterpret_cast<Uint8*>(PixelOffset);
			*pPixelDest = *pPixelSource;
			*(++pPixelDest) = *(++pPixelSource);
			*(++pPixelDest) = *(++pPixelSource);
			break;
		}
		case 4: // 32 bpp
			*reinterpret_cast<Uint32*>(PixelOffset) = static_cast<Uint32>(MixColor(ReadPoint(Point), PointColor).SDLColor(m_pSurface->format));
			break;
		default:
			throw(Wg_Ex_SDL("CPainter::DrawPoint : Unrecognized BytesPerPixel."));
			break;
		}
		UnlockSurface();
	}
}


CRGBColor CPainter::ReadPoint(const CPoint& Point)
{
	CPoint RealPoint = (m_pWindow != nullptr) ? Point + m_pWindow->GetClientRect().TopLeft() : Point;
	Uint32 PixelColor = 0;
	if (CRect(0, 0, m_pSurface->w, m_pSurface->h).HitTest(RealPoint) == CRect::RELPOS_INSIDE)
	{
		Uint8* PixelOffset = static_cast<Uint8*>(m_pSurface->pixels) +
			m_pSurface->format->BytesPerPixel * RealPoint.XPos() + m_pSurface->pitch * RealPoint.YPos();
		switch (m_pSurface->format->BytesPerPixel)
		{
		case 1: // 8 bpp
			PixelColor = *reinterpret_cast<Uint8*>(PixelOffset);
			break;
		case 2: // 16 bpp
			PixelColor = *reinterpret_cast<Uint16*>(PixelOffset);
			break;
		case 3: // 24 bpp
		{
			Uint8* pPixelDest = reinterpret_cast<Uint8*>(&PixelColor);
			Uint8* pPixelSource = reinterpret_cast<Uint8*>(PixelOffset);
			*pPixelDest = *pPixelSource;
			*(++pPixelDest) = *(++pPixelSource);
			*(++pPixelDest) = *(++pPixelSource);
			break;
		}
		case 4: // 32 bpp
			PixelColor = *reinterpret_cast<Uint32*>(PixelOffset);
			break;
		default:
			throw(Wg_Ex_SDL("CPainter::DrawPoint : Unrecognized BytesPerPixel."));
			break;
		}
	}
	return CRGBColor(&PixelColor, m_pSurface->format);
}


void CPainter::LockSurface(void)
{
	if (SDL_MUSTLOCK(m_pSurface))
	{
		if (SDL_LockSurface(m_pSurface) < 0)
		{
			SDL_Delay(10);
			if (SDL_LockSurface(m_pSurface) < 0)
			{
				throw(Wg_Ex_SDL("Unable to lock surface."));
			}
		}
	}
}


void CPainter::UnlockSurface(void)
{
	if (SDL_MUSTLOCK(m_pSurface))
	{
		SDL_UnlockSurface(m_pSurface);
	}
}


CRGBColor CPainter::MixColor(const CRGBColor& ColorBase, const CRGBColor& ColorAdd)
{
	CRGBColor MixedColor(COLOR_TRANSPARENT);
	switch (m_PaintMode)
	{
	case PAINT_IGNORE:
		MixedColor = ColorBase;
		break;
	case PAINT_REPLACE:
		MixedColor = ColorAdd;
		break;
	case PAINT_NORMAL:
		MixedColor = ColorBase.MixNormal(ColorAdd);
		break;
	case PAINT_AND:
		MixedColor = ColorBase & ColorAdd;
		break;
	case PAINT_OR:
		MixedColor = ColorBase | ColorAdd;
		break;
	case PAINT_XOR:
		MixedColor = ColorBase ^ ColorAdd;
		break;
	case PAINT_ADDITIVE:
		MixedColor = ColorBase + ColorAdd;
		break;
	}

	return MixedColor;
}


void CPainter::ReplaceColor(const CRGBColor& NewColor, const CRGBColor& OldColor)
{
	for (int y = 0; y < m_pSurface->h; ++y)
	{
		for (int x = 0; x < m_pSurface->w; ++x)
		{
			CPoint point(x, y);
			if(ReadPoint(point) == OldColor)
			{
				DrawPoint(point, NewColor);
			}
		}
	}
}


void CPainter::TransparentColor(const CRGBColor& TransparentColor)
{
	SDL_SetColorKey(m_pSurface, SDL_SRCCOLORKEY, TransparentColor.SDLColor(m_pSurface->format));
}

}

