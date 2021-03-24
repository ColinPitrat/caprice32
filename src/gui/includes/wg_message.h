// wg_message.h
//
// CMessage interface
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


#ifndef _WG_MESSAGE_H_
#define _WG_MESSAGE_H_

#include "wg_point.h"
#include "wg_rect.h"
#include "SDL.h"
#include <string>


namespace wGui
{
// Forward declarations
class CMessageClient;

//! The base message class

//! wGui uses Message object to inform other objects of events.
//! \sa CMessageServer CMessageClient

class CMessage
{
public:
	//! The various message types
	enum EMessageType
	{
		UNKNOWN = 0,  //!< An unknown message, this is not a valid type
		APP_DESTROY_FRAME,	//!< CMessage used to delete frame objects, where Source() is the frame that is to be deleted, Destination() should be 0
		APP_EXIT,  //!< CMessage used to tell controls or windows that the application is closing
		APP_PAINT,	//!< CMessage used to tell controls or windows to redraw themselves
		CTRL_DOUBLELCLICK,	//!< TIntMessage generated when a control is double clicked with the left mouse button
		CTRL_DOUBLEMCLICK,	//!< TIntMessage generated when a control is double clicked with the middle mouse button
		CTRL_DOUBLERCLICK,	//!< TIntMessage generated when a control is double clicked with the right mouse button
		CTRL_GAININGKEYFOCUS,  //!< CMessage used to tell a control that it's getting the keyboard focus
		CTRL_GAININGMOUSEFOCUS,  //!< CMessage used to tell a control that it's getting the mouse focus
		CTRL_LOSINGKEYFOCUS,  //!< CMessage used to tell a control that it's losing the keyboard focus
		CTRL_LOSINGMOUSEFOCUS,	//!< CMessage used to tell a control that it's losing the mouse focus
		CTRL_MESSAGEBOXRETURN,	//!< CValueMessage sent when a CMessageBox closes
		CTRL_RESIZE,  //!< TPointMessage used to tell the app that the view has been resized
		CTRL_SINGLELCLICK,	//!< TIntMessage generated when a control is clicked on with the left mouse button
		CTRL_SINGLEMCLICK,	//!< TIntMessage generated when a control is clicked on with the middle mouse button
		CTRL_SINGLERCLICK,	//!< TIntMessage generated when a control is clicked on with the right mouse button
		CTRL_TIMER, //!< TIntMessage used to tell when a timer has expired, where Value() is the count of times fired
		CTRL_VALUECHANGE,  //!< CValueMessage generated when a control's text or value is changed via user input
		CTRL_VALUECHANGING,  //!< CValueMessage generated when a control's text or value is in the process of changing via user input
		KEYBOARD_KEYDOWN,  //!< CKeyboardMessage generated when a keyboard key is pressed
		KEYBOARD_KEYUP,  //!< CKeyboardMessage generated when a keyboard key is released
    TEXTINPUT, //!< CTextInput generated when text is typed in a field that supports it
		MOUSE_BUTTONDOWN,  //!< CMouseMessage generated when a mouse button is pressed
		MOUSE_BUTTONUP,  //!< CMouseMessage generated when a mouse button is released
		MOUSE_MOVE,  //!< CMouseMessage generated when a mouse is moved
		SDL,  //!< An unhandled SDL event
		USER  //!< Any user defined messages of type CUserMessage
	};

	//! Construct a new message
	//! \param MessageType The type of message being created
	//! \param pDestination A pointer to the window that the message is destined for (0 for no specific destination, or to broadcast to all)
	//! \param pSource A pointer to the source of the message
	CMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource);

	//! Standard destructor
	virtual ~CMessage() = default;

	//! Gets the message type
	//! \return The message type of the message
	EMessageType MessageType() { return m_MessageType; }

	//! Gets the intended destination for the message
	//! \return A pointer to the destination of the message (0 for no specific destination, or to broadcast to all)
	const CMessageClient* Destination() { return m_pDestination; }

	//! Gets the source of the message
	//! \return A pointer to the source of the message
	const CMessageClient* Source() { return m_pSource; }

protected:
	//! The message type
	const EMessageType m_MessageType;

	//! A pointer to the message destination (0 for no specific destination, or to broadcast to all)
	const CMessageClient* m_pDestination;

	//! A pointer to the control that generated the message
	const CMessageClient* m_pSource;

private:
  CMessage(const CMessage&) = delete;
	CMessage& operator=(const CMessage&) = delete;
};


//! Any otherwise unhandled SDL messages

class CSDLMessage : public CMessage
{
public:
	//! Construct a new SDL message
	//! \param MessageType The type of message being created
	//! \param pDestination A pointer to the window that the message is destined for (0 for no specific destination, or to broadcast to all)
	//! \param pSource A pointer to the window that created the message
	//! \param SDLEvent The untranslated SDL event
	CSDLMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource, SDL_Event SDLEvent);

	SDL_Event SDLEvent;  //!< The untranslated SDL event
};


//! Any messages generated from keyboard input

class CKeyboardMessage : public CMessage
{
public:
	//! Construct a new Keyboard message
	//! \param MessageType The type of message being created
	//! \param pDestination A pointer to the window that the message is destined for (0 for no specific destination, or to broadcast to all)
	//! \param pSource A pointer to the window that created the message
	//! \param ScanCode The scan code of the key pressed
	//! \param Modifiers Any modifier keys that are being pressed (alt, ctrl, shift, etc)
	//! \param Key The SDL_Keysym that defines the key pressed
	CKeyboardMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource,
		unsigned char ScanCode, SDL_Keymod Modifiers, SDL_Keycode Key);

	unsigned char ScanCode;  //!< The scan code of the key pressed
	SDL_Keymod Modifiers;  //!< Any modifier keys that are being pressed (alt, ctrl, shift, etc)
	SDL_Keycode Key;  //!< The SDL_Keysym that defines the key pressed
};


//! Any messages generated by text input

class CTextInputMessage : public CMessage
{
public:
	//! Construct a new TextInput message
	//! \param MessageType The type of message being created
	//! \param pDestination A pointer to the window that the message is destined for (0 for no specific destination, or to broadcast to all)
	//! \param pSource A pointer to the window that created the message
	//! \param Text The text being typed.
	CTextInputMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource,
		std::string Text);

	std::string Text;  //!< The Text that has been typed
};


//! Any messages generated from mouse input

class CMouseMessage : public CMessage
{
public:
	//! Constants for all the mouse buttons, these values can be ORed together for more than one button
	enum EMouseButton
	{
		NONE = 0,				//!< No mouse button
		LEFT = 1,				//!< The left mouse button
		RIGHT = 2,			//!< The right mouse button
		MIDDLE = 4,			//!< The middle mouse button
		WHEELUP = 8,		//!< The mouse wheel moved up
		WHEELDOWN = 16	//!< The mouse wheel moved down
	};

	//! Construct a new mouse message
	//! \param MessageType The type of message being created
	//! \param pDestination A pointer to the window that the message is destined for (0 for no specific destination, or to broadcast to all)
	//! \param pSource A pointer to the window that created the message
	//! \param Point The location of the mouse cursor
	//! \param Relative The relative movement of the cursor (only valid for MOUSE_MOVE messages)
	//! \param Button An OR of all the EMouseButton values indicating which mouse buttons are pressed
	CMouseMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource,
		CPoint Point, CPoint Relative, unsigned int Button);

	//! Converst an SDLButton value into an EMouseButton value
	static unsigned int TranslateSDLButton(Uint8 SDLButton);

	//! Converts an SDLButtonState value into an ORing of EMouseButton values
	static unsigned int TranslateSDLButtonState(Uint8 SDLButtonState);

	CPoint Point;  //!< The point where the mouse cursor was at the time of the message
	CPoint Relative;  //!< The relative movement of the cursor (only valid for MOUSE_MOVE messages)
	unsigned int Button;  //!< Any mouse buttons pressed
};


//! A template for messages that contain values
//! Type T must have a valid copy constructor and assignment operator

template<typename T>
class CValueMessage : public CMessage
{
public:

	//! Construct a new template based Value message
	//! \param MessageType The type of message being created
	//! \param pDestination A pointer to the window that the message is destined for (0 for no specific destination, or to broadcast to all)
	//! \param pSource A pointer to the control that triggered the message
	//! \param Value A template type data the user has
	CValueMessage(const EMessageType MessageType, const CMessageClient* pDestination, const CMessageClient* pSource, T Value ) :
		CMessage(MessageType, pDestination, pSource),
		m_Value(std::move(Value))
	{ }

	//! Returns the value of the message
	//! \return A constant reference to the internal value
	const T& Value() { return m_Value; }

	//! Sets the value of the message
	//! \param Value The value
	void SetValue(const T& Value) { m_Value = Value; }

protected:

	//! The internal value
	T m_Value;
};


//! Some predefined value messages

typedef CValueMessage<int> TIntMessage;
typedef CValueMessage<float> TFloatMessage;
typedef CValueMessage<double> TDoubleMessage;
typedef CValueMessage<std::string> TStringMessage;
typedef CValueMessage<CPoint> TPointMessage;
typedef CValueMessage<CRect> TRectMessage;

}


#endif // _WG_MESSAGE_H_

