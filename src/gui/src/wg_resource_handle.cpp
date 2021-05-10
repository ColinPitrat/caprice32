// wg_resource_handle.cpp
//
// Resource Handle implementation
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

#include "wg_resource_handle.h"
#include "wg_error.h"
#include "wutil_debug.h"
#include "wg_application.h"
#include <map>
#include <string>

namespace wGui
{

std::map<TResourceId, unsigned int> CResourceHandle::m_RefCountMap;
std::map<TResourceId, SDL_Surface*> CBitmapResourceHandle::m_BitmapMap;
std::map<TResourceId, std::string> CStringResourceHandle::m_StringMap;
std::map<TResourceId, SDL_Cursor*> CCursorResourceHandle::m_SDLCursorMap;
TResourceId CResourceHandle::m_NextUnusedResourceId = 10000;


CResourceHandle::CResourceHandle(TResourceId resId) :
	m_ResourceId(resId)
{
	if (m_ResourceId == AUTO_CREATE_RESOURCE_ID)
	{
		while (m_RefCountMap.find(m_NextUnusedResourceId) != m_RefCountMap.end())
		{
			++m_NextUnusedResourceId;
		}
		m_ResourceId = m_NextUnusedResourceId;
		++m_NextUnusedResourceId;
	}
	if (m_RefCountMap.find(m_ResourceId) == m_RefCountMap.end() || m_RefCountMap[m_ResourceId] == 0)
	{
		m_RefCountMap[m_ResourceId] = 0;
	}
	++m_RefCountMap[m_ResourceId];
}


CResourceHandle::CResourceHandle(const CResourceHandle& resHandle)
{
	m_ResourceId = resHandle.m_ResourceId;
	++m_RefCountMap[m_ResourceId];
}


CResourceHandle::~CResourceHandle()
{
	if (GetRefCount() > 0)
	{
		--m_RefCountMap[m_ResourceId];
	}
	else
	{
    CApplication::Instance()->GetApplicationLog().AddLogEntry("CResourceHandle::~CResourceHandle : Trying to decrement refcount of zero!", APP_LOG_ERROR);
	}
}


CBitmapResourceHandle::~CBitmapResourceHandle()
{
	if (GetRefCount() == 1 && m_BitmapMap.find(m_ResourceId) != m_BitmapMap.end())
	{
		SDL_FreeSurface(m_BitmapMap[m_ResourceId]);
		m_BitmapMap.erase(m_ResourceId);
	}
}


SDL_Surface* CBitmapResourceHandle::Bitmap() const
{
	return (m_BitmapMap.find(m_ResourceId) != m_BitmapMap.end()) ? m_BitmapMap[m_ResourceId] : nullptr;
}


CBitmapFileResourceHandle::CBitmapFileResourceHandle(std::string sFilename) :
	CBitmapResourceHandle(AUTO_CREATE_RESOURCE_ID),
	m_sFilename(std::move(sFilename))
{
	if (m_BitmapMap.find(m_ResourceId) == m_BitmapMap.end())
	{
		SDL_Surface* pSurface = SDL_LoadBMP(m_sFilename.c_str());
		if (!pSurface)
		{
			throw(Wg_Ex_App("Unable to load bitmap: " + m_sFilename, "CBitmapFileResourceHandle::CBitmapFileResourceHandle"));
		}
		m_BitmapMap[m_ResourceId] = pSurface;
	}
}


CStringResourceHandle::~CStringResourceHandle()
{
	if (GetRefCount() == 1 && m_StringMap.find(m_ResourceId) != m_StringMap.end())
	{
		m_StringMap.erase(m_ResourceId);
	}
}


std::string CStringResourceHandle::String() const
{
	return (m_StringMap.find(m_ResourceId) != m_StringMap.end()) ? m_StringMap[m_ResourceId] : "";
}


CCursorResourceHandle::~CCursorResourceHandle()
{
	if (GetRefCount() == 1 && m_SDLCursorMap.find(m_ResourceId) != m_SDLCursorMap.end())
	{
		SDL_FreeCursor(m_SDLCursorMap[m_ResourceId]);
		m_SDLCursorMap.erase(m_ResourceId);
	}
}


SDL_Cursor* CCursorResourceHandle::Cursor() const
{
	return (m_SDLCursorMap.find(m_ResourceId) != m_SDLCursorMap.end()) ? m_SDLCursorMap[m_ResourceId] : nullptr;
}

}

