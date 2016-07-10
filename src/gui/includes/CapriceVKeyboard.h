#ifndef _WG_CAPRICEVKEYBOARD_H_
#define _WG_CAPRICEVKEYBOARD_H_

#include "wgui.h"
#include "wg_frame.h"
#include "wg_button.h"

using namespace std;
using namespace wGui;

namespace wGui
{
  class CapriceVKeyboard : public CFrame
  {
    public:
      CapriceVKeyboard(const CRect& WindowRect, CWindow* pParent, SDL_Surface* screen, CFontEngine* pFontEngine);
      ~CapriceVKeyboard();

      void CloseFrame(void) override;
      bool HandleMessage(CMessage* pMessage) override;
      std::list<SDL_Event> GetEvents();

      std::list<SDL_Event> StringToEvents(std::string toTranslate);
    protected:
      void moveFocus(int dx, int dy);

      std::map<char, std::pair<SDLKey,SDLMod>> m_keyFromChar;

      CEditBox* m_result;
      std::vector<std::vector<CButton*>> m_buttons;
      std::pair<int, int> m_focused;

      SDL_Surface *m_pScreenSurface;

    private:
      CapriceVKeyboard operator=(CapriceVKeyboard) = delete;
  };
}

#endif
