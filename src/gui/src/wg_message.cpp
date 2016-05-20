// wg_message.cpp
//
// CMessage class implementation
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


#include "wgui_include_config.h"
#include "wg_message.h"
#include "wutil_debug.h"
#include "std_ex.h"


namespace wGui
{

CMessage::CMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource) :
	m_MessageType(MessageType),
	m_pDestination(pDestination),
	m_pSource(pSource)
{

}


CSDLMessage::CSDLMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource, SDL_Event SDLEvent) :
	CMessage(MessageType, pDestination, pSource),
	SDLEvent(SDLEvent)
{

}


CKeyboardMessage::CKeyboardMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource,
		unsigned char ScanCode, SDLMod Modifiers, SDLKey Key, Uint16 Unicode) :
	CMessage(MessageType, pDestination, pSource),
	ScanCode(ScanCode),
	Modifiers(Modifiers),
	Key(Key),
	Unicode(Unicode)
{

}


CMouseMessage::CMouseMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource,
		CPoint Point, CPoint Relative, unsigned int Button) :
	CMessage(MessageType, pDestination, pSource),
	Point(Point),
	Relative(Relative),
	Button(Button)
{

}


unsigned int CMouseMessage::TranslateSDLButton(Uint8 SDLButton)
{
	unsigned int Button = 0;
	switch (SDLButton)
	{
	case SDL_BUTTON_LEFT:
		Button = LEFT;
		break;
	case SDL_BUTTON_RIGHT:
		Button = RIGHT;
		break;
	case SDL_BUTTON_MIDDLE:
		Button = MIDDLE;
		break;
	case SDL_BUTTON_WHEELUP:
		Button = WHEELUP;
		break;
	case SDL_BUTTON_WHEELDOWN:
		Button = WHEELDOWN;
		break;
	default:
		wUtil::Trace("Untranslated SDL Button # " + stdex::itoa(SDLButton));
		break;
	}

	return Button;
}


unsigned int CMouseMessage::TranslateSDLButtonState(Uint8 SDLButtonState)
{
	unsigned int Button = 0;
	if (SDLButtonState & SDL_BUTTON(1))
	{
		Button |= LEFT;
	}
	if (SDLButtonState & SDL_BUTTON(2))
	{
		Button |= RIGHT;
	}
	if (SDLButtonState & SDL_BUTTON(3))
	{
		Button |= MIDDLE;
	}
	if (SDLButtonState & SDL_BUTTON(4))
	{
		Button |= WHEELUP;
	}
	if (SDLButtonState & SDL_BUTTON(5))
	{
		Button |= WHEELDOWN;
	}

	return Button;
}


}


