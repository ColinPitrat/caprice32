// 'DevTools' window for Caprice32

#ifndef _WG_CAPRICE32DEVTOOLS_H_
#define _WG_CAPRICE32DEVTOOLS_H_

#include "wgui.h"
#include "wg_frame.h"
#include "wg_label.h"
#include "wg_register.h"
#include "wg_navigationbar.h"
#include "cap32.h"
#include <map>
#include <string>

class DevTools;

namespace wGui
{
    class CapriceDevTools : public CFrame {
      public:
        //! \param pParent A pointer to the parent view
        //! \param pFontEngine A pointer to the font engine to use when drawing the control
        //! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
        CapriceDevTools(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine, DevTools* devtools);
        ~CapriceDevTools() override;

        void Update();

        bool HandleMessage(CMessage* pMessage) override;

        // activate the specified tab (make its controls visible)
        void EnableTab(std::string sTabName);

        void CloseFrame() override;

      protected:
        CButton* m_pButtonPause;
        CButton* m_pButtonClose;

        // New navigation bar control (to select the different pages or tabs on the options dialog)
        CNavigationBar* m_pNavigationBar; 

        // groupbox to group the controls on each 'tab':
        CGroupBox* m_pGroupBoxTabZ80;
        CGroupBox* m_pGroupBoxTabAsm;
        CGroupBox* m_pGroupBoxTabMemory;
        CGroupBox* m_pGroupBoxTabVideo;
        CGroupBox* m_pGroupBoxTabAudio;
        CGroupBox* m_pGroupBoxTabChar;

        // Z80 screen
        // 8 bits registers
        CRegister* m_pRegA;
        CRegister* m_pRegAp;
        CRegister* m_pRegB;
        CRegister* m_pRegBp;
        CRegister* m_pRegC;
        CRegister* m_pRegCp;
        CRegister* m_pRegD;
        CRegister* m_pRegDp;
        CRegister* m_pRegE;
        CRegister* m_pRegEp;
        CRegister* m_pRegH;
        CRegister* m_pRegHp;
        CRegister* m_pRegL;
        CRegister* m_pRegLp;
        CRegister* m_pRegI;
        CRegister* m_pRegR;
        CRegister* m_pRegIXH;
        CRegister* m_pRegIXL;
        CRegister* m_pRegIYH;
        CRegister* m_pRegIYL;
        // 16 bits registers
        CRegister* m_pRegAF;
        CRegister* m_pRegAFp;
        CRegister* m_pRegBC;
        CRegister* m_pRegBCp;
        CRegister* m_pRegDE;
        CRegister* m_pRegDEp;
        CRegister* m_pRegHL;
        CRegister* m_pRegHLp;
        CRegister* m_pRegIX;
        CRegister* m_pRegIY;
        CRegister* m_pRegSP;
        CRegister* m_pRegPC;
        // TODO: Flags

        // Assembly screen
        CLabel* m_pAsmLabel;
        // Memory screen
        CLabel* m_pMemLabel;
        // Video screen
        CLabel* m_pVidLabel;
        // Audio screen
        CLabel* m_pAudLabel;
        // Characters screen
        CLabel* m_pChrLabel;

        DevTools* m_pDevTools;

      private:

        std::map<std::string, CGroupBox*> TabMap;  // mapping: <tab name> -> <groupbox that contains the 'tab'>.
        
        CapriceDevTools(const CapriceDevTools&) = delete;
        CapriceDevTools& operator=(const CapriceDevTools&) = delete;
    };
}

#endif  // _WG_CAPRICE32DEVTOOLS_H_
