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
#include "wutil_config_store.h"
#include "wutil_log.h"
#include "std_ex.h"


namespace
{
const int DEFAULT_BPP = 32;
}


namespace wGui
{

//! The various severity levels used in the application log
enum EAppLogSeverity
{
	APP_LOG_CRITICAL = 1, //!< Very, very bad stuff
	APP_LOG_ERROR    = 3, //!< Something went wrong and needs to be fixed
	APP_LOG_WARNING  = 5, //!< Something suspicious is up, and should be looked at
	APP_LOG_INFO     = 8  //!< Informational, doesn't indicate any problems
};


//! A class for encapsulating an application

//! CApplication is a singleton (only one instance of it should ever exist)
//! This handles all global level stuff like initializing SDL and creating a MessageServer
//! CApplication also takes care of turning SDL events into wGui messages

class CApplication : public CMessageClient
{
public:
	//! Standard constructor
	//! \param argc The argument count from the command line
	//! \param argv The argument array from the command line
	//! \param sFontFileName The font to use for controls, defaults to Arial
	//! \param bHandleExceptionsInternally If this is true, wGui will handle most exceptions itself, if false, all exceptions will be returned to the user (defaults to true)
	CApplication(int argc, char** argv, std::string sFontFileName = "resource/vera_sans.ttf", bool bHandleExceptionsInternally = true);

	//! Standard destructor
	virtual ~CApplication(void);

	//! Gets the single instance of the CApplication object
	//! \return A pointer to the one instance of the Application
	static CApplication* Instance(void) { return m_pInstance; }

	//! Gets the default font file name
	//! \return The default font
	virtual const std::string& GetDefaultFontFileName(void) const { return m_sFontFileName; }

	//! Gets the current exit code of the application
	//! \return The exit code of the app
	virtual int ExitCode(void) const { return m_iExitCode; }

	//! Indicates if the application is currently "running" or not
	//! \return true while the application is in it's message loop
	virtual bool IsRunning(void) const { return m_bRunning; }

	//! Indicates if the application will be attempting to handle it's own exceptions (set in the CApplication constructor)
	//! \return true if wGui should handle it's own exceptions
	virtual bool HandleExceptions(void) const { return m_bHandleExceptionsInternally; }

	//! Indicates if the CApplication object has been properly initialized
	//! \return true if Init() has been called
	virtual bool IsInitialized(void) const { return m_bInited; }

	//! This is for setting/getting the window that has the current keyboard focus
	//! Any KEYBOARD messages will have this window as their destination
	//! \param pWindow A pointer to the window that should get focus
	virtual void SetKeyFocus(CWindow* pWindow);

	//! Gets the current keyboard focus for the application
	//! \return A pointer to the window with keyboard focus
	virtual CWindow* GetKeyFocus(void) const { return m_pKeyFocusWindow; }

	//! This is for setting/getting the window that has the current mouse focus
	//! Any subsequent MOUSE messages will have this window as their destination
	//! \param pWindow A pointer to the window that should get focus ( grab )
	virtual void SetMouseFocus(CWindow* pWindow);

	//! Gets the current mouse focus for the application
	//! \return A pointer to the window with mouse focus ( grab )
	virtual CWindow* GetMouseFocus(void) const { return m_pMouseFocusWindow; }

	//! Init() must be called before Exec()
	//! Takes care of initializing SDL and other important stuff
	virtual void Init(void);

	//! The primary message loop
	virtual void Exec(void);

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
	virtual CFontEngine* GetDefaultFontEngine(void) const { return m_pDefaultFontEngine; }

	//! Gets the color depth (in bits per pixel) of the app
	//! \return The color depth of the view
	virtual int GetBitsPerPixel(void) const { return m_iBitsPerPixel; }

	//! Gets the default background color
	//! \return Default background color
	virtual CRGBColor GetDefaultBackgroundColor(void) const { return m_DefaultBackgroundColor; }

	//! Gets the default foreground color
	//! \return Default foreground color
	virtual CRGBColor GetDefaultForegroundColor(void) const { return m_DefaultForegroundColor; }

	//! Gets the default selection color
	//! \return Default selection color
	virtual CRGBColor GetDefaultSelectionColor(void) const { return m_DefaultSelectionColor; }

	//! The resource pool is used to keep wGui provided resources around even when there are no other handles begin kept by the user
	//! The internal wGui resources are automatically added to the pool if this is enabled when they are created
	//! The pool is enabled by default
	//! \param bEnable If set to false, it will clear any current resource handles from the pool, and will disable the pool
	virtual void EnableResourcePool(bool bEnable);

	//! Adds a resource handle to the resource pool if the pool is enabled
	//! Once a resource is added, the only way to remove it, is to disabled the resource pool (via EnableResourcePool() ) which will empty the pool of all resources
	//! \param ResourceHandle The resource handle to copy into the pool
	//! \return false if the resource pool is disabled
	virtual bool AddToResourcePool(CResourceHandle& ResourceHandle);

	//! Changes the mouse cursor
	//! \param pCursorResourceHandle A pointer to the cursor resource handle, if no cursor is specified, the cursor will revert to the system default
	virtual void SetMouseCursor(CCursorResourceHandle* pCursorResourceHandle = 0);

	//! Changes the mouse position
	//! When this is called a new SDL_MOUSEMOTION event is triggered
	//! \param Point the position where the mouse pointer should be warped
	virtual void SetMousePosition( const CPoint& Point )
		{ SDL_WarpMouse(stdex::safe_static_cast<Uint16>(Point.XPos()), stdex::safe_static_cast<Uint16>(Point.YPos())); }

	//! Changes the visibility of the mouse
	//! \param bVisible if the mouse has to be drawn or not
	virtual void SetMouseVisibility( bool bVisible ) { SDL_ShowCursor(bVisible); }

	//! Returns the Global Configuration, which is read in from the wGui.conf file
	//! This Global config is read-only.  Any changes should be made to the wGui.conf file outside of the app
	//! wGui will search directories for the wGui.conf file in the following order, and will include the settings from each:
	//! - 1) &lt;system settings&gt; (for *nix this is probably /etc/wGui/, for windows systems this is c:\\documents and settings\\default user\\application data\\wGui)
	//! - 2) ~/ (home directory, for Windows NT builds, this is c:\\documents and settings\\&lt;username&gt;\\application data\\wGui)
	//! - 3) Current Directory
	//! .
	//! So any settings found in the current directory will override the settings in the global directory
	//! \return A const reference to the Global Configration
	virtual const wUtil::CConfigStore& GetGlobalConfig(void) { return m_GlobalConfig; }

	//!Returns the application log, which is output to wGui.log on application exit
	//! \return A reference to the application log, which gets any wGui log messages
	virtual wUtil::CLog& GetApplicationLog(void) { return m_AppLog; }


	// CMessageClient overrides
	//! CApplication will handle the APP_EXIT message, and will close the application on it's receipt
	//! \param pMessage A pointer the the message to handle
	virtual bool HandleMessage(CMessage* pMessage);


protected:

	//! For internal use only
	//! \internal converts SDL events into wGui messages
	//! \param event An SDL Event structure
	virtual void HandleSDLEvent(SDL_Event event);

	static CApplication* m_pInstance;  //!< A pointer to the one valid instance of the application
	int m_argc;  //!< The argument count from the command line
	char** m_argv;  //!< The argument array from the command line
	std::string m_sFontFileName;  //!< The font to use for all controls
	int m_iExitCode;  //!< The exit code to be returned when the app exits
	bool m_bRunning;  //!< Indicates if the app is currently spinning in the message loop
	bool m_bInited;  //!< true if Init() has been called
	CWindow* m_pKeyFocusWindow;  //!< A pointer to the window with keyboard focus
	CWindow* m_pMouseFocusWindow;  //!< A pointer to the window with mouse focus

	typedef std::pair<std::string, unsigned char> t_FontEngineMapKey;  //!< A typedef of font name and size pairs
	typedef std::map<t_FontEngineMapKey, CFontEngine*> t_FontEngineMap;  //!< A typedef of a map of font engine pointers
	t_FontEngineMap m_FontEngines;  //!< A map of font engine pointers
	CFontEngine* m_pDefaultFontEngine;  //!< A pointer to the default font engine

	int m_iBitsPerPixel;  //!< The color depth the app will be using
	CRGBColor m_DefaultBackgroundColor; //!< Default background color
	CRGBColor m_DefaultForegroundColor; //!< Default foreground color
	CRGBColor m_DefaultSelectionColor; //!< Default selection color

	bool m_bHandleExceptionsInternally;  //!< If true, wGui will handle most exceptions itself
	bool m_bResourcePoolEnabled;  //!< If true, wGui provided resources will be added to the resource pool as they are created, defaults to true
	std::list<CResourceHandle> m_ResourceHandlePool;  //!< The resource handle pool which keeps commonly used resources alive
	std::unique_ptr<CCursorResourceHandle> m_pCurrentCursorResourceHandle;  //!< An autopointer to the handle for the current mouse cursor
	SDL_Cursor* m_pSystemDefaultCursor;  //!< A pointer to the default system cursor
	wUtil::CConfigStore m_GlobalConfig;  //!< A CConfigStore object that has the Global configuration
	wUtil::CLog m_AppLog;  //!< A log for wGui stuff
};


}

#endif // _WG_APPLICATION_H_
