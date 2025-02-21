// 'DevTools' window for Caprice32

#ifndef _WG_CAPRICE32DEVTOOLS_H_
#define _WG_CAPRICE32DEVTOOLS_H_

#include "z80_disassembly.h"
#include "cap32.h"
#include "symfile.h"
#include "types.h"
#include "cap_flag.h"
#include "cap_register.h"
#include "wg_checkbox.h"
#include "wg_dropdown.h"
#include "wg_frame.h"
#include "wg_groupbox.h"
#include "wg_label.h"
#include "wg_listbox.h"
#include "wg_textbox.h"
#include "wg_tooltip.h"
#include "wg_navigationbar.h"
#include <map>
#include <string>

class DevTools;

namespace wGui
{
    enum class Format {
      Hex,
      Char,
      U8,
      U16,
      U32,
      I8,
      I16,
      I32
    };

    enum class SearchFrom {
      Start, // Actually searching from end when searching backward
      PositionIncluded,
      PositionExcluded,
    };

    enum class SearchDir {
      Forward,
      Backward
    };

    int FormatSize(Format f);
    std::ostream& operator<<(std::ostream& os, const Format& f);
    std::ostream& operator<<(std::ostream& os, const std::vector<Format>& f);

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

        void LoadSymbols(const std::string& filename);

        //! Prepare the update by saving the CPC state and updating this devtool's state.
        //! This is separated from Step2 to allow having multiple devtools.
        void PreUpdate();
        //! The part of the update that modifies the CPC state.
        void PostUpdate();

        bool HandleMessage(CMessage* pMessage) override;

        // activate the specified tab (make its controls visible)
        void EnableTab(std::string sTabName);

        void CloseFrame() override;

        void UpdateAll();

        // Exposed for testing
        void SetDisassembly(std::vector<SListItem> items);
        std::vector<SListItem> GetSelectedAssembly();
        void SetAssemblySearch(const std::string& text);
        void AsmSearch(SearchFrom from, SearchDir dir);

      protected:
        void PauseExecution();
        void ResumeExecution();

        byte ReadMem(word address);
        void WriteMem(word address, byte value);
        void PrepareMemBankConfig();

        void UnlockRegisters();
        void LockRegisters();
        void SaveRegisters();

        void RefreshDisassembly();
        void UpdateZ80();
        void UpdateDisassembly();
        void UpdateDisassemblyPos();
        void UpdateEntryPointsList();
        void UpdateBreakPointsList();
        void UpdateWatchPointsList();
        void UpdateMemConfig();
        void UpdateTextMemory();
        void UpdateAudio();

        void RemoveEphemeralBreakpoints();

        CButton* m_pButtonStepOut;
        CButton* m_pButtonStepIn;
        CButton* m_pButtonStepOver;
        CButton* m_pButtonPause;
        CButton* m_pButtonClose;

        CToolTip* m_pToolTipStepIn;
        CToolTip* m_pToolTipStepOut;
        CToolTip* m_pToolTipStepOver;

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
        CFlag* m_pZ80FlagS;
        CFlag* m_pZ80FlagZ;
        CFlag* m_pZ80FlagX1;
        CFlag* m_pZ80FlagH;
        CFlag* m_pZ80FlagX2;
        CFlag* m_pZ80FlagPV;
        CFlag* m_pZ80FlagN;
        CFlag* m_pZ80FlagC;

        CButton* m_pZ80ModifyRegisters;

        // Stack
        CLabel* m_pZ80StackLabel;
        CListBox* m_pZ80Stack;

        // Assembly screen
        CListBox *m_pAssemblyCode;
        CLabel *m_pAssemblySearchLbl;
        CEditBox *m_pAssemblySearch;
        CButton *m_pAssemblySearchPrev;
        CButton *m_pAssemblySearchNext;
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
        Symfile m_Symfile;
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
        CLabel   *m_pMemFormatLbl;
        CDropDown *m_pMemFormat;
        CTextBox *m_pMemTextContent;

        CGroupBox* m_pMemWatchPointsGrp;
        CListBox* m_pMemWatchPoints;
        CEditBox* m_pMemNewWatchPoint;
        CButton *m_pMemAddWatchPoint;
        CButton *m_pMemRemoveWatchPoint;
        CDropDown* m_pMemWatchPointType;

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
        std::vector<Format> m_MemFormat;

        // Video screen
        CLabel* m_pVidLabel;

        // Audio screen
        CGroupBox* m_pAudPSGGrp;
        CLabel* m_pAudChannelALabel;
        CLabel* m_pAudChannelBLabel;
        CLabel* m_pAudChannelCLabel;
        CLabel* m_pAudNoiseLabel;

        CLabel* m_pAudToneLabel;
        CLabel* m_pAudVolLabel;
        CLabel* m_pAudToneOnOffLabel;
        CLabel* m_pAudNoiseOnOffLabel;

        CLabel* m_pAudFreqALabel;
        CLabel* m_pAudFreqBLabel;
        CLabel* m_pAudFreqCLabel;
        CLabel* m_pAudFreqNoiseLabel;
        CLabel* m_pAudMixerControlLabel;
        CLabel* m_pAudVolALabel;
        CLabel* m_pAudVolBLabel;
        CLabel* m_pAudVolCLabel;

        CLabel* m_pAudVolEnvFreqLabel;
        CLabel* m_pAudVolEnvShapeLabel;
        CLabel* m_pAudFreqA;
        CLabel* m_pAudFreqB;
        CLabel* m_pAudFreqC;
        CLabel* m_pAudFreqNoise;
        CLabel* m_pAudVolA;
        CLabel* m_pAudVolB;
        CLabel* m_pAudVolC;
        CCheckBox* m_pAudToneA;
        CCheckBox* m_pAudToneB;
        CCheckBox* m_pAudToneC;
        CCheckBox* m_pAudNoiseA;
        CCheckBox* m_pAudNoiseB;
        CCheckBox* m_pAudNoiseC;

        CLabel* m_pAudMixerControl;
        CLabel* m_pAudVolEnvFreq;
        CLabel* m_pAudVolEnvShape;

        // Characters screen
        CLabel* m_pChrLabel;

        DevTools* m_pDevTools;

        bool registersLocked;

      private:

        std::map<std::string, CGroupBox*> TabMap;  // mapping: <tab name> -> <groupbox that contains the 'tab'>.
        
        CapriceDevTools(const CapriceDevTools&) = delete;
        CapriceDevTools& operator=(const CapriceDevTools&) = delete;
    };
}

#endif  // _WG_CAPRICE32DEVTOOLS_H_
