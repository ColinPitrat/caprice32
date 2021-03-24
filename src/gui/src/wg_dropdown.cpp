// wg_dropdown.cpp
//
// CDropDown class implementation
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


#include "std_ex.h"
#include "wg_dropdown.h"
#include "wg_message_server.h"
#include "wg_resources.h"
#include "wg_view.h"
#include <string>

namespace wGui
{

CDropDown::CDropDown(const CRect& WindowRect, CWindow* pParent, bool bAllowEdit, unsigned int iItemHeight, CFontEngine* pFontEngine) :
  CWindow(WindowRect, pParent),
  m_bAllowEdit(bAllowEdit),
  m_iItemCount(5)
{
  m_pCViewAncestor = GetView();
  m_pEditBox = new CEditBox(CRect(0, 0, m_WindowRect.Width() - m_WindowRect.Height(), m_WindowRect.Height()), this, pFontEngine);
  if (!m_bAllowEdit)
  {
    m_pEditBox->SetReadOnly(true);
    // Override the normal read-only BG color
    m_pEditBox->SetBackgroundColor(COLOR_WHITE);
  }

  m_pListBox = new CListBox(CRect(0, m_WindowRect.Height(), m_WindowRect.Width(), m_WindowRect.Height() + iItemHeight * m_iItemCount + 2),
                            this, true, iItemHeight, pFontEngine);
  m_pListBox->SetVisible(false);
  m_pListBox->SetDropDown(this);

  m_pDropButton = new CPictureButton(
      CRect(m_WindowRect.Width() - m_WindowRect.Height() + 1, 0, m_WindowRect.Width(), m_WindowRect.Height()),
      this, CwgBitmapResourceHandle(WGRES_DOWN_ARROW_BITMAP));
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_BUTTONDOWN);
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_SINGLELCLICK);
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGE);
  Draw();
}


CDropDown::~CDropDown()
{
  if(m_pCViewAncestor)
  {
    // Reset floating window (see HideListBox() and ShowListBox()), otherwise the FloatingWindow 
    // would point to 'nothing' -> memory errors. This could occur when you open the dropdown list 
    // and immediately close the window with the keyboard.
    m_pCViewAncestor->SetFloatingWindow(nullptr);
  }
}


void CDropDown::SetListboxHeight(int iItemCount)
{
  m_iItemCount = iItemCount;
  m_pListBox->SetWindowRect(
      CRect(0, m_WindowRect.Height(), m_WindowRect.Width(), m_WindowRect.Height() + m_pListBox->GetItemHeight() * m_iItemCount + 2));
}


void CDropDown::SetWindowRect(const CRect& WindowRect)
{
  CWindow::SetWindowRect(WindowRect);
  m_pListBox->SetWindowRect(CRect(0, m_WindowRect.Height(), m_WindowRect.Width(), m_WindowRect.Height() + m_pListBox->GetItemHeight() * m_iItemCount + 2));
  m_pDropButton->SetWindowRect(CRect(m_WindowRect.Width() - m_WindowRect.Height() + 1, 0, m_WindowRect.Width(), m_WindowRect.Height()));
  m_pEditBox->SetWindowRect(CRect(0, 0, m_WindowRect.Width() - m_WindowRect.Height(), m_WindowRect.Height()));
}


void CDropDown::SetWindowText(const std::string& sWindowText)
{
  m_pEditBox->SetWindowText(sWindowText);
}


std::string CDropDown::GetWindowText() const
{
  return m_pEditBox->GetWindowText();
}


void CDropDown::MoveWindow(const CPoint& MoveDistance)
{
  CWindow::MoveWindow(MoveDistance);
  m_pListBox->MoveWindow(MoveDistance);
}

void CDropDown::SetVisible(bool bVisible) {
  CWindow::SetVisible(bVisible);
  HideListBox();
}

void CDropDown::SetIsFocusable(bool bFocusable) {
  m_pDropButton->SetIsFocusable(bFocusable);
}

bool CDropDown::HandleMessage(CMessage* pMessage)
{
  bool bHandled = false;
  CRect SubRect(m_WindowRect);
  SubRect.Grow(-3);

  if (pMessage)
  {
    switch(pMessage->MessageType())
    {
      case CMessage::KEYBOARD_KEYDOWN:
        {
          CKeyboardMessage* pKeyboardMessage = dynamic_cast<CKeyboardMessage*>(pMessage);
          if (pKeyboardMessage && pMessage->Destination() == this)
          {
            switch (pKeyboardMessage->Key)
            {
              case SDLK_UP:
                SelectItem(GetSelectedIndex() - 1);
                ShowListBox();
                break;
              case SDLK_DOWN:
                SelectItem(GetSelectedIndex() + 1);
                ShowListBox();
                break;
              case SDLK_RETURN:
              case SDLK_SPACE:
                HideListBox();
                break;
              case SDLK_TAB:
                HideListBox();
#if __GNUC__ >= 7
                [[gnu::fallthrough]]; // the parent frame will change focused widget
#endif
              default:
                // Forward all key downs to parent
                CMessageServer::Instance().QueueMessage(
                    new CKeyboardMessage(CMessage::KEYBOARD_KEYDOWN, m_pParentWindow, this,
                                         pKeyboardMessage->ScanCode, pKeyboardMessage->Modifiers,
                                         pKeyboardMessage->Key));
                break;
            }
          }
          break;
        }
      case CMessage::MOUSE_BUTTONDOWN:
        {
          CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
          if (pMouseMessage->Button == CMouseMessage::LEFT)
          {
            if (m_pListBox->IsVisible() &&
                m_pDropButton->GetWindowRect().SizeRect().HitTest(m_pDropButton->ViewToWindow(pMouseMessage->Point)) != CRect::RELPOS_INSIDE &&
                m_pListBox->GetWindowRect().SizeRect().HitTest(m_pListBox->ViewToWindow(pMouseMessage->Point)) != CRect::RELPOS_INSIDE)
            {
              HideListBox();
            }
          }
          break;
        }
      case CMessage::CTRL_SINGLELCLICK:
        {
          if (pMessage->Destination() == this)
          {
            if (pMessage->Source() == m_pDropButton)
            {
              if (m_pListBox->IsVisible())
              {
                HideListBox();
              }
              else
              {
                ShowListBox();
              }
              bHandled = true;
            }
          }
          break;
        }
      case CMessage::CTRL_VALUECHANGE:
        {
          TIntMessage* pCtrlMessage = dynamic_cast<TIntMessage*>(pMessage);
          if (pCtrlMessage && pMessage->Destination() == this)
          {
            if (pCtrlMessage->Source() == m_pListBox)
            {
              const SListItem& ListItem = m_pListBox->GetItem(pCtrlMessage->Value());
              SetWindowText(ListItem.sItemText);
              HideListBox();
              CMessageServer::Instance().QueueMessage(new TIntMessage(CMessage::CTRL_VALUECHANGE, m_pParentWindow, this, 0));
              bHandled = true;
            }
            else if (pCtrlMessage->Source() == m_pEditBox)
            {
              m_pListBox->SetAllSelections(false);
              HideListBox();
              CMessageServer::Instance().QueueMessage(new TIntMessage(CMessage::CTRL_VALUECHANGE, m_pParentWindow, this, 0));
              bHandled = true;
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

int CDropDown::GetSelectedIndex() {
    for (unsigned int i = 0; i < m_pListBox->Size(); i ++) {
        if (IsSelected(i)) {
            return i;
        }
    }
    return -1;
}

void CDropDown::SelectItem(unsigned int iItemIndex) {
    if (iItemIndex >= m_pListBox->Size()) {
        return;
    }
    m_pListBox->SetSelection(iItemIndex, true, false);
    SetWindowText(m_pListBox->GetItem(iItemIndex).sItemText);
    CMessageServer::Instance().QueueMessage(new TIntMessage(CMessage::CTRL_VALUECHANGE, this, this, 0));
    Draw();
}


void CDropDown::ShowListBox()
{
  if (!m_pListBox->IsVisible())
  {
    if (m_pCViewAncestor)
    {
      m_pCViewAncestor->SetFloatingWindow(m_pListBox);
    }
    m_pListBox->SetVisible(true);
  }
}


void CDropDown::HideListBox()
{
  if (m_pListBox->IsVisible())
  {
    m_pListBox->SetVisible(false);
    if (m_pCViewAncestor && m_pCViewAncestor->GetFloatingWindow() == m_pListBox)
    {
      m_pCViewAncestor->SetFloatingWindow(nullptr);
    }
  }
}

}

