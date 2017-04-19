// wg_timer.h
//
// CTimer class interface
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


#ifndef _WG_TIMER_H_
#define _WG_TIMER_H_

#include "wg_message_client.h"


namespace wGui
{

//! For internal use only
//! \internal The callback used by the SDL Timer
Uint32 TimerCallback(Uint32 Interval, void* param);


//! A simple timer class

//! CTimer will post a CTRL_TIMER message every time the timer expires

class CTimer : public CMessageClient
{
public:
	//! Standard constructor
	//! \param pOwner A pointer to the timer 'owner'.  This is what the timer will set as the destination for it's messages.  Use 0 to broadcast the message.
	CTimer(CMessageClient* pOwner = nullptr);

	//! Standard destructor
	virtual ~CTimer();

	//! Start the timer. When the timer expires, it will post an CTRL_TIMER message
	//! \param Interval The time interval in milliseconds before the timer will expire
	//! \param bAutoRestart If this is true, the timer will restart again as soon as it expires
	void StartTimer(unsigned long int Interval, bool bAutoRestart = false);

	//! Stops the running timer
	void StopTimer();

	//! Indicates if the timer is currently running
	//! \return true is the timer is currently running
	bool IsRunning() { return m_TimerID != nullptr; }

	//! Gets the number of times the timer has triggered since it was last reset
	//! \return The count of times the timer has fired
	long int GetCount() const { return m_iCounter; }

	//! Resets the internal counter to zero
	void ResetCount() { m_iCounter = 0; }

	//! Gets the owner of the timer
	//! \return A pointer to the owner of the timer
	CMessageClient* GetOwner() { return m_pOwner; }

	//! For internal use only
	//! \internal This is where the SDL timer calls back to, and should not be used elsewhere
	Uint32 TimerHit(Uint32 Interval);


	// CMessageClient overrides
	//! Attempt to handle the given message
	//! \return true if the object handled the message (the message will not be given to any other handlers)
	virtual bool HandleMessage(CMessage* pMessage) override;


protected:
	//! The ID of the SDL timer used
	SDL_TimerID m_TimerID;

	//! If this is true, the timer will restart as soon as it expires
	bool m_bAutoRestart;

	//! A simple counter that increments each time the timer fires
	long int m_iCounter;

	//! A pointer the the timer's owner.  this is where messages are destined.
	CMessageClient* m_pOwner;
};

}


#endif // _WG_TIMER_H_

