// wg_tooltip.cpp
//
// CToolTip interface
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

#include "wg_view.h"
#include "wg_tooltip.h"

namespace wGui
{

CToolTip::CToolTip(CWindow* pToolWindow, std::string sText, CRGBColor& FontColor, CRGBColor& BackgroundColor, CFontEngine* pFontEngine) :
	CWindow(CRect(), pToolWindow),
	m_FontColor(FontColor)
{
	m_sWindowText = sText;
	if (pFontEngine)
	{
		m_pFontEngine = pFontEngine;
	}
	else
	{
		m_pFontEngine = CApplication::Instance()->GetDefaultFontEngine();
	}
	m_pRenderedString.reset(new CRenderedString(
		m_pFontEngine, sText, CRenderedString::VALIGN_TOP, CRenderedString::HALIGN_LEFT));

	m_pTimer = new CTimer(this);

	//Now resize the window so that it fits the Tooltip text
	CPoint Dims;
	m_pRenderedString->GetMetrics(&Dims, nullptr, nullptr);
	m_BoundingRect = CRect(CPoint(0, 0), Dims + CPoint(4, 4));

	m_BackgroundColor = BackgroundColor;
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_MOVE);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_TIMER);
}


CToolTip::~CToolTip(void)
{
	delete m_pTimer;
}


void CToolTip::ShowTip(const CPoint& DrawPoint)
{
	SetWindowRect(m_pParentWindow->ViewToClient(m_BoundingRect + DrawPoint));
	SetVisible(true);
	Draw();
}


void CToolTip::HideTip(void)
{
	SetVisible(false);
	CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
}


void CToolTip::Draw(void) const
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		Painter.DrawRect(m_WindowRect.SizeRect(), false);
		CRect SubRect(m_WindowRect.SizeRect());
		SubRect.Grow(-2);
		if (m_pRenderedString.get())
		{
			m_pRenderedString->Draw(m_pSDLSurface, SubRect, SubRect.TopLeft(), m_FontColor);
		}
	}
}


void CToolTip::MoveWindow(const CPoint& MoveDistance)
{
	CWindow::MoveWindow(MoveDistance);
	m_BoundingRect = m_BoundingRect + MoveDistance;
}


void CToolTip::PaintToSurface(SDL_Surface& /*ScreenSurface*/, SDL_Surface& FloatingSurface, const CPoint& Offset) const
{
	if (m_bVisible)
	{
		SDL_Rect SourceRect = CRect(m_WindowRect.SizeRect()).SDLRect();
		SDL_Rect DestRect = CRect(m_WindowRect + Offset).SDLRect();
		SDL_BlitSurface(m_pSDLSurface, &SourceRect, &FloatingSurface, &DestRect);
		CPoint NewOffset = m_ClientRect.TopLeft() + m_WindowRect.TopLeft() + Offset;
		for (std::list<CWindow*>::const_iterator iter = m_ChildWindows.begin(); iter != m_ChildWindows.end(); ++iter)
		{
			(*iter)->PaintToSurface(FloatingSurface, FloatingSurface, NewOffset);
		}
	}
}


bool CToolTip::HandleMessage(CMessage* pMessage)
{
	bool bHandled = false;

	if (pMessage)
	{
		switch(pMessage->MessageType())
		{
			case CMessage::CTRL_TIMER:
			{
				wGui::TIntMessage* pTimerMessage = dynamic_cast<wGui::TIntMessage*>(pMessage);
				if (pTimerMessage && pMessage->Destination() == this)
				{
					// Timer has expired, so it's time to show the tooltip
					ShowTip(m_LastMousePosition + CPoint(-6, 18));
					bHandled = true;
				}
				break;
			}
		case CMessage::MOUSE_MOVE:
		{
			// We don't want to mess with the underlying control, so don't trap any MOUSE_MOVE messages
			CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
			if (pMouseMessage)
			{
				m_LastMousePosition = pMouseMessage->Point;
				m_pTimer->StopTimer();
				if (IsVisible())
				{
					HideTip();
				}
				CView* pView = GetView();
				bool bHitFloating = pView && pView->GetFloatingWindow() && pView->GetFloatingWindow()->HitTest(pMouseMessage->Point) &&
					pView->GetFloatingWindow() != m_pParentWindow;
				if (m_pParentWindow->GetWindowRect().SizeRect().HitTest(
					m_pParentWindow->ViewToWindow(m_LastMousePosition)) == CRect::RELPOS_INSIDE && !bHitFloating)
				{
					m_pTimer->StartTimer(1000);
				}
			}
		}
		default :
			bHandled = CWindow::HandleMessage(pMessage);
			break;
		}
	}

	return bHandled;
}

}

