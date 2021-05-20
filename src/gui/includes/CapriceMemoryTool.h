// 'Memory tool' window for Caprice32

#ifndef _WG_CAPRICE32MEMORYTOOL_H_
#define _WG_CAPRICE32MEMORYTOOL_H_

#include "wg_button.h"
#include "wg_dropdown.h"
#include "wg_editbox.h"
#include "wg_frame.h"
#include "wg_label.h"
#include "wg_textbox.h"

namespace wGui
{
    class CapriceMemoryTool : public CFrame {
      public:
        //! \param pParent A pointer to the parent view
        //! \param pFontEngine A pointer to the font engine to use when drawing the control
        //! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
        CapriceMemoryTool(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine);
        ~CapriceMemoryTool() override;
        bool HandleMessage(CMessage* pMessage) override;

      protected:
        CFontEngine *m_pMonoFontEngine;

        CLabel   *m_pPokeLabel;
        CLabel   *m_pPokeAdressLabel;
        CEditBox *m_pPokeAdress;
        CLabel   *m_pPokeValueLabel;
        CEditBox *m_pPokeValue;
        CButton  *m_pButtonPoke;
        CLabel   *m_pFilterLabel;
        CEditBox *m_pFilterValue;
        CButton  *m_pButtonFilter;
        CLabel   *m_pAdressLabel;
        CEditBox *m_pAdressValue;
        CButton  *m_pButtonDisplay;
        CButton  *m_pButtonCopy;
        CButton  *m_pButtonClose;
        CLabel   *m_pBytesPerLineLbl;
        CDropDown *m_pBytesPerLine;
        //CListBox *m_pListMemContent;
        CTextBox *m_pTextMemContent;

        int m_filterValue;
        int m_displayValue;
        unsigned int m_bytesPerLine;

      private:
        void UpdateTextMemory();
        CapriceMemoryTool(const CapriceMemoryTool&) = delete;
        CapriceMemoryTool& operator=(const CapriceMemoryTool&) = delete;
    };
}

#endif
