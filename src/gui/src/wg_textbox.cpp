// wg_textbox.cpp
//
// CTextBox class implementation
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


#include "wg_textbox.h"
#include "wg_message_server.h"
#include "wg_application.h"
#include "wg_view.h"
#include "wg_error.h"
#include "wutil_debug.h"
#include "std_ex.h"
#include <string>

namespace wGui
{

CTextBox::CTextBox(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
	CWindow(WindowRect, pParent),
	m_SelStart(0),
	m_SelLength(0),
	m_DragStart(0),
	m_bReadOnly(false),
	m_bMouseDown(false),
	m_bLastMouseMoveInside(false),
	m_pVerticalScrollBar(nullptr),
	m_pHorizontalScrollBar(nullptr),
	m_iLineCount(0),
	m_iRowHeight(0),
	m_iMaxWidth(0),
	m_bDrawCursor(true),
	m_bScrollToCursor(false)
 {
	m_BackgroundColor = COLOR_WHITE;
	m_ClientRect = CRect(3, 3, m_WindowRect.Width() - 17, m_WindowRect.Height() - 17);
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
	m_ClientRect.Grow(-3);
	m_pVerticalScrollBar = new CScrollBar(
		CRect(m_WindowRect.Width() - 12, 1, m_WindowRect.Width(), m_WindowRect.Height() - 12)  - m_ClientRect.TopLeft(), this, CScrollBar::VERTICAL);
	m_pHorizontalScrollBar = new CScrollBar(
		CRect(1, m_WindowRect.Height() - 12, m_WindowRect.Width() - 12, m_WindowRect.Height())  - m_ClientRect.TopLeft(), this, CScrollBar::HORIZONTAL);
	m_ScrollBarVisibilityMap[CScrollBar::VERTICAL] = SCROLLBAR_VIS_AUTO;
	m_ScrollBarVisibilityMap[CScrollBar::HORIZONTAL] = SCROLLBAR_VIS_AUTO;
	PrepareWindowText("");

	CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_BUTTONUP);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::MOUSE_MOVE);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGE);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGING);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_DOUBLELCLICK);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_TIMER);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_GAININGKEYFOCUS);
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_LOSINGKEYFOCUS);
	Draw();
}


CTextBox::~CTextBox(void)  // virtual
{
	for(std::vector<CRenderedString*>::iterator iter = m_vpRenderedString.begin(); iter !=  m_vpRenderedString.end(); ++iter)
		delete *iter;
	m_vpRenderedString.clear();

	delete m_pCursorTimer;
	delete m_pDblClickTimer;
}


void CTextBox::SetReadOnly(bool bReadOnly)  // virtual
{
	m_BackgroundColor = bReadOnly ? COLOR_LIGHTGRAY : COLOR_WHITE;
	m_bReadOnly = bReadOnly;
	Draw();
}


std::string CTextBox::GetSelText(void) const  // virtual
{
	std::string sSelText = "";
	if (m_SelLength != 0)
	{
			std::string::size_type SelStartNorm = 0;
			std::string::size_type SelLenNorm = abs(m_SelLength);
			if (m_SelLength < 0)
			{
				SelStartNorm	= m_SelStart + m_SelLength;
			}
			else
			{
				SelStartNorm	= m_SelStart;
			}
			sSelText = m_sWindowText.substr(SelStartNorm, SelLenNorm);
	}

	return sSelText;
}


void CTextBox::SetSelection(std::string::size_type iSelStart, int iSelLength)  // virtual
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


void CTextBox::SetScrollBarVisibility(CScrollBar::EScrollBarType ScrollBarType, EScrollBarVisibility Visibility)  // virtual
{
	m_ScrollBarVisibilityMap[ScrollBarType] = Visibility;
	UpdateScrollBars();
}


void CTextBox::Draw(void) const  // virtual
{
	CWindow::Draw();

	if (m_pSDLSurface)
	{
		CPainter Painter(m_pSDLSurface, CPainter::PAINT_REPLACE);
		Painter.DrawRect(m_WindowRect.SizeRect(), false, COLOR_BLACK);
		CPoint FontCenterPoint = m_WindowRect.Center();

		CRGBColor FontColor = m_bReadOnly ? DEFAULT_DISABLED_LINE_COLOR : DEFAULT_LINE_COLOR;
		if (CApplication::Instance()->GetKeyFocus() == dynamic_cast<const CWindow*>(this) && !m_bReadOnly)
		{
			// first normalize the selection
			std::string::size_type SelStartNorm = 0;
			std::string::size_type SelLenNorm = abs(m_SelLength);
			if (m_SelLength < 0)
			{
				SelStartNorm	= m_SelStart + m_SelLength;
			}
			else
			{
				SelStartNorm	= m_SelStart;
			}
			CPoint SelStartPoint(RowColFromIndex(SelStartNorm));
			CPoint SelEndPoint(RowColFromIndex(SelStartNorm + SelLenNorm));

			// now get the dimensions for each character
			std::vector<CPoint> vOffsets;
			std::vector<CPoint> vBoundingDimensions;
			std::vector<std::vector<CRect> > vCharRects;
			int iIndex = 0;
			for (std::vector<CRenderedString*>::const_iterator iter = m_vpRenderedString.begin(); iter != m_vpRenderedString.end(); ++iter, ++iIndex)
			{
				CPoint BoundingDimensions;
				CPoint Offset;
				std::vector<CRect> CharRects;
				(*iter)->GetMetrics(&BoundingDimensions, &Offset, &CharRects);
				vBoundingDimensions.push_back(BoundingDimensions);
				vOffsets.push_back(Offset);
				vCharRects.push_back(CharRects);
			}

			// move the cursor into view by scrolling if necessary
			int CursorPos = vCharRects.at(SelStartPoint.YPos()).at(SelStartPoint.XPos()).Left() +
				vOffsets.at(SelStartPoint.YPos()).XPos() + m_ClientRect.Left() - m_pHorizontalScrollBar->GetValue() * 10;
			if (m_bScrollToCursor)
			{
				if (CursorPos < m_ClientRect.Left())
				{
					m_pHorizontalScrollBar->SetValue(m_pHorizontalScrollBar->GetValue() - (m_ClientRect.Left() - CursorPos) / 10 - 1);
					CursorPos = vCharRects.at(SelStartPoint.YPos()).at(SelStartPoint.XPos()).Left() +
						vOffsets.at(SelStartPoint.YPos()).XPos() + m_ClientRect.Left() - m_pHorizontalScrollBar->GetValue() * 10;
				}
				if (CursorPos > m_ClientRect.Right())
				{
					m_pHorizontalScrollBar->SetValue(m_pHorizontalScrollBar->GetValue() + (CursorPos - m_ClientRect.Right()) / 10 + 1);
					CursorPos = vCharRects.at(SelStartPoint.YPos()).at(SelStartPoint.XPos()).Left() +
						vOffsets.at(SelStartPoint.YPos()).XPos() + m_ClientRect.Left() - m_pHorizontalScrollBar->GetValue() * 10;
				}
				if (SelStartPoint.YPos() < m_pVerticalScrollBar->GetValue())
				{
					m_pVerticalScrollBar->SetValue(SelStartPoint.YPos());
				}
				if (SelStartPoint.YPos() > stdex::safe_static_cast<int>(m_pVerticalScrollBar->GetValue() + m_ClientRect.Height() / m_iRowHeight))
				{
					m_pVerticalScrollBar->SetValue(SelStartPoint.YPos() - m_ClientRect.Height() / m_iRowHeight);
				}
				m_bScrollToCursor = false;
			}

			// draw the selection
			if (m_SelLength != 0 && SelEndPoint.YPos() >= m_pVerticalScrollBar->GetValue())
			{
				for (int CurLine = SelStartPoint.YPos(); CurLine <= SelEndPoint.YPos(); ++CurLine)
				{
					CPoint TopLeft = m_ClientRect.TopLeft() + CPoint(0, m_iRowHeight * (CurLine - m_pVerticalScrollBar->GetValue()));
					CRect SelRect(TopLeft, TopLeft + CPoint(vBoundingDimensions.at(CurLine).XPos(), m_iRowHeight - 2));
					if (CurLine == SelStartPoint.YPos())
					{
						SelRect.SetLeft(vCharRects.at(CurLine).at(SelStartPoint.XPos()).Left() +
							vOffsets.at(CurLine).XPos() + m_ClientRect.Left() - m_pHorizontalScrollBar->GetValue() * 10);
					}
					if (CurLine == SelEndPoint.YPos())
					{
						SelRect.SetRight(vCharRects.at(CurLine).at(SelEndPoint.XPos() - 1).Right() +
							vOffsets.at(CurLine).XPos() + m_ClientRect.Left() - m_pHorizontalScrollBar->GetValue() * 10);
					}
					SelRect.ClipTo(m_ClientRect);
					Painter.DrawRect(SelRect, true, CApplication::Instance()->GetDefaultSelectionColor(), CApplication::Instance()->GetDefaultSelectionColor());
				}
			}

			// draw the cursor
			if (m_SelLength == 0 && SelStartPoint.YPos() >= m_pVerticalScrollBar->GetValue() && m_bDrawCursor)
			{
				if (CursorPos >= m_ClientRect.Left() && CursorPos <= m_ClientRect.Right())
				{
					Painter.DrawVLine(m_ClientRect.Top() + m_iRowHeight * (SelStartPoint.YPos() - m_pVerticalScrollBar->GetValue()) - 1,
						m_ClientRect.Top() + m_iRowHeight * (SelStartPoint.YPos() - m_pVerticalScrollBar->GetValue() + 1) - 1, CursorPos, COLOR_BLACK);
				}
			}
		}

		// finally, draw the text
		CPoint LineOrigin = m_ClientRect.TopLeft() - CPoint(m_pHorizontalScrollBar->GetValue() * 10, 0);
		for(std::vector<CRenderedString*>::const_iterator iter = m_vpRenderedString.begin() + m_pVerticalScrollBar->GetValue();
				iter != m_vpRenderedString.end() && LineOrigin.YPos() < m_ClientRect.Bottom(); ++iter) {
			(*iter)->Draw(m_pSDLSurface, m_ClientRect, LineOrigin, FontColor);
			LineOrigin.SetY(LineOrigin.YPos() + m_iRowHeight);
		}
	}
}


void CTextBox::SetWindowText(const std::string& sText)  // virtual
{
	m_SelStart = 0;
	m_SelLength = 0;
	PrepareWindowText(sText);
	CWindow::SetWindowText(sText);
}


void CTextBox::SetWindowRect(const CRect& WindowRect)
{
	CWindow::SetWindowRect(WindowRect);
	m_ClientRect = m_WindowRect.SizeRect();
	m_ClientRect.Grow(-3);
	UpdateScrollBars();
}


bool CTextBox::OnMouseButtonDown(CPoint Point, unsigned int Button)  // virtual
{
	bool bResult = CWindow::OnMouseButtonDown(Point, Button);

	CPoint WindowPoint(ViewToWindow(Point));
 	if (!bResult && m_bVisible && (m_ClientRect.HitTest(WindowPoint) == CRect::RELPOS_INSIDE))
 	{
		if (Button == CMouseMessage::LEFT && !m_bReadOnly)
		{
			bool fSkipCursorPositioning = false;
			// If we haven't initialized the double click timer, do so.
			if (!m_pDblClickTimer->IsRunning())
			{
				m_pDblClickTimer->StartTimer(500, false);
			}
			else
			{
				// Raise double click event
				CMessageServer::Instance().QueueMessage(new TIntMessage(CMessage::CTRL_DOUBLELCLICK, this, this, 0));
				fSkipCursorPositioning = true;
			}

			if (CApplication::Instance()->GetKeyFocus() != this)
			{
				CApplication::Instance()->SetKeyFocus(this);
			}

			if (!fSkipCursorPositioning)
			{
				// set the cursor
				std::vector<CPoint> vOffsets;
				std::vector<std::vector<CRect> > vCharRects;
				int iIndex = 0;
				// get the dimensions of all the characters
				for (std::vector<CRenderedString*>::iterator iter = m_vpRenderedString.begin(); iter != m_vpRenderedString.end(); ++iter, ++iIndex)
				{
					CPoint Offset;
					std::vector<CRect> CharRects;
					(*iter)->GetMetrics(nullptr, &Offset, &CharRects);
					vOffsets.push_back(Offset);
					vCharRects.push_back(CharRects);
				}

				// figure out which line was clicked on
				unsigned int iCurLine = (WindowPoint.YPos() - m_ClientRect.Top()) / m_iRowHeight + m_pVerticalScrollBar->GetValue();
				if (iCurLine >= m_iLineCount)
				{
					iCurLine = m_iLineCount - 1;
				}
				// figure out which character was clicked on
				int xDelta = abs(WindowPoint.XPos() - (vCharRects.at(iCurLine).at(0).Left() + vOffsets.at(iCurLine).XPos() + m_ClientRect.Left()));
				m_SelStart = 0;
				for (unsigned int i = 0; i < vCharRects.at(iCurLine).size(); ++i)
				{
					if (abs(WindowPoint.XPos() - (vCharRects.at(iCurLine).at(i).Right() + vOffsets.at(iCurLine).XPos()
						+ m_ClientRect.Left() - m_pHorizontalScrollBar->GetValue() * 10)) < xDelta)
					{
						xDelta = abs(WindowPoint.XPos() - (vCharRects.at(iCurLine).at(i).Right() + vOffsets.at(iCurLine).XPos()
							+ m_ClientRect.Left() - m_pHorizontalScrollBar->GetValue() * 10));
						m_SelStart = i + 1;
					}
				}
				for (unsigned int iChar = 0; iChar < iCurLine; ++iChar)
				{
					m_SelStart += vCharRects.at(iChar).size();
				}
			}

			m_DragStart = m_SelStart;
			m_SelLength = 0;
			m_bMouseDown = true;
			Draw();
			bResult = true;
		}
		if (Button == CMouseMessage::WHEELUP)
		{
			m_pVerticalScrollBar->SetValue(m_pVerticalScrollBar->GetValue() - 1);
			bResult = true;
		}
		else if (Button == CMouseMessage::WHEELDOWN)
		{
			m_pVerticalScrollBar->SetValue(m_pVerticalScrollBar->GetValue() + 1);
			bResult = true;
		}
	}

	return bResult;
}


bool CTextBox::HandleMessage(CMessage* pMessage)  // virtual
{
	bool bHandled = false;

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
					// get the dimensions for all the characters
					std::vector<CPoint> vOffsets;
					std::vector<std::vector<CRect> > vCharRects;
					int iIndex = 0;
					for (std::vector<CRenderedString*>::iterator iter = m_vpRenderedString.begin(); iter != m_vpRenderedString.end(); ++iter, ++iIndex)
					{
						CPoint Offset;
						std::vector<CRect> CharRects;
						(*iter)->GetMetrics(nullptr, &Offset, &CharRects);
						vOffsets.push_back(Offset);
						vCharRects.push_back(CharRects);
					}

					// figure out which line was clicked on
					unsigned int iCurLine = 0;
					if (m_ClientRect.HitTest(WindowPoint) == CRect::RELPOS_INSIDE)
					{
						iCurLine = (WindowPoint.YPos() - m_ClientRect.Top()) / m_iRowHeight + m_pVerticalScrollBar->GetValue();
						if (iCurLine >= m_iLineCount)
						{
							iCurLine = m_iLineCount - 1;
						}
					}
					else if (m_ClientRect.HitTest(WindowPoint) == CRect::RELPOS_BELOW)
					{
						iCurLine = m_iLineCount - 1;
					}

					// figure out which character was clicked on
					int xDelta = abs(WindowPoint.XPos() - (vCharRects.at(iCurLine).at(0).Left() + vOffsets.at(iCurLine).XPos() + m_ClientRect.Left()));
					std::string::size_type CursorPos = 0;
					for (unsigned int i = 0; i < vCharRects.at(iCurLine).size(); ++i)
					{
						if (abs(WindowPoint.XPos() - (vCharRects.at(iCurLine).at(i).Right() + vOffsets.at(iCurLine).XPos()
							+ m_ClientRect.Left() - m_pHorizontalScrollBar->GetValue() * 10)) < xDelta)
						{
							xDelta = abs(WindowPoint.XPos() - (vCharRects.at(iCurLine).at(i).Right() + vOffsets.at(iCurLine).XPos()
								+ m_ClientRect.Left() - m_pHorizontalScrollBar->GetValue() * 10));
							CursorPos = i + 1;
						}
					}
					for (unsigned int iChar = 0; iChar < iCurLine; ++iChar)
					{
						CursorPos += vCharRects.at(iChar).size();
					}

					// set the highlighting
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
				m_bDrawCursor = !m_bDrawCursor;
				Draw();
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
					case SDLK_DOWN:
						if (m_SelStart <= sBuffer.length())
						{
							if (pKeyboardMessage->Modifiers & KMOD_SHIFT)
							{
								CPoint CursorPoint(RowColFromIndex(m_SelStart + m_SelLength));
 								if (CursorPoint.YPos() == stdex::safe_static_cast<int>(m_iLineCount - 1))
								{
									m_SelLength = stdex::safe_static_cast<int>(m_sWindowText.size() - m_SelStart);
 								}
								else
								{
									m_SelLength = stdex::safe_static_cast<int>(IndexFromRowCol(CursorPoint.YPos() + 1, CursorPoint.XPos()))
										- stdex::safe_static_cast<int>(m_SelStart);
 								}
							}
							else if(m_SelLength == 0)
							{
								CPoint CursorPoint(RowColFromIndex(m_SelStart));
								if (CursorPoint.YPos() == stdex::safe_static_cast<int>(m_iLineCount - 1))
								{
									m_SelStart = m_sWindowText.size();
								}
								else
								{
									m_SelStart = IndexFromRowCol(CursorPoint.YPos() + 1, CursorPoint.XPos());
								}
							}
							else
							{
								if (m_SelLength > 0)
								{
									m_SelStart = m_SelStart + m_SelLength;
								}
								m_SelLength = 0;
							}
						}
						break;
					case SDLK_UP:
						if (m_SelStart <= sBuffer.length())
						{
							if (pKeyboardMessage->Modifiers & KMOD_SHIFT)
							{
								CPoint CursorPoint(RowColFromIndex(m_SelStart + m_SelLength));
 								if (CursorPoint.YPos() == 0)
								{
									m_SelLength = -stdex::safe_static_cast<int>(m_SelStart);
								}
								else
								{
									m_SelLength = stdex::safe_static_cast<int>(IndexFromRowCol(CursorPoint.YPos() - 1, CursorPoint.XPos()))
										- stdex::safe_static_cast<int>(m_SelStart);
								}
							}
							else if(m_SelLength == 0 )
							{
								CPoint CursorPoint(RowColFromIndex(m_SelStart));
								if (CursorPoint.YPos() == 0)
								{
									m_SelStart = 0;
								}
								else
								{
									m_SelStart = IndexFromRowCol(CursorPoint.YPos() - 1, CursorPoint.XPos());
								}
							}
							else
							{
								if (m_SelLength < 0)
								{
									m_SelStart = m_SelStart + m_SelLength;
								}
								m_SelLength = 0;
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
					case SDLK_RETURN:
						SelDelete(&sBuffer);
						sBuffer.insert(m_SelStart++, 1, '\n');
						break;
					default:
						if (pKeyboardMessage->Unicode)
						{
							if ((pKeyboardMessage->Unicode & 0xFF80) == 0)
							{
								SelDelete(&sBuffer);
								// we are deliberately truncating the unicode data, so don't use safe_static_cast
								sBuffer.insert(m_SelStart++, 1, static_cast<char>(pKeyboardMessage->Unicode & 0x7F));
							}
							else
							{
								wUtil::Trace("CTextBox::HandleMessage : CTextBox can't handle Unicode characters yet.");
							}
						}
						break;
					}

					if (m_sWindowText != sBuffer)
					{
						CMessageServer::Instance().QueueMessage(new TStringMessage(CMessage::CTRL_VALUECHANGE, m_pParentWindow, this, sBuffer));
						m_sWindowText = sBuffer;
						PrepareWindowText(sBuffer);
					}
					m_bDrawCursor = true;
					m_bScrollToCursor = true;  // a key was pressed, so we need to make sure that the cursor is visible
					Draw();
				}
				break;
			}
		case CMessage::CTRL_VALUECHANGE:
		case CMessage::CTRL_VALUECHANGING:
		{
			if (pMessage->Source() == m_pVerticalScrollBar || pMessage->Source() == m_pHorizontalScrollBar)
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


void CTextBox::SelDelete(std::string* psString)
{
	//This means we've selected something, therefore, should replace it
	if (m_SelLength != 0)
	{
		std::string::size_type SelStartNorm = 0;
		std::string::size_type SelLenNorm = abs(m_SelLength);
		if (m_SelLength < 0)
		{
			SelStartNorm	= m_SelStart + m_SelLength;
		}
		else
		{
			SelStartNorm	= m_SelStart;
		}
		psString->erase(SelStartNorm, SelLenNorm);
		//Since we're deleting the stuff here and setting the selection length to zero, we also need to set the selection start properly
		m_SelStart = SelStartNorm;
		m_SelLength = 0;
	}
}


void CTextBox::PrepareWindowText(const std::string& sText)
{
	for(std::vector<CRenderedString*>::iterator iter = m_vpRenderedString.begin(); iter !=  m_vpRenderedString.end(); ++iter)
		delete *iter;
	m_vpRenderedString.clear();

	m_iLineCount = 0;
	std::string::size_type start = 0;
	std::string::size_type loc = 0;
	m_iMaxWidth = 0;
	while (loc != std::string::npos)
	{
		loc = sText.find("\n", start);
		m_vpRenderedString.push_back(new CRenderedString(
			m_pFontEngine, sText.substr(start, loc - start), CRenderedString::VALIGN_TOP, CRenderedString::HALIGN_LEFT));
		CPoint BoundingDimensions;
		m_vpRenderedString.back()->GetMetrics(&BoundingDimensions, nullptr, nullptr);
		if (BoundingDimensions.XPos() > stdex::safe_static_cast<int>(m_iMaxWidth))
		{
			m_iMaxWidth = BoundingDimensions.XPos();
		}
		start = loc + 1;
		++m_iLineCount;
	}
	m_iRowHeight = m_vpRenderedString.at(0)->GetMaxFontHeight() + 2;

	UpdateScrollBars();
}


void CTextBox::UpdateScrollBars(void)
{
	m_ClientRect = CRect(3, 3, m_WindowRect.Width() - 5, m_WindowRect.Height() - 5);
	bool bVertVisible = m_ScrollBarVisibilityMap[CScrollBar::VERTICAL] == SCROLLBAR_VIS_ALWAYS ||
		(m_ScrollBarVisibilityMap[CScrollBar::VERTICAL] == SCROLLBAR_VIS_AUTO && m_iLineCount > (m_ClientRect.Height() - 12) / m_iRowHeight);
	bool bHorzVisible = m_ScrollBarVisibilityMap[CScrollBar::HORIZONTAL] == SCROLLBAR_VIS_ALWAYS ||
		(m_ScrollBarVisibilityMap[CScrollBar::HORIZONTAL] == SCROLLBAR_VIS_AUTO && stdex::safe_static_cast<int>(m_iMaxWidth) > (m_ClientRect.Width() - 12));
	int iMaxHorzLimit = (stdex::safe_static_cast<int>(m_iMaxWidth) - (m_ClientRect.Width() - 12)) / 10;
	if (iMaxHorzLimit < 0)
	{
		iMaxHorzLimit = 0;
	}

	m_pVerticalScrollBar->SetVisible(bVertVisible);
	m_pHorizontalScrollBar->SetVisible(bHorzVisible);
	int iLastLine = m_iLineCount - 1;
	if (iLastLine < 0)
	{
		iLastLine = 0;
	}
	if (bVertVisible)
	{
		m_ClientRect.SetRight(m_WindowRect.Width() - 17);
	}
	if (bHorzVisible)
	{
		m_ClientRect.SetBottom(m_WindowRect.Height() - 17);
	}

	m_pVerticalScrollBar->SetMaxLimit(iLastLine);
	if (m_pVerticalScrollBar->GetValue() > iLastLine)
	{
		m_pVerticalScrollBar->SetValue(iLastLine);
	}
	m_pVerticalScrollBar->SetWindowRect(CRect(m_ClientRect.Width() + 2, 1 - m_ClientRect.Top(), m_ClientRect.Width() + 14, m_ClientRect.Height()));
	m_pHorizontalScrollBar->SetMaxLimit(iMaxHorzLimit);
	if (m_pHorizontalScrollBar->GetValue() > iMaxHorzLimit)
	{
		m_pHorizontalScrollBar->SetValue(iMaxHorzLimit);
	}
	m_pHorizontalScrollBar->SetWindowRect(CRect(1 - m_ClientRect.Left(), m_ClientRect.Height() + 2, m_ClientRect.Width(), m_ClientRect.Height() + 14));
}


CPoint CTextBox::RowColFromIndex(std::string::size_type Index) const
{
	int iRow = 0;
	int iCol = stdex::safe_static_cast<int>(Index);
	std::string::size_type loc = m_sWindowText.find("\n");
	std::string::size_type start = 0;
	while (loc != std::string::npos && loc < Index)
	{
		++iRow;
		iCol -= (stdex::safe_static_cast<int>(loc - start) + 1);
		start = loc + 1;
		loc = m_sWindowText.find("\n", start);
	}
	return CPoint(iCol, iRow);
}


std::string::size_type CTextBox::IndexFromRowCol(std::string::size_type Row, std::string::size_type Col) const
{
	std::string::size_type Index = 0;
	for (std::string::size_type iRow = 0; iRow < Row && iRow < m_vpRenderedString.size(); ++iRow)
		Index += m_vpRenderedString.at(iRow)->GetLength() + 1;
	if (Col > m_vpRenderedString.at(Row)->GetLength())
		Col = m_vpRenderedString.at(Row)->GetLength();
	return Index + Col;
}

}
