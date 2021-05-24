// wg_checkbox.cpp
//
// CCheckBox class implementation
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


#include "wg_checkbox.h"
#include "wg_application.h"


namespace wGui
{

CCheckBox::CCheckBox(const CRect& WindowRect, CWindow* pParent) :
	CWindow(WindowRect, pParent),
	m_eCheckBoxState(UNCHECKED),
	m_MouseButton(0),
  m_hBitmapCheck(CwgBitmapResourceHandle(Application(), WGRES_CHECK_BITMAP))

{
	m_BackgroundColor = DEFAULT_CHECKBOX_BACK_COLOR;
	Application().MessageServer()->RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
	Application().MessageServer()->RegisterMessageClient(this, CMessage::MOUSE_BUTTONUP);
	Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_SINGLELCLICK);
	Draw();
}


CCheckBox::~CCheckBox() = default;


void CCheckBox::SetReadOnly(bool bReadOnly)
{
	m_BackgroundColor = bReadOnly ? COLOR_LIGHTGRAY : COLOR_WHITE;
	m_bReadOnly = bReadOnly;
  SetIsFocusable(!bReadOnly);
	Draw();
}


void CCheckBox::SetCheckBoxState(EState eState)
{
	if (m_eCheckBoxState != eState)
	{
		m_eCheckBoxState = eState;
		Draw();
	}
}


void CCheckBox::ToggleCheckBoxState()
{
  if (m_bReadOnly) return;
  switch (m_eCheckBoxState)
  {
    case UNCHECKED:
      SetCheckBoxState(CHECKED);
      Application().MessageServer()->QueueMessage(new TIntMessage(CMessage::CTRL_VALUECHANGE, m_pParentWindow, this, 1));
      break;
    case CHECKED:
      SetCheckBoxState(UNCHECKED);
      Application().MessageServer()->QueueMessage(new TIntMessage(CMessage::CTRL_VALUECHANGE, m_pParentWindow, this, 0));
      break;
    default:
      break;
  }
}


void CCheckBox::Draw() const
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CRect SubRect(m_WindowRect.SizeRect());
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		Painter.DrawRect(m_WindowRect.SizeRect(), false, COLOR_WHITE);
		if (m_eCheckBoxState != DISABLED)
		{
			Painter.DrawRect(SubRect, false, COLOR_LIGHTGRAY);
			Painter.DrawHLine(SubRect.Left(), SubRect.Right(), SubRect.Top(), COLOR_BLACK);
			Painter.DrawVLine(SubRect.Top(), SubRect.Bottom(), SubRect.Left(), COLOR_BLACK);
			SubRect.Grow(-1);
      if (m_bHasFocus)
      {
        Painter.DrawRect(SubRect, false, COLOR_GRAY);
      }
			SubRect.Grow(-1);
			if (m_eCheckBoxState == CHECKED)
			{
//				Painter.DrawLine(SubRect.TopLeft(), SubRect.BottomRight(), DEFAULT_LINE_COLOR);
//				Painter.DrawLine(SubRect.BottomLeft(), SubRect.TopRight(), DEFAULT_LINE_COLOR);
        SDL_Rect SourceRect = m_WindowRect.SizeRect().SDLRect();
        SDL_Rect DestRect = SubRect.SDLRect();
        SDL_BlitSurface(m_hBitmapCheck.Bitmap(), &SourceRect, m_pSDLSurface, &DestRect);
			}
		}
	}
}


bool CCheckBox::OnMouseButtonDown(CPoint Point, unsigned int Button)
{
	bool bResult = CWindow::OnMouseButtonDown(Point, Button);

 	if (!bResult && m_bVisible && !m_bReadOnly && (m_eCheckBoxState != DISABLED) &&
		(m_ClientRect.HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE))
	{
		m_MouseButton = Button;
		bResult = true;
	}

	return bResult;
}


bool CCheckBox::OnMouseButtonUp(CPoint Point, unsigned int Button)
{
	bool bResult = CWindow::OnMouseButtonUp(Point, Button);

	if (!bResult && m_bVisible && !m_bReadOnly && (m_eCheckBoxState != DISABLED) &&
      (m_MouseButton == Button) &&
      (m_ClientRect.HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE))
	{
		CMessage::EMessageType MessageType =  CMessage::UNKNOWN;
		switch (m_MouseButton)
		{
		case CMouseMessage::LEFT:
			MessageType = CMessage::CTRL_SINGLELCLICK;
			break;
		case CMouseMessage::RIGHT:
			MessageType = CMessage::CTRL_SINGLERCLICK;
			break;
		case CMouseMessage::MIDDLE:
			MessageType = CMessage::CTRL_SINGLEMCLICK;
			break;
		}
		Application().MessageServer()->QueueMessage(new TIntMessage(MessageType, this, this, 0));
		bResult = true;
	}

	return bResult;
}


bool CCheckBox::HandleMessage(CMessage* pMessage)
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
        switch (pKeyboardMessage->Key)
        {
          case SDLK_RETURN:  // intentional fall through
          case SDLK_SPACE:
            ToggleCheckBoxState();
            break;
          default:
            // Forward all key downs to parent
            Application().MessageServer()->QueueMessage(new CKeyboardMessage(CMessage::KEYBOARD_KEYDOWN, m_pParentWindow, this,
                  pKeyboardMessage->ScanCode, pKeyboardMessage->Modifiers, pKeyboardMessage->Key));
            break;
        }
      }
      break;
    }
		case CMessage::MOUSE_BUTTONUP:
		{
			CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
			if (pMouseMessage && (m_ClientRect.HitTest(ViewToWindow(pMouseMessage->Point)) != CRect::RELPOS_INSIDE)
				&& (m_MouseButton == pMouseMessage->Button))
			{
				m_MouseButton = 0;
				bHandled = true;
			}
			break;
		}
		case CMessage::CTRL_SINGLELCLICK:
			if (pMessage->Destination() == this)
			{
        ToggleCheckBoxState();
				bHandled = true;
			}
			break;
		default :
			bHandled = CWindow::HandleMessage(pMessage);
			break;
		}
	}

	return bHandled;
}

}



