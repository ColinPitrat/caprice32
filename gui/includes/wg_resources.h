// wg_resources.h
//
// wgui resources
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


#ifndef _WG_RESOURCES_H_
#define _WG_RESOURCES_H_

#include "SDL.h"
#include "wg_resource_handle.h"
#include "wg_color.h"


namespace wGui
{

//! The list of wGui resources
enum EwgResourceId {
// Bitmaps
	WGRES_UP_ARROW_BITMAP = 1,
	WGRES_DOWN_ARROW_BITMAP,
	WGRES_LEFT_ARROW_BITMAP,
	WGRES_RIGHT_ARROW_BITMAP,
	WGRES_X_BITMAP,
	WGRES_RADIOBUTTON_BITMAP,
	WGRES_CHECK_BITMAP,
	WGRES_MAXIMIZE_UNMAXED_BITMAP,
	WGRES_MAXIMIZE_MAXED_BITMAP,
	WGRES_MINIMIZE_BITMAP,

// Strings
	WGRES_WGUI_STRING,
	WGRES_VERSION_STRING,

// Cursors
	WGRES_POINTER_CURSOR,
	WGRES_IBEAM_CURSOR,
	WGRES_WAIT_CURSOR,
	WGRES_MOVE_CURSOR,
	WGRES_ZOOM_CURSOR
};


// Since the internal resources will probably be used fairly frequently,
// we cheat a little and keep a clone of any that have been instantiated for the lifetime for the program
//static std::set<CResourceHandle> wgInternalResourcePool;


//! Resource handle class for the internal wGui bitmap resources

class CwgBitmapResourceHandle : public CBitmapResourceHandle
{
public:
	//! CwgBitmapResourceHandles must be instantiated with a valid resource ID
	//! \param resId The ID of the resource which the handle will represent
	CwgBitmapResourceHandle(EwgResourceId resId);

private:
	SDL_Surface* DrawBitmap(CRGBColor Data[], int iDataLength, int iWidth, int iHeight) const;
};


//! Resource handle class for the internal wGui string resources

class CwgStringResourceHandle : public CStringResourceHandle
{
public:
	//! CwgStringResourceHandles must be instantiated with a valid resource ID
	//! \param resId The ID of the resource which the handle will represent
	CwgStringResourceHandle(EwgResourceId resId);
};


//! Resource handle class for the internal wGui cursor resources

class CwgCursorResourceHandle : public CCursorResourceHandle
{
public:
	//! CwgCursorResourceHandle must be instantiated with a valid resource ID
	//! \param resId The ID of the resource which the handle will represent
	CwgCursorResourceHandle(EwgResourceId resId);

private:
	enum ECursorDataMask {
		O, // empty
		M, // masked
		D, // data
		X // data and mask
	};

	SDL_Cursor* CreateCursor(const char DataIn[], int iDataLength, int iWidth, int iHeight, int iXHotSpot, int iYHotSpot) const;
};

}

#endif  // _WG_RESOURCES_H_

