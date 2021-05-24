// 'DevTools' window for Caprice32

#ifndef _WG_CAPRICE32DEVTOOLS_H_
#define _WG_CAPRICE32DEVTOOLS_H_

#include "z80_disassembly.h"
#include "cap32.h"
#include "types.h"
#include "wg_checkbox.h"
#include "wg_dropdown.h"
#include "wg_frame.h"
#include "wg_groupbox.h"
#include "wg_label.h"
#include "wg_listbox.h"
#include "wg_register.h"
#include "wg_textbox.h"
#include "wg_navigationbar.h"
#include <map>
#include <string>

class DevTools;

namespace wGui
{

    class RAMConfig {
      public:
        std::string RAMConfigText();
        static std::string RAMConfigText(int i);
        static RAMConfig CurrentConfig();

        bool LoROMEnabled;
        bool HiROMEnabled;
        int RAMBank = 0;
        int RAMCfg = 0;
    };

    class CapriceDevTools : public CFrame {
      public:
        //! \param pParent A pointer to the parent view
        //! \param pFontEngine A pointer to the font engine to use when drawing the control
        //! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
        CapriceDevTools(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine, DevTools* devtools);
        ~CapriceDevTools() override;

        //! Prepare the update by saving the CPC state and updating this devtool's state.
        //! This is separated from Step2 to allow having multiple devtools.
        void PreUpdate();
        //! The part of the update that modifies the CPC state.
        void PostUpdate();

        bool HandleMessage(CMessage* pMessage) override;

        // activate the specified tab (make its controls visible)
        void EnableTab(std::string sTabName);

        void CloseFrame() override;

      protected:
        void PauseExecution();
        void ResumeExecution();

        byte ReadMem(word address);
        void WriteMem(word address, byte value);
        void PrepareMemBankConfig();

        void RefreshDisassembly();
        void UpdateAll();
        void UpdateZ80();
        void UpdateDisassembly();
        void UpdateDisassemblyPos();
        void UpdateEntryPointsList();
        void UpdateBreakPointsList();
        void UpdateWatchPointsList();
        void UpdateMemConfig();
        void UpdateTextMemory();

        CButton* m_pButtonStepIn;
        CButton* m_pButtonStepOver;
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
        CRegister* m_pZ80RegA;
        CRegister* m_pZ80RegAp;
        CRegister* m_pZ80RegB;
        CRegister* m_pZ80RegBp;
        CRegister* m_pZ80RegC;
        CRegister* m_pZ80RegCp;
        CRegister* m_pZ80RegD;
        CRegister* m_pZ80RegDp;
        CRegister* m_pZ80RegE;
        CRegister* m_pZ80RegEp;
        CRegister* m_pZ80RegH;
        CRegister* m_pZ80RegHp;
        CRegister* m_pZ80RegL;
        CRegister* m_pZ80RegLp;
        CRegister* m_pZ80RegI;
        CRegister* m_pZ80RegR;
        CRegister* m_pZ80RegIXH;
        CRegister* m_pZ80RegIXL;
        CRegister* m_pZ80RegIYH;
        CRegister* m_pZ80RegIYL;
        // 16 bits registers
        CRegister* m_pZ80RegAF;
        CRegister* m_pZ80RegAFp;
        CRegister* m_pZ80RegBC;
        CRegister* m_pZ80RegBCp;
        CRegister* m_pZ80RegDE;
        CRegister* m_pZ80RegDEp;
        CRegister* m_pZ80RegHL;
        CRegister* m_pZ80RegHLp;
        CRegister* m_pZ80RegIX;
        CRegister* m_pZ80RegIY;
        CRegister* m_pZ80RegSP;
        CRegister* m_pZ80RegPC;
        // Flags
        CRegister* m_pZ80RegF;
        CRegister* m_pZ80RegFp;
        CLabel* m_pZ80FlagsLabel;
        CLabel* m_pZ80FlagSLbl;
        CEditBox* m_pZ80FlagS;
        CLabel* m_pZ80FlagZLbl;
        CEditBox* m_pZ80FlagZ;
        CLabel* m_pZ80FlagHLbl;
        CEditBox* m_pZ80FlagH;
        CLabel* m_pZ80FlagPVLbl;
        CEditBox* m_pZ80FlagPV;
        CLabel* m_pZ80FlagNLbl;
        CEditBox* m_pZ80FlagN;
        CLabel* m_pZ80FlagCLbl;
        CEditBox* m_pZ80FlagC;
        // Stack
        CLabel* m_pZ80StackLabel;
        CListBox* m_pZ80Stack;

        // Assembly screen
        CListBox *m_pAssemblyCode;
        CButton *m_pAssemblyRefresh;
        CLabel* m_pAssemblyStatusLabel;
        CEditBox* m_pAssemblyStatus;

        CGroupBox* m_pAssemblyEntryPointsGrp;
        CListBox* m_pAssemblyEntryPoints;
        CEditBox* m_pAssemblyNewEntryPoint;
        CButton *m_pAssemblyAddPCEntryPoint;
        CButton *m_pAssemblyAddEntryPoint;
        CButton *m_pAssemblyRemoveEntryPoint;
        std::vector<word> m_EntryPoints;

        CGroupBox* m_pAssemblyBreakPointsGrp;
        CListBox* m_pAssemblyBreakPoints;
        CEditBox* m_pAssemblyNewBreakPoint;
        CButton *m_pAssemblyAddBreakPoint;
        CButton *m_pAssemblyRemoveBreakPoint;

        CGroupBox* m_pAssemblyMemConfigGrp;
        CLabel* m_pAssemblyMemConfigAsmLbl;
        CLabel* m_pAssemblyMemConfigCurLbl;
        CLabel* m_pAssemblyMemConfigROMLbl;
        CLabel* m_pAssemblyMemConfigLoLbl;
        CLabel* m_pAssemblyMemConfigHiLbl;
        CLabel* m_pAssemblyMemConfigRAMLbl;
        CLabel* m_pAssemblyMemConfigBankLbl;
        CLabel* m_pAssemblyMemConfigConfigLbl;

        CCheckBox* m_pAssemblyMemConfigAsmLoROM;
        CCheckBox* m_pAssemblyMemConfigAsmHiROM;
        CEditBox* m_pAssemblyMemConfigAsmRAMBank;
        CEditBox* m_pAssemblyMemConfigAsmRAMConfig;
        CCheckBox* m_pAssemblyMemConfigCurLoROM;
        CCheckBox* m_pAssemblyMemConfigCurHiROM;
        CEditBox* m_pAssemblyMemConfigCurRAMBank;
        CEditBox* m_pAssemblyMemConfigCurRAMConfig;

        DisassembledCode m_Disassembled;
        RAMConfig m_AsmRAMConfig;

        // Memory screen
        CLabel   *m_pMemPokeLabel;
        CLabel   *m_pMemPokeAdressLabel;
        CEditBox *m_pMemPokeAdress;
        CLabel   *m_pMemPokeValueLabel;
        CEditBox *m_pMemPokeValue;
        CButton  *m_pMemButtonPoke;
        CLabel   *m_pMemFilterLabel;
        CEditBox *m_pMemFilterValue;
        CButton  *m_pMemButtonFilter;
        CButton  *m_pMemButtonSaveFilter;
        CButton  *m_pMemButtonApplyFilter;
        CLabel   *m_pMemAdressLabel;
        CEditBox *m_pMemAdressValue;
        CButton  *m_pMemButtonDisplay;
        CButton  *m_pMemButtonCopy;
        CLabel   *m_pMemBytesPerLineLbl;
        CDropDown *m_pMemBytesPerLine;
        CTextBox *m_pMemTextContent;

        CGroupBox* m_pMemWatchPointsGrp;
        CListBox* m_pMemWatchPoints;
        CEditBox* m_pMemNewWatchPoint;
        CButton *m_pMemAddWatchPoint;
        CButton *m_pMemRemoveWatchPoint;

        CGroupBox* m_pMemConfigGrp;
        CLabel* m_pMemConfigMemLbl;
        CLabel* m_pMemConfigCurLbl;
        CLabel* m_pMemConfigROMLbl;
        CLabel* m_pMemConfigLoLbl;
        CLabel* m_pMemConfigHiLbl;
        CLabel* m_pMemConfigRAMLbl;
        CLabel* m_pMemConfigBankLbl;
        CLabel* m_pMemConfigConfigLbl;

        CCheckBox* m_pMemConfigMemLoROM;
        CCheckBox* m_pMemConfigMemHiROM;
        CDropDown* m_pMemConfigMemRAMBank;
        CDropDown* m_pMemConfigMemRAMConfig;
        CCheckBox* m_pMemConfigCurLoROM;
        CCheckBox* m_pMemConfigCurHiROM;
        CEditBox* m_pMemConfigCurRAMBank;
        CEditBox* m_pMemConfigCurRAMConfig;

        RAMConfig m_MemRAMConfig;

        // Variables for saved filters.
        // Lines currently displayed
        std::vector<word> m_currentlyDisplayed;
        // Lines to filter in (if empty, do not apply saved filter)
        std::vector<word> m_currentlyFiltered;
        // Saved filter (only applied when its content is copied in m_currentlyFiltered)
        std::vector<word> m_savedFilter;

        int m_MemFilterValue;
        int m_MemDisplayValue;
        unsigned int m_MemBytesPerLine;

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
