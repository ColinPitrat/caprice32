// wg_message_server.cpp
//
// CMessageServer class implementation
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


#include "wg_message_server.h"
#include "wg_application.h"
#include "std_ex.h"
#include "wg_message_client.h"
#include "wg_error.h"
#include "wutil_debug.h"
#include <sstream>
#include <algorithm>
#include <functional>


namespace wGui
{

CMessageServer* CMessageServer::m_pInstance = nullptr;


CMessageServer::CMessageServer(void) : m_bIgnoreAllNewMessages(true)
{
	m_pSemaphore = SDL_CreateSemaphore(0);
}


CMessageServer::~CMessageServer(void)
{

}


CMessageServer& CMessageServer::Instance(void)
{
	if (!m_pInstance)
	{
		m_pInstance = new CMessageServer;
		if (!m_pInstance)
		{
			throw(Wg_Ex_App("Unable to instantiate Message Server!", "CMessageServer::Instance"));
		}
	}

	return *m_pInstance;
}


void CMessageServer::RegisterMessageClient(CMessageClient* pClient, CMessage::EMessageType eMessageType, unsigned char Priority)
{
	if (!pClient)
	{
		CApplication::Instance()->GetApplicationLog().AddLogEntry(
			"CMessageServer::RegisterMessageClient : Attempting to register a non-existent message client.", APP_LOG_ERROR);
	}
	else
	{
		m_MessageClients[eMessageType].insert(std::make_pair(Priority, s_MessageClientActive(pClient, false)));
	}
}


void CMessageServer::DeregisterMessageClient(CMessageClient* pClient, CMessage::EMessageType eMessageType)
{
	t_MessageClientPriorityMap& PriorityMap = m_MessageClients[eMessageType];
	t_MessageClientPriorityMap::iterator iter = PriorityMap.begin();
	while (iter != PriorityMap.end())
	{
		if (iter->second.pClient == pClient)
		{
			PriorityMap.erase(iter);
			iter = PriorityMap.begin();
		}
		else
		{
			++iter;
		}
	}
}


void CMessageServer::DeregisterMessageClient(CMessageClient* pClient)
{
	for (t_MessageClientMap::iterator iter = m_MessageClients.begin(); iter != m_MessageClients.end(); ++iter)
	{
		t_MessageClientPriorityMap::iterator iter2 = iter->second.begin();
		while (iter2 != iter->second.end())
		{
			if (iter2->second.pClient == pClient)
			{
				iter->second.erase(iter2);
				iter2 = iter->second.begin();
			}
			else
			{
				++iter2;
			}
		}
	}
}


void CMessageServer::DeliverMessage(void)
{
	if (m_MessageQueue.size() > 0)
	{
		CMessage* pMessage = m_MessageQueue.front();
		t_MessageClientPriorityMap& PriorityMap = m_MessageClients[pMessage->MessageType()];

		// we have to make sure that each client only gets the message once,
		// even if the handling of one of these messages changes the message map
		for (t_MessageClientPriorityMap::iterator iter = PriorityMap.begin(); iter != PriorityMap.end(); ++iter)
		{
			iter->second.bWaitingForMessage = true;
		}

		bool bFinished = false;
		while (! bFinished)
		{
			t_MessageClientPriorityMap::iterator iter = PriorityMap.begin();
			for (; iter != PriorityMap.end(); ++iter)
			{
				if (iter->second.bWaitingForMessage)
				{
					iter->second.bWaitingForMessage = false;
					bFinished = iter->second.pClient->HandleMessage(pMessage);
					break;
				}
			}
			if (iter == PriorityMap.end())
			{
				bFinished = true;
			}
		}

		m_MessageQueue.pop_front();
		delete pMessage;
	}
}


//! A functor for finding duplicate APP_PAINT messages

struct Duplicate_APP_PAINT : public std::unary_function<CMessage*, bool>
{
public:
	//! The functor constructor
	//! \param pClient The destination of the message that is being checked
	Duplicate_APP_PAINT(const CMessageClient* pClient) : m_pClient(pClient) { }
	//! Checks to see if the message is a duplicate of an existing APP_PAINT message
	//! \param pMessage A pointer to the message that is being checked against
	//! \return true of the message is a duplicate
	bool operator() (CMessage* pMessage) const
	{
		bool bResult = (pMessage->MessageType() == CMessage::APP_PAINT) && (pMessage->Destination() == m_pClient);
		return bResult;
	}

private:
	//! The destination of the message that is being checked
	const CMessageClient* m_pClient;
};


void CMessageServer::QueueMessage(CMessage* pMessage)
{
	if (!m_bIgnoreAllNewMessages)
	{
		// check for and remove any redundant APP_PAINT messages in the queue
		if (pMessage->MessageType() == CMessage::APP_PAINT)
		{
			m_MessageQueue.erase(std::remove_if(m_MessageQueue.begin(), m_MessageQueue.end(), Duplicate_APP_PAINT(pMessage->Destination())), m_MessageQueue.end());
		}

		m_MessageQueue.push_back(pMessage);
	}
}

void CMessageServer::PurgeQueuedMessages()
{
  m_MessageQueue.clear();
}

}
