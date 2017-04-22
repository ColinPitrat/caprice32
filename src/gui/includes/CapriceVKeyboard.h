#ifndef _WG_CAPRICEVKEYBOARD_H_
#define _WG_CAPRICEVKEYBOARD_H_

#include "wgui.h"
#include "wg_frame.h"
#include "wg_button.h"

namespace wGui
{
  class CapriceVKeyboard : public CFrame
  {
    public:
      CapriceVKeyboard(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine);
      ~CapriceVKeyboard() override;

      void CloseFrame() override;
      bool HandleMessage(CMessage* pMessage) override;
      std::list<SDL_Event> GetEvents();

      static std::list<SDL_Event> StringToEvents(std::string toTranslate);
    protected:
      void moveFocus(int dx, int dy);

      CEditBox* m_result;
      std::vector<std::vector<CButton*>> m_buttons;
      std::pair<int, int> m_focused;

    private:
      CapriceVKeyboard(const CapriceVKeyboard&) = delete;
      CapriceVKeyboard& operator=(const CapriceVKeyboard&) = delete;
  };
}

#endif
