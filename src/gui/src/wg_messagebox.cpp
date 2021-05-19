// wg_messagebox.cpp
//
// CMessageBox class implementation
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


#include "wg_messagebox.h"
#include <string>

namespace wGui
{

CMessageBox::CMessageBox(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine, const std::string& sTitle, const std::string& sMessage, int iButtons) :
	CFrame(WindowRect, pParent, pFontEngine, sTitle),
	m_iButtons(iButtons)
{
	//m_pMessageLabel = new CLabel(CRect(75, 10, GetClientRect().Right() - 75, GetClientRect().Bottom() - 40), this, sMessage);
	m_pMessageLabel = new CLabel(CPoint(10, 10), this, sMessage);
  // judb Position buttons relative to lower right corner:
	CPoint BottomRight(GetClientRect().Right() - 20, GetClientRect().Bottom() - 30);
	if (iButtons & CMessageBox::BUTTON_CANCEL)
	{
    CButton *button = new CButton(CRect(BottomRight - CPoint(50, 18), BottomRight), this, "Cancel");
    button->SetIsFocusable(true);
		m_ButtonMap.insert(std::make_pair(CMessageBox::BUTTON_CANCEL, button));
		BottomRight = BottomRight - CPoint(60, 0);
	}
	if (iButtons & CMessageBox::BUTTON_OK)
	{
    CButton *button = new CButton(CRect(BottomRight - CPoint(50, 18), BottomRight), this, "Ok");
    button->SetIsFocusable(true);
		m_ButtonMap.insert(std::make_pair(CMessageBox::BUTTON_OK, button));
		BottomRight = BottomRight - CPoint(60, 0);
	}
	if (iButtons & CMessageBox::BUTTON_NO)
	{
    CButton *button = new CButton(CRect(BottomRight - CPoint(50, 18), BottomRight), this, "No");
    button->SetIsFocusable(true);
		m_ButtonMap.insert(std::make_pair(CMessageBox::BUTTON_NO, button));
		BottomRight = BottomRight - CPoint(60, 0);
	}
	if (iButtons & CMessageBox::BUTTON_YES)
	{
    CButton *button = new CButton(CRect(BottomRight - CPoint(50, 18), BottomRight), this, "Yes");
    button->SetIsFocusable(true);
		m_ButtonMap.insert(std::make_pair(CMessageBox::BUTTON_YES, button));
		BottomRight = BottomRight - CPoint(60, 0);
	}
}


bool CMessageBox::HandleMessage(CMessage* pMessage)
{
	bool bHandled = false;

	if (pMessage)
	{
    switch(pMessage->MessageType())
		{
		case CMessage::CTRL_SINGLELCLICK:
		{
			if (pMessage->Destination() == this)
			{
        for (const auto& button : m_ButtonMap)
				{
					if (pMessage->Source() == button.second)
					{
						Application().MessageServer()->QueueMessage(new CValueMessage<CMessageBox::EButton>(CMessage::CTRL_MESSAGEBOXRETURN, m_pParentWindow, nullptr, button.first));
						CloseFrame();
            bHandled = true;
						break;
					}
				}
			}
			break;
		}
		default:
			break;
		}
		if (!bHandled)
		{
			bHandled = CFrame::HandleMessage(pMessage);
		}
	}

	return bHandled;
}

}
