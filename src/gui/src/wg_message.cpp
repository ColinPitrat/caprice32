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


#include "wg_message.h"
#include "std_ex.h"
#include "log.h"
#include <string>


namespace wGui
{

CMessage::CMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource) :
	m_MessageType(MessageType),
	m_pDestination(pDestination),
	m_pSource(pSource)
{

}


std::string CMessage::ToString(EMessageType message_type) {
  switch (message_type) {
    case UNKNOWN: return "UNKNOWN";
    case APP_DESTROY_FRAME: return "APP_DESTROY_FRAME";
    case APP_EXIT: return "APP_EXIT";
    case APP_PAINT: return "APP_PAINT";
    case CTRL_DOUBLELCLICK: return "CTRL_DOUBLELCLICK";
    case CTRL_DOUBLEMCLICK: return "CTRL_DOUBLEMCLICK";
    case CTRL_DOUBLERCLICK: return "CTRL_DOUBLERCLICK";
    case CTRL_GAININGKEYFOCUS: return "CTRL_GAININGKEYFOCUS";
    case CTRL_GAININGMOUSEFOCUS: return "CTRL_GAININGMOUSEFOCUS";
    case CTRL_LOSINGKEYFOCUS: return "CTRL_LOSINGKEYFOCUS";
    case CTRL_LOSINGMOUSEFOCUS: return "CTRL_LOSINGMOUSEFOCUS";
    case CTRL_MESSAGEBOXRETURN: return "CTRL_MESSAGEBOXRETURN";
    case CTRL_RESIZE: return "CTRL_RESIZE";
    case CTRL_SINGLELCLICK: return "CTRL_SINGLELCLICK";
    case CTRL_SINGLEMCLICK: return "CTRL_SINGLEMCLICK";
    case CTRL_SINGLERCLICK: return "CTRL_SINGLERCLICK";
    case CTRL_TIMER: return "CTRL_TIMER";
    case CTRL_VALUECHANGE: return "CTRL_VALUECHANGE";
    case CTRL_VALUECHANGING: return "CTRL_VALUECHANGING";
    case KEYBOARD_KEYDOWN: return "KEYBOARD_KEYDOWN";
    case KEYBOARD_KEYUP: return "KEYBOARD_KEYUP";
    case TEXTINPUT: return "TEXTINPUT";
    case MOUSE_BUTTONDOWN: return "MOUSE_BUTTONDOWN";
    case MOUSE_BUTTONUP: return "MOUSE_BUTTONUP";
    case MOUSE_MOVE: return "MOUSE_MOVE";
    case SDL: return "SDL";
    case USER: return "USER";
  }
  return " ** Unimplemented **";
}


CSDLMessage::CSDLMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource, SDL_Event SDLEvent) : // NOLINT(modernize-pass-by-value): if we pass by value and move SDLEvent, clang produce another warning (misc-move-const-arg) about moving a trivially-copyable type
	CMessage(MessageType, pDestination, pSource),
	SDLEvent(SDLEvent)
{

}


CKeyboardMessage::CKeyboardMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource,
		unsigned char ScanCode, SDL_Keymod Modifiers, SDL_Keycode Key) :
	CMessage(MessageType, pDestination, pSource),
	ScanCode(ScanCode),
	Modifiers(Modifiers),
	Key(Key)
{

}


CTextInputMessage::CTextInputMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource,
		std::string Text) :
	CMessage(MessageType, pDestination, pSource),
	Text(std::move(Text))
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
	default:
		LOG_ERROR("Untranslated SDL Button # " + stdex::itoa(SDLButton));
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


