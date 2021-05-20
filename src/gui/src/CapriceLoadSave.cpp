// 'Load/save' box for Caprice32
// Inherited from CMessageBox

#include "CapriceLoadSave.h"
#include "cap32.h"
#include "slotshandler.h"
#include "cartridge.h"
#include "stringutils.h"
#include "log.h"
#include "wg_messagebox.h"

#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>

#ifdef WINDOWS
#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#endif


// CPC emulation properties, defined in cap32.h:
extern t_CPC CPC;
extern t_drive driveA;
extern t_drive driveB;

namespace wGui {

CapriceLoadSave::CapriceLoadSave(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine) :
  CFrame(WindowRect, pParent, pFontEngine, "Load / Save", false)
{
  SetModal(true);
  // Make this window listen to incoming CTRL_VALUECHANGE messages (used for updating drop down values)
  Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_VALUECHANGE);
  Application().MessageServer()->RegisterMessageClient(this, CMessage::CTRL_VALUECHANGING);

  // File type (.SNA, .DSK, .TAP, .VOC)
  m_pTypeLabel = new CLabel(          CPoint(15, 25),             this, "File type: ");
  m_pTypeValue = new CDropDown( CRect(CPoint(80, 20), 150, 20),    this, false);
#ifndef WITH_IPF
  m_pTypeValue->AddItem(SListItem("Drive A (.dsk)"));
  m_pTypeValue->AddItem(SListItem("Drive B (.dsk)"));
#else
  m_pTypeValue->AddItem(SListItem("Drive A (.dsk/.ipf)"));
  m_pTypeValue->AddItem(SListItem("Drive B (.dsk/.ipf)"));
#endif
  m_pTypeValue->AddItem(SListItem("Snapshot (.sna)"));
  m_pTypeValue->AddItem(SListItem("Tape (.cdt/.voc)"));
  m_pTypeValue->AddItem(SListItem("Cartridge (.cpr)"));
  m_pTypeValue->SetListboxHeight(5);
  m_pTypeValue->SelectItem(0);
  m_pTypeValue->SetIsFocusable(true);
  m_fileSpec = { ".dsk", ".zip" };

  // Action: load / save
  m_pActionLabel = new CLabel(          CPoint(15, 55),             this, "Action: ");
  m_pActionValue = new CDropDown( CRect(CPoint(80, 50), 150, 20),   this, false);
  m_pActionValue->AddItem(SListItem("Load"));
  m_pActionValue->AddItem(SListItem("Save"));
  m_pActionValue->SetListboxHeight(2);
  m_pActionValue->SelectItem(0);
  m_pActionValue->SetIsFocusable(true);

  // Directory
  m_pDirectoryLabel = new CLabel(          CPoint(15, 85),             this, "Directory: ");
  m_pDirectoryValue = new CEditBox( CRect( CPoint(80, 80), 150, 20),    this);
  m_pDirectoryValue->SetWindowText(simplifyDirPath(CPC.current_dsk_path));
  m_pDirectoryValue->SetReadOnly(true);

  // File list
  m_pFilesList = new CListBox(CRect(CPoint(80, 115), 150, 80), this, true);
  m_pFilesList->SetIsFocusable(true);
  UpdateFilesList();

  // File name
  m_pFileNameLabel  = new CLabel(          CPoint(15, 215),              this, "File: ");
  m_pFileNameValue  = new CEditBox( CRect( CPoint(80, 210), 150, 20),    this);
  m_pFileNameValue->SetWindowText("");
  m_pFileNameValue->SetReadOnly(true);

  // Buttons
  m_pCancelButton   = new CButton(  CRect( CPoint(250, 180), 50, 20), this, "Cancel");
  m_pCancelButton->SetIsFocusable(true);
  m_pLoadSaveButton = new CButton(  CRect( CPoint(250, 210), 50, 20), this, "Load");
  m_pLoadSaveButton->SetIsFocusable(true);
}

CapriceLoadSave::~CapriceLoadSave() = default;

bool CapriceLoadSave::HandleMessage(CMessage* pMessage)
{
  bool bHandled = false;

  if (pMessage)
  {
    switch(pMessage->MessageType())
    {
      case CMessage::CTRL_SINGLELCLICK:
        {
          if (pMessage->Destination() == this)
          {
            if (pMessage->Source() == m_pCancelButton) {
              CloseFrame();
              bHandled = true;
              break;
            }
            if (pMessage->Source() == m_pLoadSaveButton) {
              bool actionDone = false;
              std::string filename = m_pFileNameValue->GetWindowText();
              if(!filename.empty()) {
                std::string directory = m_pDirectoryValue->GetWindowText();
                filename = directory + '/' + filename;
                switch (m_pActionValue->GetSelectedIndex()) {
                  case 0: // Load
                    {
                      DRIVE drive;
                      switch (m_pTypeValue->GetSelectedIndex()) {
                        case 0: // Drive A
                          drive = DSK_A;
                          actionDone = true;
                          CPC.current_dsk_path = directory;
                          break;
                        case 1: // Drive B
                          drive = DSK_B;
                          actionDone = true;
                          CPC.current_dsk_path = directory;
                          break;
                        case 2: // Snapshot
                          drive = OTHER;
                          actionDone = true;
                          CPC.current_snap_path = directory;
                          break;
                        case 3: // Tape
                          drive = OTHER;
                          actionDone = true;
                          CPC.current_tape_path = directory;
                          break;
                        case 4: // Cartridge
                          drive = OTHER;
                          actionDone = true;
                          CPC.current_cart_path = directory;
                          break;
                      }
                      if (actionDone) {
                        file_load(filename, drive);
                      }
                      if (m_pTypeValue->GetSelectedIndex() == 4) {
                        emulator_reset();
                      }
                      break;
                    }
                  case 1: // Save
                    // TODO(cpitrat): Ensure the proper extension is present in the filename, otherwise add it.
                    switch (m_pTypeValue->GetSelectedIndex()) {
                      case 0: // Drive A
                        std::cout << "Save dsk A: " << filename << std::endl;
                        dsk_save(filename, &driveA);
                        actionDone = true;
                        break;
                      case 1: // Drive B
                        std::cout << "Save dsk B: " << filename << std::endl;
                        dsk_save(filename, &driveB);
                        actionDone = true;
                        break;
                      case 2: // Snapshot
                        std::cout << "Save snapshot: " << filename << std::endl;
                        snapshot_save(filename);
                        actionDone = true;
                        break;
                      case 3: // Tape
                        {
                          std::cout << "Save tape: " << filename << std::endl;
                          // Unsupported
                          wGui::CMessageBox *pMessageBox = new wGui::CMessageBox(CRect(CPoint(m_ClientRect.Width() /2 - 125, m_ClientRect.Height() /2 - 30), 250, 60), this, nullptr, "Not implemented", "Saving tape not yet implemented", CMessageBox::BUTTON_OK);
                          pMessageBox->SetModal(true);
                          //tape_save(filename);
                          break;
                        }
                      case 4: // Cartridge
                        {
                          std::cout << "Save cartridge: " << filename << std::endl;
                          // Unsupported
                          wGui::CMessageBox *pMessageBox = new wGui::CMessageBox(CRect(CPoint(m_ClientRect.Width() /2 - 125, m_ClientRect.Height() /2 - 30), 250, 60), this, nullptr, "Not implemented", "Saving cartridge not yet implemented", CMessageBox::BUTTON_OK);
                          pMessageBox->SetModal(true);
                          //cpr_save(filename);
                          break;
                        }
                    }
                    break;
                }
              }
              if(actionDone) {
                CloseFrame();
              }
              bHandled = true;
              break;
            }
          }
        }
        break;

      case CMessage::CTRL_VALUECHANGE:
        if (pMessage->Destination() == m_pActionValue) {
          switch (m_pActionValue->GetSelectedIndex()) {
            case 0: // Load
              m_pLoadSaveButton->SetWindowText("Load");
              m_pFileNameValue->SetReadOnly(true);
              break;
            case 1: // Save
              m_pLoadSaveButton->SetWindowText("Save");
              m_pFileNameValue->SetReadOnly(false);
              break;
          }
        }
        if (pMessage->Destination() == m_pTypeValue) {
          switch (m_pTypeValue->GetSelectedIndex()) {
            case 0: // Drive A
              m_pDirectoryValue->SetWindowText(simplifyDirPath(CPC.current_dsk_path));
#ifndef WITH_IPF
              m_fileSpec = { ".dsk", ".zip" };
#else
              m_fileSpec = { ".dsk", ".ipf", ".zip" };
#endif
              UpdateFilesList();
              break;
            case 1: // Drive B
              m_pDirectoryValue->SetWindowText(simplifyDirPath(CPC.current_dsk_path));
              m_fileSpec = { ".dsk", ".zip" };
              UpdateFilesList();
              break;
            case 2: // Snapshot
              m_pDirectoryValue->SetWindowText(simplifyDirPath(CPC.current_snap_path));
              m_fileSpec = { ".sna", ".zip" };
              UpdateFilesList();
              break;
            case 3: // Tape
              m_pDirectoryValue->SetWindowText(simplifyDirPath(CPC.current_tape_path));
              m_fileSpec = { ".cdt", ".voc", ".zip" };
              UpdateFilesList();
              break;
            case 4: // Cartridge
              m_pDirectoryValue->SetWindowText(simplifyDirPath(CPC.current_cart_path));
              m_fileSpec = { ".cpr", ".zip" };
              UpdateFilesList();
              break;
          }
        }
        if (pMessage->Source() == m_pFilesList) {
          int idx = m_pFilesList->getFirstSelectedIndex();
          std::string fn;
          if (idx != -1) {
            fn = m_pFilesList->GetItem(idx).sItemText;
          }
          if(!fn.empty() && fn[fn.size()-1] == '/') {
            m_pDirectoryValue->SetWindowText(simplifyDirPath(m_pDirectoryValue->GetWindowText() + '/' + fn));
            m_pFileNameValue->SetWindowText("");
            UpdateFilesList();
          } else {
            m_pFileNameValue->SetWindowText(fn);
          }
        }
        break;

      default :
        break;
    }
  }
  if (!bHandled) {
    bHandled = CFrame::HandleMessage(pMessage);
  }
  return bHandled;
}

std::string CapriceLoadSave::simplifyDirPath(std::string path)
{
#ifdef WINDOWS
  char simplepath[_MAX_PATH+1];
#else
  char simplepath[PATH_MAX+1];
#endif
  if(realpath(path.c_str(), simplepath) == nullptr) {
    LOG_ERROR("Couldn't simplify path '" << path << "': " << strerror(errno));
    return ".";
  }
  struct stat entry_infos;
  if(stat(simplepath, &entry_infos) != 0) {
    LOG_ERROR("Could not retrieve info on " << simplepath << ": " << strerror(errno));
    return ".";
  }
  if(!S_ISDIR(entry_infos.st_mode)) {
    LOG_ERROR(simplepath << " is not a directory.");
    return ".";
  }
  return std::string(simplepath);
}

bool CapriceLoadSave::MatchCurrentFileSpec(const char* filename)
{
  for(const auto &ext : m_fileSpec) {
    size_t lenFileName = strlen(filename);
    if (lenFileName < ext.size()) continue;
    if (strncasecmp(&(filename[lenFileName-ext.size()]), ext.c_str(), ext.size()) == 0) {
      return true;
    }
  }
  return false;
}

void CapriceLoadSave::UpdateFilesList()
{
  m_pFilesList->ClearItems();

  DIR *dp;
  struct dirent *ep;

  dp = opendir(m_pDirectoryValue->GetWindowText().c_str());
  if(dp == nullptr) {
    LOG_ERROR("Could not open " << m_pDirectoryValue->GetWindowText() << ": " << strerror(errno));
  } else {
    std::vector<std::string> directories;
    std::vector<std::string> files;
    while((ep = readdir(dp)) != nullptr) {
      std::string entry_name = ep->d_name;
      // ep->d_type is always set to DT_UNKNOWN on my computer => use a call to stat to determine if it's a directory
      struct stat entry_infos;
      std::string full_name = m_pDirectoryValue->GetWindowText() + "/" + entry_name;
      if(stat(full_name.c_str(), &entry_infos) != 0) {
        LOG_ERROR("Could not retrieve info on " << full_name << ": " << strerror(errno));
      }
      if(/*ep->d_type == DT_DIR*/S_ISDIR(entry_infos.st_mode) && (ep->d_name[0] != '.' || entry_name == "..")) {
        directories.push_back(entry_name + "/");
      } else if(/*ep->d_type == DT_REG*/S_ISREG(entry_infos.st_mode) && MatchCurrentFileSpec(ep->d_name)) {
        files.push_back(entry_name);
      }
    }
    if(closedir(dp) != 0) {
      LOG_ERROR("Could not close directory: " << strerror(errno));
    }
    std::sort(directories.begin(), directories.end(), stringutils::caseInsensitiveCompare);
    std::sort(files.begin(), files.end(), stringutils::caseInsensitiveCompare);
    for(const auto &directory : directories) {
      m_pFilesList->AddItem(SListItem(directory));
    }
    for(const auto &file : files) {
      m_pFilesList->AddItem(SListItem(file));
    }
  }
}

}
