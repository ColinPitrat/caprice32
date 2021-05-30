// Developers' tools for Caprice32
#ifndef _WG_CAPRICEDEVTOOLSVIEW_H
#define _WG_CAPRICEDEVTOOLSVIEW_H

#include <string>
#include "wg_point.h"
#include "wg_rect.h"
#include "wg_view.h"
#include "CapriceDevTools.h"

class DevTools;

constexpr int DEVTOOLS_WIDTH = 640;
constexpr int DEVTOOLS_HEIGHT = 480;

class CapriceDevToolsView : public wGui::CView
{
  protected:
    wGui::CapriceDevTools *m_pDevToolsFrame;
    SDL_Renderer *m_pRenderer;
    SDL_Texture *m_pTexture;

  public:
    CapriceDevToolsView(wGui::CApplication& application, SDL_Surface* surface, SDL_Renderer* renderer, SDL_Texture* texture, const wGui::CRect& WindowRect, DevTools* devtools);
    ~CapriceDevToolsView() final = default;

    void LoadSymbols(const std::string& filename);

    void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const wGui::CPoint& Offset) const override;

    void PreUpdate();
    void PostUpdate();

    void Flip() const override;

    void Close() override;
};

#endif
