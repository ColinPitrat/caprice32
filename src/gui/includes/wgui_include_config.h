// wgui_include_config.h
//
// An intermediate header file for including the correct config header based on the OS
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


#ifndef _WGUI_INCLUDE_CONFIG_H_
#define _WGUI_INCLUDE_CONFIG_H_

#include <string>
#include <stdlib.h>

#ifdef WIN32
	// For WIN32 compiling using Microsoft Visual C++
	#include "wgui_win32_config.h"

	#ifndef DEBUG
		// Fixes the warnings from xtree in Release builds
		#pragma warning (disable : 4702)
	#endif  // DEBUG

	#include <stdlib.h>  // needed for the getenv() function
	// OS specific constants
	static const std::string GLOBAL_CONFIG_PATH = std::string(getenv("windir")).substr(0, 1) + ":\\program files\\common files\\wgui\\";
	static const std::string USER_CONFIG_PATH = std::string(getenv("windir")).substr(0, 1) + ":\\documents and settings\\" + std::string(getenv("USERNAME")) + "\\application data\\wgui\\";
#else  // WIN32
	// For builds using the GNU tools
	#include "wgui_config.h"

	// OS specific constants
	static const std::string GLOBAL_CONFIG_PATH = "/etc/wgui/";
	//~ is a bash shortcut, we need to grab the environment variable
	static const std::string USER_CONFIG_PATH = std::string(getenv("HOME")) + "/.wgui/";
#endif  // WIN32

#endif  // _WGUI_INCLUDE_CONFIG_H_
