// wutil_log.cpp
//
// CLog class implementation
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


#include "wutil_log.h"
#include <fstream>


namespace wUtil
{

CLog::t_LogList CLog::GetLogEntries(unsigned int iSeverity) const
{
	t_LogList OutLog;
  for (const auto &log : m_LogList)
	{
		if (iSeverity == 0 || log.m_iSeverity == iSeverity)
		{
			OutLog.push_back(log);
		}
	}
	return OutLog;
}


CLog::t_LogList CLog::GetLogEntries(unsigned int iHighSeverity, unsigned int iLowSeverity) const
{
	t_LogList OutLog;
  for (const auto &log : m_LogList)
	{
		if (log.m_iSeverity >= iHighSeverity || log.m_iSeverity <= iLowSeverity)
		{
			OutLog.push_back(log);
		}
	}
	return OutLog;
}


void CLog::WriteToFile(const std::string& sFilename, bool bAppend, const std::string& sLogHeader) const
{
	std::ofstream File;

	if (bAppend)
	{
		File.open(sFilename.c_str(), std::ios::app);
	}
	else
	{
		File.open(sFilename.c_str(), std::ios::out | std::ios::trunc);
	}
	if (File.is_open())
	{
		File << sLogHeader << std::endl;
    for (const auto &log : m_LogList)
		{
			std::string sTimeStamp(asctime(localtime(&log.m_TimeStamp)));
			File << sTimeStamp.substr(0, sTimeStamp.size() - 1) << " [ " << log.m_iSeverity << " ] : " << log.m_sMessage << std::endl;
		}
		File.close();
	}
}

}


