// wg_button.cpp
//
// CButton class implementation
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
#include "wutil_debug.h"
#include "wg_button.h"
#include "wg_application.h"
#include "wg_message_server.h"
#include "std_ex.h"
#include <algorithm>


namespace wGui
{

CButton::CButton(const CRect& WindowRect, CWindow* pParent, std::string sText, CFontEngine* pFontEngine) :
	CWindow(WindowRect, pParent),
	m_eButtonState(UP),
	m_MouseButton(0)
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
		m_pFontEngine, sText, CRenderedString::VALIGN_CENTER, CRenderedString::HALIGN_CENTER));
//	m_BackgroundColor = CApplication::Instance()->GetDefaultForegroundColor();
    m_BackgroundColor = DEFAULT_BUTTON_COLOR;
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_BUTTONUP);
	Draw();
}


CButton::~CButton(void)
{

}


void CButton::SetButtonState(EState eState)
{
	if (m_eButtonState != eState)
	{
		m_eButtonState = eState;
		Draw();
	}
}


void CButton::Draw(void) const
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CPoint FontCenterPoint = m_WindowRect.SizeRect().Center();
		CRect SubRect(m_WindowRect.SizeRect());
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		CRGBColor FontColor = DEFAULT_TEXT_COLOR;
		switch (m_eButtonState)
		{
		case UP:
			Painter.Draw3DRaisedRect(SubRect, DEFAULT_BUTTON_COLOR);
			break;
		case DOWN:
			Painter.Draw3DLoweredRect(SubRect, DEFAULT_BUTTON_COLOR);
			FontCenterPoint = FontCenterPoint + CPoint(1, 1);
			break;
		case DISABLED:
			FontColor = DEFAULT_DISABLED_LINE_COLOR;
			break;
		default:
			break;
		}
		SubRect.Grow(-2);
    if (m_bHasFocus)
    {
			Painter.DrawRect(SubRect, false, COLOR_GRAY);
    }
		if (m_pRenderedString.get())
		{
			m_pRenderedString->Draw(m_pSDLSurface, SubRect, FontCenterPoint, FontColor);
		}
	}
}


void CButton::SetWindowText(const std::string& sWindowText)
{
	m_pRenderedString.reset(new CRenderedString(
		m_pFontEngine, sWindowText, CRenderedString::VALIGN_CENTER, CRenderedString::HALIGN_CENTER));

	CWindow::SetWindowText(sWindowText);
}


bool CButton::OnMouseButtonDown(CPoint Point, unsigned int Button)
{
	bool bResult = CWindow::OnMouseButtonDown(Point, Button);

 	if (!bResult && m_bVisible && (m_eButtonState == UP) && (m_ClientRect.HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE))
	{
		SetButtonState(DOWN);
		m_MouseButton = Button;
		bResult = true;
	}

	return bResult;
}


bool CButton::OnMouseButtonUp(CPoint Point, unsigned int Button)
{
	bool bResult = CWindow::OnMouseButtonUp(Point, Button);

	if (!bResult && m_bVisible && (m_eButtonState == DOWN) &&
		(m_MouseButton == Button) && (m_ClientRect.HitTest(ViewToWindow(Point)) == CRect::RELPOS_INSIDE))
	{
		SetButtonState(UP);
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
		CMessageServer::Instance().QueueMessage(new TIntMessage(MessageType, m_pParentWindow, this, 0));
		bResult = true;
	}

	return bResult;
}


bool CButton::HandleMessage(CMessage* pMessage)
{
	bool bHandled = false;

	if (pMessage)
	{
		switch(pMessage->MessageType())
		{
		case CMessage::MOUSE_BUTTONUP:
		{
			CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
			if (pMouseMessage && m_eButtonState == DOWN)
			{
				SetButtonState(UP);
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


CPictureButton::CPictureButton(const CRect& WindowRect, CWindow* pParent, std::string sPictureFile) :
	CButton(WindowRect, pParent, sPictureFile, 0)
{
	m_phBitmap.reset(new CBitmapFileResourceHandle(sPictureFile));
	Draw();
}


CPictureButton::CPictureButton(const CRect& WindowRect, CWindow* pParent, const CBitmapResourceHandle& hBitmap) :
	CButton(WindowRect, pParent, "<bitmap>", 0)
{
	m_phBitmap.reset(new CBitmapResourceHandle(hBitmap));
	Draw();
}


CPictureButton::~CPictureButton(void)
{

}


void CPictureButton::SetPicture(std::string sPictureFile)
{
	SetPicture(CBitmapFileResourceHandle(sPictureFile));
}


void CPictureButton::SetPicture(const CBitmapResourceHandle& hBitmap)
{
	m_phBitmap.reset(new CBitmapResourceHandle(hBitmap));
	Draw();
}


void CPictureButton::Draw(void) const
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CRect SubRect(m_WindowRect.SizeRect());
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		switch (m_eButtonState)
		{
		case UP:
			Painter.Draw3DRaisedRect(SubRect, DEFAULT_BUTTON_COLOR);
			break;
		case DOWN:
			Painter.Draw3DLoweredRect(SubRect, DEFAULT_BUTTON_COLOR);
			SubRect = SubRect + CPoint(1, 1);
			break;
		case DISABLED:
			break;
		default:
			break;
		}
		SubRect.Grow(-1);
		SDL_Rect SourceRect;
		SourceRect.x = stdex::safe_static_cast<short int>((m_phBitmap->Bitmap()->w - SubRect.Width()) / 2 < 0 ? 0 : (m_phBitmap->Bitmap()->w - SubRect.Width()) / 2);
		SourceRect.y = stdex::safe_static_cast<short int>((m_phBitmap->Bitmap()->h - SubRect.Height()) / 2 < 0 ? 0 : (m_phBitmap->Bitmap()->w - SubRect.Height()) / 2);
		SourceRect.w = stdex::safe_static_cast<short int>(std::min(SubRect.Width(), m_phBitmap->Bitmap()->w));
		SourceRect.h = stdex::safe_static_cast<short int>(std::min(SubRect.Height(), m_phBitmap->Bitmap()->h));
		SDL_Rect DestRect;
		DestRect.x = stdex::safe_static_cast<short int>((SubRect.Width() - m_phBitmap->Bitmap()->w) / 2 < 0 ? SubRect.Left() : SubRect.Left() + (SubRect.Width() - m_phBitmap->Bitmap()->w) / 2);
		DestRect.y = stdex::safe_static_cast<short int>((SubRect.Height() - m_phBitmap->Bitmap()->h) / 2 < 0 ? SubRect.Top() : SubRect.Top() + (SubRect.Height() - m_phBitmap->Bitmap()->h) / 2);
		DestRect.w = stdex::safe_static_cast<short int>(std::min(SubRect.Width(), m_phBitmap->Bitmap()->w));
		DestRect.h = stdex::safe_static_cast<short int>(std::min(SubRect.Height(), m_phBitmap->Bitmap()->h));
		SDL_BlitSurface(m_phBitmap->Bitmap(), &SourceRect, m_pSDLSurface, &DestRect);
	}
}

}


