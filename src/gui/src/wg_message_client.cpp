// wg_message_client.cpp
//
// CMessageClient class implementation
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


#include "wg_message_client.h"
#include "wg_application.h"

namespace wGui
{

CMessageClient::CMessageClient(CApplication& pApplication) :
  m_pApplication(pApplication) {}
  

CMessageClient::~CMessageClient()  // virtual
{
  auto message_server = Application().MessageServer();
  // Exceptionnally, MessageServer() can return null because we may be in the
  // destructor of the parent CMessageClient of CApplication
  if (message_server == nullptr) return;
  message_server->DeregisterMessageClient(this);
}

}

