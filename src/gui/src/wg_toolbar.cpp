// wg_toolbar.cpp
//
// CToolBar class implementation
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


#include "wg_toolbar.h"
#include "wg_application.h"
#include "wutil_debug.h"


namespace wGui
{

CToolBar::CToolBar(const CRect& WindowRect, CWindow* pParent) :
	CWindow(WindowRect, pParent)
{
	m_BackgroundColor = COLOR_LIGHTGRAY;
	Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_SINGLELCLICK);
	Draw();
}


CToolBar::~CToolBar() = default;


void CToolBar::InsertButton(CButton* pButton, long int iButtonID, unsigned int iPosition)
{
	if (iPosition > m_vpButtons.size())
		iPosition = stdex::safe_static_cast<unsigned int>(m_vpButtons.size());
	long int iFixedButtonID = iButtonID;
	if (pButton == nullptr)
	{
		iFixedButtonID = 0;
	}
	else
	{
		// Transfer ownership of the button to the ToolBar
		pButton->SetNewParent(this);
	}
	m_vpButtons.insert(m_vpButtons.begin() + iPosition, std::make_pair(pButton, iFixedButtonID));
	RepositionButtons();
}


void CToolBar::AppendButton(CButton* pButton, long int iButtonID)
{
	InsertButton(pButton, iButtonID, stdex::safe_static_cast<unsigned int>(m_vpButtons.size()));
}


void CToolBar::RemoveButton(unsigned int iPosition)
{
	CButton* pButton = m_vpButtons.at(iPosition).first;
	m_vpButtons.erase(m_vpButtons.begin() + iPosition);
	delete pButton;
}


void CToolBar::Clear()
{
  for(const auto &button : m_vpButtons)
	{
		delete button.first;
	}
	m_vpButtons.clear();
}


int CToolBar::GetButtonPosition(long int iButtonID)
{
	int iPosition = -1;
	for (auto iter = m_vpButtons.begin(); iter != m_vpButtons.end(); ++iter)
	{
		if (iter->second == iButtonID)
		{
			iPosition = stdex::safe_static_cast<int>(iter - m_vpButtons.begin());
		}
	}
	return iPosition;
}


void CToolBar::RepositionButtons()
{
	int xPosition = 4;
  for (auto &button : m_vpButtons)
	{
		CButton* pButton = button.first;
		if (pButton)
		{
			int xStartPosition = xPosition;
			xPosition = xPosition + 2 + pButton->GetWindowRect().Width();
			pButton->SetWindowRect(CRect(xStartPosition, 2, xPosition - 3, pButton->GetWindowRect().Height() + 1));

			// Hide any buttons that extend beyond the end of the toolbar
			pButton->SetVisible(xPosition <= m_WindowRect.Width());
		}
		else
		{
			// Spacer
			xPosition += 6;
		}
	}
}


void CToolBar::SetWindowRect(const CRect& WindowRect)
{
	CWindow::SetWindowRect(WindowRect);
	m_ClientRect = m_WindowRect.SizeRect();
	RepositionButtons();
}


bool CToolBar::HandleMessage(CMessage* pMessage)
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
				long int iButtonID = 0;
        for (const auto &button : m_vpButtons)
				{
					if (button.first == pMessage->Source())
					{
						iButtonID = button.second;
					}
				}
				Application().MessageServer()->QueueMessage(new TIntMessage(CMessage::CTRL_SINGLELCLICK, m_pParentWindow, this, iButtonID));
				bHandled = true;
			}
			break;
		}
		default :
			bHandled = CWindow::HandleMessage(pMessage);
			break;
		}
	}

	return bHandled;
}

}

