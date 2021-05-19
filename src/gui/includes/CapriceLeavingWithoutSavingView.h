#ifndef _WG_CAPRICELEAVINGWITHOUTSAVINGVIEW_H_
#define _WG_CAPRICELEAVINGWITHOUTSAVINGVIEW_H_

#include "wgui.h"
#include "wg_messagebox.h"

using namespace wGui;

class CapriceLeavingWithoutSavingView : public CView
{
  protected:
    wGui::CMessageBox *m_pMessageBox;
    bool confirmed = false;

  public:
    CapriceLeavingWithoutSavingView(CApplication& application, SDL_Surface* surface, SDL_Surface* backSurface, const CRect& WindowRect);

    bool Confirmed() const;

    void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const override;

    bool HandleMessage(CMessage* pMessage) override;
};

#endif
