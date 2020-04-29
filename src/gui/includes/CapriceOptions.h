// 'Options' window for Caprice32
// Inherited from CFrame

#ifndef _WG_CAPRICE32OPTIONS_H_
#define _WG_CAPRICE32OPTIONS_H_

#include "wgui.h"
#include "wg_frame.h"
#include "wg_label.h"
#include "wg_checkbox.h"
#include "wg_navigationbar.h"
#include "wg_radiobutton.h"
#include "wg_groupbox.h"
#include "wg_scrollbar.h"
#include "CapriceRomSlots.h"
#include "cap32.h"
#include "video.h"

namespace wGui
{
    class CapriceOptions : public CFrame {
      public:
        //! \param pParent A pointer to the parent view
        //! \param pFontEngine A pointer to the font engine to use when drawing the control
        //! If this is set to 0 it will use the default font engine specified by the CApplication (which must be set before instantiating this object)
        CapriceOptions(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine);
        ~CapriceOptions() override;

        bool HandleMessage(CMessage* pMessage) override;

        // activate the specified tab (make its controls visible)
        void EnableTab(std::string sTabName);

      protected:
        CButton* m_pButtonSave;
        CButton* m_pButtonCancel;
        CButton* m_pButtonOk;

        // subdialogs that can be opened from the options dialog, e.g. ROM selection:
        CapriceRomSlots* pRomSlotsDialog;

      // New navigation bar control (to select the different pages or tabs on the options dialog)
      CNavigationBar* m_pNavigationBar; 

      // groupbox to group the controls on each 'tab':
      CGroupBox* m_pGroupBoxTabGeneral;
      CGroupBox* m_pGroupBoxTabExpansion;
      CGroupBox* m_pGroupBoxTabVideo;
      CGroupBox* m_pGroupBoxTabAudio;
      CGroupBox* m_pGroupBoxTabDisk;
      CGroupBox* m_pGroupBoxTabInput;

      // General options
      CLabel* m_pLabelCPCModel;   
      CDropDown* m_pDropDownCPCModel;    // CPC model (464,664,6128...)

      CLabel* m_pLabelRamSize;           // amount of RAM memory (64k up to 576k)
      CScrollBar* m_pScrollBarRamSize;
      CLabel* m_pLabelRamSizeValue;

      CCheckBox* m_pCheckBoxLimitSpeed;  // 'Limit emulation speed' (to original CPC speed)
      CLabel* m_pLabelLimitSpeed;        // text label for above checkbox

      CLabel* m_pLabelCPCSpeed;          // CPC emulation speed
      CScrollBar* m_pScrollBarCPCSpeed;
      CLabel* m_pLabelCPCSpeedValue;

      CLabel* m_pLabelPrinterToFile;     // Capture printer output to file
      CCheckBox* m_pCheckBoxPrinterToFile;

      // Expansion ROMs
      std::vector<CButton *> m_pButtonRoms; // contains pointer to 16 'ROM buttons'

      // Video options
      CLabel* m_pLabelShowFps;           // text label for above checkbox
      CCheckBox* m_pCheckBoxShowFps;     // Show emulation speed
      CLabel* m_pLabelFullScreen;
      CCheckBox* m_pCheckBoxFullScreen; // Full screen toggle
      CGroupBox* m_pGroupBoxMonitor;
      CRadioButton* m_pRadioButtonColour; // Colour or monochrome monitor
      CLabel* m_pLabelColour;
      CRadioButton* m_pRadioButtonMonochrome;
      CLabel* m_pLabelMonochrome;
      CScrollBar* m_pScrollBarIntensity;  // Monitor intensity (default 1.0)
      CLabel* m_pLabelIntensity;
      CLabel* m_pLabelIntensityValue;
      CDropDown* m_pDropDownVideoPlugin; // Select video plugin
      CLabel* m_pLabelVideoPlugin;

      // Audio options
      CCheckBox* m_pCheckBoxEnableSound;    // Show emulation speed
      CLabel* m_pLabelEnableSound;

      CDropDown* m_pDropDownSamplingRate; // Select audio sampling rate
      CLabel* m_pLabelSamplingRate;

      CGroupBox* m_pGroupBoxChannels; 
      CGroupBox* m_pGroupBoxSampleSize;

      CLabel* m_pLabelSoundVolume;
      CScrollBar* m_pScrollBarVolume;
      CLabel* m_pLabelSoundVolumeValue;

      CRadioButton* m_pRadioButtonMono;
      CLabel* m_pLabelMono;
      CRadioButton* m_pRadioButtonStereo;
      CLabel* m_pLabelStereo;
      CRadioButton* m_pRadioButton8bit;
      CLabel* m_pLabel8bit;
      CRadioButton* m_pRadioButton16bit;
      CLabel* m_pLabel16bit;

      // Disk options
      CGroupBox* m_pGroupBoxDriveA;
      CGroupBox* m_pGroupBoxDriveB;
      CDropDown* m_pDropDownDriveAFormat;
      CLabel* m_pLabelDriveAFormat;
      CDropDown* m_pDropDownDriveBFormat;
      CLabel* m_pLabelDriveBFormat;

      // Input options
      CLabel* m_pLabelCPCLanguage;
      CDropDown* m_pDropDownCPCLanguage;
      CLabel* m_pLabelPCLanguage;
      CDropDown* m_pDropDownPCLanguage;
      CLabel* m_pLabelJoystickEmulation;
      CCheckBox* m_pCheckBoxJoystickEmulation;
      CLabel* m_pLabelJoysticks;
      CCheckBox* m_pCheckBoxJoysticks;

      t_CPC m_oldCPCsettings;  // we will store the current CPC settings in this variable, and 
                               // when clicking OK in the options screen, check what options have changed
                               // and take a required action (e.g. emulator reset, sound system reset...)
      bool ProcessOptionChanges(t_CPC& CPC, bool saveChanges); // see m_oldCPCsettings

      private:

        std::map<std::string, CGroupBox*> TabMap;  // mapping: <tab name> -> <groupbox that contains the 'tab'>.
        
        CapriceOptions(const CapriceOptions&) = delete;
        CapriceOptions& operator=(const CapriceOptions&) = delete;
    };
}

#endif  // _WG_CAPRICE32OPTIONS_H_
