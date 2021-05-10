// wg_rect.cpp
//
// CRect class implementation
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


#include "wg_rect.h"
#include "wg_error.h"
#include "std_ex.h"
#include <algorithm>


namespace wGui
{


SDL_Rect CRect::SDLRect() const
{
	SDL_Rect sdlRect;
	//Normalize the rect...
	sdlRect.x = stdex::safe_static_cast<short int>(std::min(m_Left, m_Right));
	sdlRect.y = stdex::safe_static_cast<short int>(std::min(m_Top, m_Bottom));
	sdlRect.w = stdex::safe_static_cast<short int>(Width());
	sdlRect.h = stdex::safe_static_cast<short int>(Height());

	return sdlRect;
}


// assignment operator
CRect& CRect::operator=(const CRect& r)
{
	m_Top = r.Top();
	m_Left = r.Left();
	m_Right = r.Right();
	m_Bottom = r.Bottom();

	return *this;
}


CRect& CRect::operator=(CRect&& r)
{
	m_Top = r.Top();
	m_Left = r.Left();
	m_Right = r.Right();
	m_Bottom = r.Bottom();

	return *this;
}


CRect& CRect::operator+=(const CPoint& p)
{
  m_Left   += p.XPos();
  m_Right  += p.XPos();
  m_Top    += p.YPos();
  m_Bottom += p.YPos();
  return *this;
}


CRect CRect::operator+(const CPoint& p) const
{
	CRect result(*this);
  result += p;
	return result;
}


CRect& CRect::operator-=(const CPoint& p)
{
  m_Left   -= p.XPos();
  m_Right  -= p.XPos();
  m_Top    -= p.YPos();
  m_Bottom -= p.YPos();
	return *this;
}


CRect CRect::operator-(const CPoint& p) const
{
	CRect result(*this);
  result -= p;
	return result;
}


CRect& CRect::Grow(int iGrowAmount)
{
	m_Top -= iGrowAmount;
	m_Left -= iGrowAmount;
	m_Right += iGrowAmount;
	m_Bottom += iGrowAmount;

	return *this;
}


CRect& CRect::Move(int iOffsetX, int iOffsetY)
{
	m_Left += iOffsetX;
	m_Top += iOffsetY;
	m_Right += iOffsetX;
	m_Bottom += iOffsetY;

	return *this;
}


bool CRect::Overlaps(const CRect& r) const
{
	bool bOverlap = false;

	if (m_Right >= r.m_Left && m_Left <= r.m_Right && m_Top <= r.m_Bottom && m_Bottom >= r.m_Top)
	{
		bOverlap = true;
	}

	return bOverlap;
}


CRect& CRect::ClipTo(const CRect& r)
{

	if (! Overlaps(r))
	{
		m_Left = 0;
		m_Top = 0;
		m_Right = 0;
		m_Bottom = 0;
	}
	else
	{
		if (m_Left < r.m_Left)
		{
			m_Left = r.m_Left;
		}
		if (m_Top < r.m_Top)
		{
			m_Top = r.m_Top;
		}
		if (m_Right > r.m_Right)
		{
			m_Right = r.m_Right;
		}
		if (m_Bottom > r.m_Bottom)
		{
			m_Bottom = r.m_Bottom;
		}
	}

	return *this;
}


// test to see if the point lies within the rect
unsigned int CRect::HitTest(const CPoint& p) const
{
	unsigned int eRelPos = 0;

	eRelPos |= (p.XPos() < m_Left) ? RELPOS_LEFT : 0;
	eRelPos |= (p.YPos() < m_Top) ? RELPOS_ABOVE: 0;
	eRelPos |= (p.XPos() > m_Right) ? RELPOS_RIGHT : 0;
	eRelPos |= (p.YPos() > m_Bottom) ? RELPOS_BELOW: 0;
	eRelPos |= (p.XPos() >= m_Left && p.XPos() <= m_Right &&
		p.YPos() >= m_Top && p.YPos() <= m_Bottom) ? RELPOS_INSIDE : 0;

	return eRelPos;
}

}

