// wg_view.cpp
//
// CView class implementation
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

#include "wutil_debug.h"
#include "wg_view.h"
#include "wg_error.h"
#include "wg_painter.h"
#include "wg_message_server.h"
#include "wg_application.h"
#include "wg_frame.h"
#include "std_ex.h"
#include "video.h"
#include "log.h"
#include <algorithm>
#include <string>

extern video_plugin* vid_plugin;


namespace wGui
{

CView* CView::m_pInstance = nullptr;


CView::CView(SDL_Surface* surface, SDL_Surface* backSurface, const CRect& WindowRect) :
	CWindow(CRect(0, 0, surface->w, surface->h), nullptr),
	m_pMenu(nullptr),
	m_pFloatingWindow(nullptr),
	m_pScreenSurface(nullptr)
{
	if (m_pInstance)
	{
		throw(Wg_Ex_App("Cannot have more than one view at a time!", "CView::CView"));
	}
	m_pInstance = this;

	CApplication::Instance()->MessageServer()->RegisterMessageClient(this, CMessage::APP_PAINT);
	CApplication::Instance()->MessageServer()->RegisterMessageClient(this, CMessage::APP_DESTROY_FRAME, CMessageServer::PRIORITY_FIRST);
	CApplication::Instance()->MessageServer()->RegisterMessageClient(this, CMessage::CTRL_RESIZE);
	CApplication::Instance()->MessageServer()->RegisterMessageClient(this, CMessage::MOUSE_BUTTONDOWN, CMessageServer::PRIORITY_FIRST);
	CApplication::Instance()->MessageServer()->RegisterMessageClient(this, CMessage::MOUSE_BUTTONUP, CMessageServer::PRIORITY_FIRST);

	// judb this works, but better rewrite this to make things clearer !
	CWindow::SetWindowRect(WindowRect);
    // judb ClientRect is relative to WindowRect (it is the client area within a Window(Rect) ) so
    // its coordinates are relative to WindowRect ->
    // here we use a rectangle with the same dimensions as WindowRect, but (0,0) as its upper left point.
    // so it covers the entire WindowRect.
	m_ClientRect     = WindowRect.SizeRect();
	m_pScreenSurface = surface;
  m_pBackSurface   = backSurface;

	// judb should not happen:-)
	if (m_pScreenSurface == nullptr)
		throw( Wg_Ex_SDL(std::string("Surface not created? : ") + SDL_GetError(), "CView::CView"));

	CApplication::Instance()->GetApplicationLog().AddLogEntry("Created new CView ", APP_LOG_INFO);
	Draw();
}


CView::~CView()
{
	delete m_pMenu;
	if (m_pInstance == this)
	{
		m_pInstance = nullptr;
	}
}


void CView::AttachMenu(CMenu* pMenu)
{
	delete m_pMenu;
	m_pMenu = pMenu;
	if (m_pMenu)
	{
		m_pMenu->SetNewParent(this);
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


void CView::SetWindowText(const std::string& sText)
{
	CWindow::SetWindowText(sText);
  SDL_SetWindowTitle(m_pWindow, m_sWindowText.c_str());
}


void CView::Flip() const
{
  vid_plugin->flip(vid_plugin);
}

bool CView::HandleMessage(CMessage* pMessage)
{
	bool bHandled = false;

	if (pMessage)
	{
		switch(pMessage->MessageType())
		{
		case CMessage::APP_PAINT :
			if (pMessage->Destination() == this || pMessage->Destination() == nullptr)
			{
				SDL_Surface* pFloatingSurface = SDL_CreateRGBSurface(0, m_pScreenSurface->w, m_pScreenSurface->h, CApplication::Instance()->GetBitsPerPixel(), 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
				PaintToSurface(*m_pScreenSurface, *pFloatingSurface, CPoint(0, 0));
				// judb use entire application SDL surface (otherwise strange clipping effects occur
				// when moving frames, also clipping of listboxes.)
//				SDL_Rect SourceRect = CRect(m_WindowRect.SizeRect()).SDLRect();
				SDL_Rect SourceRect = CRect(0, 0, m_pScreenSurface->w, m_pScreenSurface->h).SDLRect();
//				SDL_Rect DestRect = CRect(m_WindowRect.SizeRect()).SDLRect();
				SDL_Rect DestRect = CRect(0, 0, m_pScreenSurface->w, m_pScreenSurface->h).SDLRect();
				SDL_BlitSurface(pFloatingSurface, &SourceRect, m_pScreenSurface, &DestRect);
				SDL_FreeSurface(pFloatingSurface);
				//SDL_UpdateRect(m_pScreenSurface, 0, 0, 0, 0);
				Flip();

				bHandled = true;
			}
			break;
		case CMessage::APP_DESTROY_FRAME:
			if (pMessage->Destination() == this || pMessage->Destination() == nullptr)
			{
				CFrame* pFrame = dynamic_cast<CFrame*>(const_cast<CMessageClient*>(pMessage->Source()));
				if (pFrame)
				{
					pFrame->SetModal(false);
          pFrame->SetNewParent(nullptr);
					CApplication::Instance()->MessageServer()->QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
					delete pFrame;
				}
				bHandled = true;
			}
			break;
		case CMessage::CTRL_RESIZE:
		{
			TPointMessage* pResizeMessage = dynamic_cast<TPointMessage*>(pMessage);
			if (pResizeMessage && pResizeMessage->Source() == CApplication::Instance())
			{
        LOG_ERROR("CView::HandleMessage called received a CTRL_RESIZE message - not migrated to SDL2");
        /*
				CWindow::SetWindowRect(CRect(m_WindowRect.TopLeft(), m_WindowRect.TopLeft() + pResizeMessage->Value()));

				m_ClientRect = CRect(m_ClientRect.Left(), m_ClientRect.Top(), m_WindowRect.Width(), m_WindowRect.Height());
				m_ClientRect.ClipTo(m_WindowRect.SizeRect());

				m_pScreenSurface = SDL_SetVideoMode(m_WindowRect.Width(), m_WindowRect.Height(), DEFAULT_BPP, iFlags);
				if (m_pScreenSurface == nullptr)
					throw( Wg_Ex_SDL(std::string("Could not set video mode : ") + SDL_GetError(), "CView::HandleMessage") );
        */

				bHandled = true;
			}
			break;
		}
		case CMessage::MOUSE_BUTTONDOWN:
		{
			CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
			if (pMouseMessage && m_WindowRect.HitTest(pMouseMessage->Point) == CRect::RELPOS_INSIDE)
			{
				if (!m_pFloatingWindow || !m_pFloatingWindow->OnMouseButtonDown(pMouseMessage->Point, pMouseMessage->Button))
				{
					if (pMouseMessage->Destination() == nullptr)
					{
						OnMouseButtonDown(pMouseMessage->Point, pMouseMessage->Button);
					}
					else if (dynamic_cast<const CWindow*>(pMouseMessage->Destination()))
					{
						const_cast<CWindow*>(static_cast<const CWindow*>(pMouseMessage->Destination()))->
							OnMouseButtonDown(pMouseMessage->Point, pMouseMessage->Button);
					}
				}
			}
			break;
		}
		case CMessage::MOUSE_BUTTONUP:
		{
			CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
			if (pMouseMessage && m_WindowRect.HitTest(pMouseMessage->Point) == CRect::RELPOS_INSIDE)
			{
				if (!m_pFloatingWindow || !m_pFloatingWindow->OnMouseButtonUp(pMouseMessage->Point, pMouseMessage->Button))
				{
					if (pMouseMessage->Destination() == nullptr)
					{
						OnMouseButtonUp(pMouseMessage->Point, pMouseMessage->Button);
					}
					else if (dynamic_cast<const CWindow*>(pMouseMessage->Destination()))
					{
						const_cast<CWindow*>(static_cast<const CWindow*>(pMouseMessage->Destination()))->
							OnMouseButtonUp(pMouseMessage->Point, pMouseMessage->Button);
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

}


