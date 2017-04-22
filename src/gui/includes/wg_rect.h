// wg_rect.h
//
// CRect class interface
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


#ifndef _WG_RECT_H_
#define _WG_RECT_H_

#include "wg_point.h"
#include "std_ex.h"
#include "SDL.h"
#include <stdlib.h>
#include <math.h>

namespace wGui
{

//! A representation of a rectangle

class CRect
{
public:
	//! The default constructor will initialize all 4 corners to (0, 0)
	CRect() : m_Left(0), m_Right(0), m_Top(0), m_Bottom(0) { }

	//! \param left The left edge of the rectangle
	//! \param top The top edge of the rectangle
	//! \param right The right edge of the rectangle
	//! \param bottom The bottom edge of the rectangle
	CRect(const int left, const int top, const int right, const int bottom) :
		m_Left(left), m_Right(right), m_Top(top), m_Bottom(bottom) { }  // constructor

	//! Create a CRect using a pair of CPoints that represent the Top-Left, and Bottom-Right corners
	//! \param p1 Top left corner
	//! \param p2 Bottom right corner
	CRect(const CPoint& p1, const CPoint& p2) :
		m_Left(p1.XPos()), m_Right(p2.XPos()), m_Top(p1.YPos()), m_Bottom(p2.YPos()) { }  // constructor

    // judb create a CRect using a point (upper-left corner) , width and height.
	CRect(const CPoint& p, const int width, const int height) :
		m_Left(p.XPos()), m_Right(p.XPos() + width -1), m_Top(p.YPos()), m_Bottom(p.YPos() + height - 1) { }


	//! Copy constructor
	//! \param r A CRect that thie new CRect will be copied from
	CRect(const CRect& r) = default;

	//! Standard Destructor
	virtual ~CRect() = default;

	//! Set the Top poisition
	//! \param top The new Top coordinate
	void SetTop(const int top) { m_Top = top; }

	//! Set the Left poisition
	//! \param left The new Left coordinate
	void SetLeft(const int left) { m_Left = left; }

	//! Set the Right poisition
	//! \param right The new Right coordinate
	void SetRight(const int right) { m_Right = right; }

	//! Set the Bottom poisition
	//! \param bottom The new Bottom coordinate
	void SetBottom(const int bottom) { m_Bottom = bottom; }

	//! Gets the top of the rectangle
	//! \return The Top position
	int Top() const { return m_Top; }

	//! Gets the left of the rectangle
	//! \return The Left position
	int Left() const { return m_Left; }

	//! Gets the right of the rectangle
	//! \return The Right position
	int Right() const { return m_Right; }

	//! Gets the bottom of the rectangle
	//! \return The Bottom position
	int Bottom() const { return m_Bottom; }

	//! Gets the top-left corner of the rectangle
	//! \return A point representing the Top Left corner of the CRect
	CPoint TopLeft() const { return CPoint(m_Left, m_Top); }

	//! Gets the top-right corner of the rectangle
	//! \return A point representing the Top Right corner of the CRect
	CPoint TopRight() const { return CPoint(m_Right, m_Top); }

	//! Gets the bottom left corner of the rectangle
	//! \return A point representing the Bottom Left corner of the CRect
	CPoint BottomLeft() const { return CPoint(m_Left, m_Bottom); }

	//! Gets the bottom-right corner of the rectangle
	//! \return A point representing the Bottom Right corner of the CRect
	CPoint BottomRight() const { return CPoint(m_Right, m_Bottom); }

	//! Gets the center of the rectangle
	//! \return A point representing the center of the CRect
	CPoint Center() const { return CPoint((m_Left + m_Right) / 2, (m_Top + m_Bottom) / 2); }

	//! Gets the left side's center of the rectangle
	//! \return A point representing the CenterLeft point of the CRect
	CPoint CenterLeft() const { return CPoint( m_Left, (m_Top + m_Bottom) / 2); }

	//! Get the top's center of the rectangle
	//! \return A point representing the CenterTop point of the CRect
	CPoint CenterTop() const { return CPoint( (m_Left + m_Right) / 2, m_Top ); }

	//! Gets the bottom's center of the rectangle
	//! \return A point representing the Bottom Left corner of the CRect
	CPoint CenterBottom() const { return CPoint( (m_Left + m_Right) / 2, m_Bottom ); }

	//! Gets the right side's center of the rectangle
	//! \return A point representing the Bottom Right corner of the CRect
	CPoint CenterRight() const { return CPoint( m_Right, (m_Top + m_Bottom) / 2); }


	//! Converts the CRect into a SDL style rect
	//! \return An SDL_Rect of the same size
	SDL_Rect SDLRect() const;

	//! Gets the width of the rectangle
	//! \return The width (along the X axis) of the CRect
	int Width() const { return abs(m_Right - m_Left + 1); }

	//! Gets the height of the rectangle
	//! \return The height (along the Y axis) of the CRect
	int Height() const { return abs(m_Bottom - m_Top + 1); }

	//! Creates a CRect that has the same width and height of the rect, but has 0, 0 as it's top left coordinate
	//! \return A CRect
	CRect SizeRect() const { return CRect(0, 0, abs(m_Right - m_Left), abs(m_Bottom - m_Top)); }

	//! Assignment operator will copy the values of the other rect
	CRect& operator=(const CRect& r);  // assignment operator

	//! Addition operator to add a CPoint, will offset the CRect
	//! \param p A point to offset the CRect by
	CRect operator+(const CPoint& p) const;

	//! Addition operator to add a CPoint, will offset the CRect
	//! \param p A point to offset the CRect by
	CRect& operator+=(const CPoint& p);

	//! Subtraction operator to subtract a CPoint, will offset the CRect
	//! \param p A point to offset the CRect by
	CRect operator-(const CPoint& p) const;

	//! Subtraction operator to subtract a CPoint, will offset the CRect
	//! \param p A point to offset the CRect by
	CRect& operator-=(const CPoint& p);

	//! Equality operator
	//! \param r The rect to compare to
	bool operator==(const CRect& r) const
		{ return (m_Left == r.m_Left && m_Top == r.m_Top && m_Right == r.m_Right && m_Bottom == r.m_Bottom); }

	//! Inequality operator
	//! \param r The rect to compare to
	bool operator!=(const CRect& r) const
		{ return (m_Left != r.m_Left || m_Top != r.m_Top || m_Right != r.m_Right || m_Bottom != r.m_Bottom); }

	//! Grow will increase (or decrease) all of the dimensions by the given amount.
	//! This means that for a rect 20 wide by 10 tall, Grow(1) will increase the size to 22 wide, 12 tall.
	//! (each side is moved out by 1)
	//! \param iGrowAmount The amount to grow the CRect's dimensions by, negative values can be used to shrink the rect
	//! \return A reference to the object
	CRect& Grow(int iGrowAmount);

	//! Move will move the rect by a offset specified
	//! \param iOffsetX how many pixel to move on X axis ( + or - )
	//! \param iOffsetY how many pixel to move on Y axis ( + or - )
	//! \return A reference to the object
	CRect& Move(int iOffsetX, int iOffsetY);

	//! Tests to see if the two CRects overlap
	//! \param r The other CRect to test with
	//! \return true if the CRects overlap
	bool Overlaps(const CRect& r) const;

	//! Clips the CRect to fit in another CRect
	//! \param r The CRect to clip to
	CRect& ClipTo(const CRect& r);

	enum ERelativePosition
	{
		RELPOS_INVALID = 0, //!< This usually indicates some form of error
		RELPOS_ABOVE = 1, //!< The point is above the top of the CRect
		RELPOS_BELOW = 2, //!< The point is below the bottom of the CRect
		RELPOS_LEFT = 4, //!< The point is to the left of the CRect
		RELPOS_RIGHT = 8, //!< The point is to the right of the CRect
		RELPOS_INSIDE = 16 //!< The point lies within the bounds of the CRect
	};

	//! The HitTest will test to see where a point is in relation to the rect
	//! \param p The point to test against the CRect
	//! \return The appropriate values of the ERelativePosition enum are ORed together
	unsigned int HitTest(const CPoint& p) const;

	//! Returns the coordinates of the rectangle as a string
	//! \return A std::string with the coordinates listed as "<left>,<top>,<right>,<bottom>",  i.e. "1,2,3,4"
	std::string ToString() const
		{ return stdex::itoa(m_Left) + "," + stdex::itoa(m_Top) + "," + stdex::itoa(m_Right) + "," + stdex::itoa(m_Bottom); }

protected:
	//! X position of the left border
	int m_Left;

	//! X position of the right border
	int m_Right;

	//! Y position of the top border
	int m_Top;

	//! Y position of the bottom border
	int m_Bottom;
};

}


#endif // _WG_RECT_H_

