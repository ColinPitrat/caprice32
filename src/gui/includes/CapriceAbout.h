// 'About' box for Caprice32
// Inherited from CMessageBox


#ifndef _WG_CAPRICE32ABOUT_H_
#define _WG_CAPRICE32ABOUT_H_

#include "wg_messagebox.h"
#include "wg_label.h"
#include "wg_textbox.h"

namespace wGui
{
    class CapriceAbout : public CMessageBox {
      public:
        //! \param pParent A pointer to the parent view
        //! \param pFontEngine A pointer to the font engine to use when drawing the control
        //! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
        CapriceAbout(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine);

      private:
        CapriceAbout(const CapriceAbout&) = delete;
        CapriceAbout& operator=(const CapriceAbout&) = delete;

        CTextBox* m_pTextBox;
    };
}

#endif  // _WG_CAPRICE32ABOUT_H_
