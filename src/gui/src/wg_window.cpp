// wg_window.cpp
//
// CWindow class implementation
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

#include "wg_window.h"
#include "std_ex.h"
#include "wg_painter.h"
#include "wg_application.h"
#include "wg_error.h"
#include "wg_message_server.h"
#include "wg_view.h"
#include <algorithm>
#include <string>
#include "log.h"

namespace wGui
{

CWindow::CWindow(const CRect& WindowRect, CWindow* pParent) :
	CMessageClient(pParent->Application()),
	m_WindowRect(WindowRect),
	m_BackgroundColor(DEFAULT_BACKGROUND_COLOR),
	m_ClientRect(WindowRect.SizeRect()),
	m_pParentWindow(nullptr),
	m_pSDLSurface(nullptr),
	m_bVisible(true),
  m_bHasFocus(false),
  m_bIsFocusable(false)
{
	SetWindowRect(WindowRect);
	m_BackgroundColor = Application().GetDefaultBackgroundColor();
	SetNewParent(pParent);
}

CWindow::CWindow(CApplication& application, const CRect& WindowRect) :
	CMessageClient(application),
	m_WindowRect(WindowRect),
	m_BackgroundColor(DEFAULT_BACKGROUND_COLOR),
	m_ClientRect(WindowRect.SizeRect()),
	m_pParentWindow(nullptr),
	m_pSDLSurface(nullptr),
	m_bVisible(true),
  m_bHasFocus(false),
  m_bIsFocusable(false)
{
	SetWindowRect(WindowRect);
	m_BackgroundColor = Application().GetDefaultBackgroundColor();
}

// judb constructor like above, but without specifying a CRect ;
// In this case you need to call SetWindowRect() before using the CWindow!
CWindow::CWindow(CWindow* pParent) :
	CMessageClient(pParent->Application()),
	m_BackgroundColor(DEFAULT_BACKGROUND_COLOR),
	m_pParentWindow(nullptr),
	m_pSDLSurface(nullptr),
	m_bVisible(true),
  m_bHasFocus(false)
{
	m_BackgroundColor = Application().GetDefaultBackgroundColor();
	SetNewParent(pParent);
}


CWindow::~CWindow()
{
	// Each child window is deleted, and should in their destructors call back to this object to Deregister themselves
	Application().MessageServer()->DeregisterMessageClient(this);

	if (m_pSDLSurface)
		SDL_FreeSurface(m_pSDLSurface);
	while (!m_ChildWindows.empty())
	{
		delete *(m_ChildWindows.begin());
	}
	m_ChildWindows.clear();
	SetNewParent(nullptr);
}


void CWindow::SetWindowRect(const CRect& WindowRect)
{
	double dHorizontalScale = m_WindowRect.Width() != 0 ? static_cast<double>(WindowRect.Width()) / m_WindowRect.Width() : 0;
	double dVerticalScale = m_WindowRect.Height() != 0 ? static_cast<double>(WindowRect.Height()) / m_WindowRect.Height() : 0;
	m_WindowRect = WindowRect;
	if (m_pSDLSurface)
		SDL_FreeSurface(m_pSDLSurface);
	m_pSDLSurface = SDL_CreateRGBSurface(0, m_WindowRect.Width(), m_WindowRect.Height(),
		Application().GetBitsPerPixel(), 0x000000FF, 0x0000FF00, 0x00FF0000, /*0xFF000000*/ 0);
	if (!m_pSDLSurface)
	{
    LOG_ERROR("CWindow::SetWindowRect: Unable to create SDL Surface of size " << m_WindowRect << ": " << SDL_GetError());
	}
	m_ClientRect = CRect(stdex::safe_static_cast<int>(m_ClientRect.Left() * dHorizontalScale), stdex::safe_static_cast<int>(m_ClientRect.Top() * dVerticalScale),
		stdex::safe_static_cast<int>(m_ClientRect.Right() * dHorizontalScale), stdex::safe_static_cast<int>(m_ClientRect.Bottom() * dVerticalScale));
	Draw();  // we need to redraw here because we've got a new buffer
}


void CWindow::MoveWindow(const CPoint& MoveDistance)
{
	m_WindowRect = m_WindowRect + MoveDistance;
	Application().MessageServer()->QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
}


CWindow* CWindow::GetAncestor(EAncestor eAncestor) const
{
	CWindow* pWindow = nullptr;

	switch (eAncestor)
	{
	case PARENT:
		pWindow = m_pParentWindow;
		break;
	case ROOT:
		pWindow = m_pParentWindow;
		if (pWindow)
		{
			while (pWindow->m_pParentWindow)
			{
				pWindow = pWindow->m_pParentWindow;
			}
		}
		else
		{
			pWindow = const_cast<CWindow*>(this);
		}
		break;
	}

	return pWindow;
}


CView* CWindow::GetView() const  // virtual
{
	return dynamic_cast<CView*>(GetAncestor(ROOT));
}


bool CWindow::IsChildOf(CWindow* pWindow) const
{
	const CWindow* pCurrentWindow = this;
	bool bIsChild = false;

	while (!bIsChild && pCurrentWindow)
	{
		pCurrentWindow = pCurrentWindow->GetAncestor(PARENT);
		if (pCurrentWindow == pWindow)
		{
			bIsChild = true;
		}
	}

	return bIsChild;
}


void CWindow::SetVisible(bool bVisible)
{
	if (m_bVisible != bVisible)
	{
		m_bVisible = bVisible;
		for (std::list<CWindow*>::const_iterator iter = m_ChildWindows.begin(); iter != m_ChildWindows.end(); ++iter)
		{
			(*iter)->SetVisible(bVisible);
			if (!bVisible && (*iter) == Application().GetKeyFocus())
			{
				Application().SetKeyFocus(m_pParentWindow);
			}
		}
		Application().MessageServer()->QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
	}
}


void CWindow::SetHasFocus(bool bHasFocus)
{
  if (bHasFocus) {
    Application().SetKeyFocus(this);
  } else {
    Application().SetKeyFocus(m_pParentWindow);
  }
  m_bHasFocus = bHasFocus;
  Draw();
}


void CWindow::SetIsFocusable(bool bIsFocusable)
{
  m_bIsFocusable = bIsFocusable;
  if (m_bIsFocusable) {
    m_pParentWindow->AddFocusableWidget(this);
  } else {
    m_pParentWindow->RemoveFocusableWidget(this);
  }
}


CRect CWindow::ClientToView(const CRect& Rect) const
{
	CRect ScreenRect(Rect + m_ClientRect.TopLeft() + m_WindowRect.TopLeft());
	if (m_pParentWindow)
	{
		ScreenRect = m_pParentWindow->ClientToView(ScreenRect);
	}
	return ScreenRect;
}


CPoint CWindow::ClientToView(const CPoint& Point) const
{
	CPoint ScreenPoint(Point + m_ClientRect.TopLeft() + m_WindowRect.TopLeft());
	if (m_pParentWindow)
	{
		ScreenPoint = m_pParentWindow->ClientToView(ScreenPoint);
	}
	return ScreenPoint;
}


CRect CWindow::ViewToClient(const CRect& Rect) const
{
	CRect WindowRect(Rect - m_WindowRect.TopLeft() - m_ClientRect.TopLeft());
	if (m_pParentWindow)
	{
		WindowRect = m_pParentWindow->ViewToClient(WindowRect);
	}
	return WindowRect;
}


CPoint CWindow::ViewToClient(const CPoint& Point) const
{
	CPoint WindowPoint(Point - m_WindowRect.TopLeft() - m_ClientRect.TopLeft());
	if (m_pParentWindow)
	{
		WindowPoint = m_pParentWindow->ViewToClient(WindowPoint);
	}
	return WindowPoint;
}


CRect CWindow::ViewToWindow(const CRect& Rect) const
{
	CRect WindowRect(Rect - m_WindowRect.TopLeft());
	if (m_pParentWindow)
	{
		WindowRect = m_pParentWindow->ViewToClient(WindowRect);
	}
	return WindowRect;
}


CPoint CWindow::ViewToWindow(const CPoint& Point) const
{
	CPoint WindowPoint(Point - m_WindowRect.TopLeft());
	if (m_pParentWindow)
	{
		WindowPoint = m_pParentWindow->ViewToClient(WindowPoint);
	}
	return WindowPoint;
}


void CWindow::SetWindowText(const std::string& sText)
{
	m_sWindowText = sText;
	Draw();
}


bool CWindow::HitTest(const CPoint& Point) const
{
	if (m_WindowRect.SizeRect().HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE) {
    return true;
  };
  return std::any_of(m_ChildWindows.begin(), m_ChildWindows.end(), [&](const auto& child) { return child->HitTest(Point); });
}


void CWindow::Draw() const
{
	if (m_pSDLSurface)	{

		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		Painter.DrawRect(m_WindowRect.SizeRect(), true, m_BackgroundColor, m_BackgroundColor);
		Application().MessageServer()->QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
	}
}


void CWindow::PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const
{
	if (m_bVisible)
	{
		SDL_Rect SourceRect = CRect(m_WindowRect.SizeRect()).SDLRect();
		SDL_Rect DestRect = CRect(m_WindowRect + Offset).SDLRect();
		SDL_BlitSurface(m_pSDLSurface, &SourceRect, &ScreenSurface, &DestRect);
		CPoint NewOffset = m_ClientRect.TopLeft() + m_WindowRect.TopLeft() + Offset;
    for (const auto &child : m_ChildWindows)
		{
			if (child)
			{
				child->PaintToSurface(ScreenSurface, FloatingSurface, NewOffset);
			}
		}
	}
}


void CWindow::SetNewParent(CWindow* pNewParent)
{
	if (m_pParentWindow)
	{
		m_pParentWindow->DeregisterChildWindow(this);
	}
	if (pNewParent)
	{
		pNewParent->RegisterChildWindow(this);
	}
	m_pParentWindow = pNewParent;
}


bool CWindow::OnMouseButtonDown(CPoint Point, unsigned int Button)
{
	bool bResult = false;

	if (m_bVisible && (m_WindowRect.SizeRect().HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE))
	{
		for (auto iter = m_ChildWindows.rbegin(); iter != m_ChildWindows.rend(); ++iter)
		{
			bResult = (*iter)->OnMouseButtonDown(Point, Button);
			if (bResult)
			{
				break;
			}
		}
	}

	return bResult;
}


bool CWindow::OnMouseButtonUp(CPoint Point, unsigned int Button)
{
	bool bResult = false;

	if (m_bVisible && (m_WindowRect.SizeRect().HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE))
	{
		for (auto iter = m_ChildWindows.rbegin(); iter != m_ChildWindows.rend(); ++iter)
		{
			bResult = (*iter)->OnMouseButtonUp(Point, Button);
			if (bResult)
			{
				break;
			}
		}
	}

	return bResult;
}


void CWindow::RegisterChildWindow(CWindow* pWindow)
{
	if (!pWindow)
	{
		// anything that gets registered should be a valid CWindow
    LOG_ERROR("CWindow::RegisterChildWindow: Attempting to register a non-existent child window.");
	}
	else
	{
		m_ChildWindows.push_back(pWindow);
	}
}


void CWindow::DeregisterChildWindow(CWindow* pWindow)
{
	m_ChildWindows.remove(pWindow);
}


bool CWindow::HandleMessage(CMessage* /*pMessage*/)
{
	bool bHandled = false;

	return bHandled;
}

void CWindow::AddFocusableWidget(CWindow *pWidget)
{
  if (m_pParentWindow)
  {
    m_pParentWindow->AddFocusableWidget(pWidget);
  }
}

void CWindow::RemoveFocusableWidget(CWindow *pWidget)
{
  if (m_pParentWindow)
  {
    m_pParentWindow->RemoveFocusableWidget(pWidget);
  }
}

}

