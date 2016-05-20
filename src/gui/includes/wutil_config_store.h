// wutil_config_store.h
//
// CConfigStore interface - allows configuration settings to be stored to disk in a text file
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


#ifndef _WUTIL_CONFIG_STORE_H_
#define _WUTIL_CONFIG_STORE_H_

#include <string>
#include <map>


namespace wUtil
{

//! A class for storing configuration settings

//! CConfigStore allows settings to be stored and read from common text files

class CConfigStore
{
public:
	//! Constructor
	CConfigStore(void) { }

	//! Destructor
	virtual ~CConfigStore(void) { }

	//! Read the specified setting as a string
	//! \param sKey A string that identifies the entry to be read
	//! \return A pair, where the first bool indicates if the setting was retrieved, and the second is string value of the specified setting
	std::pair<bool, std::string> GetStringEntry(std::string sKey) const;

	//! Read the specified setting as an integer
	//! \param sKey A string that identifies the entry to be read
	//! \return A pair, where the first bool indicates if the setting was retrieved, and the second is long integer value of the specified setting
	std::pair<bool, long int> GetLongIntEntry(std::string sKey) const;

	//! Read the specified setting as a double
	//! \param sKey A string that identifies the entry to be read
	//! \return A pair, where the first bool indicates if the setting was retrieved, and the second is double value of the specified setting
	std::pair<bool, double> GetDoubleEntry(std::string sKey) const;

	//! Store a setting as a string
	//! \param sKey A string that identifies the entry to be stored
	//! \param sValue The string value to be stored
	void SetStringEntry(std::string sKey, std::string sValue);

	//! Store a setting as a long integer
	//! \param sKey A string that identifies the entry to be stored
	//! \param lValue The long integer value to be stored
	void SetLongIntEntry(std::string sKey, long int lValue);

	//! Store a setting as a double
	//! \param sKey A string that identifies the entry to be stored
	//! \param dValue The double value to be stored
	void SetDoubleEntry(std::string sKey, double dValue);

	//! Remove a specified setting
	//! \param sKey A string that identifies the entry to be removed
	void RemoveEntry(std::string sKey);

	//! Test to see if a specified entry exists
	//! \param sKey A string that identifies the entry to be tested
	//! \return true if the entry exists, otherwise false
	bool EntryExists(std::string sKey) const;

	//! Save the settings to a file
	//! \param sFilename The filename to store the settings to
	void StoreToFile(std::string sFilename) const;

	//! Read the settings from a file
	//! This will not clear any current settings in the object, but will overwrite any values with the same key
	//! \param sFilename The filename to read the settings from
	//! \return true if the file existed and we were able to read from it
	bool ReadFromFile(std::string sFilename);

	//! Clear all settings from the object
	void Clear(void);


protected:
	typedef std::map<std::string, std::string> t_SettingsMap;  //!< A map type definition for key strings to value strings
	t_SettingsMap m_SettingsMap;  //!< The map of settings
};

};


#endif  // _WUTIL_CONFIG_STORE_H_

