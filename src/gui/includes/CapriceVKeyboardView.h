#ifndef _WG_CAPRICEVKEYBOARDVIEW_H_
#define _WG_CAPRICEVKEYBOARDVIEW_H_

#include "wgui.h"
#include "CapriceVKeyboard.h"

using namespace wGui;

class CapriceVKeyboardView : public CView
{
  protected:
    CapriceVKeyboard* m_kbdFrame;

  public:
    CapriceVKeyboardView(SDL_Surface* surface, SDL_Surface* backSurface, const CRect& WindowRect);

    std::list<SDL_Event> GetEvents();

    void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const override;
};

#endif
