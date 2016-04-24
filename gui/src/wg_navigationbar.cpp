// wg_navigationbar.cpp
//
// CNavigationBar class implementation
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

#include "wg_application.h"
#include "wg_error.h"
#include "wg_message_server.h"
#include "wg_navigationbar.h"
#include "wgui_include_config.h"
#include "wutil_debug.h"
#include "std_ex.h"

namespace wGui
{

CNavigationBar::CNavigationBar(CWindow* pParent, const CPoint& UpperLeft, unsigned int iMaxItems,
                             unsigned int iItemWidth, unsigned int iItemHeight, CFontEngine* pFontEngine) :
	CWindow(CRect(UpperLeft, iMaxItems * iItemWidth + 4, iItemHeight), pParent),
	m_iItemHeight(iItemHeight),
	m_iItemWidth(iItemWidth),
	m_iSelectedItem(0),
	m_iFocusedItem(0)
{
	if (pFontEngine) {
        m_pFontEngine = pFontEngine;
	} else {
        m_pFontEngine = CApplication::Instance()->GetDefaultFontEngine();
	}
    // Make space for a 3D border and a focus rectangle around each item.
    // To make it all match (and avoid that the last item gets clipped at the right side),
    // we added + 4 to the width (in the CWindow constructor above)
	m_ClientRect = CRect(2, 2, m_WindowRect.Width() - 2, m_WindowRect.Height() - 2);
	m_BackgroundColor = COLOR_WHITE;
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGE);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGING);
	Draw();
}


CNavigationBar::~CNavigationBar(void) {
    // Delete the bitmaps
    for (unsigned int i = 0; i < m_Bitmaps.size(); i ++) {
        delete m_Bitmaps.at(i);
    }
}


void CNavigationBar::SetItemHeight(unsigned int iItemHeight) {
	m_iItemHeight = iItemHeight;
	Draw();
}

void CNavigationBar::SetItemWidth(unsigned int iItemWidth) {
	m_iItemWidth = iItemWidth;
	Draw();
}


unsigned int CNavigationBar::AddItem(SNavBarItem NavBarItem) {
	m_Items.push_back(NavBarItem);
	m_RenderedStrings.push_back(CRenderedString(m_pFontEngine, NavBarItem.sItemText, CRenderedString::VALIGN_BOTTOM, CRenderedString::HALIGN_CENTER));
    if (NavBarItem.sPictureFilename != "") {
        m_Bitmaps.push_back(new CBitmapFileResourceHandle(NavBarItem.sPictureFilename));
        // Set transparency color to COLOR_WHITE:
        SDL_SetColorKey(m_Bitmaps.at(m_Bitmaps.size() - 1)->Bitmap(), SDL_SRCCOLORKEY, COLOR_WHITE.SDLColor(m_pSDLSurface->format));
    } else {
        m_Bitmaps.push_back(nullptr);
    }
	Draw();
	return m_Items.size();
}


void CNavigationBar::RemoveItem(unsigned int iItemIndex) {
	if (iItemIndex < m_Items.size()) {
		m_Items.erase(m_Items.begin() + iItemIndex);
        m_RenderedStrings.erase(m_RenderedStrings.begin() + iItemIndex);
        delete m_Bitmaps.at(iItemIndex); // allocated pointer -> delete
        m_Bitmaps.erase(m_Bitmaps.begin() + iItemIndex);
		Draw();
	}
}


void CNavigationBar::ClearItems(void)
{
	m_Items.clear();
	Draw();
}


unsigned int CNavigationBar::getSelectedIndex() {
    return m_iSelectedItem;
}


void CNavigationBar::SelectItem(unsigned int iItemIndex) {
	if (iItemIndex < m_Items.size()) {
        m_iSelectedItem = iItemIndex;
        m_iFocusedItem = iItemIndex;
        Draw();
    }
}


void CNavigationBar::Draw(void) const {
	CWindow::Draw();
	if (m_pSDLSurface)
	{
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);        
		Painter.Draw3DLoweredRect(m_WindowRect.SizeRect(), DEFAULT_BACKGROUND_COLOR);
        SDL_Rect PictureSourceRect = CRect(CPoint(0, 0), 30, 30).SDLRect();
		for (unsigned int i = 0; i < m_Items.size(); ++i)
		{
			CRect ItemRect(CPoint(m_ClientRect.Left() + i*m_iItemWidth, m_ClientRect.Top()),
				m_iItemWidth , m_iItemHeight);
			if (ItemRect.Overlaps(m_ClientRect))
			{
				ItemRect.ClipTo(m_ClientRect);
				ItemRect.SetBottom(ItemRect.Bottom() - 1);
				ItemRect.SetRight(ItemRect.Right() - 1);
				if (i == m_iSelectedItem)
				{
					Painter.DrawRect(ItemRect, true, CApplication::Instance()->GetDefaultSelectionColor(), CApplication::Instance()->GetDefaultSelectionColor());
				}
				if (i == m_iFocusedItem)
				{
					ItemRect.Grow(1);
					Painter.DrawRect(ItemRect, false, CApplication::Instance()->GetDefaultSelectionColor() * 0.7);
					ItemRect.Grow(-1);
				}
				ItemRect.Grow(-1);
                // '- CPoint(0,1)' is to move the reference point one pixel up (otherwise the lowest pixels of p,g,q,y 
                // etc. are not fully visible.
				m_RenderedStrings.at(i).Draw(m_pSDLSurface, ItemRect, ItemRect.BottomLeft() - CPoint(0, 1) + CPoint(ItemRect.Width()/2, 0), m_Items[i].ItemColor);
               // Draw the picture (if available):
               if (m_Bitmaps.at(i) != nullptr) {
                   SDL_Rect DestRect = ItemRect.Move(9, 1).SDLRect();
                   SDL_BlitSurface(m_Bitmaps.at(i)->Bitmap(), &PictureSourceRect, m_pSDLSurface, &DestRect);
               }
			}
		}
	}
}


void CNavigationBar::SetWindowRect(const CRect& WindowRect) {
    CWindow::SetWindowRect(WindowRect);
    m_ClientRect = CRect(2, 2, m_WindowRect.Width() - 2, m_WindowRect.Height() - 2);
}


void CNavigationBar::PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const {
	CWindow::PaintToSurface(ScreenSurface, FloatingSurface, Offset);
}


bool CNavigationBar::OnMouseButtonDown(CPoint Point, unsigned int Button)
{
	CPoint WindowPoint(ViewToClient(Point));
	bool bResult = CWindow::OnMouseButtonDown(Point, Button);
 	if (!bResult && m_bVisible && (Button == CMouseMessage::LEFT))
	{
		if (!m_Items.empty() && m_ClientRect.HitTest(WindowPoint) == CRect::RELPOS_INSIDE) {
			// Prep the new selection
      // judb m_iFocusedItem should be <= the number of items in the bar (0-based, so m_Items.size() - 1)
			m_iFocusedItem = stdex::MinInt((WindowPoint.XPos() / m_iItemWidth), m_Items.size() - 1);
			SelectItem(m_iFocusedItem);
			CWindow* pDestination = m_pParentWindow;
      // could be optimized : keep 'previous' selection and only send the message if new m_iFocusedItem != previous focused item.
			CMessageServer::Instance().QueueMessage(new TIntMessage(CMessage::CTRL_VALUECHANGE, pDestination, this, m_iFocusedItem));
			Draw();
      bResult = true;
		}
	}
	return bResult;
}

// this routine doesn't do a lot for the moment :-) 
// Functionality may be added later.
bool CNavigationBar::HandleMessage(CMessage* pMessage) {

	bool bHandled = false;

	if (pMessage) {
        switch(pMessage->MessageType()) {

		case CMessage::KEYBOARD_KEYDOWN:
		{
			CKeyboardMessage* pKeyMsg = dynamic_cast<CKeyboardMessage*>(pMessage);
			if (pKeyMsg && pMessage->Destination() == this)
			{
				switch (pKeyMsg->Key)
				{
					default:
                    {
						bHandled = false;
						break;
					}
				}
			}
			break;
		}
		case CMessage::CTRL_VALUECHANGE:
		case CMessage::CTRL_VALUECHANGING:
		{
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
