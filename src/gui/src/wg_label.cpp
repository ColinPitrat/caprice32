// wg_label.cpp
//
// CLabel class implementation
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


#include "wg_label.h"
#include <string>


namespace wGui
{

CLabel::CLabel(const CRect& WindowRect, CWindow* pParent, std::string sText, CRGBColor& FontColor, CFontEngine* pFontEngine) :
	CWindow(WindowRect, pParent),
	m_FontColor(FontColor)
{
	dynamic_size = false;
	m_sWindowText = sText;
	if (pFontEngine)
	{
		m_pFontEngine = pFontEngine;
	}
	else
	{
		m_pFontEngine = Application().GetDefaultFontEngine();
	}
	m_pRenderedString.reset(new CRenderedString(
		m_pFontEngine, sText, CRenderedString::VALIGN_CENTER, CRenderedString::HALIGN_LEFT));
	m_BackgroundColor = Application().GetDefaultBackgroundColor();
	Draw();
}



CLabel::CLabel(const CPoint& point, CWindow* pParent, std::string sText, CRGBColor& FontColor, CFontEngine* pFontEngine) :
	CWindow(pParent),
	m_FontColor(FontColor)
{
	dynamic_size = true;
	m_sWindowText = sText;
	if (pFontEngine)
	{
		m_pFontEngine = pFontEngine;
	}
	else
	{
		m_pFontEngine = Application().GetDefaultFontEngine();
	}
	m_pRenderedString.reset(new CRenderedString(m_pFontEngine, sText, CRenderedString::VALIGN_TOP, CRenderedString::HALIGN_LEFT));
	m_BackgroundColor = Application().GetDefaultBackgroundColor();
	// set width and height of the label's rectangle:
	CWindow::SetWindowRect(CRect(point, m_pRenderedString->GetWidth(sText) + 1, m_pRenderedString->GetMaxFontHeight() + 1));
	Draw();
}


CLabel::~CLabel() = default;


void CLabel::Draw() const
{
	CWindow::Draw();

	if (m_pSDLSurface && m_pRenderedString.get())
	{
		// judb together with VALIGN_CENTER, the originpoint should be
		// the height of the rectangle/2 (so also vertically centered)
		//m_pRenderedString->Draw(m_pSDLSurface, m_WindowRect.SizeRect(), CPoint(0, m_WindowRect.Height()/2), m_FontColor);
		 m_pRenderedString->Draw(m_pSDLSurface, m_WindowRect.SizeRect(), CPoint(0, 0), m_FontColor);
	}
}


void CLabel::SetWindowText(const std::string& sWindowText)
{
	m_pRenderedString.reset(new CRenderedString(
		m_pFontEngine, sWindowText, CRenderedString::VALIGN_TOP, CRenderedString::HALIGN_LEFT));
	CWindow::SetWindowText(sWindowText);
	if (dynamic_size) {
		CWindow::SetWindowRect(CRect(m_WindowRect.TopLeft(), m_pRenderedString->GetWidth(sWindowText) + 1, m_pRenderedString->GetMaxFontHeight() + 1));
	}
}

}



