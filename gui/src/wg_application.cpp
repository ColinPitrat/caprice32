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

#include "wgui_include_config.h"
#include "wg_application.h"
#include "wg_error.h"
#include "wutil_debug.h"
#include "video.h"
#include <iostream>
#include <fstream>
#include "cap32.h"

// CPC emulation properties, defined in cap32.h:
extern t_CPC CPC;
// Video plugin, defined in video.h:
extern video_plugin* vid_plugin;

namespace wGui
{

// Static members
CApplication* CApplication::m_pInstance = 0;


void CApplication::HandleSDLEvent(SDL_Event Event)
{
	// this will turn an SDL event into a wGui message
	switch (Event.type)
	{
	case SDL_VIDEORESIZE:
		CMessageServer::Instance().QueueMessage(new TPointMessage(
			CMessage::CTRL_RESIZE, 0, this, CPoint(Event.resize.w, Event.resize.h)));
		break;
	case SDL_KEYDOWN:
		CMessageServer::Instance().QueueMessage(new CKeyboardMessage(
			CMessage::KEYBOARD_KEYDOWN, CApplication::Instance()->GetKeyFocus(), this,
			Event.key.keysym.scancode, Event.key.keysym.mod,
			Event.key.keysym.sym, Event.key.keysym.unicode));
		break;
	case SDL_KEYUP:
		CMessageServer::Instance().QueueMessage(new CKeyboardMessage(
			CMessage::KEYBOARD_KEYUP, CApplication::Instance()->GetKeyFocus(), this,
			Event.key.keysym.scancode, Event.key.keysym.mod,
			Event.key.keysym.sym, Event.key.keysym.unicode));
		break;
	case SDL_MOUSEBUTTONDOWN:
		CMessageServer::Instance().QueueMessage(new CMouseMessage(
			CMessage::MOUSE_BUTTONDOWN, CApplication::Instance()->GetMouseFocus(), this,
			CPoint((int)((Event.button.x-vid_plugin->x_offset)*vid_plugin->x_scale),(int)((Event.button.y-vid_plugin->y_offset)*vid_plugin->y_scale)), CPoint(),
			CMouseMessage::TranslateSDLButton(Event.button.button)));
		break;
	case SDL_MOUSEBUTTONUP:
		CMessageServer::Instance().QueueMessage(new CMouseMessage(
			CMessage::MOUSE_BUTTONUP, CApplication::Instance()->GetMouseFocus(), this,
			CPoint((int)((Event.button.x-vid_plugin->x_offset)*vid_plugin->x_scale),(int)((Event.button.y-vid_plugin->y_offset)*vid_plugin->y_scale)), CPoint(),
			CMouseMessage::TranslateSDLButton(Event.button.button)));
		break;
	case SDL_MOUSEMOTION:
		CMessageServer::Instance().QueueMessage(new CMouseMessage(
			CMessage::MOUSE_MOVE, CApplication::Instance()->GetMouseFocus(), this,
			CPoint((int)((Event.button.x-vid_plugin->x_offset)*vid_plugin->x_scale),(int)((Event.button.y-vid_plugin->y_offset)*vid_plugin->y_scale)), CPoint(),
			CMouseMessage::TranslateSDLButtonState(Event.motion.state)));
		break;
	case SDL_QUIT:
//		CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_EXIT, 0, this));
        exit(0);
		break;
	default:
		CMessageServer::Instance().QueueMessage(new CSDLMessage(CMessage::SDL, 0, this, Event));
		break;
	}
}


CApplication::CApplication(int argc, char** argv, std::string sFontFileName, bool bHandleExceptionsInternally) :
	m_argc(argc),
	m_argv(argv),
	m_sFontFileName(sFontFileName),
	m_iExitCode(EXIT_FAILURE),
	m_bRunning(false),
	m_bInited(false),
	m_pKeyFocusWindow(0),
	m_pMouseFocusWindow(0),
	m_pDefaultFontEngine(0),
	m_iBitsPerPixel(DEFAULT_BPP),
	m_DefaultBackgroundColor(DEFAULT_BACKGROUND_COLOR),
	m_DefaultForegroundColor(DEFAULT_FOREGROUND_COLOR),
	m_DefaultSelectionColor(DEFAULT_BACKGROUND_COLOR),
	m_bHandleExceptionsInternally(bHandleExceptionsInternally),
	m_bResourcePoolEnabled(true),
	m_pCurrentCursorResourceHandle(0),
	m_pSystemDefaultCursor(0)
{
	if (m_pInstance)
	{
		throw(Wg_Ex_App("CApplication::CApplication : An instance of the CApplication already exists."));
	}

	m_pInstance = this;

	// judb
        //if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE ) == -1)
	//{
	//	throw(Wg_Ex_SDL(std::string("CApplication::CApplication : Could not initialize SDL: ") + SDL_GetError()));
	//}
	//Setting the keyboard repeat rate using the SDL Default rates
	//if(SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL) == -1)
	//{
	//	throw(Wg_Ex_SDL(std::string("CApplication::Capplication : Error setting SDL keyboard repeat rate.")));
	//}

	m_pSystemDefaultCursor = SDL_GetCursor();
	m_AppLog.AddLogEntry("wGui Application starting", APP_LOG_INFO);

	atexit(SDL_Quit);
}


CApplication::~CApplication(void)
{
	if (m_pInstance == this)
	{
		m_pInstance = 0;
	}

	for(std::map<std::pair<std::string, unsigned char>, CFontEngine*>::iterator iter = m_FontEngines.begin(); iter != m_FontEngines.end(); ++iter)
	{
		delete iter->second;
		iter->second = 0;
	}

	m_AppLog.AddLogEntry("wGui Application closing", APP_LOG_INFO);
	m_AppLog.WriteToFile("wGui.log", false, "wGui Application Log (version " + std::string(VERSION) + ")\nSeverity Levels : [1] Critical, [3] Error, [5] Warning, [8] Info");
}


void CApplication::SetKeyFocus(CWindow* pWindow)
{
	if (m_pKeyFocusWindow != pWindow)
	{
		// notify the window that's losing focus to repaint itself
		if (m_pKeyFocusWindow)
		{
			CMessageServer::Instance().QueueMessage(new CMessage(CMessage::CTRL_LOSINGKEYFOCUS, m_pKeyFocusWindow, this));
		}
		m_pKeyFocusWindow = pWindow;
		CMessageServer::Instance().QueueMessage(new CMessage(CMessage::CTRL_GAININGKEYFOCUS, m_pKeyFocusWindow, this));
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



void CApplication::Init(void)
{
	CMessageServer::Instance().RegisterMessageClient(this, CMessage::APP_EXIT, CMessageServer::PRIORITY_LAST);
	SDL_EnableUNICODE(1);

    // judb removed references to wgui.conf; for caprice32 we may integrate these settings in cap32.cfg:
    m_pDefaultFontEngine = GetFontEngine(std::string(CPC.resources_path) + "/font.ttf", 8); // default size was 10
    m_DefaultBackgroundColor = DEFAULT_BACKGROUND_COLOR;
	m_DefaultForegroundColor = DEFAULT_FOREGROUND_COLOR;
	m_DefaultSelectionColor  = DEFAULT_SELECTION_COLOR;
	m_bInited = true;
}


void CApplication::Exec(void)
{
	try
	{
		if (!m_bInited)
		{
			throw(Wg_Ex_App("CApplication::Exec : Application Init() was not called!"));
		}

		m_bRunning = true;
		SDL_Event event;
		CMessageServer::Instance().IgnoreAllNewMessages(false);
		CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_PAINT, 0, this));
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
			try
			{
				CMessageServer::Instance().DeliverMessage();
			}
			catch (Wg_Ex_Base& e)
			{
				m_AppLog.AddLogEntry("Exception (wGui) : " + e.std_what(), APP_LOG_CRITICAL);
				if (!m_bHandleExceptionsInternally)
				{
					throw;
				}
				// Since we're handling exceptions internally, and it's one of our own exceptions, we're just going to
				// send something to the debug output and then continue processing the message queue
				wUtil::Trace("wGui exception while delivering message : " + e.std_what());
			}
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
	user_event.user.data1=0;
	user_event.user.data2=0;
	int iResult = SDL_PushEvent(&user_event);
	wUtil::TraceIf(iResult == -1, "CApplication::ApplicationExit - Unable to push SDL user_event.");
	m_iExitCode = iExitCode;
	m_bRunning = false;
}


CFontEngine* CApplication::GetFontEngine(std::string sFontFileName, unsigned char iFontSize)
{
	// First search to see if the requested font engine already exists
	t_FontEngineMap::iterator iterFontEngine = m_FontEngines.find(std::make_pair(sFontFileName, iFontSize));
	CFontEngine* pFontEngine = 0;

	if (iterFontEngine == m_FontEngines.end())
	{
        // Requested font engine doesn't exist, so create one and add it to the map
		try
		{
			if (sFontFileName.find_first_of("\\/") != std::string::npos)
			{
				// if the Font File name that was passed in includes a path, just check there
				std::ifstream FileTest(sFontFileName.c_str());
				bool bFileExists = FileTest.is_open();
				FileTest.close();
				if (bFileExists)
				{
					pFontEngine = new CFontEngine(sFontFileName, iFontSize);
				}
			}
			else
			{
				// otherwise check the Font Path from the global config
				std::list<std::string> FontPaths = stdex::DetokenizeString(m_GlobalConfig.GetStringEntry("FONTPATH").second, ";");
				for(std::list<std::string>::iterator iter = FontPaths.begin(); iter != FontPaths.end(); ++iter)
				{
					std::string sFullPath = *iter;
					if (!iter->empty()  && (*iter)[iter->length()] != '\\' && (*iter)[iter->length()] != '/')
					{
						sFullPath += "/";
					}
					sFullPath += sFontFileName;
					std::ifstream FileTest(sFullPath.c_str());
					bool bFileExists = FileTest.is_open();
					FileTest.close();
					if (bFileExists)
					{
						pFontEngine = new CFontEngine(sFullPath, iFontSize);
					}
				}
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
	if (bEnable == false)
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
		std::auto_ptr<CCursorResourceHandle> pNewCursorResourceHandle(new CCursorResourceHandle(*pCursorResourceHandle));
		m_pCurrentCursorResourceHandle = pNewCursorResourceHandle;
		SDL_SetCursor(m_pCurrentCursorResourceHandle->Cursor());
	}
	else
	{
		if( m_pCurrentCursorResourceHandle.get() )
		{
			#ifdef MSVC6  // VC6's auto pointers are really broken
				delete m_pCurrentCursorResourceHandle.release();
				m_pCurrentCursorResourceHandle = std::auto_ptr<CCursorResourceHandle>(0);
			#else
				m_pCurrentCursorResourceHandle.reset(0);
			#endif // MSVC6
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
