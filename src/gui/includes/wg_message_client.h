// wg_message_client.h
//
// CMessageClient interface
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


#ifndef _WG_MESSAGE_CLIENT_H_
#define _WG_MESSAGE_CLIENT_H_

#include "wg_message.h"


namespace wGui
{

//! An Abstract class for handling wGui messages
//! \sa CMessage CMessageServer

class CMessageClient
{
public:
	//! Standard constructor
	CMessageClient(void);

	//! Standard destructor
	virtual ~CMessageClient(void);

	//! This is the callback used by the Message Server to distribute messages
	//! The client must first register with the server and indicate any messages it wishes to recieve
	//! \sa CMessageServer::RegisterMessageClient()
	virtual bool HandleMessage(CMessage* pMessage) = 0;


protected:
};

}

#endif // _WG_MESSAGE_CLIENT_H_

