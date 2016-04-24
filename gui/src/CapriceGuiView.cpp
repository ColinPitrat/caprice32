#include "CapriceGuiView.h"
#include "CapriceAbout.h"
#include "CapriceOptions.h"
#include "CapriceLoadSave.h"
#include "CapriceMemoryTool.h"
#include "wg_messagebox.h"
#include <algorithm>

// Make caprice32 functions available here.
extern void emulator_reset(bool);


void CapriceGuiView::FocusNext(FocusDirection direction)
{
  CButton *to_unfocus = nullptr;
  auto loop_body = [&to_unfocus](const CapriceGuiViewButton& b) {
    if(to_unfocus != nullptr) {
      to_unfocus->SetHasFocus(false);
      b.GetButton()->SetHasFocus(true);
      to_unfocus = nullptr;
    } else if(b.GetButton()->HasFocus()) {
      to_unfocus = b.GetButton();
    }
  };

  if(direction == FocusDirection::BACKWARD)
    std::for_each(m_buttons.rbegin(), m_buttons.rend(), loop_body);
  else
    std::for_each(m_buttons.begin(), m_buttons.end(), loop_body);
}


bool CapriceGuiView::HandleMessage(CMessage* pMessage)
{
  bool bHandled = false;

  MenuItem selected(MenuItem::NONE);
  if (pMessage)
  {
    switch (pMessage->MessageType())
    {
    case CMessage::CTRL_SINGLELCLICK:
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
      if (m_bVisible && pMessage->Destination() == this) {
        CKeyboardMessage* pKeyboardMessage = dynamic_cast<CKeyboardMessage*>(pMessage);
        if (pKeyboardMessage) {
          switch (pKeyboardMessage->Key) {
            case SDLK_UP:
              bHandled = true;
              FocusNext(FocusDirection::BACKWARD);
              break;
            case SDLK_DOWN:
              bHandled = true;
              FocusNext(FocusDirection::FORWARD);
              break;
            case SDLK_TAB:
              bHandled = true;
              if(pKeyboardMessage->Modifiers & KMOD_SHIFT) {
                FocusNext(FocusDirection::BACKWARD);
              } else {
                FocusNext(FocusDirection::FORWARD);
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
      bHandled = CView::HandleMessage(pMessage);
      break;
    }
  }
  switch (selected) {
    case MenuItem::OPTIONS:
      {
        wGui::CapriceOptions* pOptionsBox = new wGui::CapriceOptions(CRect(CPoint(m_pScreenSurface->w /2 - 165, m_pScreenSurface->h /2 - 127), 330, 260), this, 0);
        pOptionsBox->SetModal(true);
        break;
      }
    case MenuItem::LOAD_SAVE:
      {
        wGui::CapriceLoadSave* pLoadSaveBox = new wGui::CapriceLoadSave(CRect(CPoint(m_pScreenSurface->w /2 - 165, m_pScreenSurface->h /2 - 127), 330, 260), this, 0);
        pLoadSaveBox->SetModal(true);
        break;
      }
    case MenuItem::MEMORY_TOOL:
      {
        wGui::CapriceMemoryTool* pMemoryTool = new wGui::CapriceMemoryTool(CRect(CPoint(m_pScreenSurface->w /2 - 165, m_pScreenSurface->h /2 - 140), 330, 270), this, 0);
        pMemoryTool->SetModal(true);
        break;
      }
    case MenuItem::RESET:
      {
        emulator_reset(false);
        // Exit gui
        CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_EXIT, 0, this));
        break;
      }
    case MenuItem::ABOUT:
      {
        wGui::CapriceAbout* pAboutBox = new wGui::CapriceAbout(CRect(CPoint(m_pScreenSurface->w /2 - 87, m_pScreenSurface->h /2 - 120), 174, 240), this, 0);
        pAboutBox->SetModal(true);
        break;
      }
    case MenuItem::RESUME:
      {
        // Exit gui
        CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_EXIT, 0, this));
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


CapriceGuiView::CapriceGuiView(SDL_Surface* surface, SDL_Surface* backSurface, const CRect& WindowRect) : CView(surface, backSurface, WindowRect)
{
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_SINGLELCLICK);
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_MESSAGEBOXRETURN);
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::KEYBOARD_KEYDOWN);
  // judb Apparently this needs to be done the first time:
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
  CRect button_rect(CPoint(m_pScreenSurface->w / 2 - 50, m_pScreenSurface->h / 2 - 90), 100, 20);

  for(auto& b : buttons) {
    m_buttons.push_back(CapriceGuiViewButton(b.first, new CButton(button_rect, this, b.second)));
    button_rect += button_space;
  }
  m_buttons.front().GetButton()->SetHasFocus(true);
}


// judb Show the Caprice32 emulation display, and our CCaGuiView (CView) on top of it.
// The only CView object in Caprice32 is the first window you see when you activate the gui.
void CapriceGuiView::PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const
{
  if (m_bVisible)
  {
    // Draw a 'fake' window so the controls (buttons) don't swim inside the emulation display.
    {
      SDL_Rect SourceRect = CRect(m_WindowRect.SizeRect()).SDLRect();
      SDL_Rect DestRect = CRect(m_WindowRect + Offset).SDLRect();

      CPoint FakeWindowAnchorPoint = CPoint(m_pScreenSurface->w / 2 - 70, 160);
      CRect FakeWindowRect = CRect(FakeWindowAnchorPoint, 140, 240);

      CPainter Painter(m_pBackSurface, CPainter::PAINT_REPLACE);
      Painter.DrawRect(FakeWindowRect, true, DEFAULT_BACKGROUND_COLOR, DEFAULT_BACKGROUND_COLOR);
      Painter.Draw3DRaisedRect(FakeWindowRect,DEFAULT_BACKGROUND_COLOR);
      SDL_BlitSurface(m_pBackSurface, &SourceRect, &ScreenSurface, &DestRect);
    }

    // Draw all buttons
    CPoint NewOffset = m_ClientRect.TopLeft() + m_WindowRect.TopLeft() + Offset;
    for (std::list<CWindow*>::const_iterator iter = m_ChildWindows.begin(); iter != m_ChildWindows.end(); ++iter)
    {
      if (*iter)
      {
        (*iter)->PaintToSurface(ScreenSurface, FloatingSurface, NewOffset);
      }
    }
  }
}
