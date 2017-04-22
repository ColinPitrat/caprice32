#include "wgui.h"

class CapriceGuiView : public wGui::CView
{
  protected:
    wGui::CFrame *m_menuFrame;

  public:
    CapriceGuiView(SDL_Surface* surface, SDL_Surface* backSurface, const wGui::CRect& WindowRect);

    void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const wGui::CPoint& Offset) const override;
};
