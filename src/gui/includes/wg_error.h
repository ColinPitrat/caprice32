// wg_error.h
//
// wGui error classes
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


// This is for all the exception classes that are used in wGui

#ifndef _WG_ERROR_H_
#define _WG_ERROR_H_

#include <exception>
#include <string>


namespace wGui
{

//! All wGui exception classes are derived from here
class Wg_Ex_Base : public std::exception
{
public:
	//! Standard constructor
	//! \param sWhat A string for more information on what caused the exception
	Wg_Ex_Base(std::string sWhat, std::string sWhere) : m_sWhat(std::move(sWhat)), m_sWhere(std::move(sWhere)) { }

	//! Standard Destructor
	~Wg_Ex_Base() throw() override = default;

	//! Gets a text description of the exception
	//! \return A string describing what caused the exception
	const char* what() const throw() override { return m_sWhat.c_str(); }

	//! Gets a text description of where the exception happened
	//! \return A string describing where the exception was raised
	virtual const char* where() const throw() { return m_sWhere.c_str(); }

	//! Gets a text description of the exception
	//! \return A std::string reference describing what caused the exception
	virtual const std::string& std_what() const throw() { return m_sWhat; }

private:
	std::string m_sWhat;
	std::string m_sWhere;
};


//! Exceptions caused by SDL errors
class Wg_Ex_SDL : public Wg_Ex_Base
{
public:
	//! Standard constructor
	//! \param sWhat A string for more information on what caused the exception
	Wg_Ex_SDL(const std::string& sWhat, const std::string& sWhere = "") : Wg_Ex_Base(sWhat, sWhere) { }
};


//! Exceptions caused by FreeType errors
class Wg_Ex_FreeType : public Wg_Ex_Base
{
public:
	//! Standard constructor
	//! \param sWhat A string for more information on what caused the exception
	Wg_Ex_FreeType(const std::string& sWhat, const std::string& sWhere = "") : Wg_Ex_Base(sWhat, sWhere) { }
};


//! General wGui errors
class Wg_Ex_App : public Wg_Ex_Base
{
public:
	//! Standard constructor
	//! \param sWhat A string for more information on what caused the exception
	Wg_Ex_App(const std::string& sWhat, const std::string& sWhere = "") : Wg_Ex_Base(sWhat, sWhere) { }
};


//! Exceptions caused by out-of-range type errors
class Wg_Ex_Range : public Wg_Ex_Base
{
public:
	//! Standard constructor
	//! \param sWhat A string for more information on what caused the exception
	Wg_Ex_Range(const std::string& sWhat, const std::string& sWhere = "") : Wg_Ex_Base(sWhat, sWhere) { }
};

}

#endif // _WG_ERROR_H_

