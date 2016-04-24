// std_ex.h
//
// Extensions to the std library
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


#ifndef _STD_EX_H_
#define _STD_EX_H_

#include <string>
#include <list>
#include <limits>
#include <stdexcept>

namespace stdex
{

//! Converts an integer to it's string representation
//! \param iValue The integer to convert to a string
//! \return A std::string representing the value
std::string itoa(const int iValue);

//! Converts a long integer to it's string representation
//! \param lValue The integer to convert to a string
//! \return A std::string representing the value
std::string ltoa(const long lValue);

//! Converts a double to it's string representation
//! \param fValue The double to convert to a string
//! \return A std::string representing the value
std::string ftoa(const float fValue);

//! Converts a double to it's string representation
//! \param dValue The double to convert to a string
//! \return A std::string representing the value
std::string dtoa(const double dValue);

//! Converts a string to it's int representation
//! \param sValue the string to convert
//! \return integer
int atoi(const std::string& sValue);

//! Converts a string to it's long integer representation
//! \param sValue the string to convert
//! \return long integer
long int atol(const std::string& sValue);

//! Converts a string to it's float representation
//! \param sValue the string to convert
//! \return float
float atof(const std::string& sValue);

//! Converts a string to it's double representation
//! \param sValue the string to convert
//! \return double
double atod(const std::string& sValue);

//! Trim the whitespace (spaces and tabs) from the beginning and end of a string
//! \param sString The string to be trimmed
//! \return the trimmed string
std::string TrimString(const std::string& sString);

int MaxInt(int x, int y) ;

int MinInt(int x, int y) ;


//! Detokenize a string given a list of delimiters
//! \param sString The string to be tokenized
//! \param sDelimiters A string of delimiter characters
//! \return A list of string tokens
std::list<std::string> DetokenizeString(const std::string& sString, const std::string& sDelimiters);

//! Do a static cast with limits checking
//! This will throw a std::out_of_range exception if the value is outside the bounds of the type it's being casted to
#ifdef WIN32
#pragma warning (push)
#pragma warning (disable : 4018)  // temporarily disable signed/unsigned comparison warning
#endif  // WIN32
template<typename TDest, typename TSrc>
TDest safe_static_cast(const TSrc& Value)
{
	return static_cast<TDest>(Value);
}
#ifdef WIN32
#pragma warning (default : 4018)  // reenable signed/unsigned comparison warning
#pragma warning (pop)
#endif  // WIN32

}

#endif  // _STD_EX_H_
