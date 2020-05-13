// wg_messagebox.h
//
// CMessageBox interface
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


#ifndef _WG_MESSAGEBOX_H_
#define _WG_MESSAGEBOX_H_

#include "wg_frame.h"
#include "wg_label.h"
#include <map>
#include <string>

namespace wGui
{

//! The CMessageBox class is a simple class that brings up a modal dialog box with a message and waits for user input

class CMessageBox : public CFrame
{
public:
	//! \param pParent A pointer to the parent view
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	//! \param sTitle The window title, which will appear in the title bar of the view
	//! \param sMessage The message to display in the message box
	//! \param iButtons A flag field to indicate which buttons to display in the message box, defaults to a single OK button
	CMessageBox(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine, const std::string& sTitle, const std::string& sMessage, int iButtons = BUTTON_OK);

	//! The return values for a message box
	enum EButton {
		BUTTON_INVALID = 0,
		BUTTON_CANCEL = 1,
		BUTTON_OK = 2,
		BUTTON_NO = 4,
		BUTTON_YES = 8
	};


	// CMessageClient overrides
	//! CScrollBars handle MOUSE_BUTTONDOWN and MOUSE_BUTTONUP messages
	//! \param pMessage A pointer to the message
	bool HandleMessage(CMessage* pMessage) override;


protected:
	CLabel* m_pMessageLabel;  //!< The label that is used for the message
	CPicture* m_pPicture; // judb an optional picture
	std::map<EButton, CButton*> m_ButtonMap;  //!< A map for the buttons
	int m_iButtons;  //!< The ORed value of the buttons to display in the message box


private:
  CMessageBox(const CMessageBox&) = delete;
	CMessageBox& operator=(const CMessageBox&) = delete;
};

}

#endif  // _WG_MESSAGEBOX_H_
