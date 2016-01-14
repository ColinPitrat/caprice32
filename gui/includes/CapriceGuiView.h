#include "wgui.h"

using namespace std;
using namespace wGui;

class CapriceGuiView : public CView
{
  protected:
    CButton* m_pBtnOptions;
    CButton* m_pBtnLoadSave;
    CButton* m_pBtnMemoryTool;
    CButton* m_pBtnReset;
    CButton* m_pBtnAbout;
    CButton* m_pBtnResume;
    CButton* m_pBtnQuit;
    CEditBox* m_pDisplay;

  public:
    CapriceGuiView(SDL_Surface* surface, SDL_Surface* backSurface, const CRect& WindowRect);
    
    // specific functionality, see .cpp file.
    virtual void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const;

    bool HandleMessage(CMessage* pMessage);
};
