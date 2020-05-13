// wutil_debug.h
//
// wUtil debug classes and code
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


// this is for useful debugging functions and classes

#ifndef _WUTIL_DEBUG_H_
#define _WUTIL_DEBUG_H_

#include <iostream>
#include <string>


namespace wUtil
{

#ifdef DEBUG

//! The trace function is for outputting informational strings to std::cerr
//! In non-debug builds, Trace will do nothing (it's #defined to nothing)
//! \param sMessage The message to output to std::cerr
inline void Trace(const std::string sMessage)
{
		std::cerr << "Trace: " << sMessage << std::endl;
}


//! TraceIf will output sMessage to std::cerr if bTest is true
//! In non-debug builds, TraceIf will do nothing (it's #defined to nothing)
//! \param bTest A boolean that indicates if the message should be output
//! \param sMessage The message to output to std::cerr
inline void TraceIf(bool bTest, const std::string sMessage)
{
	if (bTest)
	{
		std::cerr << "Trace: " << sMessage << std::endl;
	}
}


//! DEBUG_ONLY is a macro for marking code as only being included in DEBUG builds
#define DEBUG_ONLY(s) s

#else // DEBUG

inline void Trace(const std::string /*sMessage*/) { }
inline void TraceIf(bool /*bTest*/, const std::string /*sMessage*/) { }

#define DEBUG_ONLY(s)

#endif // DEBUG

}

#endif // _WUTIL_DEBUG_H_

