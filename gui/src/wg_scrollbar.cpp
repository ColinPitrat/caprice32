// wg_scrollbar.cpp
//
// CScrollBar class implementation
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
#include "wg_application.h"
#include "wg_scrollbar.h"
#include "wg_message_server.h"
#include "wg_error.h"
#include "wg_resources.h"
#include "std_ex.h"


namespace wGui
{

CScrollBar::CScrollBar(const CRect& WindowRect, CWindow* pParent, EScrollBarType ScrollBarType) :
	CRangeControl<int>(WindowRect, pParent, 0, 100, 1, 0),
	m_ScrollBarType(ScrollBarType),
	m_iJumpAmount(5),
	m_bDragging(false)
{
	m_BackgroundColor = CApplication::Instance()->GetDefaultForegroundColor();
	switch (m_ScrollBarType)
	{
	case VERTICAL:
		m_ClientRect = CRect(0, m_WindowRect.Width(), m_WindowRect.Width() - 1, m_WindowRect.Height() - m_WindowRect.Width() - 1);
		m_pBtnUpLeft = new CPictureButton(CRect(0, -m_ClientRect.Width(), m_ClientRect.Width() - 1, -1),
			this, CwgBitmapResourceHandle(WGRES_UP_ARROW_BITMAP));
		m_pBtnDownRight = new CPictureButton(
			CRect(0, m_ClientRect.Height() + 1, m_ClientRect.Width() - 1, m_ClientRect.Height() + m_ClientRect.Width()),
			this, CwgBitmapResourceHandle(WGRES_DOWN_ARROW_BITMAP));
		break;
	case HORIZONTAL:
		m_ClientRect = CRect(m_WindowRect.Height(), 0, m_WindowRect.Width() - m_WindowRect.Height() - 1, m_WindowRect.Height() - 1);
		m_pBtnUpLeft = new CPictureButton(CRect(-m_ClientRect.Height(), 0, -1, m_ClientRect.Height() - 1),
			this, CwgBitmapResourceHandle(WGRES_LEFT_ARROW_BITMAP));
		m_pBtnDownRight = new CPictureButton(
			CRect(m_ClientRect.Width() + 1, 0, m_ClientRect.Width() + m_ClientRect.Height(), m_ClientRect.Height() - 1),
			this, CwgBitmapResourceHandle(WGRES_RIGHT_ARROW_BITMAP));
		break;
	default:
		throw(Wg_Ex_App("CScrollBar::CScrollBar:  Unrecognized ScrollBar Type."));
		break;
	}
	m_ThumbRect = m_ClientRect;
	RepositionThumb();
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_BUTTONUP);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_MOVE);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_SINGLELCLICK);
	Draw();
}


CScrollBar::~CScrollBar(void)
{

}


void CScrollBar::SetValue(int iValue, bool bRedraw)  // virtual
{
	CRangeControl<int>::SetValue(iValue, false);
	RepositionThumb();
	if (bRedraw)
	{
		Draw();
	}
}


void CScrollBar::Draw(void) const
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
        Painter.DrawRect(m_WindowRect.SizeRect(), false, m_BackgroundColor * 0.7);

		if (m_MinLimit != m_MaxLimit)
		{
			CRect SubRect(m_ThumbRect);
			Painter.DrawRect(SubRect, true, DEFAULT_BUTTON_COLOR, DEFAULT_BUTTON_COLOR);
            Painter.Draw3DRaisedRect(SubRect, DEFAULT_BUTTON_COLOR);
		}
	}
}


void CScrollBar::SetWindowRect(const CRect& WindowRect)
{
	CWindow::SetWindowRect(WindowRect);
	// Resposition the thumb rect and the button controls
	switch (m_ScrollBarType)
	{
	case VERTICAL:
	{
		m_ClientRect = CRect(0, m_WindowRect.Width(), m_WindowRect.Width() - 1, m_WindowRect.Height() - m_WindowRect.Width() - 1);
		m_pBtnUpLeft->SetWindowRect(CRect(0, -m_ClientRect.Width(), m_ClientRect.Width() - 1, -1));
		m_pBtnDownRight->SetWindowRect(CRect(0, m_ClientRect.Height() + 1, m_ClientRect.Width() - 1, m_ClientRect.Height() + m_ClientRect.Width()));
		break;
	}
	case HORIZONTAL:
	{
		m_ClientRect = CRect(m_WindowRect.Height(), 0, m_WindowRect.Width() - m_WindowRect.Height() - 1, m_WindowRect.Height() - 1);
		m_pBtnUpLeft->SetWindowRect(CRect(-m_ClientRect.Height(), 0, -1, m_ClientRect.Height() - 1));
		m_pBtnDownRight->SetWindowRect(CRect(m_ClientRect.Width() + 1, 0, m_ClientRect.Width() + m_ClientRect.Height(), m_ClientRect.Height() - 1));
		break;
	}
	default:
		throw(Wg_Ex_App("CScrollBar::SetWindowRect:  Unrecognized ScrollBar Type."));
		break;
	}
	SetValue(m_Value);
}


void CScrollBar::MoveWindow(const CPoint& MoveDistance)
{
	CWindow::MoveWindow(MoveDistance);
	m_ThumbRect = m_ThumbRect + MoveDistance;
}


bool CScrollBar::OnMouseButtonDown(CPoint Point, unsigned int Button)
{
	bool bResult = CWindow::OnMouseButtonDown(Point, Button);

 	if (!bResult && m_bVisible && m_ClientRect.HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE)
	{
		if (Button == CMouseMessage::WHEELUP)
		{
			Decrement();
			bResult = true;
		}
		else if (Button == CMouseMessage::WHEELDOWN)
		{
			Increment();
			bResult = true;
		}
		else if (Button == CMouseMessage::LEFT)
		{
			switch (m_ThumbRect.HitTest(ViewToWindow(Point)))
			{
			case CRect::RELPOS_INSIDE:
				m_bDragging = true;
				break;
			case CRect::RELPOS_ABOVE:
			case CRect::RELPOS_LEFT:
				SetValue(m_Value - m_iJumpAmount );
				break;
			case CRect::RELPOS_BELOW:
			case CRect::RELPOS_RIGHT:
				SetValue(m_Value + m_iJumpAmount );
				break;
			}
			bResult = true;
		}
	}

	return bResult;
}


void CScrollBar::SetIsFocusable(bool bFocusable)
{
  m_pBtnUpLeft->SetIsFocusable(bFocusable);
  m_pBtnDownRight->SetIsFocusable(bFocusable);
}


bool CScrollBar::HandleMessage(CMessage* pMessage)
{
	bool bHandled = false;

	if (pMessage)
	{
		switch(pMessage->MessageType())
		{
		case CMessage::KEYBOARD_KEYDOWN:
    {
      CKeyboardMessage* pKeyboardMessage = dynamic_cast<CKeyboardMessage*>(pMessage);
      if (pKeyboardMessage && pMessage->Destination() == this)
      {
        // Forward all key downs to parent
        CMessageServer::Instance().QueueMessage(new CKeyboardMessage(CMessage::KEYBOARD_KEYDOWN, m_pParentWindow, this,
              pKeyboardMessage->ScanCode, pKeyboardMessage->Modifiers, pKeyboardMessage->Key, pKeyboardMessage->Unicode));
      }
      break;
    }
		case CMessage::MOUSE_BUTTONUP:
		{
			CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
			if (pMouseMessage && m_bDragging && pMouseMessage->Button == CMouseMessage::LEFT)
			{
				m_bDragging = false;
				CMessageServer::Instance().QueueMessage(new TIntMessage(CMessage::CTRL_VALUECHANGE, m_pParentWindow, this, m_Value));
				bHandled = true;
			}
			break;
		}
		case CMessage::MOUSE_MOVE:
			if (m_bDragging)
			{
				CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
				if (pMouseMessage)
				{
					int iOldPosition = m_Value;
					switch (m_ScrollBarType)
					{
					case VERTICAL:
						m_Value = ConstrainValue((ViewToWindow(pMouseMessage->Point).YPos() - m_ClientRect.Top() - m_ThumbRect.Height() / 2) *
							(m_MaxLimit - m_MinLimit) / (m_ClientRect.Height() - m_ThumbRect.Height()) + m_MinLimit);
						break;
					case HORIZONTAL:
						m_Value = ConstrainValue((ViewToWindow(pMouseMessage->Point).XPos() - m_ClientRect.Left() - m_ThumbRect.Width() / 2) *
							(m_MaxLimit - m_MinLimit) / (m_ClientRect.Width() - m_ThumbRect.Width()) + m_MinLimit);
						break;
					default:
						throw(Wg_Ex_App("CScrollBar::HandleMessage:  Unrecognized ScrollBar Type."));
						break;
					}
					if (iOldPosition != m_Value)
					{
						CMessageServer::Instance().QueueMessage(new TIntMessage(CMessage::CTRL_VALUECHANGING, m_pParentWindow, this, m_Value));
						RepositionThumb();
						Draw();
					}
				}
			}
			break;
		case CMessage::CTRL_SINGLELCLICK:
		{
			if (pMessage->Destination() == this)
			{
				if (pMessage->Source() == m_pBtnUpLeft)
				{
					Decrement();
					bHandled = true;
				}
				else if (pMessage->Source() == m_pBtnDownRight)
				{
					Increment();
					bHandled = true;
				}
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


void CScrollBar::RepositionThumb(void)  // virtual
{
	if (m_MinLimit != m_MaxLimit)
	{
		switch (m_ScrollBarType)
		{
		case VERTICAL:
		{
			int iThumbHeight = m_ClientRect.Height() / (m_MaxLimit - m_MinLimit + 1);
			if (iThumbHeight < 10)
			{
				iThumbHeight = 10;
			}
			m_ThumbRect.SetTop(m_ClientRect.Top() + (m_ClientRect.Height() - iThumbHeight) * (m_Value - m_MinLimit) / (m_MaxLimit - m_MinLimit));
            // judb removed '+ 1' at the end
			m_ThumbRect.SetBottom(m_ThumbRect.Top() + iThumbHeight);
			break;
		}
		case HORIZONTAL:
		{
			int iThumbWidth = m_ClientRect.Width() / (m_MaxLimit - m_MinLimit + 1);
			if (iThumbWidth < 10)
			{
				iThumbWidth = 10;
			}
			m_ThumbRect.SetLeft(m_ClientRect.Left() + (m_ClientRect.Width() - iThumbWidth) * (m_Value - m_MinLimit) / (m_MaxLimit - m_MinLimit));
            // judb removed '+ 1' at the end
			m_ThumbRect.SetRight(m_ThumbRect.Left() + iThumbWidth);
			break;
		}
		default:
			throw(Wg_Ex_App("CScrollBar::RepositionThumb:  Unrecognized ScrollBar Type."));
			break;
		}
	}
}

}

