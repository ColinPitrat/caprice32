// 'Menu' window for Caprice32
// Inherited from CFrame

#ifndef _WG_CAPRICE32MENU_H_
#define _WG_CAPRICE32MENU_H_

#include "wgui.h"
#include "wg_frame.h"
#include "wg_button.h"

namespace wGui
{
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

  class CapriceMenu : public CFrame {
    public:
      //! \param pParent A pointer to the parent view
      //! \param pFontEngine A pointer to the font engine to use when drawing the control
      //! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
      CapriceMenu(const CRect& WindowRect, CWindow* pParent, SDL_Surface* screen, CFontEngine* pFontEngine);
      ~CapriceMenu();

      void CloseFrame(void) override;
      bool HandleMessage(CMessage* pMessage) override;

    protected:
      std::list<CapriceGuiViewButton> m_buttons;
      SDL_Surface *m_pScreenSurface;
      
      CapriceMenu& operator=(CapriceMenu) = delete;
  };
}

#endif  // _WG_CAPRICE32MENU_H_
