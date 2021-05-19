#include "CapriceGuiView.h"
#include "CapriceMenu.h"

using namespace wGui;

CapriceGuiView::CapriceGuiView(CApplication& application, SDL_Surface* surface, SDL_Surface* backSurface, const CRect& WindowRect) : CView(application, surface, backSurface, WindowRect)
{
  Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_MESSAGEBOXRETURN);
  m_menuFrame = new CapriceMenu(CRect(CPoint(m_pScreenSurface->w / 2 - 70, m_pScreenSurface->h / 2 - 110), 140, 240), this, m_pScreenSurface, nullptr);
}

// judb Show the Caprice32 emulation display, and our CCaGuiView (CView) on top of it.
// The only CView object in Caprice32 is the first window you see when you activate the gui.
void CapriceGuiView::PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const
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
