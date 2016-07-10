// wg_progress.h
//
// CProgress interface
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


#ifndef _WG_PROGRESS_H_
#define _WG_PROGRESS_H_

#include "wg_range_control.h"
#include "wg_painter.h"


namespace wGui
{

//! A progress bar display

class CProgress : public CRangeControl<int>
{
public:
	//! Constructs a progress bar, initilizes the limits to 0, and 100 with the progress at 0
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param BarColor The color of the progress bar, defaults to Blue
	CProgress(const CRect& WindowRect, CWindow* pParent, CRGBColor BarColor = COLOR_BLUE);

	//! Standard destructor
	virtual ~CProgress(void);

	//! Gets the color of the bar
	//! \return The bar color
	CRGBColor GetBarColor(void) { return m_BarColor; }

	//! Set the bar color
	//! \param BarColor The new bar color
	void SetBarColor(CRGBColor BarColor) { m_BarColor = BarColor; }


	// CWindow overrides
	//! Draws the progress bar
	virtual void Draw(void) const override;


protected:
	CRGBColor m_BarColor;  //!< The color of the progress bar


private:
	CProgress(const CProgress&) = delete;
	CProgress& operator=(const CProgress&) = delete;
};

}


#endif  // _WG_PROGRESS_H_

