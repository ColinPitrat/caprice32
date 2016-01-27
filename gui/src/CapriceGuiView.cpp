#include "CapriceGuiView.h"
#include "CapriceAbout.h"
#include "CapriceOptions.h"
#include "CapriceLoadSave.h"
#include "CapriceMemoryTool.h"
#include "wg_messagebox.h"

// Make caprice32 functions available here.
extern void emulator_reset(bool);

bool CapriceGuiView::HandleMessage(CMessage* pMessage)
{
  bool bHandled = false;

  if (pMessage)
  {
    switch (pMessage->MessageType())
    {
    case CMessage::CTRL_SINGLELCLICK:
      if (pMessage->Destination() == this)
      {
        bHandled = true;
        const wGui::CMessageClient* pSource = pMessage->Source();
        if (pSource == m_pBtnOptions) {
          wGui::CapriceOptions* pOptionsBox = new wGui::CapriceOptions(CRect(CPoint(m_pScreenSurface->w /2 - 165, m_pScreenSurface->h /2 - 127), 330, 260), this, 0);
          pOptionsBox->SetModal(true);
          break;
        }
        if (pSource == m_pBtnLoadSave) {
          wGui::CapriceLoadSave* pLoadSaveBox = new wGui::CapriceLoadSave(CRect(CPoint(m_pScreenSurface->w /2 - 165, m_pScreenSurface->h /2 - 127), 330, 260), this, 0);
          pLoadSaveBox->SetModal(true);
          break;
        }
        if (pSource == m_pBtnMemoryTool) {
          wGui::CapriceMemoryTool* pMemoryTool = new wGui::CapriceMemoryTool(CRect(CPoint(m_pScreenSurface->w /2 - 165, m_pScreenSurface->h /2 - 140), 330, 270), this, 0);
          pMemoryTool->SetModal(true);
          break;
        }
        if (pSource == m_pBtnReset) {
          emulator_reset(false);
          // Exit gui
          CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_EXIT, 
          0, this));
          break;
        }
        if (pSource == m_pBtnAbout) {
          wGui::CapriceAbout* pAboutBox = new wGui::CapriceAbout(CRect(CPoint(m_pScreenSurface->w /2 - 87, m_pScreenSurface->h /2 - 120), 174, 240), this, 0);
          pAboutBox->SetModal(true);
          break;
        }
        if (pSource == m_pBtnResume) {
          // Exit gui, see also handling of SDLK_ESCAPE below.
          CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_EXIT, 0, this));
          break;
        }
        if (pSource == m_pBtnQuit) {
          // atexit() takes care of all the cleanup
          exit (0);
          break;
        }
      }
      break;
        case CMessage::KEYBOARD_KEYDOWN:
            if (m_bVisible && pMessage->Destination() == this) {
            CKeyboardMessage* pKeyboardMessage = dynamic_cast<CKeyboardMessage*>(pMessage);
        if (pKeyboardMessage) {
          if (pKeyboardMessage->Key == SDLK_ESCAPE) {
                        // Exit gui, see handling of m_pBtmResume above.
                CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_EXIT, 0, this));
              bHandled = true;
                    }
                }      
            }
            break;
        //case wGui::CMessage::CTRL_MESSAGEBOXRETURN:
        //  {
        //  bHandled = false;
        //      break;
        //  }
    default:
      bHandled = CView::HandleMessage(pMessage);
      break;
    }
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

  m_pBtnOptions    = new CButton(CRect(CPoint(m_pScreenSurface->w / 2 - 50, m_pScreenSurface->h / 2 - 90), 100, 20), this, "Options");
  m_pBtnLoadSave   = new CButton(CRect(CPoint(m_pScreenSurface->w / 2 - 50, m_pScreenSurface->h / 2 - 60), 100, 20), this, "Load / Save");
  m_pBtnMemoryTool = new CButton(CRect(CPoint(m_pScreenSurface->w / 2 - 50, m_pScreenSurface->h / 2 - 30), 100, 20), this, "Memory tool");
  m_pBtnReset      = new CButton(CRect(CPoint(m_pScreenSurface->w / 2 - 50, m_pScreenSurface->h / 2),      100, 20), this, "Reset (F5)");
  m_pBtnAbout      = new CButton(CRect(CPoint(m_pScreenSurface->w / 2 - 50, m_pScreenSurface->h / 2 + 30), 100, 20), this, "About");
  m_pBtnResume     = new CButton(CRect(CPoint(m_pScreenSurface->w / 2 - 50, m_pScreenSurface->h / 2 + 60), 100, 20), this, "Resume");
  m_pBtnQuit       = new CButton(CRect(CPoint(m_pScreenSurface->w / 2 - 50, m_pScreenSurface->h / 2 + 90), 100, 20), this, "Quit (F10)");
}


// judb Show the Caprice32 emulation display, and our CCaGuiView (CView) on top of it.
// The only CView object in Caprice32 is the first window you see when you activate the gui.
void CapriceGuiView::PaintToSurface(SDL_Surface& ScreenSurface, SDL_Surface& FloatingSurface, const CPoint& Offset) const
{
  if (m_bVisible)
  {
    SDL_Rect SourceRect = CRect(m_WindowRect.SizeRect()).SDLRect();
    SDL_Rect DestRect = CRect(m_WindowRect + Offset).SDLRect();
  
        CPoint FakeWindowAnchorPoint = CPoint(m_pScreenSurface->w / 2 - 80, 200);
        CRect FakeWindowRect = CRect(FakeWindowAnchorPoint, 160, 160);

        // Draw a 'fake' window so the controls (buttons) don't swim inside the emulation display.
    //CPainter Painter(m_pBackSurface, CPainter::PAINT_REPLACE);
//    CPainter Painter(m_pScreenSurface, CPainter::PAINT_REPLACE);
//    Painter.DrawRect(FakeWindowRect, true, DEFAULT_BACKGROUND_COLOR, DEFAULT_BACKGROUND_COLOR);
        // Draw shadow effect.
//    Painter.Draw3DRaisedRect(FakeWindowRect,DEFAULT_BACKGROUND_COLOR);
    // Copy everything to the visible screen:
    SDL_BlitSurface(m_pBackSurface, &SourceRect, &ScreenSurface, &DestRect);
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
