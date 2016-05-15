// wg_radiobutton.cpp
//
// CRadioButton class implementation
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
#include "wg_radiobutton.h"
#include "wg_message_server.h"

namespace wGui
{

// judb p is the upper-left corner of the radiobutton; size is the width=height
CRadioButton::CRadioButton(const CPoint& p, int size, CWindow* pParent, bool bFocusable) :
	CWindow(CRect(p, size, size), pParent, bFocusable),
	m_eRadioButtonState(UNCHECKED),
	m_MouseButton(0),
    m_hBitmapRadioButton(CwgBitmapResourceHandle(WGRES_RADIOBUTTON_BITMAP))
{
	m_BackgroundColor = DEFAULT_CHECKBOX_BACK_COLOR;
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_BUTTONUP);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_SINGLELCLICK);
	Draw();
}


CRadioButton::~CRadioButton(void)
{
   
}


void CRadioButton::SetState(EState eState)
{
	if (m_eRadioButtonState != eState)
	{
		m_eRadioButtonState = eState;
		Draw();
	}
}


void CRadioButton::Select()
{
  if (m_eRadioButtonState == UNCHECKED)
  {
    SetState(CHECKED);
    // Uncheck all other 'children' of this parent that are radiobuttons:
    std::list<CWindow*> myChildWindows = m_pParentWindow->GetChildWindows();
    for (std::list<CWindow*>::iterator iter = myChildWindows.begin(); iter != myChildWindows.end(); ++iter)
    {
      // Compare the types to find out if a child is a CRadioButton.
      if (typeid(**iter) == typeid(*this) && *iter != this)
      {
        // 'other' radiobutton found -> UNCHECK
        dynamic_cast<CRadioButton*>(*iter)->SetState(UNCHECKED);
      }
    }
    CMessageServer::Instance().QueueMessage(new TIntMessage(CMessage::CTRL_VALUECHANGE, m_pParentWindow, this, 1));
  }
}


void CRadioButton::Draw(void) const
{
	CWindow::Draw();

	if (m_pSDLSurface)
  {
    CRect SubRect(m_WindowRect.SizeRect());
    CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
    if (m_eRadioButtonState != DISABLED)
    {
      Painter.DrawRect(SubRect, false, COLOR_LIGHTGRAY);
      Painter.DrawHLine(SubRect.Left(), SubRect.Right(), SubRect.Top(), COLOR_BLACK);
      Painter.DrawVLine(SubRect.Top(), SubRect.Bottom(), SubRect.Left(), COLOR_BLACK);
			SubRect.Grow(-1);
      if (m_bHasFocus)
      {
        Painter.DrawRect(SubRect, false, COLOR_GRAY);
      }
      if (m_eRadioButtonState == CHECKED)
      {
        SubRect.Grow(-2);
        Painter.DrawRect(SubRect, true, COLOR_BLACK, COLOR_BLACK);
        /*
        SDL_Rect SourceRect = m_WindowRect.SizeRect().SDLRect();
        SDL_Rect DestRect = SubRect.SDLRect();
        SDL_BlitSurface(m_hBitmapRadioButton.Bitmap(), &SourceRect, m_pSDLSurface, &DestRect);
        */
      }
    } else {
      Painter.DrawRect(SubRect, false, COLOR_LIGHTGRAY);
    }
  }
}


bool CRadioButton::OnMouseButtonDown(CPoint Point, unsigned int Button)
{
	bool bResult = CWindow::OnMouseButtonDown(Point, Button);
    if (!bResult && m_bVisible && (m_eRadioButtonState != DISABLED) &&
		(m_ClientRect.HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE))
	{
		m_MouseButton = Button;
		bResult = true;
	}

	return bResult;
}


bool CRadioButton::OnMouseButtonUp(CPoint Point, unsigned int Button)
{
	bool bResult = CWindow::OnMouseButtonUp(Point, Button);

	if (!bResult && m_bVisible && (m_eRadioButtonState != DISABLED) && (m_MouseButton == Button) &&
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
		CMessageServer::Instance().QueueMessage(new TIntMessage(MessageType, this, this, 0));
		bResult = true;
	}

	return bResult;
}


bool CRadioButton::HandleMessage(CMessage* pMessage)
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
          case SDLK_SPACE:
            Select();
            break;
          default:
            // Forward all key downs to parent
            CMessageServer::Instance().QueueMessage(new CKeyboardMessage(CMessage::KEYBOARD_KEYDOWN, m_pParentWindow, this,
                  pKeyboardMessage->ScanCode, pKeyboardMessage->Modifiers, pKeyboardMessage->Key, pKeyboardMessage->Unicode));
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
        Select();
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
