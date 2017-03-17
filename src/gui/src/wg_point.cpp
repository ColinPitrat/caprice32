// wg_point.cpp
//
// CPoint class implementation
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


#include "wg_point.h"


namespace wGui
{

// CPoint class

// add the points
CPoint CPoint::operator+(const CPoint& p) const
{
	CPoint result;
	result.SetX(m_XPos + p.XPos());
	result.SetY(m_YPos + p.YPos());

	return result;
}


// subtract the points
CPoint CPoint::operator-(const CPoint& p) const
{
	CPoint result;
	result.SetX(m_XPos - p.XPos());
	result.SetY(m_YPos - p.YPos());

	return result;
}


// assignment operator
CPoint& CPoint::operator=(const CPoint& p)
{
	m_XPos = p.XPos();
	m_YPos = p.YPos();

	return *this;
}

}

