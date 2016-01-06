// wutil_config_store.cpp
//
// CConfigStore class implementation
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


#include "wgui_include_config.h"
#include "wutil_config_store.h"
#include "std_ex.h"
#include "wutil_debug.h"
#include <fstream>


namespace wUtil
{

std::pair<bool, std::string> CConfigStore::GetStringEntry(std::string sKey) const
{
	bool bSuccess = false;
	std::string sValue = "";
	t_SettingsMap::const_iterator iter = m_SettingsMap.find(sKey);
	if (iter != m_SettingsMap.end())
	{
		bSuccess = true;
		sValue = iter->second;
	}

	return std::make_pair(bSuccess, sValue);
}


std::pair<bool, long int> CConfigStore::GetLongIntEntry(std::string sKey) const
{
	bool bSuccess = false;
	long int lValue = 0;
	t_SettingsMap::const_iterator iter = m_SettingsMap.find(sKey);
	if (iter != m_SettingsMap.end())
	{
		bSuccess = true;
		lValue = stdex::atol(iter->second);
	}

	return std::make_pair(bSuccess, lValue);
}


std::pair<bool, double> CConfigStore::GetDoubleEntry(std::string sKey) const
{
	bool bSuccess = false;
	double dValue = 0.0;
	t_SettingsMap::const_iterator iter = m_SettingsMap.find(sKey);
	if (iter != m_SettingsMap.end())
	{
		bSuccess = true;
		dValue = stdex::atod(iter->second);
	}

	return std::make_pair(bSuccess, dValue);
}


void CConfigStore::SetStringEntry(std::string sKey, std::string sValue)
{
	m_SettingsMap[sKey] = sValue;
}


void CConfigStore::SetLongIntEntry(std::string sKey, long int lValue)
{
	m_SettingsMap[sKey] = stdex::ltoa(lValue);
}


void CConfigStore::SetDoubleEntry(std::string sKey, double dValue)
{
	m_SettingsMap[sKey] = stdex::dtoa(dValue);
}


void CConfigStore::RemoveEntry(std::string sKey)
{
	m_SettingsMap.erase(sKey);
}


bool CConfigStore::EntryExists(std::string sKey) const
{
	return (m_SettingsMap.find(sKey) != m_SettingsMap.end());
}


void CConfigStore::StoreToFile(std::string sFilename) const
{
	std::ofstream File;

	File.open(sFilename.c_str(), std::ios::out | std::ios::trunc);
	if (File.is_open())
	{
		for (t_SettingsMap::const_iterator iter = m_SettingsMap.begin(); iter != m_SettingsMap.end(); ++iter)
		{
			File << iter->first << " = " << iter->second << std::endl;
		}
		File.close();
	}
}


bool CConfigStore::ReadFromFile(std::string sFilename)
{
	std::ifstream File;
	std::string sBuffer = "";
	std::string sKey = "";
	std::string sValue = "";
	bool bSuccess = false;

	if (! sFilename.empty())
	{
		File.open(sFilename.c_str());
		if (File.is_open())
		{
			while (! File.eof())
			{
				std::getline(File, sBuffer);
				if (sBuffer[0] != '#')
				{
					std::string::size_type splitPoint = sBuffer.find_first_of("=");
					sKey = stdex::TrimString(sBuffer.substr(0, splitPoint));
					sValue = stdex::TrimString(sBuffer.substr(splitPoint + 1));
					if (! sKey.empty())
					{
						m_SettingsMap[sKey] = sValue;
					}
				}
			}
			File.close();
			bSuccess = true;
		}
	}

	return bSuccess;
}


void CConfigStore::Clear(void)
{
	m_SettingsMap.clear();
}

}

