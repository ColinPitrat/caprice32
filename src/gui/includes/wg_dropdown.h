// wg_dropdown.h
//
// CDropDown interface
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


#ifndef _WG_DROPDOWN_H_
#define _WG_DROPDOWN_H_

#include "wg_window.h"
#include "wg_painter.h"
#include "wg_editbox.h"
#include "wg_listbox.h"
#include <memory>


namespace wGui
{

//! A dropdown control, which combines an edit control and a listbox control

//! The CDropDown will generate CTRL_VALUECHANGE messages every time the text changes

class CDropDown : public CWindow
{
public:
	//! Construct a new DropDown control
	//! \param WindowRect A CRect that defines the outer limits of the control (this only controls the dimensions of the edit control portion of the drop down)
	//! \param pParent A pointer to the parent window
	//! \param bAllowEdit If false, the edit box will be read only, and the value can only be changed via the drop-down list (true by default)
	//! \param iItemHeight The height of the items in the listbox portion of the control, defaults to 15
	//! \param pFontEngine A pointer to the font engine to use when drawing the control
	//! If this is left out (or set to 0) it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
	CDropDown(const CRect& WindowRect, CWindow* pParent, bool bAllowEdit = true, unsigned int iItemHeight = 15, CFontEngine* pFontEngine = nullptr);

	//! Standard destructor
	~CDropDown() override;

	//! Adds a new item to the list
	//! \param ListItem A SListItem structure with the data for the item
	//! \return The index of the added item
	int AddItem(SListItem ListItem) { return m_pListBox->AddItem(ListItem); }

	//! Returns the desired item
	//! \param iItemIndex The index of the item to check (will throw an exception if the index is out of range)
	//! \return A reference to the SListItem struct
	SListItem& GetItem(int iItemIndex) { return m_pListBox->GetItem(iItemIndex); }

	//! Remove an item from the list
	//! \param iItemIndex The index of the item to remove
	void RemoveItem(int iItemIndex) { m_pListBox->RemoveItem(iItemIndex); }

	//! Remove all items from the list
	void ClearItems() { m_pListBox->ClearItems(); }

	//! Gets the current number of items in the listbox
	//! \return The number of items in the list
	int Size() { return m_pListBox->Size(); }

	//! \param iItemIndex The index of the item to check (will return false if the index is out of range)
	//! \return true if the item is selected
	bool IsSelected(unsigned int iItemIndex) { return m_pListBox->IsSelected(iItemIndex); }

    // judb get index of the selected item (-1 if none)
	int GetSelectedIndex();

	// judb select the item with index iItemIndex in the list, and display the item's name
    // (in the area to the left of the dropdown arrow)
	void SelectItem(unsigned int iItemIndex);
	//! Set an item as selected
	//! \param iItemIndex The index of the item to change
	//! \param bSelected Will select the item if true, or unselect if false
	void SetSelection(int iItemIndex, bool bSelected) { m_pListBox->SetSelection(iItemIndex, bSelected); }

	//! Set the selection for all of the items at once
	//! \param bSelected Will select all items if true, or unselect if false
	void SetAllSelections(bool bSelected) { m_pListBox->SetAllSelections(bSelected); }

	//! Sets the height of the drop list
	//! \param iItemCount The height of the listbox in number of items shown (this is set to 5 by default)
	void SetListboxHeight(int iItemCount);


	// CWindow overrides
	//! Giving a control a new WindowRect will move and resize the control
	//! \param WindowRect A CRect that defines the outer limits of the control
	void SetWindowRect(const CRect& WindowRect) override;

	//! Set the WindowText of the control
	//! \param sWindowText The text to assign to the window
	void SetWindowText(const std::string& sWindowText) override;

	//! Get the WindowText of the control
	std::string GetWindowText() const override;

	//! Move the window and any child windows
	//! \param MoveDistance The relative distance to move the window
	void MoveWindow(const CPoint& MoveDistance) override;

	// slight override from CWindow: if visible is set to "true", the dropdown part should stay invisible:
	void SetVisible(bool bVisible) override;

  // Override the default behaviour: a focused drop-down list is in fact it's button being focused
  void SetIsFocusable(bool bFocused) override;

	// CMessageClient overrides
	//! CDropDown will handle MOUSE_BUTTONDOWN messages
	//! \param pMessage A pointer to the message that needs to be handled
	bool HandleMessage(CMessage* pMessage) override;


protected:
	//! Shows the drop down listbox
	void ShowListBox();

	//! Hides the drop down listbox
	void HideListBox();


	CEditBox* m_pEditBox;  //!< A pointer to the drop down's edit box
	CListBox* m_pListBox;  //!< A pointer to teh drop down's list box
	CPictureButton* m_pDropButton;  //!< A pointer to the drop down's drop button
	bool m_bAllowEdit;  //!< If false, the edit box will be read only, and the value can only be changed via the drop-down list


private:
  CDropDown(const CDropDown&) = delete;
	CDropDown& operator=(const CDropDown&) = delete;
  CView* m_pCViewAncestor;   // pointer to the (unique) CView of the application.
};

}

#endif // _WG_DROPDOWN_H_




