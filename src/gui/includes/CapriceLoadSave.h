// 'Load/save' box for Caprice32

#ifndef _WG_CAPRICE32LOADSAVE_H_
#define _WG_CAPRICE32LOADSAVE_H_

#include "wgui.h"
#include "wg_frame.h"
#include "wg_navigationbar.h"

class CapriceLoadSaveTest;

namespace wGui
{
    class CapriceLoadSave : public CFrame {
      public:
        //! \param pParent A pointer to the parent view
        //! \param pFontEngine A pointer to the font engine to use when drawing the control
        //! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
        CapriceLoadSave(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine);
        ~CapriceLoadSave() override;
        bool HandleMessage(CMessage* pMessage) override;
        std::string simplifyDirPath(std::string path);
        void UpdateFilesList();
        bool MatchCurrentFileSpec(const char* filename);

      protected:
        friend CapriceLoadSaveTest;

        std::list<std::string> m_fileSpec;
        CLabel   *m_pTypeLabel;
        CDropDown *m_pTypeValue;
        CLabel   *m_pActionLabel;
        CDropDown *m_pActionValue;
        CLabel   *m_pDirectoryLabel;
        CEditBox *m_pDirectoryValue;
        CListBox *m_pFilesList;
        CLabel   *m_pFileNameLabel;
        CEditBox *m_pFileNameValue;
        CButton  *m_pCancelButton;
        CButton  *m_pLoadSaveButton;

      private:
        CapriceLoadSave(const CapriceLoadSave&) = delete;
        CapriceLoadSave& operator=(const CapriceLoadSave&) = delete;
    };
}

#endif  // _WG_CAPRICE32LOADSAVE_H_
