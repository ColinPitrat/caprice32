// wg_resources.cpp
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


#include "wgui_include_config.h"
#include "wg_resources.h"
#include "wg_error.h"
#include "wg_painter.h"
#include "wutil_debug.h"
#include "wg_application.h"
#include <list>


namespace wGui
{


CwgBitmapResourceHandle::CwgBitmapResourceHandle(EwgResourceId resId) :
	CBitmapResourceHandle(resId)
{
	if (m_BitmapMap.find(m_ResourceId) == m_BitmapMap.end())
	{
		const CRGBColor T = COLOR_TRANSPARENT;
		const CRGBColor B = DEFAULT_CHECKBOX_COLOR;
    //const CRGBColor W = COLOR_WHITE;

		switch (m_ResourceId)
		{
			case NULL_RESOURCE_ID:
				m_BitmapMap[NULL_RESOURCE_ID] = nullptr;
				break;
			case WGRES_UP_ARROW_BITMAP:
			{
				CRGBColor buf[] = {T, T, T, T, T, T, T, T,
  								   T, T, T, T, T, T, T, T,
								   T, T, T, T, T, T, T, T,
								   T, T, T, B, B, T, T, T,
								   T, T, B, B, B, B, T, T,
								   T, B, B, T, T, B, B, T,
								   T, T, T, T, T, T, T, T,
								   T, T, T, T, T, T, T, T};
				m_BitmapMap[m_ResourceId] = DrawBitmap(buf, sizeof(buf) / sizeof(CRGBColor), 8, 8);
				break;
			}
			case WGRES_DOWN_ARROW_BITMAP:
			{
				CRGBColor buf[] = {T, T, T, T, T, T, T, T,
								T, T, T, T, T, T, T, T,
								T, B, B, T, T, B, B, T,
								T, T, B, B, B, B, T, T,
								T, T, T, B, B, T, T, T,
								T, T, T, T, T, T, T, T,
								T, T, T, T, T, T, T, T,
								T, T, T, T, T, T, T, T};
				m_BitmapMap[m_ResourceId] = DrawBitmap(buf, sizeof(buf) / sizeof(CRGBColor), 8, 8);
				break;
			}
			case WGRES_LEFT_ARROW_BITMAP:
			{
				CRGBColor buf[] = {T, T, T, T, T, T, T, T,
								T, T, T, T, T, B, T, T,
								T, T, T, T, B, B, T, T,
								T, T, T, B, B, T, T, T,
								T, T, T, B, B, T, T, T,
								T, T, T, T, B, B, T, T,
								T, T, T, T, T, B, T, T,
								T, T, T, T, T, T, T, T};
				m_BitmapMap[m_ResourceId] = DrawBitmap(buf, sizeof(buf) / sizeof(CRGBColor), 8, 8);
				break;
			}
			case WGRES_RIGHT_ARROW_BITMAP:
			{
				CRGBColor buf[] = {T, T, T, T, T, T, T, T,
								T, T, B, T, T, T, T, T,
								T, T, B, B, T, T, T, T,
								T, T, T, B, B, T, T, T,
								T, T, T, B, B, T, T, T,
								T, T, B, B, T, T, T, T,
								T, T, B, T, T, T, T, T,
								T, T, T, T, T, T, T, T};
				m_BitmapMap[m_ResourceId] = DrawBitmap(buf, sizeof(buf) / sizeof(CRGBColor), 8, 8);
				break;
			}
			case WGRES_X_BITMAP:
			{
			 CRGBColor buf[] = { B, B, T, T, B, B, 
								 T, B, B, B, B, T, 
								 T, T, B, B, T, T, 
								 T, T, B, B, T, T, 
								 T, B, B, B, B, T, 
								 B, B, T, T, B, B};
				m_BitmapMap[m_ResourceId] = DrawBitmap(buf, sizeof(buf) / sizeof(CRGBColor), 6, 6);
				break;
			}
			case WGRES_RADIOBUTTON_BITMAP:
			{
			 CRGBColor buf[] = {T, T, T, T, T, T,
							    T, B, B, B, B, T,
							    T, B, B, B, B, T,
								T, B, B, B, B, T,
								T, B, B, B, B, T,
								T, T, T, T, T, T};
				m_BitmapMap[m_ResourceId] = DrawBitmap(buf, sizeof(buf) / sizeof(CRGBColor), 6, 6);
				break;
			}
			case WGRES_CHECK_BITMAP:
			{
 			 CRGBColor buf[] = { T, T, T, T, T, T, 
								 T, T, T, T, T, B, 
								 B, T, T, T, B, B, 
								 B, B, T, B, B, T, 
								 T, B, B, B, T, T, 
								 T, T, B, T, T, T};
				m_BitmapMap[m_ResourceId] = DrawBitmap(buf, sizeof(buf) / sizeof(CRGBColor), 6, 6);
				break;
			}
			case WGRES_MAXIMIZE_UNMAXED_BITMAP:
			{
				CRGBColor buf[] = {T, B, B, B, B, B, B, T,
								T, B, B, B, B, B, B, T,
								T, B, T, T, T, T, B, T,
								T, B, T, T, T, T, B, T,
								T, B, T, T, T, T, B, T,
								T, B, T, T, T, T, B, T,
								T, B, B, B, B, B, B, T,
								T, T, T, T, T, T, T, T};
				m_BitmapMap[m_ResourceId] = DrawBitmap(buf, sizeof(buf) / sizeof(CRGBColor), 8, 8);
				break;
			}
			case WGRES_MAXIMIZE_MAXED_BITMAP:
			{
				CRGBColor buf[] = {T, T, T, B, B, B, B, B,
								T, T, T, B, B, B, B, B,
								T, T, T, B, T, T, T, B,
								B, B, B, B, B, T, T, B,
								B, B, B, B, B, T, T, B,
								B, T, T, T, B, B, B, B,
								B, T, T, T, B, T, T, T,
								B, B, B, B, B, T, T, T};
				m_BitmapMap[m_ResourceId] = DrawBitmap(buf, sizeof(buf) / sizeof(CRGBColor), 8, 8);
				break;
			}
			case WGRES_MINIMIZE_BITMAP:
			{
				CRGBColor buf[] = {T, T, T, T, T, T, T, T,
								T, T, T, T, T, T, T, T,
								T, T, T, T, T, T, T, T,
								T, T, T, T, T, T, T, T,
								T, T, T, T, T, T, T, T,
								T, B, B, B, B, B, B, T,
								T, B, B, B, B, B, B, T,
								T, T, T, T, T, T, T, T};
				m_BitmapMap[m_ResourceId] = DrawBitmap(buf, sizeof(buf) / sizeof(CRGBColor), 8, 8);
				break;
			}
			case INVALID_RESOURCE_ID:
			case AUTO_CREATE_RESOURCE_ID:
			default:
				throw(Wg_Ex_App("CwgBitmapResourceHandle::AllocateResource : Invalid Resource ID."));
				break;
		}
		CResourceHandle TempHandle(m_ResourceId);
		CApplication::Instance()->AddToResourcePool(TempHandle);
	}
}


SDL_Surface* CwgBitmapResourceHandle::DrawBitmap(CRGBColor Data[], int iDataLength, int iWidth, int iHeight) const
{
	SDL_Surface* pBitmap = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, iWidth, iHeight, 
		CApplication::Instance()->GetBitsPerPixel(), 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	CPainter Painter(pBitmap, CPainter::PAINT_REPLACE);
	for (int iRow = 0; iRow < iHeight; ++iRow)
	{
		for (int iCol = 0; iCol < iWidth; ++iCol)
		{
			int iIndex = iRow * iWidth + iCol;
			if (iIndex < iDataLength)
			{
				Painter.DrawPoint(CPoint(iCol, iRow), Data[iRow * iWidth + iCol]);
			}
		}
	}

	return pBitmap;
}


CwgStringResourceHandle::CwgStringResourceHandle(EwgResourceId resId) :
	CStringResourceHandle(resId)
{
	if (m_StringMap.find(m_ResourceId) == m_StringMap.end())
	{
		switch (m_ResourceId)
		{
			case NULL_RESOURCE_ID:
				m_StringMap[m_ResourceId] = "";
				break;
			case WGRES_WGUI_STRING:
				m_StringMap[m_ResourceId] = "wGui";
				break;
			case WGRES_VERSION_STRING:
				m_StringMap[m_ResourceId] = VERSION;
				break;
			case INVALID_RESOURCE_ID:
			case AUTO_CREATE_RESOURCE_ID:
			default:
				throw(Wg_Ex_App("CwgBitmapResourceHandle::AllocateResource : Invalid Resource ID."));
				break;
		}
		CResourceHandle TempHandle(m_ResourceId);
		CApplication::Instance()->AddToResourcePool(TempHandle);
	}
}


CwgCursorResourceHandle::CwgCursorResourceHandle(EwgResourceId resId) :
	CCursorResourceHandle(resId)
{
	if (m_SDLCursorMap.find(m_ResourceId) == m_SDLCursorMap.end())
	{
		switch (m_ResourceId)
		{
			case NULL_RESOURCE_ID:
				m_SDLCursorMap[m_ResourceId] = nullptr;
				break;
			case WGRES_POINTER_CURSOR:
			{
				char buf[] = {X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,M,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,M,M,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,M,M,X,X,X,X,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,X,O,X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,X,O,O,X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,O,O,O,O,X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,X,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O};
				m_SDLCursorMap[m_ResourceId] = CreateCursor(buf, sizeof(buf) / sizeof(char), 32, 32, 0, 0);
				break;
			}
			case WGRES_IBEAM_CURSOR:
			{
				char buf[] = {D,D,D,O,D,D,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							D,D,D,O,D,D,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O};
				m_SDLCursorMap[m_ResourceId] = CreateCursor(buf, sizeof(buf) / sizeof(char), 32, 32, 3, 8);
				break;
			}
			case WGRES_WAIT_CURSOR:
			{
				char buf[] = {X,X,X,X,X,X,X,X,X,X,X,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,M,M,M,M,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,M,M,M,M,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,M,M,M,M,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,X,X,M,X,M,X,M,X,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,X,X,M,X,M,X,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,X,X,M,X,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,X,X,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,X,M,X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,X,M,M,M,X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,X,M,M,M,X,M,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,M,M,M,M,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,M,M,M,X,M,X,M,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,M,X,M,X,M,X,M,X,M,M,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							X,X,X,X,X,X,X,X,X,X,X,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O};
				m_SDLCursorMap[m_ResourceId] = CreateCursor(buf, sizeof(buf) / sizeof(char), 32, 32, 0, 0);
				break;
			}
			case WGRES_MOVE_CURSOR:
			{
				char buf[] = {
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,D,D,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,D,D,D,D,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,D,O,O,O,O,D,O,O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,D,O,O,O,O,D,O,O,O,O,D,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,D,O,O,O,O,D,O,O,O,O,D,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,D,O,O,O,O,D,O,O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,D,O,O,O,O,O,O,O,O,X,X,X,X,X,X,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,D,O,O,O,O,O,O,O,O,X,M,M,M,M,X,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,D,D,D,D,D,O,O,O,O,O,O,X,M,M,M,X,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,D,D,D,O,O,O,O,O,O,O,X,M,M,M,M,X,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,D,O,O,O,O,O,O,O,O,X,M,X,M,M,M,X,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,X,X,O,X,M,M,X,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,X,X,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O};
				m_SDLCursorMap[m_ResourceId] = CreateCursor(buf, sizeof(buf) / sizeof(char), 32, 32, 9, 9);
				break;
			}
			case WGRES_ZOOM_CURSOR:
			{
				char buf[] = {
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,D,D,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,D,O,O,O,D,O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,D,D,O,O,O,D,O,O,O,D,D,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,D,D,O,O,O,O,D,O,O,O,O,D,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,D,O,O,O,O,D,O,O,O,O,D,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,D,O,O,O,O,O,O,O,O,X,X,X,X,X,X,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,D,D,D,D,D,D,D,O,O,O,O,O,X,M,M,M,M,X,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,D,D,D,D,D,O,O,O,O,O,O,X,M,M,M,X,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,D,D,D,O,O,O,O,O,O,O,X,M,M,M,M,X,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,D,O,O,O,O,O,O,O,O,X,M,X,M,M,M,X,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,X,X,O,X,M,M,X,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,X,X,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
							O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O};
				m_SDLCursorMap[m_ResourceId] = CreateCursor(buf, sizeof(buf) / sizeof(char), 32, 32, 9, 9);
				break;
			}

			case INVALID_RESOURCE_ID:
			case AUTO_CREATE_RESOURCE_ID:
			default:
				throw(Wg_Ex_App("CwgBitmapResourceHandle::AllocateResource : Invalid Resource ID."));
				break;
		}
		CResourceHandle TempHandle(m_ResourceId);
		CApplication::Instance()->AddToResourcePool(TempHandle);
	}
}


SDL_Cursor* CwgCursorResourceHandle::CreateCursor(const char DataIn[], int iDataLength, int iWidth, int iHeight, int iXHotSpot, int iYHotSpot) const
{
	if (iWidth % 8)
	{
		throw(Wg_Ex_App("CwgCursorResourceHandle::CreateCursor : Cursors must be multiples of 8 bits wide."));
	}

	int iDataSize = iWidth * iHeight / 8;
	Uint8* pData = new Uint8[iDataSize];
	Uint8* pMask = new Uint8[iDataSize];
	int i = -1;

	for (int iRow = 0; iRow < iHeight; ++iRow)
	{
		for (int iCol = 0; iCol < iWidth; ++iCol)
		{
			int iIndex = iCol + iRow * iWidth;
			if (iIndex < iDataLength)
			{
				if (iCol % 8)
				{
					pData[i] <<= 1;
					pMask[i] <<= 1;
				}
				else
				{
					++i;
					pData[i] = 0;
					pMask[i] = 0;
				}
				switch (DataIn[iIndex])
				{
				case X:
					pData[i] |= 0x01;
					pMask[i] |= 0x01;
					break;
				case D:
					pData[i] |= 0x01;
					break;
				case M:
					pMask[i] |= 0x01;
					break;
				case O:
					break;
				}
			}
		}
	}

	return SDL_CreateCursor(pData, pMask, iWidth, iHeight, iXHotSpot, iYHotSpot);
}

}

