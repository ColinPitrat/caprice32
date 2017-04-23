// wg_listbox.cpp
//
// CListBox class implementation
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


#include "wg_listbox.h"
#include "wg_application.h"
#include "wg_message_server.h"
#include "wg_error.h"
#include "wutil_debug.h"
#include "std_ex.h"

namespace wGui
{

CListBox::CListBox(const CRect& WindowRect, CWindow* pParent, bool bSingleSelection, unsigned int iItemHeight, CFontEngine* pFontEngine) :
	CWindow(WindowRect, pParent),
	m_iItemHeight(iItemHeight),
	m_iFocusedItem(0),
	m_bSingleSelection(bSingleSelection),
	m_pDropDown(nullptr)
{
	if (pFontEngine)
	{
		m_pFontEngine = pFontEngine;
	}
	else
	{
		m_pFontEngine = CApplication::Instance()->GetDefaultFontEngine();
	}
	CRect ScrollbarRect(m_WindowRect.SizeRect());
	ScrollbarRect.Grow(-1);
	m_pVScrollbar = new CScrollBar(
		CRect(ScrollbarRect.Right() - 12, ScrollbarRect.Top(), ScrollbarRect.Right() + 1, ScrollbarRect.Bottom()) - CPoint(2, 2) /* client adjustment */,
		this, CScrollBar::VERTICAL),

	m_pVScrollbar->SetMaxLimit(0);
	m_ClientRect = CRect(2, 2, m_WindowRect.Width() - 16, m_WindowRect.Height() - 2);
	m_BackgroundColor = COLOR_WHITE;
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGE);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGING);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_GAININGKEYFOCUS);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_LOSINGKEYFOCUS);
	Draw();
}


CListBox::~CListBox() = default;


void CListBox::SetItemHeight(unsigned int iItemHeight)
{
	m_iItemHeight = iItemHeight;
	Draw();
}


unsigned int CListBox::AddItem(SListItem ListItem)
{
	m_Items.push_back(ListItem);
	m_SelectedItems.push_back(false);
	m_RenderedStrings.push_back(CRenderedString(m_pFontEngine, ListItem.sItemText, CRenderedString::VALIGN_TOP, CRenderedString::HALIGN_LEFT));
	int iMax = m_Items.empty() ? 0 : m_Items.size() - 1;
  // judb correction MaxLimit (number of 'scrolls' = number of items - visible number of items )
	m_pVScrollbar->SetMaxLimit(stdex::MaxInt(iMax - (m_ClientRect.Height() / m_iItemHeight) + 1, 0));
	Draw();
	return m_Items.size();
}


void CListBox::RemoveItem(unsigned int iItemIndex)
{
	if (iItemIndex < m_SelectedItems.size())
	{
		m_Items.erase(m_Items.begin() + iItemIndex);
		m_SelectedItems.erase(m_SelectedItems.begin() + iItemIndex);
		int iMax = m_Items.empty() ? 0 : m_Items.size() - 1;
    // judb correction MaxLimit (number of 'scrolls' = number of items - visible number of items )
		m_pVScrollbar->SetMaxLimit(stdex::MaxInt(iMax - (m_ClientRect.Height() / m_iItemHeight) + 1, 0));
		Draw();
	}
}


void CListBox::ClearItems()
{
	m_Items.clear();
	m_SelectedItems.clear();
  m_RenderedStrings.clear();
	m_pVScrollbar->SetMaxLimit(0);
  m_pVScrollbar->SetValue(0);
	Draw();
}

int CListBox::getFirstSelectedIndex() {
    for (unsigned int i = 0; i < m_Items.size(); i ++) {
        if (IsSelected(i)) {
            return i;
        }
    }
    return -1;
}

void CListBox::SetSelection(unsigned int iItemIndex, bool bSelected, bool bNotify)
{
	if (iItemIndex < m_SelectedItems.size())
  {
    if (m_bSingleSelection)
    {
      SetAllSelections(false);
    }
		m_SelectedItems.at(iItemIndex) = bSelected;
    CWindow* pDestination = m_pParentWindow;
    if (m_pDropDown)
    {
      pDestination = m_pDropDown;
    }
    if (bNotify)
    {
      CMessageServer::Instance().QueueMessage(new TIntMessage(CMessage::CTRL_VALUECHANGE, pDestination, this, m_iFocusedItem));
    }
    Draw();
  }
}


void CListBox::SetAllSelections(bool bSelected)
{
	for (unsigned int i = 0; i < m_Items.size(); ++i)
	{
		m_SelectedItems.at(i) = bSelected;
	}
}

void CListBox::SetFocus(unsigned int iItemIndex) {
    m_iFocusedItem = iItemIndex;
}

void CListBox::SetDropDown(CWindow* pDropDown)
{
  m_pDropDown = pDropDown;
  if (pDropDown == nullptr) {
    SetIsFocusable(true);
  } else {
    SetIsFocusable(false);
  }
}

void CListBox::Draw() const
{
	CWindow::Draw();    

	if (m_pSDLSurface)
	{
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		Painter.DrawRect(m_WindowRect.SizeRect(), false, COLOR_DARKGRAY);
		int iStartIndex = m_pVScrollbar->GetValue();
		for (unsigned int i = iStartIndex; i < m_Items.size(); ++i)
		{
			CRect ItemRect(m_ClientRect.Left(), m_ClientRect.Top() + (i - iStartIndex) * m_iItemHeight,
				m_ClientRect.Right(), m_ClientRect.Top() + (i - iStartIndex + 1) * m_iItemHeight - 1);
			if (ItemRect.Overlaps(m_ClientRect))
			{
				ItemRect.ClipTo(m_ClientRect);
				ItemRect.SetBottom(ItemRect.Bottom() - 1);
				if (m_SelectedItems.at(i))
				{
					Painter.DrawRect(ItemRect, true, CApplication::Instance()->GetDefaultSelectionColor(), CApplication::Instance()->GetDefaultSelectionColor());
				}
				if (i == m_iFocusedItem && HasFocus())
				{
					ItemRect.Grow(1);
					Painter.DrawRect(ItemRect, false, COLOR_DARKGRAY);
					ItemRect.Grow(-1);
				}
				ItemRect.Grow(-1);
				m_RenderedStrings.at(i).Draw(m_pSDLSurface, ItemRect, ItemRect.TopLeft() + CPoint(0, 1), m_Items[i].ItemColor);
			}
		}
	}
  m_pVScrollbar->Draw();
}


void CListBox::SetWindowRect(const CRect& WindowRect)
{
	CWindow::SetWindowRect(WindowRect);
	m_ClientRect = CRect(2, 2, m_WindowRect.Width() - 16, m_WindowRect.Height() - 2);
	CRect ScrollbarRect(m_WindowRect.SizeRect());
	ScrollbarRect.Grow(-1);
	m_pVScrollbar->SetWindowRect(
		CRect(ScrollbarRect.Right() - 12, ScrollbarRect.Top(), ScrollbarRect.Right() + 1, ScrollbarRect.Bottom()) - CPoint(2, 2) /* client adjustment */);
}


void CListBox::PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const
{
	if (m_pDropDown)
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
	else
	{
		CWindow::PaintToSurface(ScreenSurface, FloatingSurface, Offset);
	}
}


bool CListBox::OnMouseButtonDown(CPoint Point, unsigned int Button)
{
	CPoint WindowPoint(ViewToWindow(Point));
	bool bResult = CWindow::OnMouseButtonDown(Point, Button);
 	if (!bResult && m_bVisible && (Button == CMouseMessage::LEFT) &&
		(m_WindowRect.SizeRect().HitTest(WindowPoint) == CRect::RELPOS_INSIDE))
	{
// judb for the moment disabled (this interferes with the keyboard focus handling, for example
// closing a frame with the Escape button)
// Should look at it when adding keyboard control for the entire gui.(most of the widgets don't respond
// to keyboard events yet)
//		if (CApplication::Instance()->GetKeyFocus() != this)
//		{
//			CApplication::Instance()->SetKeyFocus(this);
//		}

		if (!m_Items.empty() && m_ClientRect.HitTest(WindowPoint) == CRect::RELPOS_INSIDE)
		{
			// Prep the new selection
      // judb m_iFocusedItem should be <= the number of items in the listbox (0-based, so m_Items.size() -1)
			m_iFocusedItem = std::min((WindowPoint.YPos() + m_ClientRect.Top()) / m_iItemHeight + m_pVScrollbar->GetValue(), stdex::safe_static_cast<unsigned int>(m_Items.size()) - 1);
		}
		bResult = true;
	}

	return bResult;
}


bool CListBox::OnMouseButtonUp(CPoint Point, unsigned int Button)
{
	bool bResult = CWindow::OnMouseButtonUp(Point, Button);

	CPoint WindowPoint(ViewToWindow(Point));
	if (!bResult && m_bVisible && (Button == CMouseMessage::LEFT) && (m_ClientRect.HitTest(WindowPoint) == CRect::RELPOS_INSIDE))
	{
        // judb m_iFocusedItem should be <= the number of items in the listbox (0-based, so m_Items.size() - 1)
		if (m_iFocusedItem == std::min(((WindowPoint.YPos() - m_ClientRect.Top()) / m_iItemHeight + m_pVScrollbar->GetValue()), stdex::safe_static_cast<unsigned int>(m_Items.size()) - 1))
		{
			SetSelection(m_iFocusedItem, !IsSelected(m_iFocusedItem));
		}
		bResult = true;
	}

	return bResult;
}


bool CListBox::HandleMessage(CMessage* pMessage)
{
	bool bHandled = false;

	if (pMessage)
	{
		switch(pMessage->MessageType())
		{
    case CMessage::CTRL_GAININGKEYFOCUS:  // intentional fall through
    case CMessage::CTRL_LOSINGKEYFOCUS:
      Draw();
      break;
		case CMessage::KEYBOARD_KEYDOWN:
		{
			CKeyboardMessage* pKeyMsg = dynamic_cast<CKeyboardMessage*>(pMessage);
			if (pKeyMsg && pMessage->Destination() == this)
			{
				switch (pKeyMsg->Key)
				{
					case SDLK_DOWN:
					{
						if (m_iFocusedItem + 1 < Size())
						{
							m_iFocusedItem++;
							int diff = m_iFocusedItem - m_pVScrollbar->GetValue();
							if (m_iItemHeight * (m_pVScrollbar->GetValue() + diff + 1) > static_cast<unsigned int>(m_ClientRect.Height()))
							{
								m_pVScrollbar->SetValue(m_pVScrollbar->GetValue() + 1);
							}

							Draw();
							bHandled = true;
						}
						break;
					}
					case SDLK_UP:
					{
						if ( m_iFocusedItem > 0 )
						{
							m_iFocusedItem--;
							if (m_iFocusedItem < static_cast<unsigned int>(m_pVScrollbar->GetValue()))
							{
								m_pVScrollbar->SetValue(m_pVScrollbar->GetValue() - 1);
							}
							Draw();
							bHandled = true;
						}
						break;
					}
					case SDLK_PAGEDOWN:
					{
						unsigned int nSize = Size() - 1;
						unsigned int nItemsPerPage = m_ClientRect.Height() / m_iItemHeight;
						if (m_iFocusedItem + nItemsPerPage < nSize)
						{
							m_iFocusedItem += nItemsPerPage;
						}
						else
						{
							m_iFocusedItem = nSize;
						}
						m_pVScrollbar->SetValue(m_iFocusedItem);
						Draw();
						bHandled=true;
						break;
					}
					case SDLK_PAGEUP:
					{
						int nItemsPerPage = m_ClientRect.Height() / m_iItemHeight;
						if (m_iFocusedItem - nItemsPerPage > 0)
						{
							m_iFocusedItem -= nItemsPerPage;
						}
						else
						{
							m_iFocusedItem = 0;
						}
						m_pVScrollbar->SetValue(m_iFocusedItem);
						Draw();
						bHandled=true;
						break;
					}
					case SDLK_RETURN: // intentional fall through
					case SDLK_SPACE:
					{
						if (! m_Items.empty())
						{
							SetSelection(m_iFocusedItem, !IsSelected(m_iFocusedItem));
							Draw();
						}
						bHandled = true;
						break;
					}
					default:
					{
            // Not for us - let parent handle it
            CMessageServer::Instance().QueueMessage(new CKeyboardMessage(CMessage::KEYBOARD_KEYDOWN, m_pParentWindow, this,
                  pKeyMsg->ScanCode, pKeyMsg->Modifiers, pKeyMsg->Key, pKeyMsg->Unicode));
						bHandled=false;
						break;
					}
				}
			}
			break;
		}
		case CMessage::CTRL_VALUECHANGE:
		case CMessage::CTRL_VALUECHANGING:
		{
			if (pMessage->Source() == m_pVScrollbar)
			{
				Draw();
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

}
