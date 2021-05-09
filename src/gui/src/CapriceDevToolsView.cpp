// Developer's tools for Caprice32
#include "CapriceDevToolsView.h"
#include "CapriceDevTools.h"

using namespace wGui;

CapriceDevToolsView::CapriceDevToolsView(SDL_Surface* surface, SDL_Renderer* renderer, SDL_Texture* texture, const CRect& WindowRect, DevTools* devtools) : CView(surface, nullptr, WindowRect), m_pRenderer(renderer), m_pTexture(texture)
{
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_MESSAGEBOXRETURN);
  m_pDevToolsFrame = new CapriceDevTools(CRect(CPoint(0, 0), DEVTOOLS_WIDTH, DEVTOOLS_HEIGHT), this, nullptr, devtools);
}

void CapriceDevToolsView::PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const
{
  if (m_bVisible)
  {
    // Reset backgound
    SDL_FillRect(&ScreenSurface, nullptr, SDL_MapRGB(ScreenSurface.format, 255, 255, 255));

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

void CapriceDevToolsView::Update()
{
  m_pDevToolsFrame->Update();
}

void CapriceDevToolsView::Flip() const
{
  SDL_UpdateTexture(m_pTexture, nullptr, m_pScreenSurface->pixels, m_pScreenSurface->pitch);
  SDL_RenderClear(m_pRenderer);
  SDL_RenderCopy(m_pRenderer, m_pTexture, nullptr, nullptr);
  SDL_RenderPresent(m_pRenderer);
}
