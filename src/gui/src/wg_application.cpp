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
#include "wutil_debug.h"
#include "video.h"
#include <iostream>
#include <fstream>
#include <string>
#include "cap32.h"

// CPC emulation properties, defined in cap32.h:
extern t_CPC CPC;
// Video plugin, defined in video.h:
extern video_plugin* vid_plugin;

namespace wGui
{

// Static members
CApplication* CApplication::m_pInstance = nullptr;


void CApplication::HandleSDLEvent(SDL_Event event)
{
	// this will turn an SDL event into a wGui message
	switch (event.type)
	{
  case SDL_WINDOWEVENT_RESIZED:
    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
      CMessageServer::Instance().QueueMessage(new TPointMessage(
            CMessage::CTRL_RESIZE, nullptr, this, CPoint(event.window.data1, event.window.data2)));
    }
		break;
  case SDL_TEXTINPUT:
    std::cout << "Text input event: " << event.text.text << std::endl;
    CMessageServer::Instance().QueueMessage(new CTextInputMessage(
          CMessage::TEXTINPUT, CApplication::Instance()->GetKeyFocus(), this,
          std::string(event.text.text)));
    break;
	case SDL_KEYDOWN:
		CMessageServer::Instance().QueueMessage(new CKeyboardMessage(
			CMessage::KEYBOARD_KEYDOWN, CApplication::Instance()->GetKeyFocus(), this,
			event.key.keysym.scancode, static_cast<SDL_Keymod>(event.key.keysym.mod),
			event.key.keysym.sym));
		break;
	case SDL_KEYUP:
		CMessageServer::Instance().QueueMessage(new CKeyboardMessage(
			CMessage::KEYBOARD_KEYUP, CApplication::Instance()->GetKeyFocus(), this,
			event.key.keysym.scancode, static_cast<SDL_Keymod>(event.key.keysym.mod),
			event.key.keysym.sym));
		break;
	case SDL_MOUSEBUTTONDOWN:
		CMessageServer::Instance().QueueMessage(new CMouseMessage(
			CMessage::MOUSE_BUTTONDOWN, CApplication::Instance()->GetMouseFocus(), this,
			CPoint(static_cast<int>((event.button.x-vid_plugin->x_offset)*vid_plugin->x_scale), static_cast<int>((event.button.y-vid_plugin->y_offset)*vid_plugin->y_scale)), CPoint(),
			CMouseMessage::TranslateSDLButton(event.button.button)));
		break;
	case SDL_MOUSEBUTTONUP:
		CMessageServer::Instance().QueueMessage(new CMouseMessage(
			CMessage::MOUSE_BUTTONUP, CApplication::Instance()->GetMouseFocus(), this,
			CPoint(static_cast<int>((event.button.x-vid_plugin->x_offset)*vid_plugin->x_scale), static_cast<int>((event.button.y-vid_plugin->y_offset)*vid_plugin->y_scale)), CPoint(),
			CMouseMessage::TranslateSDLButton(event.button.button)));
		break;
  case SDL_MOUSEWHEEL:
    {
      // TODO(SDL2): Verify that mouse wheel works as expected
      unsigned int wheeldirection = CMouseMessage::NONE;
      if (event.wheel.x > 0 || event.wheel.y > 0) {
        wheeldirection = CMouseMessage::WHEELUP;
      } else {
        wheeldirection = CMouseMessage::WHEELDOWN;
      }
      CMessageServer::Instance().QueueMessage(new CMouseMessage(
            CMessage::MOUSE_BUTTONDOWN, CApplication::Instance()->GetMouseFocus(), this,
            CPoint(static_cast<int>((event.button.x-vid_plugin->x_offset)*vid_plugin->x_scale), static_cast<int>((event.button.y-vid_plugin->y_offset)*vid_plugin->y_scale)), CPoint(),
            wheeldirection));
      break;
    }
	case SDL_MOUSEMOTION:
		CMessageServer::Instance().QueueMessage(new CMouseMessage(
			CMessage::MOUSE_MOVE, CApplication::Instance()->GetMouseFocus(), this,
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
        CMessageServer::Instance().QueueMessage(new CKeyboardMessage(
              CMessage::KEYBOARD_KEYDOWN, CApplication::Instance()->GetKeyFocus(), this,
              0, KMOD_NONE, key));
        CMessageServer::Instance().QueueMessage(new CKeyboardMessage(
              CMessage::KEYBOARD_KEYUP, CApplication::Instance()->GetKeyFocus(), this,
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
        CMessageServer::Instance().QueueMessage(new CKeyboardMessage(
              type, CApplication::Instance()->GetKeyFocus(), this,
              0, mod, key));
      }
      break;
    }

	case SDL_QUIT:
//		CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_EXIT, nullptr, this));
    exit(0);
		break;
	default:
		CMessageServer::Instance().QueueMessage(new CSDLMessage(CMessage::SDL, nullptr, this, event));
		break;
	}
}


CApplication::CApplication(std::string sFontFileName, bool bHandleExceptionsInternally) :
	m_sFontFileName(std::move(sFontFileName)),
	m_iExitCode(EXIT_FAILURE),
	m_bRunning(false),
	m_bInited(false),
	m_pKeyFocusWindow(nullptr),
	m_pMouseFocusWindow(nullptr),
	m_pDefaultFontEngine(nullptr),
	m_iBitsPerPixel(DEFAULT_BPP),
	m_DefaultBackgroundColor(DEFAULT_BACKGROUND_COLOR),
	m_DefaultForegroundColor(DEFAULT_FOREGROUND_COLOR),
	m_DefaultSelectionColor(DEFAULT_BACKGROUND_COLOR),
	m_bHandleExceptionsInternally(bHandleExceptionsInternally),
	m_bResourcePoolEnabled(true),
	m_pCurrentCursorResourceHandle(nullptr),
	m_pSystemDefaultCursor(nullptr)
{
	if (m_pInstance)
	{
		throw(Wg_Ex_App("An instance of the CApplication already exists.", "CApplication::CApplication"));
	}

	m_pInstance = this;

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
	m_AppLog.AddLogEntry("wGui Application starting", APP_LOG_INFO);

	atexit(SDL_Quit);
}


CApplication::~CApplication()
{
	if (m_pInstance == this)
	{
		m_pInstance = nullptr;
	}

	for(auto& fontEngine : m_FontEngines)
	{
		delete fontEngine.second;
		fontEngine.second = nullptr;
	}

	m_AppLog.AddLogEntry("wGui Application closing", APP_LOG_INFO);
	m_AppLog.WriteToFile("wGui.log", false, "wGui Application Log (version " + std::string(VERSION_STRING) + ")\nSeverity Levels : [1] Critical, [3] Error, [5] Warning, [8] Info");

  CMessageServer::Instance().PurgeQueuedMessages();
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
        CMessageServer::Instance().QueueMessage(new CMessage(CMessage::CTRL_LOSINGKEYFOCUS, m_pKeyFocusWindow, this));
      }
      m_pKeyFocusWindow = pWindow;
      CMessageServer::Instance().QueueMessage(new CMessage(CMessage::CTRL_GAININGKEYFOCUS, m_pKeyFocusWindow, this));
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
			CMessageServer::Instance().QueueMessage(new CMessage(CMessage::CTRL_LOSINGMOUSEFOCUS, m_pMouseFocusWindow, this));
		}
		m_pMouseFocusWindow = pWindow;
		CMessageServer::Instance().QueueMessage(new CMessage(CMessage::CTRL_GAININGMOUSEFOCUS, m_pMouseFocusWindow, this));
	}
}



void CApplication::Init()
{
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::APP_EXIT, CMessageServer::PRIORITY_LAST);
  // TODO(SDL2): Fix support of text input
	//SDL_EnableUNICODE(1);

    // judb removed references to wgui.conf; for caprice32 we may integrate these settings in cap32.cfg:
    m_pDefaultFontEngine = GetFontEngine(CPC.resources_path + "/vera_sans.ttf", 8); // default size was 10
    m_DefaultBackgroundColor = DEFAULT_BACKGROUND_COLOR;
	m_DefaultForegroundColor = DEFAULT_FOREGROUND_COLOR;
	m_DefaultSelectionColor  = DEFAULT_SELECTION_COLOR;
	m_bInited = true;
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
		CMessageServer::Instance().IgnoreAllNewMessages(false);
		CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_PAINT, nullptr, this));
		m_AppLog.AddLogEntry("wGui Application entering Exec loop", APP_LOG_INFO);
		while (m_bRunning)
		{
			while (SDL_PollEvent(&event))
			{
				HandleSDLEvent(event);
			}
			while (!CMessageServer::Instance().MessageAvailable())
			{
				while (SDL_PollEvent(&event))
				{
					HandleSDLEvent(event);
				}
				SDL_Delay(5);
			}
			CMessageServer::Instance().DeliverMessage();
		}
	}
	catch (Wg_Ex_Base& e)
	{
		m_AppLog.AddLogEntry("Exception (wGui) : " + e.std_what(), APP_LOG_CRITICAL);
		m_AppLog.AddLogEntry(std::string("SDL Last Error = ") + SDL_GetError(), APP_LOG_ERROR);
		if (!m_bHandleExceptionsInternally)
		{
			throw;
		}
		// We are supposed to handle all exceptions internally, but something hasn't been handled at this point, so we need to exit gracefully
		wUtil::Trace("Unhandled wGui exception : " + e.std_what());
	}
	catch (std::exception& e)
	{
		m_AppLog.AddLogEntry("Exception (std) : " + std::string(e.what()), APP_LOG_CRITICAL);
		m_AppLog.AddLogEntry(std::string("SDL Last Error = ") + SDL_GetError(), APP_LOG_ERROR);
		if (!m_bHandleExceptionsInternally)
		{
			throw;
		}
		// We are supposed to handle all exceptions internally, but something hasn't been handled at this point, so we need to exit gracefully
		wUtil::Trace("Unhandled std exception : " + std::string(e.what()));
	}
	catch (...)
	{
		m_AppLog.AddLogEntry("Exception (non std)", APP_LOG_CRITICAL);
		m_AppLog.AddLogEntry(std::string("SDL Last Error = ") + SDL_GetError(), APP_LOG_ERROR);
		if (!m_bHandleExceptionsInternally)
		{
			throw;
		}
		// We are supposed to handle all exceptions internally, but something hasn't been handled at this point, so we need to exit gracefully
		wUtil::Trace("Unhandled exception");
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
	wUtil::TraceIf(iResult == -1, "CApplication::ApplicationExit - Unable to push SDL user_event.");
	m_iExitCode = iExitCode;
	m_bRunning = false;
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
			m_AppLog.AddLogEntry("CApplication::GetFontEngine - Exception thrown while creating Font Engine : " + e.std_what(), APP_LOG_ERROR);
			if (!m_bHandleExceptionsInternally)
			{
				throw;
			}
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
		if( m_pCurrentCursorResourceHandle.get() )
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
