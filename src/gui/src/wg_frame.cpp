// wg_frame.cpp
//
// CFrame class implementation
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


#include "wg_frame.h"
#include "wg_application.h"
#include <algorithm>
#include <iostream>
#include <string>

namespace wGui
{

CFrame::CFrame(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine, const std::string& sTitle, bool bResizable) :
	CWindow(WindowRect, pParent),
	m_TitleBarColor(DEFAULT_TITLEBAR_COLOR),
	m_TitleBarTextColor(DEFAULT_TITLEBAR_TEXT_COLOR),
	m_iTitleBarHeight(12),
	m_bResizable(bResizable),
	m_bModal(false),
	m_pMenu(nullptr),
	m_bDragMode(false)
{
	if (pFontEngine) {
		m_pFontEngine = pFontEngine;
	} else {
		m_pFontEngine = CApplication::Instance()->GetDefaultFontEngine();
	}

	m_sWindowText = sTitle;
  m_pFrameCloseButton = new CPictureButton(CRect(0, 0, 8, 8),
      this, CwgBitmapResourceHandle(WGRES_X_BITMAP));

	m_pRenderedString.reset(new CRenderedString(m_pFontEngine, m_sWindowText, CRenderedString::VALIGN_CENTER));

	SetWindowRect(WindowRect);  // must be done after the buttons are created, and after the CRenderedString is created

	CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_BUTTONUP);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_MOVE);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_SINGLELCLICK);
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
}


CFrame::~CFrame()  // virtual
{
	if (m_bModal)
	{
		SetModal(false);
	}
}


void CFrame::CloseFrame()
{
	CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_DESTROY_FRAME, nullptr, this));
}


void CFrame::SetModal(bool bModal)
{
	m_bModal = bModal;
	
	if (m_bModal) {
		CApplication::Instance()->SetMouseFocus(this);
		CApplication::Instance()->SetKeyFocus(this);
	} else if (m_pParentWindow) {
		CApplication::Instance()->SetMouseFocus(m_pParentWindow);
		CApplication::Instance()->SetKeyFocus(m_pParentWindow);
	}
}


void CFrame::Draw() const  // virtual
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		CRect SubRect(m_WindowRect.SizeRect());
		Painter.Draw3DRaisedRect(SubRect, DEFAULT_BACKGROUND_COLOR);
		SubRect.Grow(-2);
    if (m_iTitleBarHeight > 0) {
      Painter.DrawRect(m_TitleBarRect, true, m_TitleBarColor, m_TitleBarColor);
      Painter.Draw3DLoweredRect(m_TitleBarRect, m_TitleBarColor);

      CRect TextClipRect(m_TitleBarRect);
      TextClipRect.SetRight(TextClipRect.Right() - 16);
      TextClipRect.Grow(-1);
      if (m_pRenderedString.get())
      {
        m_pRenderedString->Draw(m_pSDLSurface, TextClipRect, m_TitleBarRect.TopLeft() + CPoint(6, m_iTitleBarHeight / 2 - 1), m_TitleBarTextColor);
      }
    }
	}
}


void CFrame::PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const
{
	if (m_bVisible)
	{
		SDL_Rect SourceRect = CRect(m_WindowRect.SizeRect()).SDLRect();
		if (!m_bDragMode)
		{
			SDL_Rect DestRect = CRect(m_WindowRect + Offset).SDLRect();
			SDL_BlitSurface(m_pSDLSurface, &SourceRect, &ScreenSurface, &DestRect);
			CPoint NewOffset = m_ClientRect.TopLeft() + m_WindowRect.TopLeft() + Offset;
			for (const auto& child : m_ChildWindows)
			{
				if (child)
				{
					child->PaintToSurface(ScreenSurface, FloatingSurface, NewOffset);
				}
			}
		}	
		else
		{			
			SDL_Rect DestGhostRect = CRect(m_FrameGhostRect + Offset).SDLRect();
//			SDL_BlitSurface(m_pSDLSurface, &SourceRect, &ScreenSurface, &DestGhostRect);
//			Original line:
			SDL_BlitSurface(m_pSDLSurface, &SourceRect, &FloatingSurface, &DestGhostRect);
			for (const auto& child : m_ChildWindows)
			{
				if (child)
				{
                // (*iter)->PaintToSurface(ScreenSurface, FloatingSurface, m_ClientRect.TopLeft() + m_FrameGhostRect.TopLeft() + Offset);
				// Original line:
					child->PaintToSurface(FloatingSurface, FloatingSurface, m_ClientRect.TopLeft() + m_FrameGhostRect.TopLeft() + Offset);
				}
			}
			// this is a quick trick to convert the surface to being transparent
			// judb there seems to be a problem while dragging the window ; have to find out (has to do with the
            // fact that the WindowRect and ClientRect are not at (0,0), there may be a problem with FloatingSurface
			// Original lines (together with the lines a bit higher I commented out)
			CPainter Painter(&FloatingSurface, CPainter::PAINT_AND);
			Painter.DrawRect(m_FrameGhostRect + Offset, true, CRGBColor(0xFF, 0xFF, 0xFF, 0x40), CRGBColor(0xFF, 0xFF, 0xFF, 0xC0));
		}
	}
}


void CFrame::SetTitleBarHeight(int iTitleBarHeight)
{
	m_iTitleBarHeight = iTitleBarHeight;
  if (m_iTitleBarHeight > 0) {
    delete m_pFrameCloseButton;
    m_pFrameCloseButton = new CPictureButton(CRect(0, 0, 8, 8),
        this, CwgBitmapResourceHandle(WGRES_X_BITMAP));
  } else {
    delete m_pFrameCloseButton;
    m_pFrameCloseButton = nullptr;
  }
	SetWindowRect(m_WindowRect);
}


void CFrame::AttachMenu(CMenu* pMenu)
{
	delete m_pMenu;
	m_pMenu = pMenu;
	if (m_pMenu)
	{
		int iMenuHeight = m_pMenu->GetWindowRect().Height();
		m_pMenu->SetWindowRect(CRect(0, -iMenuHeight, m_WindowRect.Width() - 1, -1));
		m_ClientRect.SetTop(iMenuHeight + 1);
		m_ClientRect.ClipTo(m_WindowRect.SizeRect());
	}
	else
	{
		m_ClientRect = m_WindowRect.SizeRect();
	}
}


void CFrame::SetWindowRect(const CRect& WindowRect)  // virtual
{
	m_TitleBarRect = CRect(3, 2, WindowRect.Width() - 4, m_iTitleBarHeight);
  if (m_pFrameCloseButton) 
    m_pFrameCloseButton->SetWindowRect(CRect(CPoint(WindowRect.Width() - 15, (-m_iTitleBarHeight / 2) - 5), 9, 9));
	m_ClientRect = CRect(2, m_iTitleBarHeight + 2, WindowRect.Width() - 1, WindowRect.Height() - 1);
	// SetWindowRect() must be called last since it calls Draw(), and needs the titlebar rect and such to be set first
	CWindow::SetWindowRect(WindowRect);
}


void CFrame::SetWindowText(const std::string& sWindowText)  // virtual
{
	m_pRenderedString.reset(new CRenderedString(m_pFontEngine, sWindowText, CRenderedString::VALIGN_CENTER));
	CWindow::SetWindowText(sWindowText);
}


bool CFrame::OnMouseButtonDown(CPoint Point, unsigned int Button)  // virtual
{
	bool bResult = CWindow::OnMouseButtonDown(Point, Button);

 	if (!bResult && m_bVisible && (m_WindowRect.SizeRect().HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE))
	{
		if (m_TitleBarRect.HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE)
		{
			m_bDragMode = true;
			m_DragPointerStart = Point;
			m_FrameGhostRect = m_WindowRect;
			CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
		}
		SetNewParent(m_pParentWindow);	// This moves the window to the top
		bResult = true;
	}

	return bResult;
}


bool CFrame::HandleMessage(CMessage* pMessage)  // virtual
{
	bool bHandled = false;

	if (pMessage)
	{
		switch(pMessage->MessageType())
		{
      case CMessage::MOUSE_MOVE:  // intentional fall through
      case CMessage::MOUSE_BUTTONUP:
      {
        CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
        if (pMouseMessage && m_bDragMode)
        {
          CRect MovedRect = m_WindowRect + (pMouseMessage->Point - m_DragPointerStart);
          CRect Bounds = m_pParentWindow->GetClientRect().SizeRect();

  //        if (MovedRect.Right() > Bounds.Right())
  //				{
  //					MovedRect.Move(Bounds.Right() - MovedRect.Right(), 0);
  //				}
  //				if (MovedRect.Left() < Bounds.Left())
  //				{
  //					MovedRect.Move(Bounds.Left() - MovedRect.Left(), 0);
  //				}
  //				if (MovedRect.Bottom() > Bounds.Bottom())
  //				{
  //					MovedRect.Move(0, Bounds.Bottom() - MovedRect.Bottom());
  //				}
  //				if (MovedRect.Top() < Bounds.Top())
  //				{
  //					MovedRect.Move(0, Bounds.Top() - MovedRect.Top());
  //				}
          if (pMessage->MessageType() == CMessage::MOUSE_BUTTONUP)
          {
            m_WindowRect = MovedRect;
            m_bDragMode = false;
            bHandled = true;
          }
          else
          {
            m_FrameGhostRect = MovedRect;
          }
          CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
        }
        break;
      }
      case CMessage::CTRL_SINGLELCLICK:
      {
        if (pMessage->Destination() == this)
        {
          if (pMessage->Source() == m_pFrameCloseButton)
          {
            CloseFrame();
            bHandled = true;
          }
        }
        break;
      }
      case CMessage::KEYBOARD_KEYDOWN:
      {
        if (m_bVisible && pMessage->Destination() == this) {
          CKeyboardMessage* pKeyboardMessage = dynamic_cast<CKeyboardMessage*>(pMessage);
          if (pKeyboardMessage) {
            switch (pKeyboardMessage->Key) {
              case SDLK_ESCAPE:
                CloseFrame();
                bHandled = true;
                break;
              case SDLK_TAB:
                bHandled = true;
                if(pKeyboardMessage->Modifiers & KMOD_SHIFT) {
                  CFrame::FocusNext(EFocusDirection::BACKWARD);
                } else {
                  CFrame::FocusNext(EFocusDirection::FORWARD);
                }
                break;
              case SDLK_SPACE:
              case SDLK_RETURN:
                {
                  CWindow *target = GetFocused();
                  if (target) {
                    bHandled = true;
                    CMessageServer::Instance().QueueMessage(new TIntMessage(
                          CMessage::CTRL_SINGLELCLICK, target->GetAncestor(PARENT), target,
                          0));
                  }
                  break;
                }
              default:
                break;
            }
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

void CFrame::AddFocusableWidget(CWindow *pWidget)
{
  if (pWidget && pWidget->IsFocusable()) {
    // TODO: we can end-up with a non-visible widget being focused. This is not a big issue but still not super cool
    //if (find_if(m_FocusableWidgets.begin(), m_FocusableWidgets.end(), [](CWindow *w) -> bool { return w->IsVisible(); }) == m_FocusableWidgets.end()) {
    if (m_FocusableWidgets.empty()) {
      pWidget->SetHasFocus(true);
    }
    m_FocusableWidgets.push_back(pWidget);
  }
}

void CFrame::RemoveFocusableWidget(CWindow *pWidget)
{
  if (pWidget && pWidget->HasFocus()) {
    FocusNext(EFocusDirection::FORWARD);
  }
  // The widget can still have the focus if it's the only focusable one
  if (pWidget && pWidget == GetFocused()) {
    pWidget->SetHasFocus(false);
  }
  m_FocusableWidgets.remove(pWidget);
}

CWindow *CFrame::GetFocused() {
  auto focused = std::find_if(m_FocusableWidgets.begin(), m_FocusableWidgets.end(), [](CWindow *w) { return w->HasFocus(); });
  if(focused == m_FocusableWidgets.end()) {
    return nullptr;
  }
  return *focused;
}

void CFrame::FocusNext(EFocusDirection direction, bool loop)
{
  CWindow *to_unfocus = nullptr;
  auto loop_body = [&to_unfocus](CWindow* w) {
    if(to_unfocus != nullptr) {
      if(w->IsVisible()) {
        to_unfocus->SetHasFocus(false);
        w->SetHasFocus(true);
        to_unfocus = nullptr;
      }
    } else if(w->HasFocus()) {
      to_unfocus = w;
    }
  };

  do {
    if(direction == EFocusDirection::BACKWARD)
      std::for_each(m_FocusableWidgets.rbegin(), m_FocusableWidgets.rend(), loop_body);
    else
      std::for_each(m_FocusableWidgets.begin(), m_FocusableWidgets.end(), loop_body);
  } while(loop && 
      (loop = !loop     // Ensure we loop only once even if there's just one focusable widget
       || to_unfocus)); // If to_unfocus is not null, it means the focused widget is the last one eligible and we need to loop
}

}

