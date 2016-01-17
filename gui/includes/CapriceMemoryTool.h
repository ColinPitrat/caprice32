// 'Memory tool' window for Caprice32

#ifndef _WG_CAPRICE32MEMORYTOOL_H_
#define _WG_CAPRICE32MEMORYTOOL_H_

#include "wgui.h"
#include "wg_frame.h"
#include "wg_editbox.h"
#include "wg_label.h"
#include "wg_button.h"

namespace wGui
{
    class CapriceMemoryTool : public CFrame {
      public:
        //! \param pParent A pointer to the parent view
        //! \param pFontEngine A pointer to the font engine to use when drawing the control
        //! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
        CapriceMemoryTool(const CRect& WindowRect, CView* pParent, CFontEngine* pFontEngine);
        ~CapriceMemoryTool(void);
        bool HandleMessage(CMessage* pMessage);

      protected:
        CLabel   *m_pPokeLabel;
        CLabel   *m_pPokeAdressLabel;
        CEditBox *m_pPokeAdress;
        CLabel   *m_pPokeValueLabel;
        CEditBox *m_pPokeValue;
        CButton  *m_pButtonPoke;
        CButton  *m_pButtonClose;
        CLabel   *m_pFilterLabel;
        CEditBox *m_pFilterValue;
        CButton  *m_pButtonFilter;
        CButton  *m_pButtonCopy;
        //CListBox *m_pListMemContent;
        CTextBox *m_pTextMemContent;

        unsigned int m_filterValue;

      private:
        void UpdateTextMemory();
        void operator=(CapriceMemoryTool) { }  //!< The assignment operator is not allowed for CWindow derived objects
    };
}

#endif
