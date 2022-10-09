#ifndef CAPRICEGUI_H
#define CAPRICEGUI_H

#include "wg_application.h"
#include "wg_view.h"
#include "SDL.h"

class CapriceGui : public wGui::CApplication
{
  public:
    CapriceGui(SDL_Window* pWindow, bool bInMainView, int scale=1) : wGui::CApplication(pWindow) {
      m_bInMainView = bInMainView;
      m_iScale = scale;
    };
    void Init() override;
};

#endif
