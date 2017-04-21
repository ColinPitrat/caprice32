// wg_progress.cpp
//
// CProgress class implementation
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


#include "wg_progress.h"
#include "std_ex.h"


namespace wGui
{

CProgress::CProgress(const CRect& WindowRect, CWindow* pParent, CRGBColor BarColor) :
	CRangeControl<int>(WindowRect, pParent, 0, 100, 1, 0),
	m_BarColor(BarColor)
{
	m_BackgroundColor = DEFAULT_FOREGROUND_COLOR;
	Draw();
}


CProgress::~CProgress() = default;


void CProgress::Draw() const
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CRect SubRect(m_WindowRect.SizeRect());
		SubRect.Grow(-1);
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		Painter.DrawRect(m_WindowRect.SizeRect(), false, COLOR_BLACK);
		Painter.DrawRect(SubRect, false, COLOR_LIGHTGRAY);
		Painter.DrawHLine(SubRect.Left(), SubRect.Right(), SubRect.Top(), COLOR_DARKGRAY);
		Painter.DrawVLine(SubRect.Top(), SubRect.Bottom(), SubRect.Left(), COLOR_DARKGRAY);
		SubRect.Grow(-2);
		if (m_Value > m_MinLimit)
		{
			if (m_Value < m_MaxLimit)
			{
				SubRect.SetRight(stdex::safe_static_cast<int>(SubRect.Left() +
					SubRect.Width() * (stdex::safe_static_cast<double>(m_Value - m_MinLimit) / (m_MaxLimit - m_MinLimit))));
			}
			Painter.DrawRect(SubRect, true, m_BarColor, m_BarColor);
		}
	}
}

}

