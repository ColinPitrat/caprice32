#include "wgui.h"

using namespace std;
using namespace wGui;

enum class MenuItem {
  NONE,
  OPTIONS,
  LOAD_SAVE,
  MEMORY_TOOL,
  RESET,
  ABOUT,
  RESUME,
  QUIT
};

class CapriceGuiViewButton
{
  public:
    CapriceGuiViewButton(MenuItem item, CButton *button) : m_item(item), m_button(button) {};
    ~CapriceGuiViewButton() {};

    CButton *GetButton() const { return m_button.get(); };
    MenuItem GetItem() const { return m_item; };

  private:
    MenuItem m_item;
    std::shared_ptr<CButton> m_button;
};

enum class FocusDirection {
  FORWARD,
  BACKWARD
};

class CapriceGuiView : public CView
{
  protected:
    std::list<CapriceGuiViewButton> m_buttons;

  public:
    CapriceGuiView(SDL_Surface* surface, SDL_Surface* backSurface, const CRect& WindowRect);
    
    void FocusNext(FocusDirection direction);

    // specific functionality, see .cpp file.
    virtual void PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const;

    bool HandleMessage(CMessage* pMessage);
};
