// std_ex.cpp
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

#include "std_ex.h"
#include <sstream>
#include <string>


namespace stdex
{

std::string itoa(const int iValue)
{
	std::ostringstream sOutStream;
	sOutStream << iValue;
	return sOutStream.str();
}


std::string ltoa(const long lValue)
{
	std::ostringstream sOutStream;
	sOutStream << lValue;
	return sOutStream.str();
}


std::string ftoa(const float fValue)
{
	std::ostringstream sOutStream;
	sOutStream << fValue;
	return sOutStream.str();
}


std::string dtoa(const double dValue)
{
	std::ostringstream sOutStream;
	sOutStream << dValue;
	return sOutStream.str();
}


int atoi(const std::string& sValue)
{
	int iResult = 0;
	std::stringstream sTranslation;
	sTranslation << sValue;
	sTranslation >> iResult;
	return iResult;
}


long atol(const std::string& sValue)
{
	long lResult = 0;
	std::stringstream sTranslation;
	sTranslation << sValue;
	sTranslation >> lResult;
	return lResult;
}


float atof(const std::string& sValue)
{
	float fResult = 0.0;
	std::stringstream sTranslation;
	sTranslation << sValue;
	sTranslation >> fResult;
	return fResult;
}


double atod(const std::string& sValue)
{
	double dResult = 0.0;
	std::stringstream sTranslation;
	sTranslation << sValue;
	sTranslation >> dResult;
	return dResult;
}

std::string TrimString(const std::string& sString)
{
	std::string::size_type start = sString.find_first_not_of(" \t");
	std::string::size_type end = sString.find_last_not_of(" \t");
	std::string sResult = "";
	if (start != std::string::npos)
	{
		sResult = sString.substr(start, end - start + 1);
	}

	return sResult;
}

int MaxInt(int x, int y) { return (x >= y) ? x : y; };

int MinInt(int x, int y) { return (x <= y) ? x : y; };


std::list<std::string> DetokenizeString(const std::string& sString, const std::string& sDelimiters)
{
	std::string sStringCopy(sString);
	std::list<std::string> Tokens;

	while (! sStringCopy.empty())
	{
		std::string::size_type DelimiterIndex = sStringCopy.find_first_of(sDelimiters);
		if (DelimiterIndex == std::string::npos)
		{
			Tokens.push_back(sStringCopy);
			sStringCopy = "";
		}
		else
		{
			Tokens.push_back(sStringCopy.substr(0, DelimiterIndex));
		}
		sStringCopy = sStringCopy.substr(DelimiterIndex + 1);
	}

	return Tokens;
}

}
