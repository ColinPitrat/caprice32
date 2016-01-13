// 'Load/save' box for Caprice32

#ifndef _WG_CAPRICE32LOADSAVE_H_
#define _WG_CAPRICE32LOADSAVE_H_

#include "wgui.h"
#include "wg_frame.h"
#include "wg_navigationbar.h"

namespace wGui
{
    class CapriceLoadSave : public CFrame {
      public:
        //! \param pParent A pointer to the parent view
        //! \param pFontEngine A pointer to the font engine to use when drawing the control
        //! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
        CapriceLoadSave(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine);

      protected:
        // Navigation bar control (to select the type of file to load/save)
        CNavigationBar* m_pNavigationBar; 

      private:
        void operator=(CapriceLoadSave) { }  //!< The assignment operator is not allowed for CWindow derived objects
    };
}

#endif  // _WG_CAPRICE32LOADSAVE_H_
