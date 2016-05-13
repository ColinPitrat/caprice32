// 'Menu' window for Caprice32
// Inherited from CFrame

#include <map>
#include "std_ex.h"
#include "CapriceMenu.h"
#include "CapriceOptions.h"
#include "CapriceLoadSave.h"
#include "CapriceMemoryTool.h"
#include "CapriceAbout.h"
#include "cap32.h"

// CPC emulation properties, defined in cap32.h:
extern t_CPC CPC;

namespace wGui {

CapriceMenu::CapriceMenu(const CRect& WindowRect, CView* pParent, CFontEngine* pFontEngine) :
  CFrame(WindowRect, pParent, pFontEngine, "Caprice32 - Menu", false), m_pParentView(pParent)
{
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
  CApplication::Instance()->SetKeyFocus(this);

  std::map<MenuItem, std::string> buttons = {
    { MenuItem::OPTIONS, "Options" },
    { MenuItem::LOAD_SAVE, "Load / Save" },
    { MenuItem::MEMORY_TOOL, "Memory tool" },
    { MenuItem::RESET, "Reset (F5)" },
    { MenuItem::ABOUT, "About" },
    { MenuItem::RESUME, "Resume" },
    { MenuItem::QUIT, "Quit (F10)" }
  };
  CPoint button_space = CPoint(0, 30);
  CRect button_rect(CPoint(20, 10), 100, 20);

  for(auto& b : buttons) {
    m_buttons.push_back(CapriceGuiViewButton(b.first, new CButton(button_rect, this, b.second)));
    button_rect += button_space;
  }
}

CapriceMenu::~CapriceMenu(void) {
}

bool CapriceMenu::HandleMessage(CMessage* pMessage)
{
  bool bHandled = false;

  MenuItem selected(MenuItem::NONE);
  if (pMessage)
  {
    switch (pMessage->MessageType())
    {
    case CMessage::CTRL_SINGLELCLICK:
      std::cout << "Received SINGLECLICK" << std::endl;
      if (pMessage->Destination() == this) {
        bHandled = true;
        for(auto& b : m_buttons) {
          if (pMessage->Source() == b.GetButton()) {
            selected = b.GetItem();
            break;
          }
        }
      }
      break;
    case CMessage::KEYBOARD_KEYDOWN:
      std::cout << "Received KEYDOWN " << m_bVisible << " - " << (pMessage->Destination() == this) << std::endl;
      if (m_bVisible && pMessage->Destination() == this) {
        std::cout << "For me" << std::endl;
        CKeyboardMessage* pKeyboardMessage = dynamic_cast<CKeyboardMessage*>(pMessage);
        if (pKeyboardMessage) {
          switch (pKeyboardMessage->Key) {
            case SDLK_UP:
              bHandled = true;
              //FocusNext(FocusDirection::BACKWARD);
              CFrame::FocusNext(EFocusDirection::BACKWARD);
              break;
            case SDLK_DOWN:
              bHandled = true;
              //FocusNext(FocusDirection::FORWARD);
              CFrame::FocusNext(EFocusDirection::FORWARD);
              break;
            case SDLK_TAB:
              bHandled = true;
              if(pKeyboardMessage->Modifiers & KMOD_SHIFT) {
                //FocusNext(FocusDirection::BACKWARD);
                CFrame::FocusNext(EFocusDirection::BACKWARD);
              } else {
                //FocusNext(FocusDirection::FORWARD);
                CFrame::FocusNext(EFocusDirection::FORWARD);
              }
              break;
            case SDLK_RETURN:
              bHandled = true;
              for(auto &b : m_buttons) {
                if(b.GetButton()->HasFocus()) {
                  selected = b.GetItem();
                }
              }
              break;
            case SDLK_o:
              bHandled = true;
              selected = MenuItem::OPTIONS;
              break;
            case SDLK_l:
              bHandled = true;
              selected = MenuItem::LOAD_SAVE;
              break;
            case SDLK_m:
              bHandled = true;
              selected = MenuItem::MEMORY_TOOL;
              break;
            case SDLK_F5:
              bHandled = true;
              selected = MenuItem::RESET;
              break;
            case SDLK_a:
              bHandled = true;
              selected = MenuItem::ABOUT;
              break;
            case SDLK_q:
            case SDLK_F10:
              bHandled = true;
              selected = MenuItem::QUIT;
              break;
            case SDLK_r:
            case SDLK_ESCAPE: 
              bHandled = true;
              selected = MenuItem::RESUME;
              break;
            default:
              break;
          }
        }      
      }
      break;
    default:
      bHandled = CFrame::HandleMessage(pMessage);
      break;
    }
  }
  SDL_Surface *pScreenSurface = m_pParentView->GetSurface();
  switch (selected) {
    case MenuItem::OPTIONS:
      {
        CapriceOptions* pOptionsBox = new CapriceOptions(CRect(CPoint(pScreenSurface->w /2 - 165, pScreenSurface->h /2 - 127), 330, 260), m_pParentView, nullptr);
        pOptionsBox->SetModal(true);
        break;
      }
    case MenuItem::LOAD_SAVE:
      {
        CapriceLoadSave* pLoadSaveBox = new CapriceLoadSave(CRect(CPoint(pScreenSurface->w /2 - 165, pScreenSurface->h /2 - 127), 330, 260), m_pParentView, nullptr);
        pLoadSaveBox->SetModal(true);
        break;
      }
    case MenuItem::MEMORY_TOOL:
      {
        CapriceMemoryTool* pMemoryTool = new CapriceMemoryTool(CRect(CPoint(pScreenSurface->w /2 - 165, pScreenSurface->h /2 - 140), 330, 270), m_pParentView, nullptr);
        pMemoryTool->SetModal(true);
        break;
      }
    case MenuItem::RESET:
      {
        emulator_reset(false);
        // Exit gui
        CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_EXIT, nullptr, this));
        break;
      }
    case MenuItem::ABOUT:
      {
        CapriceAbout* pAboutBox = new CapriceAbout(CRect(CPoint(pScreenSurface->w /2 - 87, pScreenSurface->h /2 - 120), 174, 240), m_pParentView, nullptr);
        pAboutBox->SetModal(true);
        break;
      }
    case MenuItem::RESUME:
      {
        // Exit gui
        CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_EXIT, nullptr, this));
        break;
      }
    case MenuItem::QUIT:
      {
        // atexit() takes care of all the cleanup
        exit (0);
        break;
      }
    case MenuItem::NONE:
      break;
  }

  return bHandled;
}

} // namespace wGui
