// wg_menu.cpp
//
// CMenu implementation
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


#include "wg_menu.h"
#include "std_ex.h"
#include "wg_application.h"
#include "wg_message_server.h"
#include "wg_view.h"
#include "wg_error.h"


namespace wGui
{

// CMenuBase

CMenuBase::CMenuBase(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
	CWindow(WindowRect, pParent),
	m_pHighlightedItem(nullptr),
	m_bCachedRectsValid(false),
	m_pActivePopup(nullptr),
	m_hRightArrowBitmap(Application(), WGRES_RIGHT_ARROW_BITMAP),
	m_HighlightColor(DEFAULT_BACKGROUND_COLOR),
	m_pPopupTimer(nullptr)
{
	if (pFontEngine)
	{
		m_pFontEngine = pFontEngine;
	}
	else
	{
		m_pFontEngine = Application().GetDefaultFontEngine();
	}

	m_HighlightColor = Application().GetDefaultSelectionColor();

	Application().MessageServer()->RegisterMessageClient(this, CMessage::MOUSE_MOVE);
	Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_SINGLELCLICK);
	Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_TIMER);

	m_pPopupTimer = new CTimer(pParent->Application(), this);
}


CMenuBase::~CMenuBase()
{
	delete m_pPopupTimer;
	m_pPopupTimer = nullptr;
}


void CMenuBase::InsertMenuItem(const SMenuItem& MenuItem, int iPosition)
{
	m_MenuItems.insert((iPosition == -1) ? m_MenuItems.end() : m_MenuItems.begin() + iPosition,
		s_MenuItemInfo(MenuItem, CRenderedString(m_pFontEngine, MenuItem.sItemText, CRenderedString::VALIGN_TOP), CRect()));
	m_bCachedRectsValid = false;
//	Draw();
}


void CMenuBase::RemoveMenuItem(int iPosition)
{
	m_MenuItems.erase(m_MenuItems.begin() + iPosition);
	m_bCachedRectsValid = false;
	Draw();
}


void CMenuBase::HideActivePopup()
{
	if (m_pActivePopup)
	{
		m_pActivePopup->Hide();
		m_pActivePopup = nullptr;
	}
}


bool CMenuBase::OnMouseButtonDown(CPoint Point, unsigned int Button)
{
	// We can't do quite the same thing as normal since children of menus don't appear inside their bounds
	bool bResult = false;

	if (m_bVisible)
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


bool CMenuBase::HandleMessage(CMessage* pMessage)
{
	bool bHandled = false;

	if (pMessage)
	{
		switch(pMessage->MessageType())
		{
		case CMessage::MOUSE_MOVE:
		{
			CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
			if (m_bVisible && pMouseMessage &&
				m_WindowRect.SizeRect().HitTest(ViewToWindow(pMouseMessage->Point)) == CRect::RELPOS_INSIDE)
			{
				UpdateCachedRects();
				const SMenuItem* pOldHighlight = m_pHighlightedItem;
				m_pHighlightedItem = nullptr;
				CPoint WindowPoint(ViewToWindow(pMouseMessage->Point));
				for (const auto &item : m_MenuItems)
				{
					if (item.Rect.HitTest(WindowPoint) == CRect::RELPOS_INSIDE && !item.MenuItem.bSpacer)
					{
						m_pHighlightedItem = &(item.MenuItem);
						break;
					}
				}
				if (pOldHighlight != m_pHighlightedItem)
				{
					m_pPopupTimer->StopTimer();
					if (m_pHighlightedItem && m_pHighlightedItem->pPopup)
					{
						m_pPopupTimer->StartTimer(1000);
					}
					Draw();
				}
			}
			else if (m_pHighlightedItem != nullptr)
			{
				m_pPopupTimer->StopTimer();
				m_pHighlightedItem = nullptr;
				Draw();
			}
			break;
		}
		case CMessage::CTRL_SINGLELCLICK:
		{
			TIntMessage* pCtrlMessage = dynamic_cast<TIntMessage*>(pMessage);
			if (pCtrlMessage && pCtrlMessage->Destination() == this)
			{
				for (const auto &item : m_MenuItems)
				{
					if (pCtrlMessage->Source() == item.MenuItem.pPopup)
					{
						Application().MessageServer()->QueueMessage(new TIntMessage(CMessage::CTRL_SINGLELCLICK, m_pParentWindow,
							this, pCtrlMessage->Value()));
						bHandled = true;
						break;
					}
				}
			}
			break;
		}
		case CMessage::CTRL_TIMER:
		{
			if (pMessage->Destination() == this)
			{
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


// CMenu

CMenu::CMenu(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
	CMenuBase(WindowRect, pParent, pFontEngine)
{
	Application().MessageServer()->RegisterMessageClient(this, CMessage::MOUSE_MOVE);
	Draw();
}


CMenu::~CMenu() = default;


void CMenu::InsertMenuItem(const SMenuItem& MenuItem, int iPosition)
{
	m_MenuItems.insert((iPosition == -1) ? m_MenuItems.end() : m_MenuItems.begin() + iPosition,
		s_MenuItemInfo(MenuItem, CRenderedString(m_pFontEngine, MenuItem.sItemText, CRenderedString::VALIGN_NORMAL), CRect()));
	m_bCachedRectsValid = false;
	if (MenuItem.pPopup)
	{
		MenuItem.pPopup->SetParentMenu(this);
	}
	Draw();
}


void CMenu::Draw() const
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		UpdateCachedRects();
    for (const auto &item : m_MenuItems)
		{
			if (m_pHighlightedItem == &(item.MenuItem))
			{
				Painter.DrawRect(item.Rect, true, m_HighlightColor, m_HighlightColor);
			}
			CRect TextRect(item.Rect);
			TextRect.Grow(-2);
			if (item.MenuItem.bSpacer)
			{
				Painter.DrawVLine(TextRect.Top(), TextRect.Bottom(), TextRect.Left(), COLOR_LIGHTGRAY);
				Painter.DrawVLine(TextRect.Top(), TextRect.Bottom(), TextRect.Right(), COLOR_DARKGRAY);
			}
			else
				item.RenderedString.Draw(m_pSDLSurface, TextRect, CPoint(TextRect.Left(), (TextRect.Top() + TextRect.Bottom()) * 3 / 4));
		}
	}
}


bool CMenu::OnMouseButtonDown(CPoint Point, unsigned int Button)
{
	bool bResult = CMenuBase::OnMouseButtonDown(Point, Button);

 	if (!bResult && m_bVisible && (Button == CMouseMessage::LEFT) &&
		(m_WindowRect.SizeRect().HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE))
	{
		UpdateCachedRects();
      for (const auto &item : m_MenuItems)
		{
			if (item.Rect.HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE && !item.MenuItem.bSpacer)
			{
				HideActivePopup();
				if (item.MenuItem.pPopup)
				{
					CPopupMenu* pPopup = dynamic_cast<CPopupMenu*>(item.MenuItem.pPopup);
					if (pPopup)
					{
						m_pActivePopup = pPopup;
						ShowActivePopup(item.Rect, GetAncestor(ROOT)->GetClientRect());
					}
				}
				else
				{
					Application().MessageServer()->QueueMessage(new TIntMessage(CMessage::CTRL_SINGLELCLICK, m_pParentWindow, this, item.MenuItem.iItemId));
				}
				break;
			}
		}
		bResult = true;
	}

	return bResult;
}


bool CMenu::HandleMessage(CMessage* pMessage)
{
	bool bHandled = false;

	if (pMessage)
	{
		switch(pMessage->MessageType())
		{
		case CMessage::MOUSE_MOVE:
		{
			CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
			if (m_bVisible && pMouseMessage &&
				m_WindowRect.SizeRect().HitTest(ViewToWindow(pMouseMessage->Point)) == CRect::RELPOS_INSIDE)
			{
				UpdateCachedRects();
				const SMenuItem* pOldHighlight = m_pHighlightedItem;
				m_pHighlightedItem = nullptr;
				CPoint WindowPoint(ViewToWindow(pMouseMessage->Point));
				for (const auto &item : m_MenuItems)
				{
					if (item.Rect.HitTest(WindowPoint) == CRect::RELPOS_INSIDE && !item.MenuItem.bSpacer)
					{
						m_pHighlightedItem = &(item.MenuItem);
						break;
					}
				}
				if (pOldHighlight != m_pHighlightedItem)
				{
					Draw();
				}
			}
			else if (m_pHighlightedItem != nullptr)
			{
				m_pHighlightedItem = nullptr;
				Draw();
			}
			break;
		}
		default :
			bHandled = CMenuBase::HandleMessage(pMessage);
			break;
		}
	}

	return bHandled;
}


void CMenu::UpdateCachedRects() const
{
	if (!m_bCachedRectsValid)
	{
		CRect SubRect(m_WindowRect.SizeRect());
		SubRect.Grow(-2);
		int iWidth = 5;
    for (auto &item : m_MenuItems)
		{
			if (item.MenuItem.bSpacer)
			{
				CRect TextRect(SubRect.Left() + iWidth, SubRect.Top() + 2, SubRect.Left() + iWidth + 1, SubRect.Bottom() - 2);
				TextRect.Grow(2);
				item.Rect = TextRect;
				iWidth += 9;
			}
			else
			{
				CPoint Dims;
				item.RenderedString.GetMetrics(&Dims, nullptr, nullptr);
				CRect TextRect(SubRect.Left() + iWidth, SubRect.Top() + 2, SubRect.Left() + iWidth + Dims.XPos(), SubRect.Bottom() - 2);
				TextRect.Grow(2);
				item.Rect = TextRect;
				iWidth += Dims.XPos() + 8;
			}
		}
		m_bCachedRectsValid = true;
	}
}


void CMenu::ShowActivePopup(const CRect& ParentRect, const CRect& BoundingRect)
{
	// TODO: Add bounds checking for all edges.	Apply this same bounds checking logic to the popup root menu too.
	if (!m_pActivePopup)
	{
		throw(Wg_Ex_App("Trying to show active popup menu when pActivePopup is NULL!", "CMenu::ShowActivePopup"));
	}
	CRect MenuRect = m_pActivePopup->GetWindowRect();
	if (BoundingRect.HitTest(ParentRect.BottomLeft() + CPoint(MenuRect.Width(), 0)) & CRect::RELPOS_RIGHT)
	{
		int xDelta = BoundingRect.Right() - ParentRect.Left() + MenuRect.Width();
		if (!(BoundingRect.HitTest(ParentRect.BottomLeft() + CPoint(xDelta, 0)) & CRect::RELPOS_LEFT))
		{
			m_pActivePopup->Show(ParentRect.BottomLeft() + CPoint(xDelta, 5));
		}
	}
	else
	{
		m_pActivePopup->Show(ParentRect.BottomLeft() + CPoint(0, 5));
	}
	// Pop the popup to the top of the draw order and everything
	m_pActivePopup->SetNewParent(m_pActivePopup->GetAncestor(PARENT));
}


// CPopupMenu

CPopupMenu::CPopupMenu(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
	CMenuBase(WindowRect, pParent, pFontEngine),
	m_pParentMenu(nullptr)
{
	m_bVisible = false;
	Application().MessageServer()->RegisterMessageClient(this, CMessage::MOUSE_BUTTONDOWN);
	Draw();
}


CPopupMenu::~CPopupMenu() = default;


void CPopupMenu::Show(CPoint Position)
{
	if (m_bVisible)
	{
		Hide();
	}

	int iHeight = 4;
    for (const auto &item : m_MenuItems)
	{
		if (item.MenuItem.bSpacer)
			iHeight += 6;
		else
		{
			CPoint Dims;
			item.RenderedString.GetMetrics(&Dims, nullptr, nullptr);
			iHeight += Dims.YPos() + 5;
		}
	}

	//Bounds checking for menus, so that we don't draw off screen, since we can't.
	CRect TestRect = GetAncestor(ROOT)->GetWindowRect();

	if ( Position.XPos() < 0 )
	{
		Position.SetX(0);
	}

	if ( Position.YPos() < 0 )
	{
		Position.SetY(0);
	}

	if ( Position.XPos() + m_WindowRect.Width() > TestRect.Width() )
	{
		Position.SetX(Position.XPos() - ((Position.XPos() + m_WindowRect.Width()) - TestRect.Width()));
	}

	if ( Position.YPos() + m_WindowRect.Height() > TestRect.Height() )
	{
		Position.SetY(Position.YPos() - ((Position.YPos() + m_WindowRect.Height()) - TestRect.Bottom()));
	}

	SetWindowRect(GetAncestor(PARENT)->ViewToClient(CRect(Position, Position + CPoint(m_WindowRect.Width() - 1, iHeight + 2))));
	m_bVisible = true;
	CView* pView = GetView();
	if (pView && !dynamic_cast<CPopupMenu*>(m_pParentWindow))
	{
		pView->SetFloatingWindow(this);
	}
	Draw();
}


void CPopupMenu::Hide()
{
	HideActivePopup();
	m_bVisible = false;
	m_bCachedRectsValid = false;
	CView* pView = GetView();
	if (!dynamic_cast<CPopupMenu*>(m_pParentWindow) && pView && pView->GetFloatingWindow() == this)
	{
		pView->SetFloatingWindow(nullptr);
	}
	Application().MessageServer()->QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
	m_pHighlightedItem = nullptr;
	Draw();
}


void CPopupMenu::HideAll()
{
	CPopupMenu* pParentPopup = dynamic_cast<CPopupMenu*>(m_pParentWindow);
	if (pParentPopup)
	{
		pParentPopup->HideAll();
	}
	else
	{
		CMenu* pRootMenu = dynamic_cast<CMenu*>(m_pParentWindow);
		if (pRootMenu)
		{
			pRootMenu->HideActivePopup();
		}
		else
		{
			Hide();
		}
	}
}


bool CPopupMenu::IsInsideChild(const CPoint& Point) const
{
	if (m_WindowRect.SizeRect().HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE)
	{
		return true;
	}
	if (!m_pActivePopup)
	{
		return false;
	}
	return m_pActivePopup->IsInsideChild(Point);
}


void CPopupMenu::Draw() const
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		Painter.DrawRect(m_WindowRect.SizeRect(), false, COLOR_LIGHTGRAY);
		Painter.DrawHLine(0, m_WindowRect.Width(), m_WindowRect.Height(), COLOR_DARKGRAY);
		Painter.DrawVLine(0, m_WindowRect.Height(), m_WindowRect.Width(), COLOR_DARKGRAY);
		UpdateCachedRects();
      for (const auto &item : m_MenuItems)
		{
			if (m_pHighlightedItem == &(item.MenuItem))
			{
				Painter.DrawRect(item.Rect, true, m_HighlightColor, m_HighlightColor);
			}
			CRect TextRect(item.Rect);
			TextRect.Grow(-2);
			if (item.MenuItem.bSpacer)
			{
				Painter.DrawHLine(TextRect.Left(), TextRect.Right(), TextRect.Top(), COLOR_LIGHTGRAY);
				Painter.DrawHLine(TextRect.Left(), TextRect.Right(), TextRect.Bottom(), COLOR_DARKGRAY);
			}
			else
				item.RenderedString.Draw(m_pSDLSurface, TextRect, TextRect.TopLeft());
			if (item.MenuItem.pPopup)
			{
				CRect ArrowRect(item.Rect);
				ArrowRect.SetLeft(ArrowRect.Right() - m_hRightArrowBitmap.Bitmap()->w);
				SDL_Rect SourceRect;
				SourceRect.x = stdex::safe_static_cast<short int>(0);
				SourceRect.y = stdex::safe_static_cast<short int>((m_hRightArrowBitmap.Bitmap()->h - ArrowRect.Height()) / 2 < 0 ?
					0 : (m_hRightArrowBitmap.Bitmap()->h - ArrowRect.Height()) / 2);
				SourceRect.w = stdex::safe_static_cast<short int>(m_hRightArrowBitmap.Bitmap()->w);
				SourceRect.h = stdex::safe_static_cast<short int>(std::min(ArrowRect.Height(), m_hRightArrowBitmap.Bitmap()->h));
				SDL_Rect DestRect;
				DestRect.x = stdex::safe_static_cast<short int>(ArrowRect.Left());
				DestRect.y = stdex::safe_static_cast<short int>((ArrowRect.Height() - m_hRightArrowBitmap.Bitmap()->h) / 2 < 0 ?
					ArrowRect.Top() : ArrowRect.Top() + (ArrowRect.Height() - m_hRightArrowBitmap.Bitmap()->h) / 2);
				DestRect.w = stdex::safe_static_cast<short int>(m_hRightArrowBitmap.Bitmap()->w);
				DestRect.h = stdex::safe_static_cast<short int>(std::min(ArrowRect.Height(), m_hRightArrowBitmap.Bitmap()->h));
				SDL_BlitSurface(m_hRightArrowBitmap.Bitmap(), &SourceRect, m_pSDLSurface, &DestRect);
			}
		}
	}
}


void CPopupMenu::PaintToSurface(SDL_Surface& /*ScreenSurface*/, SDL_Surface& FloatingSurface, const CPoint& Offset) const
{
	if (m_bVisible)
	{
		SDL_Rect SourceRect = CRect(m_WindowRect.SizeRect()).SDLRect();
		SDL_Rect DestRect = CRect(m_WindowRect + Offset).SDLRect();
		SDL_BlitSurface(m_pSDLSurface, &SourceRect, &FloatingSurface, &DestRect);
		CPoint NewOffset = m_ClientRect.TopLeft() + m_WindowRect.TopLeft() + Offset;
    for (const auto& child : m_ChildWindows)
		{
			child->PaintToSurface(FloatingSurface, FloatingSurface, NewOffset);
		}
	}
}


bool CPopupMenu::OnMouseButtonDown(CPoint Point, unsigned int Button)
{
	bool bResult = CMenuBase::OnMouseButtonDown(Point, Button);

	CPoint WindowPoint(ViewToWindow(Point));
 	if (!bResult && m_bVisible && (m_WindowRect.SizeRect().HitTest(WindowPoint) == CRect::RELPOS_INSIDE))
	{
		UpdateCachedRects();
    for (const auto &item : m_MenuItems)
		{
			if (item.Rect.HitTest(WindowPoint) == CRect::RELPOS_INSIDE && !item.MenuItem.bSpacer)
			{
				if (!item.MenuItem.pPopup)
				{
					CMessageClient* pDestination = m_pParentWindow;
					if (m_pParentMenu)
					{
						pDestination = m_pParentMenu;
					}
					Application().MessageServer()->QueueMessage(new TIntMessage(CMessage::CTRL_SINGLELCLICK, pDestination, this, item.MenuItem.iItemId));
					HideAll();
				}
				else
				{
					HideActivePopup();
					m_pActivePopup = item.MenuItem.pPopup;
					ShowActivePopup(item.Rect, GetAncestor(ROOT)->GetClientRect());
				}
				break;
			}
		}
		bResult = true;
	}

	return bResult;
}


bool CPopupMenu::HandleMessage(CMessage* pMessage)
{
	bool bHandled = false;

	if (pMessage)
	{
		switch(pMessage->MessageType())
		{
		case CMessage::MOUSE_BUTTONDOWN:
		{
			CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
			if (m_bVisible && pMouseMessage &&
				m_WindowRect.SizeRect().HitTest(ViewToWindow(pMouseMessage->Point)) != CRect::RELPOS_INSIDE)
			{
				// If the user clicked outside the window, we just want to hide the window,
				// and then pass the message on (bHandled = false)
				// But we only want the root popup to do this, and only if none of it's children were hit
				if (!dynamic_cast<CPopupMenu*>(m_pParentWindow) && !IsInsideChild(pMouseMessage->Point) &&
					!(m_pParentMenu && m_pParentMenu->GetWindowRect().SizeRect().HitTest(
					m_pParentMenu->ViewToWindow(pMouseMessage->Point)) == CRect::RELPOS_INSIDE))
				{
					HideAll();
				}
			}
			break;
		}
		case CMessage::CTRL_TIMER:
		{
			if (pMessage->Destination() == this)
			{
				CRect ItemRect;
				for (const auto &item : m_MenuItems)
				{
					if (m_pHighlightedItem == &(item.MenuItem))
					{
						ItemRect = item.Rect;
						break;
					}
				}
				if (m_pHighlightedItem && m_pHighlightedItem->pPopup)
				{
					HideActivePopup();
					m_pActivePopup = m_pHighlightedItem->pPopup;
					ShowActivePopup(ItemRect, GetAncestor(ROOT)->GetClientRect());
				}
				bHandled = true;
			}
			break;
		}
		default :
			bHandled = CMenuBase::HandleMessage(pMessage);
			break;
		}
	}

	return bHandled;
}


void CPopupMenu::UpdateCachedRects() const
{
	if (!m_bCachedRectsValid)
	{
		CRect SubRect(m_WindowRect.SizeRect());
		SubRect.Grow(-2);
		int iHeight = 4;
    for (auto &item : m_MenuItems)
		{
			if (item.MenuItem.bSpacer)
			{
				CRect TextRect(SubRect.Left() + 3, SubRect.Top() + iHeight, SubRect.Right() - 3, SubRect.Top() + iHeight + 1);
				TextRect.Grow(2);
				item.Rect = TextRect;
				iHeight += 6;
			}
			else
			{
				CPoint Dims;
				item.RenderedString.GetMetrics(&Dims, nullptr, nullptr);
				CRect TextRect(SubRect.Left() + 3, SubRect.Top() + iHeight, SubRect.Right() - 3, SubRect.Top() + iHeight + Dims.YPos());
				TextRect.Grow(2);
				item.Rect = TextRect;
				iHeight += Dims.YPos() + 5;
			}
		}
		m_bCachedRectsValid = true;
	}
}


void CPopupMenu::ShowActivePopup(const CRect& ParentRect, const CRect& BoundingRect)
{
	// TODO: Add bounds checking for all edges.	Apply this same bounds checking logic to the popup root menu too.
	if (!m_pActivePopup)
	{
		throw(Wg_Ex_App("Trying to show active popup menu when pActivePopup is NULL!", "CPopupMenu::ShowActivePopup"));
	}

	CRect MenuRect = m_pActivePopup->GetWindowRect();

	//Test to see if we're going to draw the menu off the right side of the screen, if we are, then we'll correct it to draw to the left of the menu
	if (BoundingRect.HitTest(ParentRect.TopRight() + CPoint(5, 0) + CPoint(MenuRect.Width(), 0)) & CRect::RELPOS_RIGHT)
	{
		m_pActivePopup->Show(ClientToView(ParentRect.TopLeft() - CPoint(MenuRect.Width() + 5, 0)) - m_ClientRect.TopLeft());
	}
	else
	{
		m_pActivePopup->Show(ClientToView(ParentRect.TopRight() + CPoint(5, 0)) - m_ClientRect.TopLeft());
	}
}

}
