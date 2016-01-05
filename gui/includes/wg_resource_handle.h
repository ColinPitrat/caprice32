// wg_resource_handle.h
//
// Resource handles interface
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


#ifndef _WG_RESOURCE_HANDLE_H_
#define _WG_RESOURCE_HANDLE_H_

#include "SDL.h"
#include <map>
#include <string>

namespace wGui
{
//! The resource ID type
typedef long int TResourceId;

const TResourceId AUTO_CREATE_RESOURCE_ID = -2;  //!< This is an internally used value for the Resource ID, that specifies that the next open resource ID should be used
const TResourceId INVALID_RESOURCE_ID = -1;  //!< This is an internally used value for the resource ID that indicates an invalid resource
const TResourceId NULL_RESOURCE_ID = 0;  //!< This is a resource ID for no resource


//! CResourceHandles are a basic smart handle for a resource pool
//! It will handle reference counting, allocation, and freeing of the resource
//! Classes derived from CResourceHandle should not have any personal data members, any data should be held in a static map
class CResourceHandle
{
public:
	//! CResourceHandles must be instantiated with a valid resource ID
	//! \param resId The ID of the resource which the handle will represent
	CResourceHandle(TResourceId resId);

	//! A copying constructor
	//! \param resHandle An existing resource handle that will be cloned
	CResourceHandle(const CResourceHandle& resHandle);

	//! Standard destructor will decrement the refcount for the resource and will deallocate it if the refcount hits zero
	virtual ~CResourceHandle(void);

	//! Gets the resource ID of the handle
	//! \return The resource ID of the handle
	TResourceId GetResourceId(void) { return m_ResourceId; }

	//! Gets the handle's internal reference count
	//! \return The reference count of the handle
	unsigned int GetRefCount(void) { return m_RefCountMap[m_ResourceId]; }

protected:
	//! The resource ID for the handle
	TResourceId m_ResourceId;

private:
	//! Resource handles are not assignable
	void operator=(CResourceHandle) { }

	//! The refcount for all the resources
	static std::map<TResourceId, unsigned int> m_RefCountMap;

	//! An internally used variable for keeping track of the next unused resource ID
	static TResourceId m_NextUnusedResourceId;
};


//! CBitmapResourceHandle is a resource handle for bitmaps
//! It will allocate the bitmaps as needed and can be cast as a bitmap
class CBitmapResourceHandle : public CResourceHandle
{
public:
	//! CBitmapResourceHandles must be instantiated with a valid resource ID
	//! \param resId The ID of the resource which the handle will represent
	CBitmapResourceHandle(TResourceId resId) : CResourceHandle(resId) { }

	//! A copying constructor
	//! \param resHandle An existing resource handle that will be cloned
	CBitmapResourceHandle(const CBitmapResourceHandle& resHandle) :
		CResourceHandle(resHandle) { }

	//! Standard destructor, which frees the bitmap if the refcount is zero
	virtual ~CBitmapResourceHandle(void);

	//! Gets the handle's bitmap
	//! \return An SDL_Surface pointer (the bitmap)
	SDL_Surface* Bitmap(void) const;

protected:
	//! The map of bitmaps held by the handles
	static std::map<TResourceId, SDL_Surface*> m_BitmapMap;

private:
	//! Resource handles are not assignable
	void operator=(CBitmapResourceHandle) { }
};


//! A resource handle for bitmap files
//! This will create a unique resource ID which can be used elsewhere
class CBitmapFileResourceHandle : public CBitmapResourceHandle
{
public:
	//! CBitmapFileResourceHandle must be instantiated with a valid bitmap file
	//! \param sFilename The bitmap file that will be loaded as a resource
	CBitmapFileResourceHandle(std::string sFilename);

protected:
	std::string m_sFilename;  //!< The filename of the resource

private:
	//! Resource handles are not assignable
	void operator=(CBitmapFileResourceHandle) { }
};


//! CStringResourceHandle is a resource handle for strings
class CStringResourceHandle : public CResourceHandle
{
public:
	//! CStringResourceHandles must be instantiated with a valid resource ID
	//! \param resId The ID of the resource which the handle will represent
	CStringResourceHandle(TResourceId resId) : CResourceHandle(resId) { }

	//! A copying constructor
	//! \param resHandle An existing resource handle that will be cloned
	CStringResourceHandle(const CStringResourceHandle& resHandle) :
		CResourceHandle(resHandle) { }

	//! Standard destructor, which frees the string if the refcount is zero
	virtual ~CStringResourceHandle(void);

	//! Returns the string
	//! \return A string
	const std::string String(void) const;

protected:
	//! A map of strings that are used by the handles
	static std::map<TResourceId, std::string> m_StringMap;

private:
	//! Resource handles are not assignable
	void operator=(CStringResourceHandle) { }
};


//! CCursorResourceHandle is a resource handle for mouse cursors
class CCursorResourceHandle : public CResourceHandle
{
public:
	//! CCursorResourceHandles must be instantiated with a valid resource ID
	//! \param resId The ID of the resource which the handle will represent
	CCursorResourceHandle(TResourceId resId) : CResourceHandle(resId) { }

	//! A copying constructor
	//! \param resHandle An existing resource handle that will be cloned
	CCursorResourceHandle(const CCursorResourceHandle& resHandle) :
		CResourceHandle(resHandle) { }

	//! Standard destructor, which frees the cursor if the refcount is zero
	virtual ~CCursorResourceHandle(void);

	//! Returns the SDL Cursor pointer
	//! \return A pointer to an SDL cursor object
	SDL_Cursor* Cursor(void) const;

protected:
	//! A map of cursors used by the handles
	static std::map<TResourceId, SDL_Cursor*> m_SDLCursorMap;

private:
	//! Resource handles are not assignable
	void operator=(CCursorResourceHandle) { }
};

}

#endif  // _WG_RESOURCE_HANDLE_H

