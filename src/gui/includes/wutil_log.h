// wutil_log.h
//
// CLog interface - Provides basic logging functionality
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


#ifndef _WUTIL_LOG_H_
#define _WUTIL_LOG_H_

#include <string>
#include <time.h>
#include <list>

namespace wUtil
{

//! A class for logging messages

//! CLog Provides basic logging functionality

class CLog
{
public:
	//! A log entry struct
	struct SLogEntry
	{
		//! The constructor
		SLogEntry(time_t TimeStamp, unsigned int iSeverity, std::string sMessage) :
			m_TimeStamp(TimeStamp), m_iSeverity(iSeverity), m_sMessage(std::move(sMessage)) { }
		time_t m_TimeStamp;  //!< The time the log entry was created
		unsigned int m_iSeverity;  //!< The severity of the log entry
		std::string m_sMessage;  //!< The actual text of the log entry
	};

	typedef std::list<SLogEntry> t_LogList;  //!< A typedef for a list of log entries

	//! Add a log entry
	//! \param sMessage A text string with the log message
	//! \param iSeverity An arbitrary severity indicator, where lower numbers are considered more severe (1 is the most severe)
	//! \param TimeStamp The time that the log entry corresponds to
	void AddLogEntry(const std::string& sMessage, unsigned int iSeverity, time_t TimeStamp)
		{ m_LogList.push_back(SLogEntry(TimeStamp, iSeverity, sMessage)); }

	//! Add a log entry with the timestamp set to the current time
	//! \param sMessage A text string with the log message
	//! \param iSeverity An arbitrary severity indicator, where lower numbers are considered more severe (1 is the most severe), defaults to 5
	void AddLogEntry(const std::string& sMessage, unsigned int iSeverity = 5)
		{ AddLogEntry(sMessage, iSeverity, time(nullptr)); }

	//! Get a list of the log entries
	//! \param iSeverity If this is zero, all log entries will be returned, otherwise it will only return log entries with the specified severity (defaults to 0)
	//! \return A list of the log entries
	t_LogList GetLogEntries(unsigned int iSeverity = 0) const;

	//! Get a list of the log entries that fit a severity range
	//! \param iHighSeverity The high end of the severity range (lower number)
	//! \param iLowSeverity The low end of the severity range (higher number)
	//! \return A list of the log entries
	t_LogList GetLogEntries(unsigned int iHighSeverity, unsigned int iLowSeverity) const;

	//! Write the log to a file
	//! \param sFilename The file to write the log to
	//! \param bAppend Indicates if the log should be appended, or overwrite the file (deaults to true)
	//! \param sLogHeader A string that will be output to the beginning of the log file (defaults to "")
	void WriteToFile(const std::string& sFilename, bool bAppend = true, const std::string& sLogHeader = "") const;

	//! Clear all log entries
	void Clear()
		{ m_LogList.clear(); }

protected:
	t_LogList m_LogList;  //!< The list of log entries
};

};


#endif  // _WUTIL_LOG_H_

