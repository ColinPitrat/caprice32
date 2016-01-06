// wg_picture.cpp
//
// CPicture class implementation
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
#include "wg_picture.h"


namespace wGui
{

CPicture::CPicture(const CRect& WindowRect, CWindow* pParent, const std::string& sPictureFile,
		bool bDrawBorder, const CRGBColor& BorderColor) :
	CWindow(WindowRect, pParent),
	m_bDrawBorder(bDrawBorder),
	m_BorderColor(BorderColor),
	m_hBitmap(CBitmapFileResourceHandle(sPictureFile))
{
	if (m_bDrawBorder)
	{
		m_ClientRect.Grow(-1);
	}
	Draw();
}


CPicture::CPicture(const CRect& WindowRect, CWindow* pParent, const CBitmapResourceHandle& hBitmap,
		bool bDrawBorder, const CRGBColor& BorderColor) :
	CWindow(WindowRect, pParent),
	m_bDrawBorder(bDrawBorder),
	m_BorderColor(BorderColor),
	m_hBitmap(hBitmap)
{
	if (m_bDrawBorder)
	{
		m_ClientRect.Grow(-1);
	}
	Draw();
}


CPicture::~CPicture(void)
{

}


void CPicture::Draw(void) const
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		SDL_Rect SourceRect = m_ClientRect.SizeRect().SDLRect();
		SDL_Rect DestRect = m_ClientRect.SDLRect();
		SDL_BlitSurface(m_hBitmap.Bitmap(), &SourceRect, m_pSDLSurface, &DestRect);

		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		if (m_bDrawBorder) {
			Painter.DrawRect(m_WindowRect.SizeRect(), false, m_BorderColor);
		}
	}
}


void CPicture::SetWindowRect(const CRect& WindowRect)
{
	CWindow::SetWindowRect(WindowRect);
	m_ClientRect = m_WindowRect.SizeRect();
	m_ClientRect.Grow(-1);
}

}

