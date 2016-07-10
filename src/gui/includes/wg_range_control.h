// wg_range_control.h
//
// CRangeControl interface
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


#ifndef _WG_RANGE_CONTROL_H_
#define _WG_RANGE_CONTROL_H_

#include "wg_window.h"
#include "wg_message_server.h"


namespace wGui
{

//! A template class that handles all the basics of a control that uses a value that is constrained to a certain range
//! Sends a CTRL_VALUECHANGE message whenever the value changes

template<typename T>
class CRangeControl : public CWindow
{
public:
	//! Constructs a range control
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	//! \param minLimit The minimum limit for the control
	//! \param maxLimit The maximum limit for the control
	//! \param stepSize The amount to increase/decrease the value by for Increment and Decrement
	//! \param defaultValue The amount to initialize the value to
	CRangeControl(const CRect& WindowRect, CWindow* pParent, T minLimit, T maxLimit, T stepSize, T defaultValue) :
		CWindow(WindowRect, pParent), m_MinLimit(minLimit), m_MaxLimit(maxLimit), m_StepSize(stepSize), m_Value(defaultValue) { }

	//! Set the lower limit for the control
	//! \param minLimit The lower limit of the control
	virtual void SetMinLimit(T minLimit) { m_MinLimit = minLimit; }

	//! Gets the lower limit of the control
	//! \return The minimum limit of the control
	virtual T GetMinLimit(void) const { return m_MinLimit; }

	//! Set the upper limit for the control
	//! \param maxLimit The upper limit of the control
	virtual void SetMaxLimit(T maxLimit) { m_MaxLimit = maxLimit; }

	//! Gets teh upper limit of the control
	//! \return The maximum limit of the control
	virtual T GetMaxLimit(void) const { return m_MaxLimit; }

	//! Set the current step size.
	//! \param stepSize The amount to increment the value by for Increment() and Decrement() calls
	virtual void SetStepSize(T stepSize) { m_StepSize = stepSize; }

	//! Gets the current step size of the control
	//! \return The current step size
	virtual T GetStepSize(void) const { return m_StepSize; }

	//! Set the current value.
	//! \param value The new value for the control
	//! \param bRedraw indicates if the control should be redrawn (defaults to true)
	virtual void SetValue(T value, bool bRedraw = true)
	{
		m_Value = ConstrainValue(value);
		CMessageServer::Instance().QueueMessage(new CValueMessage<T>(CMessage::CTRL_VALUECHANGE, m_pParentWindow, this, m_Value));

		if (bRedraw)
		{
			Draw();
		}
	}

	//! Gets the current value of the control
	//! \return The current value
	virtual T GetValue(void) const { return m_Value; }

	//! Increase the value by one step size
	//! \param bRedraw indicates if the control should be redrawn (defaults to true)
	virtual void Increment(bool bRedraw = true) { SetValue(m_Value + m_StepSize, bRedraw); }

	//! Decrease the value by one step size
	//! \param bRedraw indicates if the control should be redrawn (defaults to true)
	virtual void Decrement(bool bRedraw = true) { SetValue(m_Value - m_StepSize, bRedraw); }

	//! takes the value and makes sure it's in it's limits
	//! \param value The value to be checked
	//! \return The closest value that's within the limits
	virtual T ConstrainValue(T value) const
	{
		if (value < m_MinLimit)
		{
			value = m_MinLimit;
		}
		if (value > m_MaxLimit)
		{
			value = m_MaxLimit;
		}
		return value;
	}


protected:
	T m_MinLimit;  //!< The minimum value of the control
	T m_MaxLimit;  //!< The maximum value of the control
	T m_StepSize;  //!< The step size of the control
	T m_Value;  //!< The current value of the control


private:
	CRangeControl& operator=(CRangeControl) = delete;
};

}


#endif  // _WG_RANGE_CONTROL_H_

