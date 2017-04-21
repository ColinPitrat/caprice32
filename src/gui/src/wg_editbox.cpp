// wg_editbox.cpp
//
// CEditBox class implementation
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


#include "wg_editbox.h"
#include "wg_message_server.h"
#include "wg_application.h"
#include "wg_timer.h"
#include "wg_error.h"
#include "wg_view.h"
#include "wutil_debug.h"
#include "std_ex.h"
#include <string>

namespace wGui
{

CEditBox::CEditBox(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
	CWindow(WindowRect, pParent),
	m_SelStart(0),
	m_SelLength(0),
	m_DragStart(0),
	m_ScrollOffset(0),
	m_bReadOnly(false),
	m_bMouseDown(false),
	m_bUseMask(false),
	m_bLastMouseMoveInside(false),
  m_contentType(ANY),
	m_bDrawCursor(true)
{
	m_BackgroundColor = COLOR_WHITE;
	m_ClientRect.Grow(-4);
	if (pFontEngine)
	{
		m_pFontEngine = pFontEngine;
	}
	else
	{
		m_pFontEngine = CApplication::Instance()->GetDefaultFontEngine();
	}
	m_pDblClickTimer = new CTimer();
	m_pCursorTimer = new CTimer(this);
	m_pRenderedString.reset(new CRenderedString(
		m_pFontEngine, "", CRenderedString::VALIGN_NORMAL, CRenderedString::HALIGN_LEFT));
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_BUTTONUP);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_MOVE);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_DOUBLELCLICK);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_TIMER);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_GAININGKEYFOCUS);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_LOSINGKEYFOCUS);

	Draw();
}


CEditBox::~CEditBox()  // virtual
{
	delete m_pCursorTimer;
	delete m_pDblClickTimer;
}


void CEditBox::SetReadOnly(bool bReadOnly)
{
	m_BackgroundColor = bReadOnly ? COLOR_LIGHTGRAY : COLOR_WHITE;
	m_bReadOnly = bReadOnly;
  SetIsFocusable(!bReadOnly);
	Draw();
}


std::string CEditBox::GetSelText() const
{
	if (m_bUseMask)
	{
		return "";
	}
	if (m_SelLength != 0)
	{
			std::string::size_type SelStartNorm = 0;
			std::string::size_type SelLenNorm = 0;
			if (m_SelLength < 0)
			{
				SelStartNorm	= m_SelLength + m_SelStart;
				SelLenNorm = abs(m_SelLength);
			}
			else
			{
				SelStartNorm	= m_SelStart;
				SelLenNorm = m_SelLength;
			}
			return m_sWindowText.substr(SelStartNorm, SelLenNorm);
	}
	else
	{
		return "";
	}
}


void CEditBox::SetSelection(std::string::size_type iSelStart, int iSelLength)
{
	if (iSelStart < m_sWindowText.length())
	{
		m_SelStart = iSelStart;
		if (iSelStart + iSelLength <= m_sWindowText.length())
			m_SelLength = iSelLength;
		else
			m_SelLength = stdex::safe_static_cast<int>(m_sWindowText.length() - iSelStart);
	}
	else
	{
		m_SelStart = 0;
		m_SelLength = 0;
	}
}


std::string::size_type CEditBox::GetIndexFromPoint(const CPoint& Point) const  // virtual
{
	CPoint Offset;
	std::vector<CRect> CharRects;
	m_pRenderedString->GetMetrics(nullptr, &Offset, &CharRects);
	CRect SubRect(m_WindowRect.SizeRect());
	SubRect.Grow(-3);
	std::string::size_type index = 0;
	CPoint BoundedPoint = Point;
	if (BoundedPoint.XPos() < SubRect.Left()) {
		BoundedPoint.SetX(SubRect.Left());
	}
	if (BoundedPoint.XPos() > SubRect.Right()) {
		BoundedPoint.SetX(SubRect.Right());
	}
	if (!CharRects.empty())
	{
		int xDelta = abs(BoundedPoint.XPos() - (CharRects.front().Left() + Offset.XPos() + SubRect.Left()));
		for (unsigned int i = 0; i < m_pRenderedString->GetLength(); ++i)
		{
			if (abs(BoundedPoint.XPos() - (CharRects.at(i).Right() + Offset.XPos() + SubRect.Left() + m_ScrollOffset)) < xDelta)
			{
				xDelta = abs(BoundedPoint.XPos() - (CharRects.at(i).Right() + Offset.XPos() + SubRect.Left() + m_ScrollOffset));
				index = i + 1;
			}
		}
	}

	return index;
}


void CEditBox::Draw() const
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		CRect SubRect(m_WindowRect.SizeRect());
		SubRect.Grow(-3);
		Painter.DrawRect(m_WindowRect.SizeRect(), false, COLOR_DARKGRAY);
		CPoint FontCenterPoint = m_WindowRect.SizeRect().Center();

		if (m_bUseMask)
		{
			m_pRenderedString->SetMaskChar('*');
		}
		else
		{
			m_pRenderedString->SetMaskChar(' ');
		}

		CRGBColor FontColor = m_bReadOnly ? DEFAULT_DISABLED_LINE_COLOR : COLOR_BLACK;
		if (CApplication::Instance()->GetKeyFocus() == dynamic_cast<const CWindow*>(this) && !m_bReadOnly)
		{
			CPoint BoundedDims;
			CPoint Offset;
			std::vector<CRect> CharRects;
			m_pRenderedString->GetMetrics(&BoundedDims, &Offset, &CharRects);

			std::string::size_type SelStartNorm = 0;
			std::string::size_type SelLenNorm = abs(m_SelLength);
			if (m_SelLength < 0)
			{
				SelStartNorm  = m_SelStart + m_SelLength;
			}
			else
			{
				SelStartNorm  = m_SelStart;
			}

			// Handle scrolling
			// Patch 	for "Scrolling while selecting to left in editbox" by Oldrich Dlouhy
			if (! m_bMouseDown)
			{
				if (CharRects.empty() || BoundedDims.XPos() < SubRect.Width())
				{
					m_ScrollOffset = 0;
				}
				else
				{
					int iCursorPos = 0;
					if (m_SelStart + m_SelLength >= CharRects.size())
					{
						iCursorPos = CharRects.back().Right() + Offset.XPos() + SubRect.Left() + m_ScrollOffset;
					}
					else
					{
						iCursorPos = CharRects.at(m_SelStart + m_SelLength).Left() + Offset.XPos() + SubRect.Left() + m_ScrollOffset;
					}
					if (iCursorPos < SubRect.Left())
					{
						m_ScrollOffset = -(iCursorPos - m_ScrollOffset - SubRect.Left() - Offset.XPos());
					}
					else if (iCursorPos > SubRect.Right())
					{
						m_ScrollOffset = -(iCursorPos - m_ScrollOffset - SubRect.Left() - Offset.XPos() - SubRect.Width() + 1);
					}

					if (m_ScrollOffset < 0 && (CharRects.back().Right() + Offset.XPos() + SubRect.Left() + m_ScrollOffset < SubRect.Right()))
					{
						m_ScrollOffset = SubRect.Right() - CharRects.back().Right() - 1;
						if (m_ScrollOffset > 0)
						{
							m_ScrollOffset = 0;
						}
					}
				}
			}

			// Selection
			if (m_SelLength != 0)
			{
				CRect SelRect;
				SelRect.SetBottom(SubRect.Bottom());
				SelRect.SetTop(SubRect.Top());
				SelRect.SetLeft(CharRects.at(SelStartNorm).Left() + Offset.XPos() + SubRect.Left() + m_ScrollOffset);
				SelRect.SetRight(CharRects.at(SelLenNorm + SelStartNorm - 1).Right() + Offset.XPos() + SubRect.Left() + m_ScrollOffset);
				SelRect.ClipTo(SubRect);
				Painter.DrawRect(SelRect, true, CApplication::Instance()->GetDefaultSelectionColor(), CApplication::Instance()->GetDefaultSelectionColor());
			}
			else if (m_bDrawCursor)
			{
				//RenderStringWithCursor
				int CursorPos = Offset.XPos() + SubRect.Left() + m_ScrollOffset;
				if (m_SelStart + m_SelLength >= CharRects.size() && !CharRects.empty())
				{
					CursorPos += CharRects.back().Right();
				}
				else if (m_SelStart + m_SelLength < CharRects.size())
				{
					CursorPos += CharRects.at(m_SelStart + m_SelLength).Left();
				}
				if (CursorPos >= SubRect.Left() && CursorPos <= SubRect.Right())
				{
					Painter.DrawVLine(SubRect.Top(), SubRect.Bottom(), CursorPos, COLOR_BLACK);
				}
			}
		}

		if (m_pRenderedString.get())
		{
			m_pRenderedString->Draw(m_pSDLSurface, SubRect,
				CPoint(SubRect.Left() + m_ScrollOffset, SubRect.Bottom() - m_pRenderedString->GetMaxFontHeight() / 4), FontColor);
		}
	}
}


void CEditBox::SetWindowText(const std::string& sText)
{
	m_SelStart = 0;
	m_SelLength = 0;
	m_pRenderedString.reset(new CRenderedString(
		m_pFontEngine, sText, CRenderedString::VALIGN_NORMAL, CRenderedString::HALIGN_LEFT));
	CWindow::SetWindowText(sText);
}


bool CEditBox::OnMouseButtonDown(CPoint Point, unsigned int Button)
{
	bool bResult = CWindow::OnMouseButtonDown(Point, Button);

	CPoint WindowPoint(ViewToWindow(Point));
 	if (!bResult && m_bVisible && (Button == CMouseMessage::LEFT) &&
		!m_bReadOnly && (m_ClientRect.HitTest(WindowPoint) == CRect::RELPOS_INSIDE))
	{
		bool fSkipCursorPositioning = false;
		//If we haven't initialized the double click timer, do so.
		if (!m_pDblClickTimer->IsRunning())
		{
			m_pDblClickTimer->StartTimer(500, false);
		}
		else
		{
			//Raise double click event
			//This message is being dispatched, but to where?
			CMessageServer::Instance().QueueMessage(new TIntMessage(CMessage::CTRL_DOUBLELCLICK, this, this, 0));
			m_pDblClickTimer->StopTimer();
			fSkipCursorPositioning = true;
		}
		if (CApplication::Instance()->GetKeyFocus() != this)
		{
			CApplication::Instance()->SetKeyFocus(this);
		}

		if (!fSkipCursorPositioning)
		{
			m_SelStart = GetIndexFromPoint(WindowPoint);
			m_DragStart = m_SelStart;
			m_SelLength = 0;
			m_bMouseDown = true;
			Draw();
			bResult = true;
		}
	}
	return bResult;
}


bool CEditBox::HandleMessage(CMessage* pMessage)
{
	bool bHandled = false;
	CRect SubRect(m_WindowRect);
	SubRect.Grow(-3);

	if (pMessage)
	{
		switch(pMessage->MessageType())
		{
		case CMessage::CTRL_DOUBLELCLICK:
			if (pMessage->Destination() == this)
			{
				//wUtil::Trace("Got the double click message!");
				m_SelStart = 0;
				m_SelLength = stdex::safe_static_cast<int>(m_sWindowText.length());
				Draw();
				bHandled = true;
			}
			break;
		case CMessage::MOUSE_BUTTONUP:
			m_bMouseDown = false;
			break;
		case CMessage::MOUSE_MOVE:
		{
			CMouseMessage* pMouseMessage = dynamic_cast<CMouseMessage*>(pMessage);
			if (pMouseMessage && m_bVisible && !m_bReadOnly)
			{
				CPoint WindowPoint(ViewToWindow(pMouseMessage->Point));
				//If the cursor is within the control then check to see if we've already
				// set the cursor to the I Beam, if we have, don't do anything.  If we
				// havent, set it to the ibeam.
				//Else if it's outside the control and the I Beam cursor is set, set it
				// back to a normal cursor.
				CView* pView = GetView();
				bool bHitFloating = pView && pView->GetFloatingWindow() && pView->GetFloatingWindow()->HitTest(pMouseMessage->Point);
				if (m_ClientRect.HitTest(WindowPoint) == CRect::RELPOS_INSIDE && !bHitFloating && !m_bLastMouseMoveInside)
				{
					m_bLastMouseMoveInside = true;
					CwgCursorResourceHandle IBeamHandle(WGRES_IBEAM_CURSOR);
					CApplication::Instance()->SetMouseCursor(&IBeamHandle);
				}
				else if ((m_ClientRect.HitTest(WindowPoint) != CRect::RELPOS_INSIDE || bHitFloating) && m_bLastMouseMoveInside)
				{
					m_bLastMouseMoveInside= false;
					CApplication::Instance()->SetMouseCursor();
				}

				if (m_bMouseDown)
				{
					std::string::size_type CursorPos = GetIndexFromPoint(WindowPoint);

					if (CursorPos < m_DragStart)
					{
						m_SelLength = stdex::safe_static_cast<int>(m_DragStart) - stdex::safe_static_cast<int>(CursorPos);
						m_SelStart = CursorPos;
					}
					else
					{
						m_SelStart = m_DragStart;
						m_SelLength = stdex::safe_static_cast<int>(CursorPos) - stdex::safe_static_cast<int>(m_SelStart);
					}
					bHandled = true;
					Draw();
				}
			}
			break;
		}
		case CMessage::CTRL_TIMER:
			if (pMessage->Destination() == this && pMessage->Source() == m_pCursorTimer)
			{
				//This redraws the whole control each time we want to show/not show the cursor, that's probably a bad idea.
				if (m_SelLength == 0)
				{
					m_bDrawCursor = !m_bDrawCursor;
					Draw();
				}
				bHandled = true;
			}
			break;
		case CMessage::CTRL_GAININGKEYFOCUS:
			if (pMessage->Destination() == this)
			{
				m_pCursorTimer->StartTimer(750, true);
				m_bDrawCursor = true;
				Draw();
				bHandled = true;
			}
			break;
		case CMessage::CTRL_LOSINGKEYFOCUS:
			if (pMessage->Destination() == this)
			{
				m_pCursorTimer->StopTimer();
				Draw();
				bHandled = true;
			}
			break;
		case CMessage::KEYBOARD_KEYDOWN:
			if (m_bVisible)
			{
				CKeyboardMessage* pKeyboardMessage = dynamic_cast<CKeyboardMessage*>(pMessage);
				if (pKeyboardMessage && pMessage->Destination() == this && !m_bReadOnly)
				{
					std::string sBuffer = m_sWindowText;
          bHandled = true;

					switch(pKeyboardMessage->Key)
					{
					case SDLK_BACKSPACE:
						if (m_SelLength > 0)
						{
							SelDelete(&sBuffer);
						}
						else
						{
							if (m_SelStart > 0)
							{
								sBuffer.erase(--m_SelStart, 1);
							}
						}
						break;

					case SDLK_DELETE:
						if (m_SelStart < sBuffer.length())
						{
							if (m_SelLength > 0)
							{
								SelDelete(&sBuffer);
							}
							else
							{
								sBuffer.erase(m_SelStart, 1);
							}
						}
						break;

					case SDLK_LEFT:
		 				if (pKeyboardMessage->Modifiers & KMOD_SHIFT) //Shift modifier
		  			{
							if (m_SelStart > 0)
							{
								if ((m_SelLength > 0) || ((m_SelStart - abs(m_SelLength)) > 0))
								{
									if (pKeyboardMessage->Modifiers & KMOD_CTRL)
									{
										std::string::size_type pos = sBuffer.rfind(" ", (m_SelStart + m_SelLength) - 1);
										if (pos != std::string::npos)
										{
											m_SelLength = stdex::safe_static_cast<int>(pos) - stdex::safe_static_cast<int>(m_SelStart);
										}
										else
										{
											m_SelLength = stdex::safe_static_cast<int>(m_SelStart) * -1;
										}
									}
									else
									{
										m_SelLength--;
									}
								}
							}
		  			}
						else if (m_SelLength != 0)
						{
							if (m_SelLength < 0)
							{
								m_SelStart = m_SelStart + m_SelLength;
							}
							m_SelLength = 0;

						}
						else if (m_SelStart > 0)
						{
							if (pKeyboardMessage->Modifiers & KMOD_CTRL)
							{
								std::string::size_type pos = sBuffer.rfind(" ", m_SelStart - 1);
								if (pos != std::string::npos)
								{
									m_SelStart = pos;
								}
								else
								{
									m_SelStart = 0;
								}
							}
							else
							{
								--m_SelStart;
							}
							m_SelLength = 0;
						}
						break;
					case SDLK_RIGHT:
						if (m_SelStart <= sBuffer.length())
						{
							if (pKeyboardMessage->Modifiers & KMOD_SHIFT)
							{
								if (pKeyboardMessage->Modifiers & KMOD_CTRL)
								{
									std::string::size_type pos = sBuffer.find(" ", m_SelStart + m_SelLength);
									if (pos != std::string::npos)
									{
										m_SelLength = stdex::safe_static_cast<int>(pos) - stdex::safe_static_cast<int>(m_SelStart) + 1;
									}
									else
									{
										m_SelLength = stdex::safe_static_cast<int>(sBuffer.length()) - stdex::safe_static_cast<int>(m_SelStart);
									}
								}
								else if (m_SelStart + m_SelLength < sBuffer.length())
								{
									m_SelLength++; //Selecting, one character at a time, increase the selection length by one.
								}
							}
							else if(m_SelLength == 0 && m_SelStart < sBuffer.length())
							{
								//With the ctrl modifier used, we look for the next instance of a space.
								// If we find one, we set the cursor position to that location.
								// If we can't find one, we set the cursor position to the end of the string.
								// If we don't have the ctrl modifier, then we just incriment the cursor position by one character
								if (pKeyboardMessage->Modifiers & KMOD_CTRL)
								{
									std::string::size_type pos = sBuffer.find(" ", m_SelStart + 1);
									if (pos != std::string::npos)
									{
										m_SelStart = pos + 1;
									}
									else
									{
										m_SelStart = sBuffer.length();
									}
								}
								else
								{
									++m_SelStart; //We don't have anything selected, so we'll just incriment the start position
								}
							}
							else
							{
								if (m_SelLength > 0)
								{
									m_SelStart = m_SelStart + m_SelLength; //Reset cursor position to the end of the selection
								}
								m_SelLength = 0; //Set selection length to zero
							}
						}
						break;
					case SDLK_END:
						if (pKeyboardMessage->Modifiers & KMOD_SHIFT)
						{
							m_SelLength = stdex::safe_static_cast<int>(sBuffer.length()) - stdex::safe_static_cast<int>(m_SelStart);
						}
						else
						{
							m_SelLength = 0;
							m_SelStart = sBuffer.length();
						}
						break;
					case SDLK_HOME:
						if (pKeyboardMessage->Modifiers & KMOD_SHIFT)
						{
							m_SelLength = stdex::safe_static_cast<int>(m_SelStart);
							m_SelStart = 0;
						}
						else
						{
							m_SelLength = 0;
							m_SelStart = 0;
						}
						break;
          case SDLK_ESCAPE:  // intentional fall through
          case SDLK_TAB:
            // Not for us - let parent handle it
            CMessageServer::Instance().QueueMessage(new CKeyboardMessage(CMessage::KEYBOARD_KEYDOWN, m_pParentWindow, this,
                  pKeyboardMessage->ScanCode, pKeyboardMessage->Modifiers, pKeyboardMessage->Key, pKeyboardMessage->Unicode));
            break;
					default:
						if (pKeyboardMessage->Unicode)
						{
							if ((pKeyboardMessage->Unicode & 0xFF80) == 0)
							{
								SelDelete(&sBuffer);
								// we are deliberately truncating the unicode data, so don't use safe_static_cast
                char val = static_cast<char>(pKeyboardMessage->Unicode & 0x7F);
                if(m_contentType == NUMBER && (val < '0' || val > '9')) {
                  break;
                }
                if(m_contentType == HEXNUMBER && (val < '0' || val > '9') && (val < 'A' || val > 'F') && (val < 'a' || val > 'f')) {
                  break;
                }
                if(m_contentType == HEXNUMBER && val >= 'a' && val <= 'z') {
                    val += 'A' - 'a';
                }
                if(m_contentType == ALPHA && (val < 'A' || val > 'Z') && (val < 'a' || val > 'z')) {
                  break;
                }
                if(m_contentType == ALPHANUM && (val < '0' || val > '9') && (val < 'A' || val > 'Z') && (val < 'a' || val > 'z')) {
                  break;
                }
								sBuffer.insert(m_SelStart++, 1, val);
							}
							else
							{
								wUtil::Trace("CEditBox::HandleMessage : CEditBox can't handle Unicode characters yet.");
							}
						}
						break;
					}

					if (m_sWindowText != sBuffer)
					{
						CMessageServer::Instance().QueueMessage(new TStringMessage(CMessage::CTRL_VALUECHANGE, m_pParentWindow, this, sBuffer));
					}
					m_sWindowText = sBuffer;
					CWindow::SetWindowText(sBuffer);

					m_pRenderedString.reset(new CRenderedString(m_pFontEngine, sBuffer, CRenderedString::VALIGN_NORMAL, CRenderedString::HALIGN_LEFT));

					m_bDrawCursor = true;
					Draw();
				}
				break;
			}
		default :
			break;
		}
	}
  if (!bHandled) {
    bHandled = CWindow::HandleMessage(pMessage);
  }

	return bHandled;
}


void CEditBox::SelDelete(std::string* psString)
{
	//This means we've selected something, therefore, should replace it
	if (m_SelLength != 0)
	{
		std::string::size_type SelStartNorm = 0;
		std::string::size_type SelLenNorm = 0;
		if (m_SelLength < 0)
		{
			SelStartNorm	= m_SelLength + m_SelStart;
			SelLenNorm = abs(m_SelLength);
		}
		else
		{
			SelStartNorm	= m_SelStart;
			SelLenNorm = m_SelLength;
		}

		psString->erase(SelStartNorm, SelLenNorm);
		//Since we're deleting the stuff here and setting the selection length to zero, we also need to set the selection start properly
		m_SelStart = SelStartNorm;
		m_SelLength = 0;
	}
}

}
