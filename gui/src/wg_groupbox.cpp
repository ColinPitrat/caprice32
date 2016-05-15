// wg_groupbox.cpp
//
// CGroupBox class implementation
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
#include "wg_groupbox.h"
#include <iostream> // TODO: remove


namespace wGui
{

CGroupBox::CGroupBox(const CRect& WindowRect, CWindow* pParent, std::string sText, CRGBColor& FontColor, CFontEngine* pFontEngine) :
	CWindow(WindowRect, pParent),
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
	m_ClientRect.Grow(-2);
	m_ClientRect.SetTop(15);
	m_pRenderedString.reset(new CRenderedString(
		m_pFontEngine, sText, CRenderedString::VALIGN_TOP, CRenderedString::HALIGN_LEFT));
	m_BackgroundColor = CApplication::Instance()->GetDefaultBackgroundColor();
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
	Draw();
}


CGroupBox::~CGroupBox(void)
{

}


void CGroupBox::Draw(void) const
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		CRect rect = CRect(0, 5, m_WindowRect.Width() - 2, m_WindowRect.Height() - 6);
        Painter.DrawRect(rect, false, m_BackgroundColor * 0.3);
        rect = rect + CPoint(1, 1);
        Painter.DrawRect(rect, false, m_BackgroundColor * 1.6);
		CPoint Dims, Offset;
		m_pRenderedString->GetMetrics(&Dims, &Offset, nullptr);
		Painter.DrawRect(CRect(CPoint(6, 0), CPoint(14, 0) + Dims),
			true, m_BackgroundColor, m_BackgroundColor);

		if (m_pRenderedString.get())
		{
			m_pRenderedString->Draw(m_pSDLSurface, m_WindowRect.SizeRect(), CPoint(10, 0), m_FontColor);
		}
	}
}


void CGroupBox::SetWindowText(const std::string& sWindowText)
{
	m_pRenderedString.reset(new CRenderedString(
		m_pFontEngine, sWindowText, CRenderedString::VALIGN_TOP, CRenderedString::HALIGN_LEFT));
	CWindow::SetWindowText(sWindowText);
}


void CGroupBox::SetWindowRect(const CRect& WindowRect)
{
	CWindow::SetWindowRect(WindowRect);
	m_ClientRect = m_WindowRect.SizeRect();
	m_ClientRect.Grow(-2);
	m_ClientRect.SetTop(15);
}

bool CGroupBox::HandleMessage(CMessage* pMessage)
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
		default :
			bHandled = CWindow::HandleMessage(pMessage);
			break;
		}
	}

	return bHandled;
}

}
