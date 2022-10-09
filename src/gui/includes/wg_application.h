// wg_application.h
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


#ifndef _WG_APPLICATION_H_
#define _WG_APPLICATION_H_

#include "SDL.h"
#include <list>
#include <string>
#include <map>
#include <memory>
#include "wg_window.h"
#include "wg_message_server.h"
#include "wg_message_client.h"
#include "wg_fontengine.h"
#include "wg_resources.h"
#include "std_ex.h"


namespace wGui
{

const int DEFAULT_BPP = 32;

//! A class for encapsulating an application

//! CApplication is a singleton (only one instance of it should ever exist)
//! This handles all global level stuff like initializing SDL and creating a MessageServer
//! CApplication also takes care of turning SDL events into wGui messages

class CApplication : public CMessageClient
{
public:
	//! Standard constructor
	//! \param pWindow The SDL window this application runs in.
	//! \param sFontFileName The font to use for controls, defaults to Arial
	explicit CApplication(SDL_Window* pWindow, std::string sFontFileName = "resource/vera_sans.ttf");

	//! Standard destructor
	~CApplication() override;

  //! Register the view for this application
	//! \param The view to register
  virtual void RegisterView(CView* pView);

	//! Gets the default font file name
	//! \return The default font
	virtual const std::string& GetDefaultFontFileName() const { return m_sFontFileName; }

	//! Gets the current exit code of the application
	//! \return The exit code of the app
	virtual int ExitCode() const { return m_iExitCode; }

	//! Indicates if the application is currently "running" or not
	//! \return true while the application is in it's message loop
	virtual bool IsRunning() const { return m_bRunning; }

	//! Indicates if the CApplication object has been properly initialized
	//! \return true if Init() has been called
	virtual bool IsInitialized() const { return m_bInited; }

	//! This is for setting/getting the window that has the current keyboard focus
	//! Any KEYBOARD messages will have this window as their destination
	//! \param pWindow A pointer to the window that should get focus
	virtual void SetKeyFocus(CWindow* pWindow);

	//! Gets the current keyboard focus for the application
	//! \return A pointer to the window with keyboard focus
	virtual CWindow* GetKeyFocus() const { return m_pKeyFocusWindow; }

	//! This is for setting/getting the window that has the current mouse focus
	//! Any subsequent MOUSE messages will have this window as their destination
	//! \param pWindow A pointer to the window that should get focus ( grab )
	virtual void SetMouseFocus(CWindow* pWindow);

	//! Gets the current mouse focus for the application
	//! \return A pointer to the window with mouse focus ( grab )
	virtual CWindow* GetMouseFocus() const { return m_pMouseFocusWindow; }

	//! Init() must be called before Exec() or ProcessEvent()
	//! Takes care of initializing SDL and other important stuff
	virtual void Init();

	//! A single step of the primary message loop. Useful to continue to update things outside of the application.
	virtual void Update();

	//! Processing one event. Useful when events can also affect things outside of the application.
	virtual bool ProcessEvent(SDL_Event& e);

	//! The primary message loop
	virtual void Exec();

	//! This is called just before the application exits
	//! \param iExitCode The exit code to return, defaults to EXIT_SUCCESS
	virtual void ApplicationExit(int iExitCode = EXIT_SUCCESS);

	//! Creates a font engine
	//! A seperate font engine is created for each font file and font size
	//! \param sFontFileName The font to use
	//! \param iFontSize The size (in points) of the font, defaults to 12
	//! \return A pointer to the font engine, 0 if the font engine can't be loaded
	virtual CFontEngine* GetFontEngine(std::string sFontFileName, unsigned char iFontSize = 8);

	//! Sets the default font engine
	//! If a default font engine is not set, a valid font engine must be passed to any controls that display text
	virtual void SetDefaultFontEngine(CFontEngine* pFontEngine) { m_pDefaultFontEngine = pFontEngine; }

	//! Gets the default font engine
	//! \return A pointer to the default font engine
	virtual CFontEngine* GetDefaultFontEngine() const { return m_pDefaultFontEngine; }

	//! Gets the color depth (in bits per pixel) of the app
	//! \return The color depth of the view
	virtual int GetBitsPerPixel() const { return DEFAULT_BPP; }

	//! Gets the default background color
	//! \return Default background color
	virtual CRGBColor GetDefaultBackgroundColor() const { return m_DefaultBackgroundColor; }

	//! Gets the default foreground color
	//! \return Default foreground color
	virtual CRGBColor GetDefaultForegroundColor() const { return m_DefaultForegroundColor; }

	//! Gets the default selection color
	//! \return Default selection color
	virtual CRGBColor GetDefaultSelectionColor() const { return m_DefaultSelectionColor; }

	//! Adds a resource handle to the resource pool if the pool is enabled
	//! Once a resource is added, it cannot be removed.
	//! \param ResourceHandle The resource handle to copy into the pool
	//! \return false if the resource pool is disabled
	virtual bool AddToResourcePool(CResourceHandle& ResourceHandle);

	//! Changes the mouse cursor
	//! \param pCursorResourceHandle A pointer to the cursor resource handle, if no cursor is specified, the cursor will revert to the system default
	virtual void SetMouseCursor(CCursorResourceHandle* pCursorResourceHandle = nullptr);

  CMessageServer* MessageServer() const { return m_pMessageServer.get(); }

	// CMessageClient overrides
	//! CApplication will handle the APP_EXIT message, and will close the application on it's receipt
	//! \param pMessage A pointer the the message to handle
	bool HandleMessage(CMessage* pMessage) override;

protected:

	//! For internal use only
	//! \internal converts SDL events into wGui messages
	//! \param event An SDL Event structure
	virtual bool HandleSDLEvent(SDL_Event event);

  SDL_Window* m_pSDLWindow;
  bool m_bInMainView = true;  //!< Whether the GUI is displayed through Caprice main view (i.e going through video_plugin)
  int m_iScale = 1;   //!< The scaling of the UI (necessary on high-DPI screens)
  CView* m_pMainView;
  bool m_Focused = true;
  mutable std::unique_ptr<CMessageServer> m_pMessageServer;
	std::string m_sFontFileName;  //!< The font to use for all controls
	int m_iExitCode;  //!< The exit code to be returned when the app exits
	bool m_bRunning;  //!< Indicates if the app is currently spinning in the message loop
	bool m_bInited;  //!< true if Init() has been called
	CWindow* m_pKeyFocusWindow;  //!< A pointer to the window with keyboard focus
	CWindow* m_pMouseFocusWindow;  //!< A pointer to the window with mouse focus

	using t_FontEngineMapKey = std::pair<std::string, unsigned char>;  //!< A typedef of font name and size pairs
	using t_FontEngineMap = std::map<t_FontEngineMapKey, CFontEngine*>;  //!< A typedef of a map of font engine pointers
	t_FontEngineMap m_FontEngines;  //!< A map of font engine pointers
	CFontEngine* m_pDefaultFontEngine;  //!< A pointer to the default font engine

	CRGBColor m_DefaultBackgroundColor; //!< Default background color
	CRGBColor m_DefaultForegroundColor; //!< Default foreground color
	CRGBColor m_DefaultSelectionColor; //!< Default selection color

	std::list<CResourceHandle> m_ResourceHandlePool;  //!< The resource handle pool which keeps commonly used resources alive
	std::unique_ptr<CCursorResourceHandle> m_pCurrentCursorResourceHandle;  //!< An autopointer to the handle for the current mouse cursor
	SDL_Cursor* m_pSystemDefaultCursor;  //!< A pointer to the default system cursor
};


}

#endif // _WG_APPLICATION_H_
