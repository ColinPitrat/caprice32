#ifndef _WG_CAPRICEVKEYBOARDVIEW_H_
#define _WG_CAPRICEVKEYBOARDVIEW_H_

#include "CapriceVKeyboardView.h"

CapriceVKeyboardView::CapriceVKeyboardView(SDL_Surface* surface, SDL_Surface* backSurface, const CRect& WindowRect) : CView(surface, backSurface, WindowRect)
{
  m_kbdFrame = new CapriceVKeyboard(CRect(CPoint(0, 0), 384, 270), this, m_pScreenSurface, nullptr);
}

std::list<SDL_Event> CapriceVKeyboardView::GetEvents()
{
  return m_kbdFrame->GetEvents();
}

void CapriceVKeyboardView::PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const
{
  if (m_bVisible)
  {
    // Reset backgound
    SDL_BlitSurface(m_pBackSurface, nullptr, &ScreenSurface, nullptr);

    // Draw all child windows recursively
    for (std::list<CWindow*>::const_iterator iter = m_ChildWindows.begin(); iter != m_ChildWindows.end(); ++iter)
    {
      if (*iter)
      {
        (*iter)->PaintToSurface(ScreenSurface, FloatingSurface, Offset);
      }
    }
  }
}

#endif
