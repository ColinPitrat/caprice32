// wg_timer.cpp
//
// CTimer class implementation
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


#include "wg_timer.h"
#include "wg_message_server.h"


namespace wGui
{

Uint32 TimerCallback(Uint32 Interval, void* param)
{
	return static_cast<CTimer*>(param)->TimerHit(Interval);
}


CTimer::CTimer(CMessageClient* pOwner) :
	m_TimerID(nullptr),
	m_bAutoRestart(false),
	m_iCounter(0),
	m_pOwner(pOwner)
{ }


CTimer::~CTimer(void)
{
	StopTimer();
}


void CTimer::StartTimer(unsigned long int Interval, bool bAutoRestart)
{
	m_bAutoRestart = bAutoRestart;
	if (m_TimerID != nullptr)
	{
		StopTimer();
	}
	m_TimerID = SDL_AddTimer(Interval, &TimerCallback, this);
}


void CTimer::StopTimer(void)
{
	if (m_TimerID)
	{
		SDL_RemoveTimer(m_TimerID);
		m_TimerID = nullptr;
	}
}


Uint32 CTimer::TimerHit(Uint32 Interval)
{
	m_iCounter++;
	CMessageServer::Instance().QueueMessage(new TIntMessage(CMessage::CTRL_TIMER, m_pOwner, this, m_iCounter));
	if (!m_bAutoRestart)
	{
		StopTimer();
	}

	return Interval;
}


bool CTimer::HandleMessage(CMessage* /*pMessage*/)
{
	bool bHandled = false;

	return bHandled;
}

}

