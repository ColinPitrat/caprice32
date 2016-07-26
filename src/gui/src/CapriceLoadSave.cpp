// 'Load/save' box for Caprice32
// Inherited from CMessageBox

#include "CapriceLoadSave.h"
#include "cap32.h"
#include "cartridge.h"

#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <string>
#include <algorithm>

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
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGE);
  CMessageServer::Instance().RegisterMessageClient(this, CMessage::CTRL_VALUECHANGING);

  // File type (.SNA, .DSK, .TAP, .VOC)
  m_pTypeLabel = new CLabel(          CPoint(15, 25),             this, "File type: ");
  m_pTypeValue = new CDropDown( CRect(CPoint(80, 20), 150, 20),    this, false);
  m_pTypeValue->AddItem(SListItem("Snapshot (.sna)"));
  m_pTypeValue->AddItem(SListItem("Drive A (.dsk)"));
  m_pTypeValue->AddItem(SListItem("Drive B (.dsk)"));
  m_pTypeValue->AddItem(SListItem("Tape (.cdt/.voc)"));
  m_pTypeValue->AddItem(SListItem("Cartridge (.cpr)"));
  m_pTypeValue->SetListboxHeight(5);
  m_pTypeValue->SelectItem(0);
  m_pTypeValue->SetIsFocusable(true);
  m_fileSpec = { ".sna" };

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
  m_pDirectoryValue->SetWindowText(simplifyPath(CPC.snap_path));
  m_pDirectoryValue->SetReadOnly(true);

  // File list
  m_pFilesList = new CListBox(CRect(CPoint(80, 115), 150, 80), this, true, 12);
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

CapriceLoadSave::~CapriceLoadSave() {
}

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
                filename = m_pDirectoryValue->GetWindowText() + '/' + filename;
                switch (m_pActionValue->GetSelectedIndex()) {
                  case 0: // Load
                    switch (m_pTypeValue->GetSelectedIndex()) {
                      case 0: // Snapshot
                        std::cout << "Load snapshot: " << filename << std::endl;
                        snapshot_load(filename.c_str());
                        actionDone = true;
                        break;
                      case 1: // Drive A
                        std::cout << "Load dsk A: " << filename << std::endl;
                        dsk_load(filename.c_str(), &driveA);
                        actionDone = true;
                        break;
                      case 2: // Drive B
                        std::cout << "Load dsk B: " << filename << std::endl;
                        dsk_load(filename.c_str(), &driveB);
                        actionDone = true;
                        break;
                      case 3: // Tape
                        std::cout << "Load tape: " << filename << std::endl;
                        tape_insert(filename.c_str());
                        actionDone = true;
                        break;
                      case 4: // Cartridge
                        std::cout << "Load cartridge: " << filename << std::endl;
                        cpr_load(filename.c_str());
                        emulator_reset(false);
                        actionDone = true;
                        break;
                    }
                    break;
                  case 1: // Save
                    switch (m_pTypeValue->GetSelectedIndex()) {
                      case 0: // Snapshot
                        std::cout << "Save snapshot: " << filename << std::endl;
                        snapshot_save(filename.c_str());
                        actionDone = true;
                        break;
                      case 1: // Drive A
                        std::cout << "Save dsk A: " << filename << std::endl;
                        dsk_save(filename.c_str(), &driveA);
                        actionDone = true;
                        break;
                      case 2: // Drive B
                        std::cout << "Save dsk B: " << filename << std::endl;
                        dsk_save(filename.c_str(), &driveB);
                        actionDone = true;
                        break;
                      case 3: // Tape
                        {
                          std::cout << "Save tape: " << filename << std::endl;
                          // Unsupported
                          wGui::CMessageBox *pMessageBox = new wGui::CMessageBox(CRect(CPoint(m_ClientRect.Width() /2 - 125, m_ClientRect.Height() /2 - 30), 250, 60), this, nullptr, "Not implemented", "Saving tape not yet implemented", CMessageBox::BUTTON_OK);
                          pMessageBox->SetModal(true);
                          //tape_save(filename.c_str());
                          break;
                        }
                      case 4: // Cartridge
                        {
                          std::cout << "Save cartridge: " << filename << std::endl;
                          // Unsupported
                          wGui::CMessageBox *pMessageBox = new wGui::CMessageBox(CRect(CPoint(m_ClientRect.Width() /2 - 125, m_ClientRect.Height() /2 - 30), 250, 60), this, nullptr, "Not implemented", "Saving cartridge not yet implemented", CMessageBox::BUTTON_OK);
                          pMessageBox->SetModal(true);
                          //cpr_save(filename.c_str());
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
            case 0: // Snapshot
              m_pDirectoryValue->SetWindowText(simplifyPath(CPC.snap_path));
              m_fileSpec = { ".sna" };
              UpdateFilesList();
              break;
            case 1: // Drive A
              m_pDirectoryValue->SetWindowText(simplifyPath(CPC.drvA_path));
              m_fileSpec = { ".dsk" };
              UpdateFilesList();
              break;
            case 2: // Drive B
              m_pDirectoryValue->SetWindowText(simplifyPath(CPC.drvB_path));
              m_fileSpec = { ".dsk" };
              UpdateFilesList();
              break;
            case 3: // Tape
              m_pDirectoryValue->SetWindowText(simplifyPath(CPC.tape_path));
              m_fileSpec = { ".cdt", ".voc" };
              UpdateFilesList();
              break;
            case 4: // Cartridge
              m_pDirectoryValue->SetWindowText(simplifyPath(CPC.cart_path));
              m_fileSpec = { ".cpr" };
              UpdateFilesList();
              break;
          }
        }
				if (pMessage->Source() == m_pFilesList) {
					std::string fn = m_pFilesList->GetItem(m_pFilesList->getFirstSelectedIndex()).sItemText;
          if(fn[fn.size()-1] == '/') {
            m_pDirectoryValue->SetWindowText(simplifyPath(m_pDirectoryValue->GetWindowText() + '/' + fn));
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

std::string CapriceLoadSave::simplifyPath(std::string path)
{
  char simplepath[PATH_MAX+1];
  if(realpath(path.c_str(), simplepath) == nullptr) {
    std::cerr << "Couldn't simplify path '" << path << "': " << strerror(errno) << std::endl;
    return path;
  } 
  return std::string(simplepath);
}

bool CapriceLoadSave::MatchCurrentFileSpec(const char* filename)
{
  for(const auto &ext : m_fileSpec) {
    size_t lenFileName = strlen(filename);
    if (lenFileName < ext.size()) continue;
    if (strncmp(&(filename[lenFileName-ext.size()]), ext.c_str(), ext.size()) == 0) {
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
    std::cerr << "Could not open " << m_pDirectoryValue->GetWindowText() << ": " << strerror(errno) << std::endl;
  } else {
    std::vector<std::string> directories;
    std::vector<std::string> files;
    while((ep = readdir(dp)) != nullptr) {
      std::string entry_name = ep->d_name;
      // ep->d_type is always set to DT_UNKNOWN on my computer => use a call to stat to determine if it's a directory
      struct stat entry_infos;
      std::string full_name = m_pDirectoryValue->GetWindowText() + "/" + entry_name;
      if(stat(full_name.c_str(), &entry_infos) != 0) {
        std::cerr << "Could not retrieve info on " << full_name << ": " << strerror(errno) << std::endl;
      }
      if(/*ep->d_type == DT_DIR*/S_ISDIR(entry_infos.st_mode) && (ep->d_name[0] != '.' || entry_name == "..")) {
        directories.push_back(entry_name + "/");
      } else if(/*ep->d_type == DT_REG*/S_ISREG(entry_infos.st_mode) && MatchCurrentFileSpec(ep->d_name)) {
        files.push_back(entry_name);
      }
    }
    if(closedir(dp) != 0) {
      std::cerr << "Could not close directory: " << strerror(errno) << std::endl;
    }
    std::sort(directories.begin(), directories.end());
    std::sort(files.begin(), files.end());
    for(std::vector<std::string>::iterator it = directories.begin(); it != directories.end(); ++it) {
      m_pFilesList->AddItem(SListItem(*it));
    }
    for(std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it) {
      m_pFilesList->AddItem(SListItem(*it));
    }
  }
}

}
