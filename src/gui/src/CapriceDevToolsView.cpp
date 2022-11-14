// Developers' tools for Caprice32
#include "CapriceDevToolsView.h"
#include "CapriceDevTools.h"
#include <string>

using namespace wGui;

CapriceDevToolsView::CapriceDevToolsView(CApplication& application, SDL_Surface* surface, SDL_Renderer* renderer, SDL_Texture* texture, const CRect& WindowRect, DevTools* devtools) : CView(application, surface, nullptr, WindowRect), m_pRenderer(renderer), m_pTexture(texture)
{
  Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_MESSAGEBOXRETURN);
  m_pDevToolsFrame = new CapriceDevTools(CRect(CPoint(0, 0), DEVTOOLS_WIDTH, DEVTOOLS_HEIGHT), this, nullptr, devtools);
  m_pDevToolsFrame->UpdateAll();
}

void CapriceDevToolsView::LoadSymbols(const std::string& filename)
{
  m_pDevToolsFrame->LoadSymbols(filename);
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

void CapriceDevToolsView::PreUpdate()
{
  m_pDevToolsFrame->PreUpdate();
}

void CapriceDevToolsView::PostUpdate()
{
  m_pDevToolsFrame->PostUpdate();
}

void CapriceDevToolsView::Flip() const
{
  SDL_UpdateTexture(m_pTexture, nullptr, m_pScreenSurface->pixels, m_pScreenSurface->pitch);
  SDL_RenderClear(m_pRenderer);
  SDL_RenderCopy(m_pRenderer, m_pTexture, nullptr, nullptr);
  SDL_RenderPresent(m_pRenderer);
}

void CapriceDevToolsView::Close()
{
  m_pDevToolsFrame->CloseFrame();
}
