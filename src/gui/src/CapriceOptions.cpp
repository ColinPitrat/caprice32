// 'Options' window for Caprice32
// Inherited from CFrame

#include <map>
#include <string>
#include "log.h"
#include "std_ex.h"
#include "CapriceOptions.h"
#include "cap32.h"
#include "keyboard.h"
#include "fileutils.h"
#include "wg_messagebox.h"

// CPC emulation properties, defined in cap32.h:
extern t_CPC CPC;
std::vector<std::string> mapFileList;

namespace wGui {

CapriceOptions::CapriceOptions(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
  CFrame(WindowRect, pParent, pFontEngine, "Options", false)
{
    SetModal(true);
    // Make this window listen to incoming CTRL_VALUECHANGE messages (used for updating scrollbar values)
    Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_VALUECHANGE);
    Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_VALUECHANGING);

    // remember the current CPC configuration.
    m_oldCPCsettings = CPC;

    // Navigation bar
    m_pNavigationBar = new CNavigationBar(this, CPoint(10, 5), 6, 50, 50);
    m_pNavigationBar->AddItem(SNavBarItem("General", CPC.resources_path + "/general.bmp"));
    m_pNavigationBar->AddItem(SNavBarItem("ROMs",    CPC.resources_path + "/rom.bmp"));
    m_pNavigationBar->AddItem(SNavBarItem("Video",   CPC.resources_path + "/video.bmp"));
    m_pNavigationBar->AddItem(SNavBarItem("Audio",   CPC.resources_path + "/audio.bmp"));
    m_pNavigationBar->AddItem(SNavBarItem("Input",   CPC.resources_path + "/input.bmp"));
    m_pNavigationBar->SelectItem(0);
    m_pNavigationBar->SetIsFocusable(true);

    // Groupboxes containing controls for each 'tab' (easier to make a 'tab page' visible or invisible)
    m_pGroupBoxTabGeneral   = new CGroupBox(CRect(CPoint(5, 60), m_ClientRect.Width() - 12, m_ClientRect.Height() - 80), this, "");
    m_pGroupBoxTabExpansion = new CGroupBox(CRect(CPoint(5, 60), m_ClientRect.Width() - 12, m_ClientRect.Height() - 80), this, "ROM slots");
    m_pGroupBoxTabVideo     = new CGroupBox(CRect(CPoint(5, 60), m_ClientRect.Width() - 12, m_ClientRect.Height() - 80), this, "");
    m_pGroupBoxTabAudio     = new CGroupBox(CRect(CPoint(5, 60), m_ClientRect.Width() - 12, m_ClientRect.Height() - 80), this, "");
    m_pGroupBoxTabInput     = new CGroupBox(CRect(CPoint(5, 60), m_ClientRect.Width() - 12, m_ClientRect.Height() - 80), this, "");

    // Store associations, see EnableTab method:
    TabMap["general"] = m_pGroupBoxTabGeneral;
    TabMap["expansion"] = m_pGroupBoxTabExpansion;
    TabMap["video"] = m_pGroupBoxTabVideo;
    TabMap["audio"] = m_pGroupBoxTabAudio;
    TabMap["input"] = m_pGroupBoxTabInput;

    // ---------------- 'General' Options ----------------
    m_pLabelCPCModel    = new CLabel(CPoint(10, 3), m_pGroupBoxTabGeneral, "CPC Model");
    m_pDropDownCPCModel = new CDropDown(CRect(CPoint(80, 0), 80, 16), m_pGroupBoxTabGeneral, false);
    m_pDropDownCPCModel->AddItem(SListItem("CPC 464"));
    m_pDropDownCPCModel->AddItem(SListItem("CPC 664"));
    m_pDropDownCPCModel->AddItem(SListItem("CPC 6128"));
    m_pDropDownCPCModel->AddItem(SListItem("CPC 6128+"));
    m_pDropDownCPCModel->SetListboxHeight(4);
       // index and model match, i.e. 0 -> 464, 1 -> 664, 2 -> 6128:
    m_pDropDownCPCModel->SelectItem(CPC.model);
    m_pDropDownCPCModel->SetIsFocusable(true);

    m_pLabelRamSize = new CLabel(CPoint(10, 28), m_pGroupBoxTabGeneral, "RAM memory");
    m_pScrollBarRamSize = new CScrollBar(CRect(CPoint(90, 25), 120, 12), m_pGroupBoxTabGeneral,
                                                                             CScrollBar::HORIZONTAL);
    m_pScrollBarRamSize->SetMinLimit(1); // * 64.  Minimum is 128k if model is 6128!
    m_pScrollBarRamSize->SetMaxLimit(9); // * 64 = 576k
    m_pScrollBarRamSize->SetStepSize(1);  // will multiply by 64. With the current scrollbar, it would otherwise
                                          // (stepsize of 64) be possible to select values that are no multiple
                                          // of 64.
    m_pScrollBarRamSize->SetValue(CPC.ram_size / 64);
    m_pScrollBarRamSize->SetIsFocusable(true);
    m_pLabelRamSizeValue = new CLabel(CPoint(217,28), m_pGroupBoxTabGeneral, stdex::itoa(CPC.ram_size) + "k     ");

    m_pCheckBoxLimitSpeed   = new CCheckBox(CRect(CPoint(10, 49), 10, 10), m_pGroupBoxTabGeneral);
    m_pCheckBoxLimitSpeed->SetIsFocusable(true);
    m_pLabelLimitSpeed      = new CLabel(CPoint(27, 50), m_pGroupBoxTabGeneral, "Limit emulation speed");
    if (CPC.limit_speed == 1) {
        m_pCheckBoxLimitSpeed->SetCheckBoxState(CCheckBox::CHECKED);
    }
    m_pLabelCPCSpeed  = new CLabel(CPoint(10, 71), m_pGroupBoxTabGeneral, "CPC Speed");;
    m_pScrollBarCPCSpeed  = new CScrollBar(CRect(CPoint(78, 68), 120, 12), m_pGroupBoxTabGeneral,
                                                                             CScrollBar::HORIZONTAL);
    m_pScrollBarCPCSpeed->SetMinLimit(MIN_SPEED_SETTING);
    m_pScrollBarCPCSpeed->SetMaxLimit(MAX_SPEED_SETTING);
    m_pScrollBarCPCSpeed->SetStepSize(1);
    m_pScrollBarCPCSpeed->SetValue(CPC.speed);
    m_pScrollBarCPCSpeed->SetIsFocusable(true);
       // Actual emulation speed = value * 25 e.g. 4 -> 100%; values range between 2 and 32
    m_pLabelCPCSpeedValue = new CLabel(CPoint(205, 71), m_pGroupBoxTabGeneral, stdex::itoa(CPC.speed * 25) + "%  ");
    m_pCheckBoxPrinterToFile = new CCheckBox(CRect(CPoint(10, 90), 10, 10), m_pGroupBoxTabGeneral);
    m_pLabelPrinterToFile    = new CLabel(CPoint(27, 91), m_pGroupBoxTabGeneral, "Capture printer output to file");
    if (CPC.printer  == 1) {
        m_pCheckBoxPrinterToFile->SetCheckBoxState(CCheckBox::CHECKED);
    }
    m_pCheckBoxPrinterToFile->SetIsFocusable(true);

    // ---------------- Expansion ROMs ----------------
    std::string romFileName;
    for (unsigned int i = 0; i < 16; i ++) { // create 16 'ROM' buttons
      new CLabel(CPoint((i<8)?5:135, 4 + 18*(i%8)), m_pGroupBoxTabExpansion, stdex::itoa(i));

      if (!CPC.rom_file[i].empty()) { // if CPC.rom_file[i] is not empty
          romFileName = CPC.rom_file[i];
      } else {
          romFileName = "...";
      }
      CButton* romButton = new CButton(CRect(CPoint((i<8)?20:150, 1 + 18*(i%8)), 100, 15), m_pGroupBoxTabExpansion, romFileName);
      romButton->SetIsFocusable(true);
      m_pButtonRoms.push_back(romButton); // element i corresponds with ROM slot i.
    }

    // ---------------- 'Video' options ----------------
    m_pDropDownVideoPlugin = new CDropDown(CRect(CPoint(50,0),120,16), m_pGroupBoxTabVideo, false); // Select video plugin
    unsigned int i = 0;
    for(const auto& plugin : video_plugin_list)
    {
      if (!plugin.hidden) {
        m_pDropDownVideoPlugin->AddItem(SListItem(plugin.name, reinterpret_cast<void*>(i)));
      }
      if (i == CPC.scr_style) {
        m_pDropDownVideoPlugin->SelectItem(m_pDropDownVideoPlugin->Size()-1);
      }
      i++;
    }
    m_pDropDownVideoPlugin->SetListboxHeight(5);
    m_pDropDownVideoPlugin->SetIsFocusable(true);

    m_pLabelVideoPlugin = new CLabel(CPoint(10, 2), m_pGroupBoxTabVideo, "Plugin");

    m_pDropDownVideoScale = new CDropDown(CRect(CPoint(220, 0),50,16), m_pGroupBoxTabVideo, false);
    for(int scale = 1; scale <= 8; scale++)
    {
      std::string scale_str = std::to_string(scale) + "x";
      m_pDropDownVideoScale->AddItem(SListItem(scale_str));
    }
    m_pDropDownVideoScale->SelectItem(CPC.scr_scale-1);
    m_pDropDownVideoScale->SetListboxHeight(5);
    m_pDropDownVideoScale->SetIsFocusable(true);

    m_pLabelVideoScale = new CLabel(CPoint(180, 2), m_pGroupBoxTabVideo, "Scale");

    m_pGroupBoxMonitor   = new CGroupBox(CRect(CPoint(10, 30), 280, 55), m_pGroupBoxTabVideo, "Monitor");
    m_pRadioButtonColour = new CRadioButton(CPoint(10, 1), 10, m_pGroupBoxMonitor); // Colour or monochrome monitor
    m_pLabelColour       = new CLabel(CPoint(27,2), m_pGroupBoxMonitor, "Colour");
    m_pRadioButtonMonochrome   = new CRadioButton(CPoint(10, 16), 10, m_pGroupBoxMonitor); // Colour or monochrome monitor;
    m_pLabelMonochrome         = new CLabel(CPoint(27,17), m_pGroupBoxMonitor, "Mono");
    if (CPC.scr_tube == 1) {
        m_pRadioButtonMonochrome->SetState(CRadioButton::CHECKED);
    } else {
        m_pRadioButtonColour->SetState(CRadioButton::CHECKED);
    }
    m_pRadioButtonColour->SetIsFocusable(true);
    m_pRadioButtonMonochrome->SetIsFocusable(true);

    m_pScrollBarIntensity = new CScrollBar(CRect(CPoint(130, 8), 90, 12), m_pGroupBoxMonitor, CScrollBar::HORIZONTAL);
    m_pScrollBarIntensity->SetMinLimit(5);
    m_pScrollBarIntensity->SetMaxLimit(15);
    m_pScrollBarIntensity->SetStepSize(1);
    m_pScrollBarIntensity->SetValue(CPC.scr_intensity);
    m_pScrollBarIntensity->SetIsFocusable(true);

    m_pLabelIntensity    = new CLabel(CPoint(80, 10), m_pGroupBoxMonitor, "Intensity");

    // intensity values are indeed from 5 to 15, but we display intensity/10 :
    char intensityValue[5];
    sprintf(intensityValue, "%2.1f ", CPC.scr_intensity / 10.0);
    m_pLabelIntensityValue = new CLabel(CPoint(230, 10), m_pGroupBoxMonitor, intensityValue);

    m_pCheckBoxShowFps      = new CCheckBox(CRect(CPoint(10, 90), 10, 10), m_pGroupBoxTabVideo);
    if (CPC.scr_fps == 1) {
        m_pCheckBoxShowFps->SetCheckBoxState(CCheckBox::CHECKED);
    }
    m_pCheckBoxShowFps->SetIsFocusable(true);
    m_pLabelShowFps      = new CLabel(CPoint(27, 91), m_pGroupBoxTabVideo, "Show emulation speed");
    m_pCheckBoxFullScreen   = new CCheckBox(CRect(CPoint(10, 110), 10, 10), m_pGroupBoxTabVideo);
    if (CPC.scr_window == 0) {
        m_pCheckBoxFullScreen->SetCheckBoxState(CCheckBox::CHECKED);
    }
    m_pCheckBoxFullScreen->SetIsFocusable(true);
    m_pLabelFullScreen      = new CLabel(CPoint(27, 111), m_pGroupBoxTabVideo, "Full screen");
    m_pCheckBoxAspectRatio   = new CCheckBox(CRect(CPoint(10, 130), 10, 10), m_pGroupBoxTabVideo);
    if (CPC.scr_preserve_aspect_ratio == 1) {
        m_pCheckBoxAspectRatio->SetCheckBoxState(CCheckBox::CHECKED);
    }
    m_pCheckBoxAspectRatio->SetIsFocusable(true);
    m_pLabelAspectRatio      = new CLabel(CPoint(27, 131), m_pGroupBoxTabVideo, "Preserve aspect ratio");
    // ---------------- 'Audio' Options ----------------
    m_pCheckBoxEnableSound = new CCheckBox(CRect(CPoint(10,0), 10,10), m_pGroupBoxTabAudio);    // Show emulation speed
    if (CPC.snd_enabled == 1) {
        m_pCheckBoxEnableSound->SetCheckBoxState(CCheckBox::CHECKED);
    }
    m_pCheckBoxEnableSound->SetIsFocusable(true);
    m_pLabelEnableSound    = new CLabel(CPoint(28, 1), m_pGroupBoxTabAudio, "Enable Sound Emulation");

    m_pDropDownSamplingRate = new CDropDown(CRect(CPoint(100,25),100,16), m_pGroupBoxTabAudio, false); // Select audio sampling rate
    m_pDropDownSamplingRate->AddItem(SListItem("11025 Hz"));
    m_pDropDownSamplingRate->AddItem(SListItem("22050 Hz"));
    m_pDropDownSamplingRate->AddItem(SListItem("44100 Hz"));
    m_pDropDownSamplingRate->AddItem(SListItem("48000 Hz"));
    m_pDropDownSamplingRate->AddItem(SListItem("96000 Hz"));
    m_pDropDownSamplingRate->SetListboxHeight(4);
    m_pDropDownSamplingRate->SelectItem(CPC.snd_playback_rate);
    m_pDropDownSamplingRate->SetIsFocusable(true);

    m_pLabelSamplingRate = new CLabel(CPoint(10, 27), m_pGroupBoxTabAudio, "Playback Rate");

    m_pGroupBoxChannels    = new CGroupBox(CRect(CPoint(10, 55), 130, 40), m_pGroupBoxTabAudio, "Channels");
    m_pGroupBoxSampleSize  = new CGroupBox(CRect(CPoint(150, 55), 130, 40), m_pGroupBoxTabAudio, "Sample Size");
    m_pRadioButtonMono   = new CRadioButton(CPoint(5, 2), 10, m_pGroupBoxChannels);
    m_pLabelMono         = new CLabel(CPoint(20,3), m_pGroupBoxChannels, "Mono");
    m_pRadioButtonStereo = new CRadioButton(CPoint(55, 2), 10, m_pGroupBoxChannels); // position is within the parent! (groupbox)
    m_pLabelStereo      = new CLabel(CPoint(70,3), m_pGroupBoxChannels, "Stereo");
    if (CPC.snd_stereo == 0) {
        m_pRadioButtonMono->SetState(CRadioButton::CHECKED);
    } else {
      m_pRadioButtonStereo->SetState(CRadioButton::CHECKED);
    }
    m_pRadioButtonMono->SetIsFocusable(true);
    m_pRadioButtonStereo->SetIsFocusable(true);
    m_pRadioButton8bit  = new CRadioButton(CPoint(5, 2), 10, m_pGroupBoxSampleSize);
    m_pLabel8bit        = new CLabel(CPoint(20,3), m_pGroupBoxSampleSize, "8 bit");
    m_pRadioButton16bit = new CRadioButton(CPoint(55, 2), 10, m_pGroupBoxSampleSize);
    m_pLabel16bit       = new CLabel(CPoint(70, 3), m_pGroupBoxSampleSize, "16 bit");
    if (CPC.snd_bits == 0)  {
      m_pRadioButton8bit->SetState(CRadioButton::CHECKED);
    } else {
        m_pRadioButton16bit->SetState(CRadioButton::CHECKED);
    }
    m_pRadioButton8bit->SetIsFocusable(true);
    m_pRadioButton16bit->SetIsFocusable(true);

    m_pLabelSoundVolume    = new CLabel(CPoint(10, 108), m_pGroupBoxTabAudio, "Volume");
    m_pScrollBarVolume     = new CScrollBar(CRect(CPoint(60, 105), 120, 16), m_pGroupBoxTabAudio, CScrollBar::HORIZONTAL);
    m_pScrollBarVolume->SetMinLimit(0);
    m_pScrollBarVolume->SetMaxLimit(100);
    m_pScrollBarVolume->SetStepSize(5);
    m_pScrollBarVolume->SetValue(CPC.snd_volume);
    m_pScrollBarVolume->SetIsFocusable(true);
    m_pLabelSoundVolumeValue = new CLabel(CPoint(190, 108), m_pGroupBoxTabAudio, stdex::itoa(CPC.snd_volume) + "%  ");

    // ---------------- 'Input' Options ----------------
    // option 'keyboard' which is the CPC language
    m_pLabelCPCLanguage    = new CLabel(CPoint(10,3), m_pGroupBoxTabInput, "CPC language");;
    m_pDropDownCPCLanguage = new CDropDown(CRect(CPoint(130,1),140,16), m_pGroupBoxTabInput, false);
    m_pDropDownCPCLanguage->AddItem(SListItem("English CPC"));
    m_pDropDownCPCLanguage->AddItem(SListItem("French CPC"));
    m_pDropDownCPCLanguage->AddItem(SListItem("Spanish CPC"));
    m_pDropDownCPCLanguage->SetListboxHeight(3);
    m_pDropDownCPCLanguage->SelectItem(CPC.keyboard);
    m_pDropDownCPCLanguage->SetIsFocusable(true);
    // option 'kbd_layout' which is the platform keyboard layout (i.e. the PC keyboard layout)
    m_pLabelPCLanguage    = new CLabel(CPoint(10,33), m_pGroupBoxTabInput, "PC Keyboard layout");;
    m_pDropDownPCLanguage = new CDropDown(CRect(CPoint(130,31),140,16), m_pGroupBoxTabInput, false);

    mapFileList = listDirectoryExt(CPC.resources_path, "map");
    unsigned int currentMapIndex = 0;
    for (unsigned int i=0; i < mapFileList.size(); i++) {
        std::string mapFileName = mapFileList[i];
        m_pDropDownPCLanguage->AddItem(SListItem(mapFileName));
        if (mapFileName == CPC.kbd_layout) {
            currentMapIndex = i;
        }
    }
    m_pDropDownPCLanguage->SetListboxHeight(mapFileList.size());
    m_pDropDownPCLanguage->SelectItem(currentMapIndex);
    m_pDropDownPCLanguage->SetIsFocusable(true);

    m_pCheckBoxJoystickEmulation   = new CCheckBox(CRect(CPoint(10, 62), 10, 10), m_pGroupBoxTabInput);
    if (CPC.joystick_emulation == 1) {
        m_pCheckBoxJoystickEmulation->SetCheckBoxState(CCheckBox::CHECKED);
    }
    m_pLabelJoystickEmulation      = new CLabel(CPoint(27, 63), m_pGroupBoxTabInput, "Joystick emulation");
    m_pCheckBoxJoystickEmulation->SetIsFocusable(true);

    m_pCheckBoxJoysticks           = new CCheckBox(CRect(CPoint(10, 92), 10, 10), m_pGroupBoxTabInput);
    if (CPC.joysticks == 1) {
        m_pCheckBoxJoysticks->SetCheckBoxState(CCheckBox::CHECKED);
    }
    m_pLabelJoysticks              = new CLabel(CPoint(27, 93), m_pGroupBoxTabInput, "Joysticks support");
    m_pCheckBoxJoysticks->SetIsFocusable(true);

    EnableTab("general");

    m_pButtonSave   = new CButton(CRect(CPoint(70, m_ClientRect.Height() - 20), 50, 15), this, "Save");
    m_pButtonSave->SetIsFocusable(true);
    m_pButtonCancel = new CButton(CRect(CPoint(130, m_ClientRect.Height() - 20), 50, 15), this, "Cancel");
    m_pButtonCancel->SetIsFocusable(true);
    m_pButtonOk   = new CButton(CRect(CPoint(190, m_ClientRect.Height() - 20), 50, 15), this, "Ok");
    m_pButtonOk->SetIsFocusable(true);
}

CapriceOptions::~CapriceOptions() = default;

bool CapriceOptions::HandleMessage(CMessage* pMessage)
{
  bool bHandled = false;

  if (pMessage)
  {
    LOG_DEBUG("CapriceOptions::HandleMessage for " << CMessage::ToString(pMessage->MessageType()))
    switch(pMessage->MessageType())
    {
      case CMessage::CTRL_SINGLELCLICK:
        {
          if (pMessage->Destination() == this)
          {
            if (pMessage->Source() == m_pButtonCancel) {
              CloseFrame();
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pButtonSave || pMessage->Source() == m_pButtonOk) {
              // save settings + close

              // 'General' settings
              CPC.model    = m_pDropDownCPCModel->GetSelectedIndex();
              CPC.ram_size = m_pScrollBarRamSize->GetValue() * 64;
              CPC.limit_speed = (m_pCheckBoxLimitSpeed->GetCheckBoxState() == CCheckBox::CHECKED)?1:0;
              CPC.speed    = m_pScrollBarCPCSpeed->GetValue();
              CPC.printer  = (m_pCheckBoxPrinterToFile->GetCheckBoxState() == CCheckBox::CHECKED)?1:0;
              // Selected ROM slots ( "..." is empty)
              // Take the text on each 'ROM' button, if it is "...", clear the ROM, else
              // set the ROM filename:
              for (unsigned int i = 0; i < m_pButtonRoms.size(); i ++) {
                std::string romFileName = m_pButtonRoms.at(i)->GetWindowText();
                if (romFileName == "...") {
                  CPC.rom_file[i] = "";
                } else {
                  CPC.rom_file[i] = romFileName;
                }
              }
              // 'Video' settings
              CPC.scr_fps = (m_pCheckBoxShowFps->GetCheckBoxState() == CCheckBox::CHECKED)?1:0;
              CPC.scr_window = (m_pCheckBoxFullScreen->GetCheckBoxState() == CCheckBox::CHECKED)?0:1;
              CPC.scr_preserve_aspect_ratio = (m_pCheckBoxAspectRatio->GetCheckBoxState() == CCheckBox::CHECKED)?1:0;
              CPC.scr_tube = (m_pRadioButtonMonochrome->GetState() == CRadioButton::CHECKED)?1:0;
              CPC.scr_intensity = m_pScrollBarIntensity->GetValue();
              CPC.scr_style = reinterpret_cast<intptr_t>(m_pDropDownVideoPlugin->GetItem(m_pDropDownVideoPlugin->GetSelectedIndex()).pItemData);
              CPC.scr_scale = m_pDropDownVideoScale->GetSelectedIndex()+1;
              // 'Audio' settings
              CPC.snd_enabled = (m_pCheckBoxEnableSound->GetCheckBoxState() == CCheckBox::CHECKED)?1:0;
              // index in listbox = index in array defining sample rate (maybe rewrite this so
              // it's less dependent on this?
              // + todo : audio needs to be restarted if sampling rate has changed!
              CPC.snd_playback_rate = m_pDropDownSamplingRate->GetSelectedIndex();
              CPC.snd_volume = m_pScrollBarVolume->GetValue();
              CPC.snd_stereo = m_pRadioButtonStereo->GetState()==CRadioButton::CHECKED ? 1 : 0;
              CPC.snd_bits   = m_pRadioButton16bit->GetState()==CRadioButton::CHECKED ? 1 : 0;

              // 'Input' settings
              CPC.keyboard = m_pDropDownCPCLanguage->GetSelectedIndex();
              CPC.kbd_layout = mapFileList[m_pDropDownPCLanguage->GetSelectedIndex()];
              CPC.joysticks = (m_pCheckBoxJoysticks->GetCheckBoxState() == CCheckBox::CHECKED)?1:0;
              CPC.joystick_emulation = (m_pCheckBoxJoystickEmulation->GetCheckBoxState() == CCheckBox::CHECKED)?1:0;

              // Check if any reset or re-init is required, e.g. emulator reset, sound system reset...
              if (ProcessOptionChanges(CPC, pMessage->Source() == m_pButtonSave)) {
                CloseFrame();
              }
              bHandled = true;
              break;
            }
          }

          // 'ROM' button clicked: open the ROM selection dialog:
          if (pMessage->Destination() == m_pGroupBoxTabExpansion) {
            for (unsigned int i = 0; i < m_pButtonRoms.size(); i ++) {
              if (pMessage->Source() == m_pButtonRoms.at(i)) {
                pRomSlotsDialog = new wGui::CapriceRomSlots(CRect(
                      CPoint(m_pSDLSurface->w /2 - 140, 30), 250, 200), this, nullptr, "", i, m_pButtonRoms.at(i));
                break;
              }
            }
          }
          break;
        }

      case CMessage::CTRL_VALUECHANGE:
        if (pMessage->Destination() == this) {
          if (pMessage->Source() == m_pNavigationBar) {
            switch (m_pNavigationBar->getSelectedIndex()) {
              case 0 : { // 'General'
                         EnableTab("general");
                         break;
                       }
              case 1 : { // 'Expansion' or 'ROMs'
                         EnableTab("expansion");
                         break;
                       }
              case 2 : { // 'Video'
                         EnableTab("video");
                         break;
                       }
              case 3 : { // 'Audio'
                         EnableTab("audio");
                         break;
                       }
              case 4 : { // 'Input'
                         EnableTab("input");
                         break;
                       }
            }
          }
        }
#if __GNUC__ >= 7
        [[gnu::fallthrough]];
#endif
      case CMessage::CTRL_VALUECHANGING:
        if (pMessage->Destination() == m_pGroupBoxTabGeneral) {
          // Update the CPC speed %
          if (pMessage->Source() == m_pScrollBarCPCSpeed) {
            m_pLabelCPCSpeedValue->SetWindowText(stdex::itoa(m_pScrollBarCPCSpeed->GetValue() * 25) + "%  ");
          }
          // Update the RAM size value:
          if (pMessage->Source() == m_pScrollBarRamSize) {
            // if CPC.model = 2 (CPC 6128), minimum RAM size is 128k:
            int newRamSize = m_pScrollBarRamSize->GetValue();
            if (m_pDropDownCPCModel->GetSelectedIndex() >= 2) { // selection in Dropdown is 'CPC 6128'
              if (newRamSize < 2) {
                newRamSize = 2; // *64k
                m_pScrollBarRamSize->SetValue(2);
              }
            }
            m_pLabelRamSizeValue->SetWindowText(stdex::itoa(newRamSize * 64) + "k     ");
          }

          if (pMessage->Source() == m_pDropDownCPCModel) {
            if (m_pDropDownCPCModel->GetSelectedIndex() >= 2) { // selection changes to 'CPC 6128'
              if (m_pScrollBarRamSize->GetValue() < 2) {
                m_pScrollBarRamSize->SetValue(2);  // *64k
                m_pLabelRamSizeValue->SetWindowText("128k     ");
              }
            }
          }

        }

        // Update the monitor intensity value
        if (pMessage->Destination() == m_pGroupBoxMonitor) {
          if (pMessage->Source() == m_pScrollBarIntensity) {
            char intensityValue[5];
            sprintf(intensityValue, "%2.1f ", m_pScrollBarIntensity->GetValue()/ 10.0);
            m_pLabelIntensityValue->SetWindowText(intensityValue);
          }
        }

        // Update the sound volume %
        if (pMessage->Destination() == m_pGroupBoxTabAudio) {
          if (pMessage->Source() == m_pScrollBarVolume) {
            m_pLabelSoundVolumeValue->SetWindowText(stdex::itoa(m_pScrollBarVolume->GetValue()) + "%  ");
          }
        }
        break;

      default :
        break;
    }
  }
  if (!bHandled) {
    LOG_DEBUG("CapriceOptions::HandleMessage forwarding " << CMessage::ToString(pMessage->MessageType()) << " to CFrame")
    bHandled = CFrame::HandleMessage(pMessage);
  }
  return bHandled;
}

// Enable a 'tab', i.e. make the corresponding CGroupBox (and its content) visible.
void CapriceOptions::EnableTab(std::string sTabName) {
    std::map<std::string, CGroupBox*>::const_iterator iter;
    for (iter=TabMap.begin(); iter != TabMap.end(); ++iter) {
           iter->second->SetVisible(iter->first == sTabName);
    }
}


// Reinitialize parts of Caprice32 depending on options that have changed.
bool CapriceOptions::ProcessOptionChanges(t_CPC& CPC, bool saveChanges) {
    // if one of the following options has changed, re-init the CPC emulation :
    //  - CPC Model
    //  - amount of RAM
    //  - Configuration of expansion ROMs
    //  - new keyboard layouts
    if (CPC.model != m_oldCPCsettings.model || CPC.ram_size != m_oldCPCsettings.ram_size ||
        CPC.keyboard != m_oldCPCsettings.keyboard || CPC.kbd_layout != m_oldCPCsettings.kbd_layout) {
        emulator_init();
    }
    // compare the ROM configuration & call emulator_init if required:
    bool bRomsChanged = false;
    for (int i = 0; i < 16; i ++) {
        if (CPC.rom_file[i] != m_oldCPCsettings.rom_file[i]) {
            bRomsChanged = true;
        }
    }
    if (bRomsChanged) {
        emulator_init();
    }
    // if scr_tube has changed (colour-> mono or mono->colour) or if the intensity value has changed,
    // call video_set_palette():
    if (CPC.scr_tube !=  m_oldCPCsettings.scr_tube || CPC.scr_intensity != m_oldCPCsettings.scr_intensity) {
        video_set_palette();
    }

    // Update CPC emulation speed:
    if (CPC.speed != m_oldCPCsettings.speed) {
        update_cpc_speed();
    }

    // Stop/start capturing printer output:
    if (CPC.printer != m_oldCPCsettings.printer) {
            if (CPC.printer) {
            printer_start();
        } else {
            printer_stop();
        }
    }

    if (CPC.snd_enabled != m_oldCPCsettings.snd_enabled) {
        if (CPC.snd_enabled) { // disabled -> enabled: reinit required in case the user has changed sound
                               // options (sample size etc.) in between.
            audio_shutdown();
            audio_init();
        }

    }

    // Restart audio subsystem if playback rate, sample size or channels (mono/stereo) or volume has changed:
    if (CPC.snd_stereo != m_oldCPCsettings.snd_stereo || CPC.snd_bits != m_oldCPCsettings.snd_bits ||
        CPC.snd_volume != m_oldCPCsettings.snd_volume ||
        CPC.snd_playback_rate != m_oldCPCsettings.snd_playback_rate) {
            // audio restart:
            if (CPC.snd_enabled) {
                audio_shutdown();
                audio_init();
            }
    }

    // Restart video subsystem
    if (CPC.model != m_oldCPCsettings.model || CPC.scr_window != m_oldCPCsettings.scr_window || CPC.scr_style != m_oldCPCsettings.scr_style || CPC.scr_scale != m_oldCPCsettings.scr_scale || CPC.scr_preserve_aspect_ratio != m_oldCPCsettings.scr_preserve_aspect_ratio)
    {
        audio_pause();
        SDL_Delay(20);
        video_shutdown();
        if (video_init())
        {
          LOG_ERROR("Couldn't apply new video settings, reverting to old ones");
          // we failed video init, restore previous plugin
          CPC.scr_style = m_oldCPCsettings.scr_style;
          CPC.scr_scale = m_oldCPCsettings.scr_scale;
          video_init();
        }
        audio_resume();

        Application().MessageServer()->QueueMessage(new CMessage(CMessage::APP_EXIT, nullptr, this));
    }

    // Activate/deactivate joystick emulation
    if (CPC.joystick_emulation != m_oldCPCsettings.joystick_emulation)
    {
       CPC.InputMapper->set_joystick_emulation();
    }

    if (saveChanges)
    {
        std::string configuration_file = getConfigurationFilename(true /* forWrite */);
        if (!saveConfiguration(CPC, configuration_file)) {
          std::string message = "Couldn't save to " + configuration_file;
          wGui::CMessageBox *pMessageBox = new wGui::CMessageBox(CRect(CPoint(m_ClientRect.Width() /2 - 125, m_ClientRect.Height() /2 - 30), 250, 60), this, nullptr, "Saving configuration failed", message, CMessageBox::BUTTON_OK);
          pMessageBox->SetModal(true);
          return false;
        }
    }

    return true;
}

} // namespace wGui
