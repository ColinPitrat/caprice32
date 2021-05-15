// wg_message_server.h
//
// CMessageServer interface
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


#ifndef _WG_MESSAGE_SERVER_H_
#define _WG_MESSAGE_SERVER_H_


#include "wg_message.h"
#include <deque>
#include <map>
#include "SDL.h"
#include "SDL_thread.h"


namespace wGui
{

class CMessageClient;

//! A struct that associates message client pointers with a flag that indicates if the client has recieved a particular message
struct s_MessageClientActive
{
	//! struct constructor
	//! \param pC a pointer to the message client
	//! \param bW a boolean indicating if the client has gotten the current message
	s_MessageClientActive(CMessageClient* pC, bool bW) :
		pClient(pC), bWaitingForMessage(bW)
	{ }

	CMessageClient* pClient;  //!< a pointer to the message client
	bool bWaitingForMessage;  //!< indicates if the client has recieved the current message
};

//! Multimap of message clients ordered by priority
using t_MessageClientPriorityMap = std::multimap<unsigned char, s_MessageClientActive, std::greater<> >;

//! Map of different message types
using t_MessageClientMap = std::map<wGui::CMessage::EMessageType, t_MessageClientPriorityMap>;


//! A server which queues and dispatches messages

//! CMessageServer is a singeton (only one instance of it is allowed to exist at any time)
//! Clients must register to get messages sent to them
//! \sa CMessage CMessageClient

class CMessageServer
{
protected:
	//! The CMessageServer class cannot be directly instantiated, it must be access through Instance()
	CMessageServer();

	//! Standard constructor
	virtual ~CMessageServer();


public:
	//! Used for marking the priority of registered message clients, where the higher priority clients will get messages first
	enum EClientPriority
	{
		PRIORITY_LAST = 0,  //!< The absolute lowest priority available
		PRIORITY_LOW = 50,  //!< Low priority
		PRIORITY_NORMAL = 100,  //!< Standard priority
		PRIORITY_HIGH = 150,  //!< High priority
		PRIORITY_FIRST = 255  //!< The absolute highest priority available
	};

	//! Gets the single instance of the message server
	//! \return The single valid instance of the message server, or create one if it doesn't already exist
	static CMessageServer& Instance();

	//! Register a client to recieve messages
	//! \param pClient A pointer to the client which should recieve the messages
	//! \param eMessageType The message type the client wishes to recieve
	//! \param Priority The priority of the client for recieving the message
	void RegisterMessageClient(CMessageClient* pClient, CMessage::EMessageType eMessageType, unsigned char Priority = PRIORITY_NORMAL);

	//! Deregister a client for a certain message type
	//! \param pClient A pointer to the message client to be deregistered
	//! \param eMessageType The message type for which the client no longer should recieve messages
	void DeregisterMessageClient(CMessageClient* pClient, CMessage::EMessageType eMessageType);

	//! Deregister a client for all message types
	//! \param pClient A pointer to the message client to be deregistered
	void DeregisterMessageClient(CMessageClient* pClient);

	//! Takes the next message in the queue and dispatches it to any registered clients in priority order
	void DeliverMessage();

	//! Adds a message to the message queue
	//! \param pMessage A pointer to the message to be queued
	void QueueMessage(CMessage* pMessage);

	//! Indicates if there are any messages available
	//! \return true if there's a message available in the queue
	bool MessageAvailable() { return !m_MessageQueue.empty(); }

	//! Sets the server to ignore any new incoming messages (messages already in the queue are unaffected)
	//! \param bIgnore if true, the message queue will ignore any new messages
	void IgnoreAllNewMessages(bool bIgnore) { m_bIgnoreAllNewMessages = bIgnore; }

  //! Discard all pending messages
  void PurgeQueuedMessages();


protected:
	static CMessageServer* m_pInstance;  //!< A pointer to the single instande of the message server
	std::deque<CMessage*> m_MessageQueue;  //!< The message queue
	t_MessageClientMap m_MessageClients;  //!< A map of all the registered clients
	SDL_sem* m_pSemaphore;  //!< A semaphore indicating how many messages are in the queue
	bool m_bIgnoreAllNewMessages;  //!< Locks the queue so that no new messages are added, this is used during initialization, defaults to true
};

}


#endif // _WG_MESSAGE_SERVER_H_

