#include "wgui.h"

using namespace std;
using namespace wGui;

class CapriceGuiView : public CView
{
  protected:
    CFrame *m_menuFrame;

  public:
    CapriceGuiView(SDL_Surface* surface, SDL_Surface* backSurface, const CRect& WindowRect);
    
    virtual void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const;
};
