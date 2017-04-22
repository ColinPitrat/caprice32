// wg_toolbar.h
//
// CToolBar class interface
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


#ifndef _WG_TOOLBAR_H_
#define _WG_TOOLBAR_H_

#include "wg_window.h"
#include "wg_button.h"
#include <vector>


namespace wGui
{

//! A Toolbar control that groups and organizes buttons

//! Toolbars support CButton derived controls

class CToolBar : public CWindow
{
public:
	//! Constructs a new ToolBar
  	//! \param WindowRect A CRect that defines the outer limits of the control
	//! \param pParent A pointer to the parent window
	CToolBar(const CRect& WindowRect, CWindow* pParent);

	//! Standard destructor
	~CToolBar() override;


	//! Add a button to the toolbar
	//! The toolbar will become the button's parent
	//! The toolbar will catch all CTRL_SINGLELCLICK messages from the buttons and will post a CTRL_SINGLELCLICK message from the toolbar with the iButtonID value as the iNewValue
	//! \param pButton A pointer to the button to be inserted, inserts a spacer if this is NULL
	//! \param iButtonID An identifier that the toolbar will return when a button is clicked on, defaults to 0
	//! \param iPosition The position to insert the button at (defaults to adding to the beginning of the toolbar)
	void InsertButton(CButton* pButton, long int iButtonID = 0, unsigned int iPosition = 0);

	//! Add a button to the end of toolbar
	//! The toolbar will become the button's parent
	//! The toolbar will catch all CTRL_SINGLELCLICK messages from the buttons and will post a CTRL_SINGLELCLICK message from the toolbar with the iButtonID value as the iNewValue
	//! \param pButton A pointer to the button to be inserted, inserts a spacer if this is NULL
	//! \param iButtonID An identifier that the toolbar will return when a button is clicked on, defaults to 0
	void AppendButton(CButton* pButton, long int iButtonID = 0);

	//! Remove a button from the toolbar
	//! This will automatically delete the button
	//! \param iPosition The position of the button to remove, an exception will be thrown if this is out of range
	void RemoveButton(unsigned int iPosition);

	//! Remove all buttons from the ToolBar
	void Clear();

	//! Gets the number of items on the toolbar (including spacers)
	//! \return The number of buttons in the toolbar
	unsigned int GetButtonCount() { return stdex::safe_static_cast<unsigned int>(m_vpButtons.size()); }

	//! \param iPosition The position of the button to get the ID for.  An exception will be thrown if this is out of range.
	//! \return The ButtonID of the button at the given position (spacers always return 0)
	long int GetButtonID(unsigned int iPosition) { return m_vpButtons.at(iPosition).second; }

	//! \param iButtonID The ID of the button to get the position for
	//! \return The position of the button, or -1 if it can't find the ButtonID
	int GetButtonPosition(long int iButtonID);


	//! CWindow overrides
	//! Giving a control a new WindowRect will move and resize the control
	//! \param WindowRect A CRect that defines the outer limits of the control
	void SetWindowRect(const CRect& WindowRect) override;


	// CMessageClient overrides
	//! CToolBars handle CTRL_SINGLELCLICK messages
	//! \param pMessage A pointer to the message
	bool HandleMessage(CMessage* pMessage) override;


protected:

	//! Reposition all the buttons in the toolbar
	void RepositionButtons();

	typedef std::pair<CButton*, long int> t_ButtonIDPair;  //!< A typedef of CButton pointer to ID pair
	typedef std::vector<t_ButtonIDPair> t_ButtonVector;  //!< A typedef of a vector of Button ID pairs
	t_ButtonVector m_vpButtons;  //!< A vector of pointers to the buttons and their IDs in the toolbar


private:
  CToolBar(const CToolBar&) = delete;
	CToolBar& operator=(const CToolBar&) = delete;
};

}


#endif  // _WG_TOOLBAR_H_

