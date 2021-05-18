#include "CapriceVKeyboard.h"
#include "cap32.h"
#include "keyboard.h"
#include <string>

extern t_CPC CPC;

namespace wGui {

  CapriceVKeyboard::CapriceVKeyboard(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
    CFrame(WindowRect, pParent, pFontEngine, "Caprice32 - Virtual Keyboard", false)
  {
    // TODO: This became ugly with time ... Make this more generic by creating a
    // class key that has a displayable string and SDL events associated
    SetModal(true);
    std::vector<std::string> keys{ "ABCDEFGHIJ", "KLMNOPQRST", "UVWXYZabcd", "efghijklmn", "opqrstuvwx", "yz01234567", "89&#\"'(-_)", "=,.:!|?./*", "+%<>[]{}\\`"};
    // TODO: make this configurable
    std::vector<std::string> keywords{ "cat\n", "run\n", "run\"", "cls\n", "mode ", "|cpm\n", "|tape\n", "|a\n", "|b\n" };
    // TODO: add files that are on disk
    m_focused.first = 0;
    m_focused.second = 0;
    m_result = new CEditBox(CRect(CPoint(10, 10), 364, 15), this);
    m_result->SetReadOnly(true);
    // Add one char keys
    int y = 30;
    for(auto& l : keys)
    {
      std::vector<CButton*> line;
      int x = 10;
      for(auto& c : l)
      {
        CButton *button = new CButton(CRect(CPoint(x, y), 15, 15), this, std::string(1, c));
        button->SetIsFocusable(true);
        line.push_back(button);
        x += 20;
      }
      y += 20;
      m_buttons.push_back(line);
    }
    // Add function keys
    {
      std::vector<CButton*> line;
      int x = 10;
      for(int i = 0; i < 10; i++)
      {
        char fn = '0' + i;
        std::string label = std::string("F") + std::string(1, fn);
        CButton *button = new CButton(CRect(CPoint(x, y), 15, 15), this, label);
        button->SetIsFocusable(true);
        line.push_back(button);
        x += 20;
      }
      y += 18;
      m_buttons.push_back(line);
    }
    // Add ESC, SPACE, DELETE and RETURN buttons
    // TODO: TAB ? COPY ?
    std::vector<CButton*> line;
    CButton *esc = new CButton(CRect(CPoint(10,  y), 41, 15), this, "ESC");
    CButton *space = new CButton(CRect(CPoint(62,  y), 41, 15), this, "SPACE");
    CButton *retur = new CButton(CRect(CPoint(113,  y), 41, 15), this, "RETURN");
    CButton *backs = new CButton(CRect(CPoint(164, y), 41, 15), this, "DELETE");
    esc->SetIsFocusable(true);
    space->SetIsFocusable(true);
    retur->SetIsFocusable(true);
    backs->SetIsFocusable(true);
    line.push_back(esc);
    line.push_back(space);
    line.push_back(retur);
    line.push_back(backs);
    m_buttons.push_back(line);
    // Add keywords
    int kx = 160;
    int ky = 0;
    int nb_lines = m_buttons.size();
    int i = nb_lines;
    for(auto& w : keywords)
    {
      if(i >= nb_lines) {
        if(kx > 290) break;
        i -= nb_lines;
        ky = 30;
        kx += 70;
      }
      CButton *button = new CButton(CRect(CPoint(kx, ky), 60, 15), this, w);
      button->SetIsFocusable(true);
      m_buttons[i++].push_back(button);
      ky += 18;
    }
  }

  CapriceVKeyboard::~CapriceVKeyboard() = default;

  void CapriceVKeyboard::CloseFrame() {
    // Exit gui
    CApplication::Instance()->MessageServer()->QueueMessage(new CMessage(CMessage::APP_EXIT, nullptr, this));
  }

  std::list<SDL_Event> CapriceVKeyboard::GetEvents() {
    return CPC.InputMapper->StringToEvents(m_result->GetWindowText());
  }

  void CapriceVKeyboard::moveFocus(int dx, int dy) {
    m_buttons[m_focused.first][m_focused.second]->SetHasFocus(false);

    m_focused.first += dy;
    int height = m_buttons.size();
    if(m_focused.first < 0) m_focused.first += height;
    if(m_focused.first >= height) m_focused.first -= height;

    m_focused.second += dx;
    int width = m_buttons[m_focused.first].size();
    if(m_focused.second < 0) m_focused.second += width;
    if(m_focused.second >= width) {
      if(dx == 0) {
        m_focused.second = width - 1;
      } else {
        m_focused.second -= width;
      }
    }

    m_buttons[m_focused.first][m_focused.second]->SetHasFocus(true);
  }

  bool CapriceVKeyboard::HandleMessage(CMessage* pMessage)
  {
    bool bHandled = false;

    if (pMessage)
    {
      switch (pMessage->MessageType())
      {
        case CMessage::APP_DESTROY_FRAME:
          bHandled = true;
          CloseFrame();
          break;
        case CMessage::CTRL_SINGLELCLICK:
          if (pMessage->Destination() == this && pMessage->Source() != m_pFrameCloseButton) {
            std::string pressed = static_cast<const CWindow*>(pMessage->Source())->GetWindowText();
            if(pressed == "SPACE") {
              pressed = " ";
            }
            else if(pressed == "RETURN") {
              pressed = "\n";
            }
            else if(pressed == "ESC") {
              pressed = "\a";
              pressed += static_cast<char>(CPC_ESC);
            }
            else if(pressed == "DELETE") {
              std::string result = m_result->GetWindowText();
              // If the string is not empty, and last char is not backspace remove it
              if(!result.empty() && result[result.size()-1] != '\b') {
                result = result.substr(0, result.size()-1);
                // If the char was a special char, also remove the escaping char
                if(!result.empty() && result[result.size()-1] == '\a') {
                  result = result.substr(0, result.size()-1);
                }
                m_result->SetWindowText(result);
                break;
              }
              // Otherwise put backspace in the output
              pressed = "\b";
            } else if(pressed.size() == 2 && pressed[0] == 'F' && pressed[1] >= '0' && pressed[1] <= '9') {
               int fkey = CPC_F0 + pressed[1] - '0';
               pressed = "\a";
               pressed += static_cast<char>(fkey);
            }
            std::string result = m_result->GetWindowText() + pressed;
            m_result->SetWindowText(result);
          }
          break;
        case CMessage::KEYBOARD_KEYDOWN:
          if (m_bVisible && pMessage->Destination() == this) {
            CKeyboardMessage* pKeyboardMessage = dynamic_cast<CKeyboardMessage*>(pMessage);
            if (pKeyboardMessage) {
              switch (pKeyboardMessage->Key) {
                case SDLK_UP:
                  moveFocus(0, -1);
                  bHandled = true;
                  break;
                case SDLK_DOWN:
                  moveFocus(0, 1);
                  bHandled = true;
                  break;
                case SDLK_LEFT:
                  moveFocus(-1, 0);
                  bHandled = true;
                  break;
                case SDLK_RIGHT:
                  moveFocus(1, 0);
                  bHandled = true;
                  break;
                case SDLK_ESCAPE:
                  bHandled = true;
                  CloseFrame();
                  break;
                default:
                  break;
              }
            }
          }
          break;
        default:
          break;
      }
    }
    if(!bHandled) {
      bHandled = CFrame::HandleMessage(pMessage);
    }

    return bHandled;
  }
}
