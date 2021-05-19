#include "CapriceVKeyboardView.h"

CapriceVKeyboardView::CapriceVKeyboardView(CApplication& application, SDL_Surface* surface, SDL_Surface* backSurface, const CRect& WindowRect) : CView(application, surface, backSurface, WindowRect)
{
  m_kbdFrame = new CapriceVKeyboard(CRect(CPoint(0, 0), 384, 270), this, nullptr);
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
    for (const auto child : m_ChildWindows)
    {
      if (child)
      {
        child->PaintToSurface(ScreenSurface, FloatingSurface, Offset);
      }
    }
  }
}
