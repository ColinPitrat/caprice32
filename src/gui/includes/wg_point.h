// wg_point.h
//
// CPoint class interface
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


#ifndef _WG_POINT_H_
#define _WG_POINT_H_

#include "std_ex.h"

namespace wGui
{

//! CPoint defines a point in cartestian (X, Y) space.

//! Screen coordinates are assumed, where the origin is in the top left corner of the screen
//! and Y increases in the downward direction

class CPoint
{
public:
	//! Initializes the point to (0, 0)
	CPoint() : m_XPos(0), m_YPos(0) { }

	//! Initialize the point to (x, y)
	//! \param x X coordinate
	//! \param y Y coordinate
	CPoint(const int x, const int y) : m_XPos(x), m_YPos(y) { }

	//! Copy constructor
	CPoint(const CPoint& p) : m_XPos(p.m_XPos), m_YPos(p.m_YPos) { }

	//! Standard Destructor
	virtual ~CPoint() { }

	//! Set the X coordinate
	//! \param x X coordinate
	void SetX(const int x) { m_XPos = x; }

	//! Set the Y coordinate
	//! \param y Y coordinate
	void SetY(const int y) { m_YPos = y; }

	//! Gets the X coordinate
	//! \return X coordinate
	int XPos(void) const { return m_XPos; }

	//! Gets the Y coordinate
	//! \return Y coordinate
	int YPos(void) const { return m_YPos; }

	//! Add the X and Y coordinates of the points
	CPoint operator+(const CPoint& p) const;

	//! Subtract the X and Y coordinates of the points
	CPoint operator-(const CPoint& p) const;

	//! Assign the value of point p to the point
	CPoint& operator=(const CPoint& p);

	//! Equality operator evaluates to true if the x and y coordinates are the same for both points
	bool operator==(const CPoint& p) const { return ((m_XPos == p.m_XPos) && (m_YPos == p.m_YPos)); }

	//! Inequality operator evaluates to true if the x and y coordinates are the same for both points
	bool operator!=(const CPoint& p) const { return ((m_XPos != p.m_XPos) || (m_YPos != p.m_YPos)); }

	//! Indicates if a point is to the left of the point
	//! \return true if the point is to the left of point p
	bool leftof(const CPoint& p) const { return (m_XPos < p.m_XPos); }

	//! Indicates if a point is to the right of the point
	//! \return true if the point is to the right of point p
	bool rightof(const CPoint& p) const { return (m_XPos > p.m_XPos); }

	//! Indicates if a point is above the point
	//! \return true if the point is above point p
	bool above(const CPoint& p) const { return (m_YPos < p.m_YPos); }

	//! Indicates if a point is below the point
	//! \return true if the point is below point p
	bool below(const CPoint& p) const { return (m_YPos > p.m_YPos); }

	//! Gives a string representation of the coordinates
	//! \return The coordinates in a string "<x>,<y>" i.e. "10,20"
	std::string ToString(void) const { return stdex::itoa(m_XPos) + "," + stdex::itoa(m_YPos); }

protected:
	//! The X coordinate
	int m_XPos;

	//! The Y coordinate
	int m_YPos;
};

}

#endif // _WG_POINT_H_

