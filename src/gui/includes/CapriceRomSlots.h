// Caprice32 ROM slot selection window
// Inherited from CFrame

#ifndef _WG_CAPRICE32ROMSLOTS_H_
#define _WG_CAPRICE32ROMSLOTS_H_

#include "wgui.h"
#include "wg_frame.h"
#include "wg_label.h"
#include <string>

namespace wGui
{
    class CapriceRomSlots : public CFrame {
      public:
        //! \param pParent A pointer to the parent view
        //! \param pFontEngine A pointer to the font engine to use when drawing the control
        //! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
        // selectedRomButton is the button that was clicked to open this dialog (not the nicest solution, but it works...)
        CapriceRomSlots(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine, std::string sTitle, int selectedRomSlot, CButton* pSelectedRomButton);
        bool HandleMessage(CMessage* pMessage) override;
        // Returns a list with the available ROM files (filenames)
        std::vector<std::string> getAvailableRoms();

        int getRomSlot() const { return romSlot; };
        void setRomSlot(int newRomSlot) { romSlot = newRomSlot; } ;

      protected:

        int romSlot;  // selected ROM slot number
  
        CButton* m_pSelectedRomButton; // the button that was clicked to open this dialog

        CButton* m_pButtonInsert;  // Inserts the selected ROM in the ROM slot.
        CButton* m_pButtonClear;   // Clears the selected ROM slot.
        CButton* m_pButtonCancel;  // Close the dialog without action.

        CListBox* m_pListBoxRoms;  // Lists the available ROM files (in the ROMS subdirectory)

      private:
        CapriceRomSlots(const CapriceRomSlots&) = delete;
        CapriceRomSlots& operator=(const CapriceRomSlots&) = delete;
    };
}

#endif  // _WG_CAPRICE32ROMSLOTS_H_
