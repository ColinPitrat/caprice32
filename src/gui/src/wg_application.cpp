// wg_application.cpp
//
// CApplication interface
//
//
// Copyright (c) 2002-2004 Rob Wiskow
// rob-dev@boxedchaos.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//

#include "wg_application.h"
#include "wg_error.h"
#include "wg_view.h"
#include "video.h"
#include <iostream>
#include <fstream>
#include <string>
#include "cap32.h"
#include "log.h"

// CPC emulation properties, defined in cap32.h:
extern t_CPC CPC;
// Video plugin, defined in video.h:
extern video_plugin* vid_plugin;

namespace wGui
{

bool CApplication::HandleSDLEvent(SDL_Event event)
{
  auto windowId = SDL_GetWindowID(m_pSDLWindow);
  bool forMe = m_Focused;
  switch (event.type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      if (event.key.windowID == windowId) forMe = true;
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      if (event.button.windowID == windowId) forMe = true;
      break;
    case SDL_TEXTEDITING:
      if (event.edit.windowID == windowId) forMe = true;
      break;
    case SDL_MOUSEMOTION:
      if (event.motion.windowID == windowId) forMe = true;
      break;
    case SDL_TEXTINPUT:
      if (event.text.windowID == windowId) forMe = true;
      break;
    case SDL_MOUSEWHEEL:
      if (event.wheel.windowID == windowId) forMe = true;
      break;
    case SDL_WINDOWEVENT:
      if (event.window.windowID == windowId) forMe = true;
      if (forMe && event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
        m_Focused = true;
        return true;
      }
      if (forMe && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        m_Focused = false;
        return true;
      }
      if (forMe && event.window.event == SDL_WINDOWEVENT_CLOSE) {
        MessageServer()->QueueMessage(new CMessage(CMessage::APP_EXIT, nullptr, this));
        return true;
      }
      break;
  }
  if (!forMe) return false;

	// this will turn an SDL event into a wGui message
	switch (event.type)
	{
  case SDL_WINDOWEVENT:
    MessageServer()->QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
    if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
        event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
      MessageServer()->QueueMessage(new TPointMessage(
            CMessage::CTRL_RESIZE, nullptr, this, CPoint(event.window.data1, event.window.data2)));
    }
		break;
  case SDL_TEXTINPUT:
    MessageServer()->QueueMessage(new CTextInputMessage(
          CMessage::TEXTINPUT, GetKeyFocus(), this,
          std::string(event.text.text)));
    break;
	case SDL_KEYDOWN:
		MessageServer()->QueueMessage(new CKeyboardMessage(
			CMessage::KEYBOARD_KEYDOWN, GetKeyFocus(), this,
			event.key.keysym.scancode, static_cast<SDL_Keymod>(event.key.keysym.mod),
			event.key.keysym.sym));
		break;
	case SDL_KEYUP:
		MessageServer()->QueueMessage(new CKeyboardMessage(
			CMessage::KEYBOARD_KEYUP, GetKeyFocus(), this,
			event.key.keysym.scancode, static_cast<SDL_Keymod>(event.key.keysym.mod),
			event.key.keysym.sym));
		break;
	case SDL_MOUSEBUTTONDOWN:
		MessageServer()->QueueMessage(new CMouseMessage(
			CMessage::MOUSE_BUTTONDOWN, GetMouseFocus(), this,
			CPoint(static_cast<int>((event.button.x-vid_plugin->x_offset)*vid_plugin->x_scale), static_cast<int>((event.button.y-vid_plugin->y_offset)*vid_plugin->y_scale)), CPoint(),
			CMouseMessage::TranslateSDLButton(event.button.button)));
		break;
	case SDL_MOUSEBUTTONUP:
		MessageServer()->QueueMessage(new CMouseMessage(
			CMessage::MOUSE_BUTTONUP, GetMouseFocus(), this,
			CPoint(static_cast<int>((event.button.x-vid_plugin->x_offset)*vid_plugin->x_scale), static_cast<int>((event.button.y-vid_plugin->y_offset)*vid_plugin->y_scale)), CPoint(),
			CMouseMessage::TranslateSDLButton(event.button.button)));
		break;
  case SDL_MOUSEWHEEL:
    {
      unsigned int wheeldirection = CMouseMessage::NONE;
      if (event.wheel.x > 0 || event.wheel.y > 0) {
        wheeldirection = CMouseMessage::WHEELUP;
      } else {
        wheeldirection = CMouseMessage::WHEELDOWN;
      }
      int x, y;
      SDL_GetMouseState(&x, &y);
      MessageServer()->QueueMessage(new CMouseMessage(
            CMessage::MOUSE_BUTTONDOWN, GetMouseFocus(), this,
            CPoint(static_cast<int>((x-vid_plugin->x_offset)*vid_plugin->x_scale), static_cast<int>((y-vid_plugin->y_offset)*vid_plugin->y_scale)), CPoint(),
            wheeldirection));
      break;
    }
	case SDL_MOUSEMOTION:
		MessageServer()->QueueMessage(new CMouseMessage(
			CMessage::MOUSE_MOVE, GetMouseFocus(), this,
			CPoint(static_cast<int>((event.button.x-vid_plugin->x_offset)*vid_plugin->x_scale), static_cast<int>((event.button.y-vid_plugin->y_offset)*vid_plugin->y_scale)), CPoint(),
			CMouseMessage::TranslateSDLButtonState(event.motion.state)));
		break;
  case SDL_JOYAXISMOTION:
    {
      SDL_Keycode key(SDLK_UNKNOWN);
      switch(event.jaxis.axis) {
        case 0:
        case 2:
          // TODO: validate with a joystick with non-binary axis
          // should we add some timing or consider only state changes ?
          if(event.jaxis.value < -JOYSTICK_AXIS_THRESHOLD) {
            key = SDLK_LEFT;
          } else if(event.jaxis.value > JOYSTICK_AXIS_THRESHOLD) {
            key = SDLK_RIGHT;
          }
          break;
        case 1:
        case 3:
          if(event.jaxis.value < -JOYSTICK_AXIS_THRESHOLD) {
            key = SDLK_UP;
          } else if(event.jaxis.value > JOYSTICK_AXIS_THRESHOLD) {
            key = SDLK_DOWN;
          }
          break;
      }
      if (key != SDLK_UNKNOWN) {
        MessageServer()->QueueMessage(new CKeyboardMessage(
              CMessage::KEYBOARD_KEYDOWN, GetKeyFocus(), this,
              0, KMOD_NONE, key));
        MessageServer()->QueueMessage(new CKeyboardMessage(
              CMessage::KEYBOARD_KEYUP, GetKeyFocus(), this,
              0, KMOD_NONE, key));
      }
      break;
    }
  case SDL_JOYBUTTONUP:
  case SDL_JOYBUTTONDOWN:
    {
      CMessage::EMessageType type = CMessage::KEYBOARD_KEYDOWN;
      if (event.type == SDL_JOYBUTTONUP) {
        type = CMessage::KEYBOARD_KEYUP;
      }
      SDL_Keycode key(SDLK_UNKNOWN);
      SDL_Keymod mod = KMOD_NONE;
      bool ignore_event = false;
      // TODO: arbitrary binding: validate with various joystick models
      switch (event.jbutton.button) {
        case 0:
          key = SDLK_RETURN;
          break;
        case 1:
        case 4:
          key = SDLK_TAB;
          break;
        case 2:
        case 5:
          key = SDLK_TAB;
          mod = KMOD_RSHIFT;
          break;
        case 3:
          key = SDLK_ESCAPE;
          break;
        default:
          ignore_event = true;
          break;
      }
      if (!ignore_event) {
        MessageServer()->QueueMessage(new CKeyboardMessage(
              type, GetKeyFocus(), this,
              0, mod, key));
      }
      break;
    }

	case SDL_QUIT:
//		MessageServer()->QueueMessage(new CMessage(CMessage::APP_EXIT, nullptr, this));
    exit(0);
		break;
	default:
		MessageServer()->QueueMessage(new CSDLMessage(CMessage::SDL, nullptr, this, event));
		break;
	}
  return true;
}


CApplication::CApplication(SDL_Window* pWindow, std::string sFontFileName) :
  CMessageClient(*this),
  m_pSDLWindow(pWindow),
  m_pMainView(nullptr),
	m_sFontFileName(std::move(sFontFileName)),
	m_iExitCode(EXIT_FAILURE),
	m_bRunning(false),
	m_bInited(false),
	m_pKeyFocusWindow(nullptr),
	m_pMouseFocusWindow(nullptr),
	m_pDefaultFontEngine(nullptr),
	m_DefaultBackgroundColor(DEFAULT_BACKGROUND_COLOR),
	m_DefaultForegroundColor(DEFAULT_FOREGROUND_COLOR),
	m_DefaultSelectionColor(DEFAULT_BACKGROUND_COLOR),
	m_bResourcePoolEnabled(true),
	m_pCurrentCursorResourceHandle(nullptr),
	m_pSystemDefaultCursor(nullptr)
{
  m_pMessageServer = std::make_unique<CMessageServer>();

	// judb
  //if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE ) == -1)
	//{
	//	throw(Wg_Ex_SDL(std::string("Could not initialize SDL: ") + SDL_GetError(), "CApplication::CApplication"));
	//}
	//Setting the keyboard repeat rate using the SDL Default rates
	//if(SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL) == -1)
	//{
	//	throw(Wg_Ex_SDL("Error setting SDL keyboard repeat rate.", "CApplication::Capplication"));
	//}

	m_pSystemDefaultCursor = SDL_GetCursor();

	atexit(SDL_Quit);
}


CApplication::~CApplication()
{
  m_pMessageServer = nullptr;

	for(auto& fontEngine : m_FontEngines)
	{
		delete fontEngine.second;
		fontEngine.second = nullptr;
	}
}


void CApplication::RegisterView(CView* pView)
{
  if (m_pMainView != nullptr)
  {
    throw(Wg_Ex_App("This application already has a view registered.", "CApplication::RegisterView"));
  }
  m_pMainView = pView;
}


void CApplication::SetKeyFocus(CWindow* pWindow)
{
	if (pWindow && m_pKeyFocusWindow != pWindow)
	{
    if (pWindow->IsVisible())
    {
      // notify the window that's losing focus to repaint itself
      if (m_pKeyFocusWindow)
      {
        MessageServer()->QueueMessage(new CMessage(CMessage::CTRL_LOSINGKEYFOCUS, m_pKeyFocusWindow, this));
      }
      m_pKeyFocusWindow = pWindow;
      MessageServer()->QueueMessage(new CMessage(CMessage::CTRL_GAININGKEYFOCUS, m_pKeyFocusWindow, this));
    }
    else
    {
      SetKeyFocus(pWindow->GetAncestor(CWindow::PARENT));
    }
	}
}


void CApplication::SetMouseFocus(CWindow* pWindow)
{
	if (m_pMouseFocusWindow != pWindow)
	{
		// notify the window that's losing focus to repaint itself
		if (m_pMouseFocusWindow)
		{
			MessageServer()->QueueMessage(new CMessage(CMessage::CTRL_LOSINGMOUSEFOCUS, m_pMouseFocusWindow, this));
		}
		m_pMouseFocusWindow = pWindow;
		MessageServer()->QueueMessage(new CMessage(CMessage::CTRL_GAININGMOUSEFOCUS, m_pMouseFocusWindow, this));
	}
}



void CApplication::Init()
{
	MessageServer()->RegisterMessageClient(this, CMessage::APP_EXIT, CMessageServer::PRIORITY_LAST);

	// judb removed references to wgui.conf; for caprice32 we may integrate these settings in cap32.cfg:
	m_pDefaultFontEngine = GetFontEngine(CPC.resources_path + "/vera_sans.ttf", 10); // default size was 10
	m_DefaultBackgroundColor = DEFAULT_BACKGROUND_COLOR;
	m_DefaultForegroundColor = DEFAULT_FOREGROUND_COLOR;
	m_DefaultSelectionColor  = DEFAULT_SELECTION_COLOR;
	m_bInited = true;
}


bool CApplication::ProcessEvent(SDL_Event& event)
{
  if (!m_bInited)
  {
    throw(Wg_Ex_App("Application Init() was not called!", "CApplication::Step"));
  }

  m_bRunning = true;
  MessageServer()->IgnoreAllNewMessages(false);
  MessageServer()->QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
  auto result = HandleSDLEvent(event);
  MessageServer()->DeliverMessage();
  return result;
}


void CApplication::Update()
{
  if (!m_bInited)
  {
    throw(Wg_Ex_App("Application Init() was not called!", "CApplication::Step"));
  }

  m_bRunning = true;
  MessageServer()->IgnoreAllNewMessages(false);
  MessageServer()->QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
  MessageServer()->DeliverMessage();
}


void CApplication::Exec()
{
	try
	{
		if (!m_bInited)
		{
			throw(Wg_Ex_App("Application Init() was not called!", "CApplication::Exec"));
		}

		m_bRunning = true;
		SDL_Event event;
		MessageServer()->IgnoreAllNewMessages(false);
		MessageServer()->QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
		while (m_bRunning)
		{
      do {
        while (SDL_PollEvent(&event)) {
          HandleSDLEvent(event);
        }
        SDL_Delay(5);
      } while (!MessageServer()->MessageAvailable());
			MessageServer()->DeliverMessage();
		}
	}
	catch (Wg_Ex_Base& e)
	{
		LOG_ERROR("Exception (wGui) : " << e.std_what() << " - SDL Last Error: " << SDL_GetError());
	}
	catch (std::exception& e)
	{
		LOG_ERROR("Exception (std) : " << std::string(e.what()) << " - SDL Last Error: " << SDL_GetError());
	}
	catch (...)
	{
		LOG_ERROR("Exception (non std) - SDL Last Error: " << SDL_GetError());
	}
}


void CApplication::ApplicationExit(int iExitCode)
{
	// push an event into the SDL queue so the SDLEventLoopThread can exit
	// the actual contents of the event are not a concern as it serves just to trigger the SDL_WaitEvent
	SDL_Event user_event;
	user_event.type=SDL_USEREVENT;
	user_event.user.code=0;
	user_event.user.data1=nullptr;
	user_event.user.data2=nullptr;
	int iResult = SDL_PushEvent(&user_event);
  if (iResult == -1) {
    LOG_ERROR("CApplication::ApplicationExit - Unable to push SDL user_event: " << SDL_GetError());
  }
	m_iExitCode = iExitCode;
	m_bRunning = false;
  if (m_pMainView) m_pMainView->Close();
}


CFontEngine* CApplication::GetFontEngine(std::string sFontFileName, unsigned char iFontSize)
{
	// First search to see if the requested font engine already exists
	auto iterFontEngine = m_FontEngines.find(std::make_pair(sFontFileName, iFontSize));
	CFontEngine* pFontEngine = nullptr;

	if (iterFontEngine == m_FontEngines.end())
	{
        // Requested font engine doesn't exist, so create one and add it to the map
		try
		{
      std::ifstream FileTest(sFontFileName.c_str());
      bool bFileExists = FileTest.is_open();
      FileTest.close();
      if (bFileExists)
      {
        pFontEngine = new CFontEngine(sFontFileName, iFontSize);
      }
      else
      {
        throw(Wg_Ex_App("File not found: " + sFontFileName, "CApplication::GetFontEngine"));
      }
			if (pFontEngine)
			{
				m_FontEngines.insert(std::make_pair(std::make_pair(sFontFileName, iFontSize), pFontEngine));
			}
		}
		catch (Wg_Ex_FreeType& e)
		{
			LOG_ERROR("CApplication::GetFontEngine - Exception thrown while creating Font Engine : " + e.std_what());
		}
	}
	else
	{
		pFontEngine = iterFontEngine->second;
	}

	return pFontEngine;
}


void CApplication::EnableResourcePool(bool bEnable)
{
	m_bResourcePoolEnabled = bEnable;
	if (!bEnable)
	{
		m_ResourceHandlePool.clear();
	}
}


bool CApplication::AddToResourcePool(CResourceHandle& ResourceHandle)
{
	bool bSuccess = false;

	if (m_bResourcePoolEnabled)
	{
		m_ResourceHandlePool.push_back(ResourceHandle);
		bSuccess = true;
	}

	return bSuccess;
}


void CApplication::SetMouseCursor(CCursorResourceHandle* pCursorResourceHandle)
{
	// The auto pointer is used to make sure that the cursor handle is valid until we're done using the cursor
	if (pCursorResourceHandle && pCursorResourceHandle != m_pCurrentCursorResourceHandle.get())
	{
		m_pCurrentCursorResourceHandle.reset(new CCursorResourceHandle(*pCursorResourceHandle));
		SDL_SetCursor(m_pCurrentCursorResourceHandle->Cursor());
	}
	else
	{
		if( m_pCurrentCursorResourceHandle )
		{
			m_pCurrentCursorResourceHandle.reset(nullptr);
			SDL_SetCursor(m_pSystemDefaultCursor);
		}
	}
}


bool CApplication::HandleMessage(CMessage* pMessage)
{
	bool bHandled = false;

	if (pMessage)
	{
		switch (pMessage->MessageType())
		{
		case CMessage::APP_EXIT :
			ApplicationExit();
			bHandled = true;
			break;
		default:
			break;
		}
	}

	return bHandled;
}

}
