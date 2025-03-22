/* Caprice32 - Amstrad CPC Emulator
   (c) Copyright 1997-2005 Ulrich Doewich

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include "cap32.h"
#include "disk.h"
#include "slotshandler.h"
#include "ipf.h"

#include "errors.h"
#include "cartridge.h"
#include "log.h"
#include "fileutils.h"
#include "stringutils.h"
#include "tape.h"
#include "z80.h"
#include "zip.h"

extern t_CPC CPC;
extern t_CRTC CRTC;
extern t_FDC FDC;
extern t_VDU VDU;
extern t_GateArray GateArray;
extern t_PPI PPI;
extern t_PSG PSG;
extern t_drive driveA;
extern t_drive driveB;
extern t_z80regs z80;
extern byte bit_values[8];
extern t_flags1 flags1;
extern word MaxVSync;
extern byte *pbROM;
extern std::string chROMFile[];

byte *pbTapeImageEnd = nullptr;
extern std::vector<byte> pbTapeImage;
extern byte *pbGPBuffer;
extern byte *pbRAM;
extern byte *pbRAMbuffer;

extern FILE *pfileObject;

struct file_loader
{
  DRIVE drive;
  std::string extension;
  int (*load_from_filename)(const std::string& filename);
  int (*load_from_file)(FILE *file);
};

file_loader files_loader_list[] =
{
  { DRIVE::DSK_A, ".dsk",
    [](const std::string& filename) -> int { return dsk_load(filename, &driveA); },
    [](FILE* file) -> int { return dsk_load(file, &driveA); } },

  { DRIVE::DSK_B, ".dsk",
    [](const std::string& filename) -> int { return dsk_load(filename, &driveB); },
    [](FILE* file) -> int { return dsk_load(file, &driveB); } },

  { DRIVE::DSK_A, ".ipf",
    [](const std::string& filename) -> int { return ipf_load(filename, &driveA); },
    [](FILE* file) -> int { return ipf_load(file, &driveA); } },

  { DRIVE::DSK_B, ".ipf",
    [](const std::string& filename) -> int { return ipf_load(filename, &driveB); },
    [](FILE* file) -> int { return ipf_load(file, &driveB); } },

  { DRIVE::DSK_A, ".raw",
    [](const std::string& filename) -> int { return ipf_load(filename, &driveA); },
    [](FILE* file) -> int { return ipf_load(file, &driveA); } },

  { DRIVE::DSK_B, ".raw",
    [](const std::string& filename) -> int { return ipf_load(filename, &driveB); },
    [](FILE* file) -> int { return ipf_load(file, &driveB); } },

  { DRIVE::SNAPSHOT, ".sna",
    &snapshot_load,
    &snapshot_load },

  { DRIVE::TAPE, ".cdt",
    &tape_insert,
    &tape_insert },

  { DRIVE::TAPE, ".voc",
    &tape_insert,
    &tape_insert },

  { DRIVE::CARTRIDGE, ".cpr",
    &cartridge_load,
    &cartridge_load },
};

t_disk_format disk_format[MAX_DISK_FORMAT] = {
   { "178K Data Format", 40, 1, 9, 2, 0x52, 0xe5, {{ 0xc1, 0xc6, 0xc2, 0xc7, 0xc3, 0xc8, 0xc4, 0xc9, 0xc5 }} },
   { "169K Vendor Format", 40, 1, 9, 2, 0x52, 0xe5, {{ 0x41, 0x46, 0x42, 0x47, 0x43, 0x48, 0x44, 0x49, 0x45 }} }
};

inline bool fillSlot(t_slot& slot, bool &processedvar, const std::string& fullpath, const std::string& extension, const std::string& type_ext, const std::string& type_desc) {
   if ((!processedvar) && (extension == type_ext)) {
      LOG_VERBOSE("Loading " << type_desc << " file: " << fullpath);
      slot.file = fullpath;
      slot.zip_index = 0;
      processedvar = true;
      return true;
   }
   return false;
}

// Parses a list of files and fill in the relevant CPC fields
// according to what is found.
// All we do here is fill the proper xxx_file entry.
void fillSlots (std::vector<std::string> slot_list, t_CPC& CPC)
{
   bool have_DSKA = false;
   bool have_DSKB = false;
   bool have_SNA = false;
   bool have_TAP = false;
   bool have_CPR = false;

   for (const auto& slot : slot_list) { // loop for all command line arguments
      LOG_DEBUG("Handling arg " << slot);
      std::string fullpath = stringutils::trim(slot, '"'); // remove quotes if arguments quoted
      if (fullpath.length() > 5) { // minumum for a valid filename
         int pos = fullpath.length() - 4;
         std::string extension = stringutils::lower(fullpath.substr(pos));

         if (extension == ".zip") { // are we dealing with a zip archive?
           zip::t_zip_info zip_info;
           zip_info.filename = fullpath;
           zip_info.extensions = ".dsk.sna.cdt.voc.cpr.ipf.raw";
           if (zip::dir(&zip_info)) {
             continue; // error or nothing relevant found
           }
           std::string filename = zip_info.filesOffsets[0].first;
           pos = filename.length() - 4;
           extension = filename.substr(pos); // grab the extension
         }

         if (fillSlot(CPC.driveA, have_DSKA, fullpath, extension, ".dsk", "drive A disk"))
           continue;
         if (fillSlot(CPC.driveA, have_DSKA, fullpath, extension, ".ipf", "drive A disk (IPF)"))
            continue;
         if (fillSlot(CPC.driveA, have_DSKA, fullpath, extension, ".raw", "drive A disk (CT-RAW)"))
           continue;
         if (fillSlot(CPC.driveB, have_DSKB, fullpath, extension, ".dsk", "drive B disk"))
            continue;
         if (fillSlot(CPC.driveB, have_DSKB, fullpath, extension, ".ipf", "drive B disk (IPF)"))
            continue;
         if (fillSlot(CPC.driveB, have_DSKB, fullpath, extension, ".raw", "drive B disk (CT-IPF)"))
            continue;
         if (fillSlot(CPC.snapshot, have_SNA, fullpath, extension, ".sna", "CPC state snapshot"))
            continue;
         if (fillSlot(CPC.tape, have_TAP, fullpath, extension, ".cdt", "tape (CDT)"))
            continue;
         if (fillSlot(CPC.tape, have_TAP, fullpath, extension, ".voc", "tape (VOC)"))
            continue;
         if (fillSlot(CPC.cartridge, have_CPR, fullpath, extension, ".cpr", "cartridge"))
            continue;
      }
   }
}

void loadSlots() {
   memset(&driveA, 0, sizeof(t_drive)); // clear disk drive A data structure
   file_load(CPC.driveA);
   memset(&driveB, 0, sizeof(t_drive)); // clear disk drive B data structure
   file_load(CPC.driveB);
   file_load(CPC.tape);
   file_load(CPC.snapshot);
   // Cartridge was loaded by emulator_init which called cartridge_load if needed
}

// Extract 'filename' from 'zipfile'. Filename must end with one of the extensions listed in 'ext'.
// FILE handle returned must be closed once finished with.
// nullptr is returned if file couldn't be extracted for any reason.
FILE *extractFile(const std::string& zipfile, const std::string& filename, const std::string& ext) {
  zip::t_zip_info zip_info;
  zip_info.filename = zipfile;
  zip_info.extensions = ext;
  if (!zip::dir(&zip_info)) { // parse the zip for relevant files
    for (const auto& fn : zip_info.filesOffsets) {
      if (!strcasecmp(filename.c_str(), fn.first.c_str())) { // do we have a match?
        FILE *file = nullptr;
        zip_info.dwOffset = fn.second; // get the offset into the zip archive
        if (!zip::extract(zip_info, &file)) {
          return file;
        }
      }
    }
  }
  return nullptr;
}

t_disk_format parseDiskFormat(const std::string& format)
{
  t_disk_format result;
  dword dwVal;
  std::vector<std::string> tokens = stringutils::split(format, ',');
  if (tokens.size() < 7) { // Minimum number of values required
    return result;
  }
  dwVal = strtoul(tokens[1].c_str(), nullptr, 0);
  if ((dwVal < 1) || (dwVal > DSK_TRACKMAX)) { // invalid value?
    return result;
  }
  result.tracks = dwVal;
  dwVal = strtoul(tokens[2].c_str(), nullptr, 0);
  if ((dwVal < 1) || (dwVal > DSK_SIDEMAX)) { // invalid value?
    return result;
  }
  result.sides = dwVal;
  dwVal = strtoul(tokens[3].c_str(), nullptr, 0);
  if ((dwVal < 1) || (dwVal > DSK_SECTORMAX)) { // invalid value?
    return result;
  }
  result.sectors = dwVal;
  dwVal = strtoul(tokens[4].c_str(), nullptr, 0);
  if ((dwVal < 1) || (dwVal > 6)) { // invalid value?
    return result;
  }
  result.sector_size = dwVal;
  dwVal = strtoul(tokens[5].c_str(), nullptr, 0);
  if ((dwVal < 1) || (dwVal > 255)) { // invalid value?
    return result;
  }
  result.gap3_length = dwVal;
  dwVal = strtoul(tokens[6].c_str(), nullptr, 0);
  result.filler_byte = static_cast<byte>(dwVal);
  unsigned int i = 7;
  for (int iSide = 0; iSide < static_cast<int>(result.sides); iSide++) {
    for (int iSector = 0; iSector < static_cast<int>(result.sectors); iSector++) {
      if (i >= tokens.size()) { // value missing?
        dwVal = iSector+1;
      } else {
        dwVal = strtoul(tokens[i++].c_str(), nullptr, 0);
      }
      result.sector_ids[iSide][iSector] = static_cast<byte>(dwVal);
    }
  }
  // Fill the label only if the disk format is valid
  result.label = tokens[0];
  return result;
}

std::string serializeDiskFormat(const t_disk_format& format)
{
  std::ostringstream oss;
  if (!format.label.empty()) {
    oss << format.label << ",";
    oss << format.tracks << ",";
    oss << format.sides << ",";
    oss << format.sectors << ",";
    oss << format.sector_size << ",";
    oss << format.gap3_length << ",";
    oss << static_cast<unsigned int>(format.filler_byte);
    for (int iSide = 0; iSide < static_cast<int>(format.sides); iSide++) {
      for (int iSector = 0; iSector < static_cast<int>(format.sectors); iSector++) {
        oss << "," << static_cast<unsigned int>(format.sector_ids[iSide][iSector]);
      }
    }
  }
  return oss.str();
}

int snapshot_save (const std::string &filename)
{
   t_SNA_header sh;
   dword dwFlags;

   memset(&sh, 0, sizeof(sh));
   memcpy(sh.id, "MV - SNA", sizeof(sh.id));
   sh.version = 3;
// Z80
   sh.AF[1] = z80.AF.b.h;
   sh.AF[0] = z80.AF.b.l;
   sh.BC[1] = z80.BC.b.h;
   sh.BC[0] = z80.BC.b.l;
   sh.DE[1] = z80.DE.b.h;
   sh.DE[0] = z80.DE.b.l;
   sh.HL[1] = z80.HL.b.h;
   sh.HL[0] = z80.HL.b.l;
   sh.R = (z80.R & 0x7f) | (z80.Rb7 & 0x80);
   sh.I = z80.I;
   if (z80.IFF1)
      sh.IFF0 = 1;
   if (z80.IFF2)
      sh.IFF1 = 1;
   sh.IX[1] = z80.IX.b.h;
   sh.IX[0] = z80.IX.b.l;
   sh.IY[1] = z80.IY.b.h;
   sh.IY[0] = z80.IY.b.l;
   sh.SP[1] = z80.SP.b.h;
   sh.SP[0] = z80.SP.b.l;
   sh.PC[1] = z80.PC.b.h;
   sh.PC[0] = z80.PC.b.l;
   sh.IM = z80.IM;
   sh.AFx[1] = z80.AFx.b.h;
   sh.AFx[0] = z80.AFx.b.l;
   sh.BCx[1] = z80.BCx.b.h;
   sh.BCx[0] = z80.BCx.b.l;
   sh.DEx[1] = z80.DEx.b.h;
   sh.DEx[0] = z80.DEx.b.l;
   sh.HLx[1] = z80.HLx.b.h;
   sh.HLx[0] = z80.HLx.b.l;
// Gate Array
   sh.ga_pen = GateArray.pen;
   for (int n = 0; n < 17; n++) { // loop for all colours + border
      sh.ga_ink_values[n] = GateArray.ink_values[n];
   }
   sh.ga_ROM_config = GateArray.ROM_config;
   sh.ga_RAM_config = GateArray.RAM_config;
// CRTC
   sh.crtc_reg_select = CRTC.reg_select;
   for (int n = 0; n < 18; n++) { // loop for all CRTC registers
      sh.crtc_registers[n] = CRTC.registers[n];
   }
// ROM select
   sh.upper_ROM = GateArray.upper_ROM;
// PPI
   sh.ppi_A = PPI.portA;
   sh.ppi_B = PPI.portB;
   sh.ppi_C = PPI.portC;
   sh.ppi_control = PPI.control;
// PSG
   sh.psg_reg_select = PSG.reg_select;
   for (int n = 0; n < 16; n++) { // loop for all PSG registers
      sh.psg_registers[n] = PSG.RegisterAY.Index[n];
   }

   sh.ram_size[0] = CPC.ram_size & 0xff;
   sh.ram_size[1] = (CPC.ram_size >> 8) & 0xff;
// version 2 info
   sh.cpc_model = CPC.model;
// version 3 info
   sh.fdc_motor = FDC.motor;
   sh.drvA_current_track = driveA.current_track;
   sh.drvB_current_track = driveB.current_track;
   sh.printer_data = CPC.printer_port ^ 0x80; // invert bit 7 again
   sh.psg_env_step = PSG.AmplitudeEnv >> 1; // divide by 2 to bring it into the 0 - 15 range
   if (PSG.FirstPeriod) {
      switch (PSG.RegisterAY.EnvType)
      {
         case 0:
         case 1:
         case 2:
         case 3:
         case 8:
         case 9:
         case 10:
         case 11:
            sh.psg_env_direction = 0xff; // down
            break;
         case 4:
         case 5:
         case 6:
         case 7:
         case 12:
         case 13:
         case 14:
         case 15:
            sh.psg_env_direction = 0x01; // up
            break;
      }
   } else {
      switch (PSG.RegisterAY.EnvType)
      {
         case 0:
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
         case 6:
         case 7:
         case 9:
         case 11:
         case 13:
         case 15:
            sh.psg_env_direction = 0x00; // hold
            break;
         case 8:
         case 14:
            sh.psg_env_direction = 0xff; // down
            break;
         case 10:
         case 12:
            sh.psg_env_direction = 0x01; // up
            break;
      }
   }
   sh.crtc_addr[0] = CRTC.addr & 0xff;
   sh.crtc_addr[1] = (CRTC.addr >> 8) & 0xff;
   sh.crtc_scanline[0] = VDU.scanline & 0xff;
   sh.crtc_scanline[1] = (VDU.scanline >> 8) & 0xff;
   sh.crtc_char_count[0] = CRTC.char_count;
   sh.crtc_line_count = CRTC.line_count;
   sh.crtc_raster_count = CRTC.raster_count;
   sh.crtc_hsw_count = CRTC.hsw_count;
   sh.crtc_vsw_count = CRTC.vsw_count;
   dwFlags = 0;
   if (CRTC.flag_invsync) { // vsync active?
      dwFlags |= 1;
   }
   if (flags1.inHSYNC) { // hsync active?
      dwFlags |= 2;
   }
   if (CRTC.flag_invta) { // in vertical total adjust?
      dwFlags |= 0x80;
   }
   sh.crtc_flags[0] = dwFlags & 0xff;
   sh.crtc_flags[1] = (dwFlags >> 8) & 0xff;
   sh.ga_int_delay = GateArray.hs_count;
   sh.ga_sl_count = GateArray.sl_count;
   sh.z80_int_pending = z80.int_pending;

   if ((pfileObject = fopen(filename.c_str(), "wb")) != nullptr) {
      if (fwrite(&sh, sizeof(sh), 1, pfileObject) != 1) { // write snapshot header
         fclose(pfileObject);
         return ERR_SNA_WRITE;
      }
      if (fwrite(pbRAM, CPC.ram_size*1024, 1, pfileObject) != 1) { // write memory contents to snapshot file
         fclose(pfileObject);
         return ERR_SNA_WRITE;
      }
      fclose(pfileObject);
   } else {
      return ERR_SNA_WRITE;
   }

   return 0;
}

void dsk_eject (t_drive *drive)
{
   dword track, side;

   if (drive->eject_hook)
     drive->eject_hook(drive); // additional cleanup

   for (track = 0; track < DSK_TRACKMAX; track++) { // loop for all tracks
      for (side = 0; side < DSK_SIDEMAX; side++) { // loop for all sides
        delete [] drive->track[track][side].data; // release memory allocated for this track
      }
   }
   dword dwTemp = drive->current_track; // save the drive head position
   memset(drive, 0, sizeof(t_drive)); // clear drive info structure
   drive->current_track = dwTemp;
}

int dsk_load (FILE *pfile, t_drive *drive)
{
  LOG_DEBUG("Loading disk");
  dword dwTrackSize, track, side, sector, dwSectorSize, dwSectors;
  byte *pbPtr, *pbDataPtr, *pbTempPtr, *pbTrackSizeTable;
  byte dsk_header[0x100];
  if(fread(dsk_header, 0x100, 1, pfile) != 1) { // read DSK header
    LOG_ERROR("Couldn't read DSK header");
    return ERR_DSK_INVALID;
  }
  pbPtr = dsk_header;

  if (memcmp(pbPtr, "MV - CPC", 8) == 0) { // normal DSK image?
    LOG_DEBUG("Loading normal disk");
    drive->tracks = *(pbPtr + 0x30); // grab number of tracks
    if (drive->tracks > DSK_TRACKMAX) { // compare against upper limit
      drive->tracks = DSK_TRACKMAX; // limit to maximum
    }
    drive->sides = *(pbPtr + 0x31); // grab number of sides
    if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
      LOG_ERROR("DSK header has " << drive->sides << " sides, expected " << DSK_SIDEMAX << " or less");
      dsk_eject(drive);
      return ERR_DSK_SIDES;
    }
    dwTrackSize = (*(pbPtr + 0x32) + (*(pbPtr + 0x33) << 8)) - 0x100; // determine track size in bytes, minus track header
    drive->sides--; // zero base number of sides
    for (track = 0; track < drive->tracks; track++) { // loop for all tracks
      for (side = 0; side <= drive->sides; side++) { // loop for all sides
        byte track_header[0x100];
        if(fread(track_header, 0x100, 1, pfile) != 1) { // read track header
          LOG_ERROR("Couldn't read DSK track header for track " << track << " side " << side);
          dsk_eject(drive);
          return ERR_DSK_INVALID;
        }
        pbPtr = track_header;
        if (memcmp(pbPtr, "Track-Info", 10) != 0) { // abort if ID does not match
          LOG_ERROR("Corrupted DSK track header for track " << track << " side " << side);
          dsk_eject(drive);
          return ERR_DSK_INVALID;
        }
        dwSectorSize = 0x80 << *(pbPtr + 0x14); // determine sector size in bytes
        dwSectors = *(pbPtr + 0x15); // grab number of sectors
        if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
          LOG_ERROR("DSK track with " << dwSectors << " sectors, expected " << DSK_SECTORMAX << "or less");
          dsk_eject(drive);
          return ERR_DSK_SECTORS;
        }
        drive->track[track][side].sectors = dwSectors; // store sector count
        drive->track[track][side].size = dwTrackSize; // store track size
        drive->track[track][side].data = new byte[dwTrackSize]; // attempt to allocate the required memory
        pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
        pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
        for (sector = 0; sector < dwSectors; sector++) { // loop for all sectors
          memcpy(drive->track[track][side].sector[sector].CHRN, (pbPtr + 0x18), 4); // copy CHRN
          memcpy(drive->track[track][side].sector[sector].flags, (pbPtr + 0x1c), 2); // copy ST1 & ST2
          drive->track[track][side].sector[sector].setSizes(dwSectorSize, dwSectorSize);
          drive->track[track][side].sector[sector].setData(pbDataPtr); // store pointer to sector data
          pbDataPtr += dwSectorSize;
          pbPtr += 8;
        }
        if (dwTrackSize > 0 && !fread(pbTempPtr, dwTrackSize, 1, pfile)) { // read entire track data in one go
          LOG_ERROR("Couldn't read track data for track " << track << " side " << side);
          dsk_eject(drive);
          return ERR_DSK_INVALID;
        }
      }
    }
    drive->altered = false; // disk is as yet unmodified
  } else {
    if (memcmp(pbPtr, "EXTENDED", 8) == 0) { // extended DSK image?
      LOG_DEBUG("Loading extended disk");
      drive->tracks = *(pbPtr + 0x30); // number of tracks
      LOG_DEBUG("with " << drive->tracks << " tracks");
      if (drive->tracks > DSK_TRACKMAX) {  // limit to maximum possible
        drive->tracks = DSK_TRACKMAX;
      }
      drive->random_DEs = *(pbPtr + 0x31) & 0x80; // simulate random Data Errors?
      drive->sides = *(pbPtr + 0x31) & 3; // number of sides
      LOG_DEBUG("with " << drive->sides << " sides");
      if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
        LOG_ERROR("DSK header has " << drive->sides << " sides, expected " << DSK_SIDEMAX << " or less");
        dsk_eject(drive);
        return ERR_DSK_SIDES;
      }
      pbTrackSizeTable = pbPtr + 0x34; // pointer to track size table in DSK header
      drive->sides--; // zero base number of sides
      for (track = 0; track < drive->tracks; track++) { // loop for all tracks
        for (side = 0; side <= drive->sides; side++) { // loop for all sides
          dwTrackSize = (*pbTrackSizeTable++ << 8); // track size in bytes
          LOG_DEBUG("Track " << track << ", side " << side << ", size " << dwTrackSize);
          if (dwTrackSize != 0) { // only process if track contains data
            dwTrackSize -= 0x100; // compensate for track header
            byte track_header[0x100];
            if(fread(track_header, 0x100, 1, pfile) != 1) { // read track header
              LOG_ERROR("Couldn't read DSK track header for track " << track << " side " << side);
              dsk_eject(drive);
              return ERR_DSK_INVALID;
            }
            pbPtr = track_header;
            if (memcmp(pbPtr, "Track-Info", 10) != 0) { // valid track header?
              LOG_ERROR("Corrupted DSK track header for track " << track << " side " << side);
              dsk_eject(drive);
              return ERR_DSK_INVALID;
            }
            dwSectors = *(pbPtr + 0x15); // number of sectors for this track
            LOG_DEBUG("with " << dwSectors << " sectors");
            if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
              LOG_ERROR("DSK track with " << dwSectors << " sectors, expected " << DSK_SECTORMAX << "or less");
              dsk_eject(drive);
              return ERR_DSK_SECTORS;
            }
            drive->track[track][side].sectors = dwSectors; // store sector count
            drive->track[track][side].size = dwTrackSize; // store track size
            drive->track[track][side].data = new byte[dwTrackSize]; // attempt to allocate the required memory
            pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
            pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
            pbPtr += 0x18;
            for (sector = 0; sector < dwSectors; sector++) { // loop for all sectors
              memcpy(drive->track[track][side].sector[sector].CHRN, pbPtr, 4); // copy CHRN
              memcpy(drive->track[track][side].sector[sector].flags, (pbPtr + 0x04), 2); // copy ST1 & ST2
              dword dwRealSize = 0x80 << *(pbPtr+0x03);
              dwSectorSize = *(pbPtr + 0x6) + (*(pbPtr + 0x7) << 8); // sector size in bytes
              drive->track[track][side].sector[sector].setSizes(dwRealSize, dwSectorSize);
              drive->track[track][side].sector[sector].setData(pbDataPtr); // store pointer to sector data
              pbDataPtr += dwSectorSize;
              pbPtr += 8;
              LOG_DEBUG("Sector " << sector << " size: " << dwSectorSize << " real size: " << dwRealSize << " CHRN: " << chrn_to_string(drive->track[track][side].sector[sector].CHRN));
            }
            if (dwTrackSize > 0 && !fread(pbTempPtr, dwTrackSize, 1, pfile)) { // read entire track data in one go
              LOG_ERROR("Couldn't read track data for track " << track << " side " << side);
              dsk_eject(drive);
              return ERR_DSK_INVALID;
            }
          } else {
            LOG_DEBUG("empty track");
            memset(&drive->track[track][side], 0, sizeof(t_track)); // track not formatted
          }
        }
      }
      drive->altered = false; // disk is as yet unmodified
    } else {
      LOG_ERROR("Unknown DSK type");
      dsk_eject(drive);
      return ERR_DSK_INVALID; // file could not be identified as a valid DSK
    }
  }
  return 0;
}

int dsk_load (const std::string &filename, t_drive *drive)
{
   int iRetCode = 0;

   LOG_DEBUG("Loading disk: " << filename);
   dsk_eject(drive);
   if ((pfileObject = fopen(filename.c_str(), "rb")) != nullptr) {
     iRetCode = dsk_load(pfileObject, drive);
     fclose(pfileObject);
   } else {
      LOG_ERROR("File not found: " << filename);
      iRetCode = ERR_FILE_NOT_FOUND;
   }

   if (iRetCode != 0) { // on error, 'eject' disk from drive
      dsk_eject(drive);
   }
   return iRetCode;
}

int dsk_save (const std::string &filename, t_drive *drive)
{
   t_DSK_header dh;
   t_track_header th;
   dword track, side, pos, sector;

   // If there are no tracks, don't save
   if (drive->tracks==0) {
      LOG_ERROR("No tracks to save");
      return ERR_DSK_WRITE;
   }
   if ((pfileObject = fopen(filename.c_str(), "wb")) != nullptr) {
      memset(&dh, 0, sizeof(dh));
      memcpy(dh.id, "EXTENDED CPC DSK File\r\nDisk-Info\r\n", sizeof(dh.id));
      strncpy(dh.unused1, "Caprice32\r\n", sizeof(dh.unused1));
      dh.tracks = drive->tracks;
      dh.sides = (drive->sides+1) | (drive->random_DEs); // correct side count and indicate random DEs, if necessary
      pos = 0;
      for (track = 0; track < drive->tracks; track++) { // loop for all tracks
         for (side = 0; side <= drive->sides; side++) { // loop for all sides
            if (drive->track[track][side].size) { // track is formatted?
               dh.track_size[pos] = (drive->track[track][side].size + 0x100) >> 8; // track size + header in bytes
            }
            pos++;
         }
      }
      if (!fwrite(&dh, sizeof(dh), 1, pfileObject)) { // write header to file
         fclose(pfileObject);
         LOG_ERROR("Error while writing to the file '" << filename << "'");
         return ERR_DSK_WRITE;
      }

      memset(&th, 0, sizeof(th));
      memcpy(th.id, "Track-Info\r\n", sizeof(th.id));
      for (track = 0; track < drive->tracks; track++) { // loop for all tracks
         for (side = 0; side <= drive->sides; side++) { // loop for all sides
            if (drive->track[track][side].size) { // track is formatted?
               th.track = track;
               th.side = side;
               th.bps = 2;
               th.sectors = drive->track[track][side].sectors;
               th.gap3 = 0x4e;
               th.filler = 0xe5;
               for (sector = 0; sector < th.sectors; sector++) {
                  memcpy(&th.sector[sector][0], drive->track[track][side].sector[sector].CHRN, 4); // copy CHRN
                  memcpy(&th.sector[sector][4], drive->track[track][side].sector[sector].flags, 2); // copy ST1 & ST2
                  th.sector[sector][6] = drive->track[track][side].sector[sector].getTotalSize() & 0xff;
                  th.sector[sector][7] = (drive->track[track][side].sector[sector].getTotalSize() >> 8) & 0xff; // sector size in bytes
               }
               if (!fwrite(&th, sizeof(th), 1, pfileObject)) { // write track header
                  fclose(pfileObject);
                  LOG_ERROR("Error while writing to the file '" << filename << "'");
                  return ERR_DSK_WRITE;
               }
               if (!fwrite(drive->track[track][side].data, drive->track[track][side].size, 1, pfileObject)) { // write track data
                  fclose(pfileObject);
                  LOG_ERROR("Error while writing to the file '" << filename << "'");
                  return ERR_DSK_WRITE;
               }
            }
         }
      }
      drive->altered = false;  // Drive is not modified anymore
      fclose(pfileObject);
   } else {
      LOG_ERROR("Error while opening the file '" << filename << "' for write: " << strerror(errno));
      return ERR_DSK_WRITE; // write attempt failed
   }

   return 0;
}

int dsk_format (t_drive *drive, int iFormat)
{
   int iRetCode = 0;
   drive->tracks = disk_format[iFormat].tracks;
   if (drive->tracks > DSK_TRACKMAX) { // compare against upper limit
      drive->tracks = DSK_TRACKMAX; // limit to maximum
   }
   drive->sides = disk_format[iFormat].sides;
   if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
      iRetCode = ERR_DSK_SIDES;
      goto exit;
   }
   drive->sides--; // zero base number of sides
   for (dword track = 0; track < drive->tracks; track++) { // loop for all tracks
      for (dword side = 0; side <= drive->sides; side++) { // loop for all sides
         dword dwSectorSize = 0x80 << disk_format[iFormat].sector_size; // determine sector size in bytes
         dword dwSectors = disk_format[iFormat].sectors;
         if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
            iRetCode = ERR_DSK_SECTORS;
            goto exit;
         }
         dword dwTrackSize = dwSectorSize * dwSectors; // determine track size in bytes, minus track header
         drive->track[track][side].sectors = dwSectors; // store sector count
         drive->track[track][side].size = dwTrackSize; // store track size
         drive->track[track][side].data = new byte[dwTrackSize]; // attempt to allocate the required memory
         byte *pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
         byte *pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
         byte CHRN[4];
         CHRN[0] = static_cast<byte>(track);
         CHRN[1] = static_cast<byte>(side);
         CHRN[3] = static_cast<byte>(disk_format[iFormat].sector_size);
         for (dword sector = 0; sector < dwSectors; sector++) { // loop for all sectors
            CHRN[2] = disk_format[iFormat].sector_ids[side][sector];
            memcpy(drive->track[track][side].sector[sector].CHRN, CHRN, 4); // copy CHRN
            drive->track[track][side].sector[sector].setSizes(dwSectorSize, dwSectorSize);
            drive->track[track][side].sector[sector].setData(pbDataPtr); // store pointer to sector data
            pbDataPtr += dwSectorSize;
         }
         memset(pbTempPtr, disk_format[iFormat].filler_byte, dwTrackSize);
      }
   }
   drive->altered = true; // flag disk as having been modified

exit:
   if (iRetCode != 0) { // on error, 'eject' disk from drive
      dsk_eject(drive);
   }
   return iRetCode;
}

void tape_eject ()
{
  pbTapeImage.clear();
}

int snapshot_load (FILE *pfile)
{
  int n;
  dword dwSnapSize, dwModel, dwFlags;
  byte val;
  reg_pair port;
  t_SNA_header sh;

  memset(&sh, 0, sizeof(sh));
  if(fread(&sh, sizeof(sh), 1, pfile) != 1) { // read snapshot header
    LOG_ERROR("Error loading snapshot: couldn't read header");
    return ERR_SNA_INVALID;
  }
  if (memcmp(sh.id, "MV - SNA", 8) != 0) { // valid SNApshot image?
    LOG_ERROR("Error loading snapshot: invalid header");
    return ERR_SNA_INVALID;
  }
  dwSnapSize = sh.ram_size[0] + (sh.ram_size[1] * 256); // memory dump size
  dwSnapSize &= ~0x3f; // limit to multiples of 64
  if (!dwSnapSize) {
    LOG_ERROR("Error loading snapshot: zero size");
    return ERR_SNA_SIZE;
  }
  if (dwSnapSize > CPC.ram_size) { // memory dump size differs from current RAM size?
    byte *pbTemp;

    pbTemp = new byte [dwSnapSize*1024 + 1];
    delete [] pbRAMbuffer;
    CPC.ram_size = dwSnapSize;
    pbRAMbuffer = pbTemp;
    // Ensure 1 byte is available before pbRAM as prerender_normal*_plus can read it
    pbRAM = pbRAMbuffer + 1;
  }
  emulator_reset();
  n = fread(pbRAM, dwSnapSize*1024, 1, pfile); // read memory dump into CPC RAM
  if (!n) {
    emulator_reset();
    LOG_ERROR("Error loading snapshot: couldn't read RAM");
    return ERR_SNA_INVALID;
  }

  // Z80
  z80.AF.b.h = sh.AF[1];
  z80.AF.b.l = sh.AF[0];
  z80.BC.b.h = sh.BC[1];
  z80.BC.b.l = sh.BC[0];
  z80.DE.b.h = sh.DE[1];
  z80.DE.b.l = sh.DE[0];
  z80.HL.b.h = sh.HL[1];
  z80.HL.b.l = sh.HL[0];
  z80.R = sh.R & 0x7f;
  z80.Rb7 = sh.R & 0x80; // bit 7 of R
  z80.I = sh.I;
  if (sh.IFF0)
    z80.IFF1 = Pflag;
  if (sh.IFF1)
    z80.IFF2 = Pflag;
  z80.IX.b.h = sh.IX[1];
  z80.IX.b.l = sh.IX[0];
  z80.IY.b.h = sh.IY[1];
  z80.IY.b.l = sh.IY[0];
  z80.SP.b.h = sh.SP[1];
  z80.SP.b.l = sh.SP[0];
  z80.PC.b.h = sh.PC[1];
  z80.PC.b.l = sh.PC[0];
  z80.IM = sh.IM; // interrupt mode
  z80.AFx.b.h = sh.AFx[1];
  z80.AFx.b.l = sh.AFx[0];
  z80.BCx.b.h = sh.BCx[1];
  z80.BCx.b.l = sh.BCx[0];
  z80.DEx.b.h = sh.DEx[1];
  z80.DEx.b.l = sh.DEx[0];
  z80.HLx.b.h = sh.HLx[1];
  z80.HLx.b.l = sh.HLx[0];
  // Gate Array
  port.b.h = 0x7f;
  for (n = 0; n < 17; n++) { // loop for all colours + border
    GateArray.pen = n;
    val = sh.ga_ink_values[n]; // GA palette entry
    z80_OUT_handler(port, val | (1<<6));
  }
  val = sh.ga_pen; // GA pen
  z80_OUT_handler(port, (val & 0x3f));
  val = sh.ga_ROM_config; // GA ROM configuration
  z80_OUT_handler(port, (val & 0x3f) | (2<<6));
  val = sh.ga_RAM_config; // GA RAM configuration
  z80_OUT_handler(port, (val & 0x3f) | (3<<6));
  // CRTC
  port.b.h = 0xbd;
  for (n = 0; n < 18; n++) { // loop for all CRTC registers
    val = sh.crtc_registers[n];
    CRTC.reg_select = n;
    z80_OUT_handler(port, val);
  }
  port.b.h = 0xbc;
  val = sh.crtc_reg_select; // CRTC register select
  z80_OUT_handler(port, val);
  // ROM select
  port.b.h = 0xdf;
  val = sh.upper_ROM; // upper ROM number
  z80_OUT_handler(port, val);
  // PPI
  port.b.h = 0xf4; // port A
  z80_OUT_handler(port, sh.ppi_A);
  port.b.h = 0xf5; // port B
  z80_OUT_handler(port, sh.ppi_B);
  port.b.h = 0xf6; // port C
  z80_OUT_handler(port, sh.ppi_C);
  port.b.h = 0xf7; // control
  z80_OUT_handler(port, sh.ppi_control);
  // PSG
  PSG.control = PPI.portC;
  PSG.reg_select = sh.psg_reg_select;
  for (n = 0; n < 16; n++) { // loop for all PSG registers
    SetAYRegister(n, sh.psg_registers[n]);
  }

  if (sh.version > 1) { // does the snapshot have version 2 data?
    dwModel = sh.cpc_model; // determine the model it was saved for
    if (dwModel != CPC.model) { // different from what we're currently running?
      if (dwModel > 3) { // not one of the known models?
        emulator_reset();
        LOG_ERROR("Error loading snapshot: invalid or unsupported CPC model");
        return ERR_SNA_CPC_TYPE;
      }
      std::string romFilename = CPC.rom_path + "/" + chROMFile[dwModel];
      if ((pfileObject = fopen(romFilename.c_str(), "rb")) != nullptr) {
        n = fread(pbROM, 2*16384, 1, pfileObject);
        fclose(pfileObject);
        if (!n) {
          emulator_reset();
          LOG_ERROR("Error loading snapshot: couldn't read ROM");
          return ERR_CPC_ROM_MISSING;
        }
        CPC.model = dwModel;
      } else { // ROM image load failed
        emulator_reset();
        LOG_ERROR("Error loading snapshot: couldn't open ROM '" << romFilename << "'");
        return ERR_CPC_ROM_MISSING;
      }
    }
  }
  if (sh.version > 2) { // does the snapshot have version 3 data?
    FDC.motor = sh.fdc_motor;
    driveA.current_track = sh.drvA_current_track;
    driveB.current_track = sh.drvB_current_track;
    CPC.printer_port = sh.printer_data ^ 0x80; // invert bit 7 again
    PSG.AmplitudeEnv = sh.psg_env_step << 1; // multiply by 2 to bring it into the 0 - 30 range
    PSG.FirstPeriod = false;
    if (sh.psg_env_direction == 0x01) { // up
      switch (PSG.RegisterAY.EnvType)
      {
        case 4:
        case 5:
        case 6:
        case 7:
        case 13:
        case 14:
        case 15:
          PSG.FirstPeriod = true;
          break;
      }
    } else if (sh.psg_env_direction == 0xff) { // down
      switch (PSG.RegisterAY.EnvType)
      {
        case 0:
        case 1:
        case 2:
        case 3:
        case 9:
        case 10:
        case 11:
          PSG.FirstPeriod = true;
          break;
      }
    }
    CRTC.addr = sh.crtc_addr[0] + (sh.crtc_addr[1] * 256);
    VDU.scanline = sh.crtc_scanline[0] + (sh.crtc_scanline[1] * 256);
    if (VDU.scanline > MaxVSync) {
      VDU.scanline = MaxVSync; // limit to max value
    }
    CRTC.char_count = sh.crtc_char_count[0];
    CRTC.line_count = sh.crtc_line_count & 127;
    CRTC.raster_count = sh.crtc_raster_count & 31;
    CRTC.hsw_count = sh.crtc_hsw_count & 15;
    CRTC.vsw_count = sh.crtc_vsw_count & 15;
    dwFlags = sh.crtc_flags[0] + (sh.crtc_flags[1] * 256);
    CRTC.flag_invsync = dwFlags & 1 ? 1 : 0; // vsync active?
    if (dwFlags & 2) { // hsync active?
      flags1.inHSYNC = 0xff;
      CRTC.flag_hadhsync = 1;
      if ((CRTC.hsw_count >= 3) && (CRTC.hsw_count < 7)) {
        CRTC.flag_inmonhsync = 1;
      }
    }
    CRTC.flag_invta = dwFlags & 0x80 ? 1 : 0; // in vertical total adjust?
    GateArray.hs_count = sh.ga_int_delay & 3;
    GateArray.sl_count = sh.ga_sl_count;
    z80.int_pending = sh.z80_int_pending;
  }
  return 0;
}

int snapshot_load (const std::string &filename)
{
   if ((pfileObject = fopen(filename.c_str(), "rb")) != nullptr) {
     return snapshot_load(pfileObject);
   }
   LOG_ERROR("Error loading snapshot: file not found: '" << filename << "'");
   return ERR_FILE_NOT_FOUND;
}

int tape_insert (FILE *pfile)
{
   tape_eject();
   if(fread(pbGPBuffer, 10, 1, pfile) != 1) { // read beginning of header
      LOG_ERROR("Error loading tape: couldn't read header");
      return ERR_TAP_INVALID;
   }
   // Reset so that the next method can recheck the header
   fseek(pfile, 0, SEEK_SET);
   byte *pbPtr = pbGPBuffer;
   if (memcmp(pbPtr, "ZXTape!\032", 8) == 0) { // CDT file?
      LOG_DEBUG("tape_insert CDT file");
      return tape_insert_cdt(pfile);
   }
   if (memcmp(pbPtr, "Creative", 8) == 0) { // VOC file ?
      LOG_DEBUG("tape_insert VOC file");
      return tape_insert_voc(pfile);
   }
   // Unknown file
   LOG_ERROR("Error loading tape: Unrecognized file type");
   return ERR_TAP_INVALID;
}

int tape_insert (const std::string &filename)
{
   LOG_DEBUG("tape_insert " << filename);
   FILE *pfile;
   if ((pfile = fopen(filename.c_str(), "rb")) == nullptr) {
      LOG_ERROR("Error loading tape: File not found: '" << filename << "'");
      return ERR_FILE_NOT_FOUND;
   }

   int iRetCode = tape_insert(pfile);
   fclose(pfile);

   return iRetCode;
}

int tape_insert_cdt (FILE *pfile)
{
   long lFileSize;
   int iBlockLength;
   byte bID;
   byte *pbPtr, *pbBlock;

   if(fread(pbGPBuffer, 10, 1, pfile) != 1) { // read CDT header
      LOG_ERROR("Couldn't read CDT header");
      return ERR_TAP_INVALID;
   }
   pbPtr = pbGPBuffer;
   if (memcmp(pbPtr, "ZXTape!\032", 8) != 0) { // valid CDT file?
      LOG_ERROR("Invalid CDT header '" << pbPtr << "'");
      return ERR_TAP_INVALID;
   }
   if (*(pbPtr + 0x08) != 1) { // major version must be 1
      LOG_ERROR("Invalid CDT major version");
      return ERR_TAP_INVALID;
   }
   lFileSize = file_size(fileno(pfile)) - 0x0a;
   if (lFileSize <= 0) { // the tape image should have at least one block...
      LOG_ERROR("Invalid CDT file size");
      return ERR_TAP_INVALID;
   }
   pbTapeImage.resize(lFileSize+6);
   pbTapeImage[0] = 0x20; // start off with a pause block
   *reinterpret_cast<word *>(&pbTapeImage[1]) = 2000; // set the length to 2 seconds
   if(fread(&pbTapeImage[3], lFileSize, 1, pfile) != 1) { // append the entire CDT file
      LOG_ERROR("Couldn't read CDT file");
     return ERR_TAP_INVALID;
   }
   *(&pbTapeImage[lFileSize+3]) = 0x20; // end with a pause block
   *reinterpret_cast<word *>(&pbTapeImage[lFileSize+3+1]) = 2000; // set the length to 2 seconds

   #ifdef DEBUG_TAPE
   fputs("--- New Tape\r\n", pfoDebug);
   #endif
   pbTapeImageEnd = &pbTapeImage[lFileSize+6];
   pbBlock = &pbTapeImage[0];
   bool bolGotDataBlock = false;
   while (pbBlock < pbTapeImageEnd) {
      bID = *pbBlock++;
      switch(bID) {
         case 0x10: // standard speed data block
            iBlockLength = *reinterpret_cast<word *>(pbBlock+2) + 4;
            bolGotDataBlock = true;
            break;
         case 0x11: // turbo loading data block
            iBlockLength = (*reinterpret_cast<dword *>(pbBlock+0x0f) & 0x00ffffff) + 0x12;
            bolGotDataBlock = true;
            break;
         case 0x12: // pure tone
            iBlockLength = 4;
            bolGotDataBlock = true;
            break;
         case 0x13: // sequence of pulses of different length
            iBlockLength = *pbBlock * 2 + 1;
            bolGotDataBlock = true;
            break;
         case 0x14: // pure data block
            iBlockLength = (*reinterpret_cast<dword *>(pbBlock+0x07) & 0x00ffffff) + 0x0a;
            bolGotDataBlock = true;
            break;
         case 0x15: // direct recording
            iBlockLength = (*reinterpret_cast<dword *>(pbBlock+0x05) & 0x00ffffff) + 0x08;
            bolGotDataBlock = true;
            break;
         case 0x20: // pause
            if ((!bolGotDataBlock) && (pbBlock != &pbTapeImage[1])) {
               *reinterpret_cast<word *>(pbBlock) = 0; // remove any pauses (execept ours) before the data starts
            }
            iBlockLength = 2;
            break;
         case 0x21: // group start
            iBlockLength = *pbBlock + 1;
            break;
         case 0x22: // group end
            iBlockLength = 0;
            break;
         case 0x23: // jump to block
            LOG_ERROR("Couldn't load CDT file: unsupported block ID: " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = 2;
            break;
         case 0x24: // loop start
            LOG_ERROR("Couldn't load CDT file: unsupported block ID: " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = 2;
            break;
         case 0x25: // loop end
            LOG_ERROR("Couldn't load CDT file: unsupported block ID: " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = 0;
            break;
         case 0x26: // call sequence
            LOG_ERROR("Couldn't load CDT file: unsupported block ID: " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = (*reinterpret_cast<word *>(pbBlock) * 2) + 2;
            break;
         case 0x27: // return from sequence
            LOG_ERROR("Couldn't load CDT file: unsupported block ID: " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = 0;
            break;
         case 0x28: // select block
            LOG_ERROR("Couldn't load CDT file: unsupported block ID: " << bID);
            return ERR_TAP_UNSUPPORTED;
            iBlockLength = *reinterpret_cast<word *>(pbBlock) + 2;
            break;
         case 0x30: // text description
            iBlockLength = *pbBlock + 1;
            break;
         case 0x31: // message block
            iBlockLength = *(pbBlock+1) + 2;
            break;
         case 0x32: // archive info
            iBlockLength = *reinterpret_cast<word *>(pbBlock) + 2;
            break;
         case 0x33: // hardware type
            iBlockLength = (*pbBlock * 3) + 1;
            break;
         case 0x34: // emulation info
            iBlockLength = 8;
            break;
         case 0x35: // custom info block
            iBlockLength = *reinterpret_cast<dword *>(pbBlock+0x10) + 0x14;
            break;
         case 0x40: // snapshot block
            iBlockLength = (*reinterpret_cast<dword *>(pbBlock+0x01) & 0x00ffffff) + 0x04;
            break;
         case 0x5A: // another tzx/cdt file
            iBlockLength = 9;
            break;

         default: // "extension rule"
            iBlockLength = *reinterpret_cast<dword *>(pbBlock) + 4;
      }

      #ifdef DEBUG_TAPE
      fprintf(pfoDebug, "%02x %d\r\n", bID, iBlockLength);
      #endif

      pbBlock += iBlockLength;
   }
   if (pbBlock != pbTapeImageEnd) {
      LOG_ERROR("CDT file error: Didn't reach end of tape");
      tape_eject();
      return ERR_TAP_INVALID;
   }
   Tape_Rewind();
   return 0;
}

int tape_insert_voc (FILE *pfile)
{
   long lFileSize, lOffset, lInitialOffset, lSampleLength;
   int iBlockLength;
   byte *pbPtr, *pbTapeImagePtr, *pbVocDataBlock, *pbVocDataBlockPtr;
   bool bolDone;

   tape_eject();
   if(fread(pbGPBuffer, 26, 1, pfile) != 1) { // read VOC header
     LOG_ERROR("Reading VOC file: Invalid VOC file: error reading header");
     return ERR_TAP_BAD_VOC;
   }
   pbPtr = pbGPBuffer;
   if (memcmp(pbPtr, "Creative Voice File\032", 20) != 0) { // valid VOC file?
     LOG_ERROR("Reading VOC file: Invalid VOC file");
     return ERR_TAP_BAD_VOC;
   }
   lOffset =
   lInitialOffset = *reinterpret_cast<word *>(pbPtr + 0x14);
   lFileSize = file_size(fileno(pfile));
   if ((lFileSize-26) <= 0) { // should have at least one block...
     LOG_ERROR("Reading VOC file: Invalid VOC file: no block");
     return ERR_TAP_BAD_VOC;
   }

   #ifdef DEBUG_TAPE
   fputs("--- New Tape\r\n", pfoDebug);
   #endif
   iBlockLength = 0;
   lSampleLength = 0;
   byte bSampleRate = 0;
   bolDone = false;
   while ((!bolDone) && (lOffset < lFileSize)) {
      fseek(pfile, lOffset, SEEK_SET);
      if(fread(pbPtr, 16, 1, pfile) != 1) { // read block ID + size
        LOG_ERROR("Reading VOC file: Invalid VOC file: couldn't read block at " << lOffset);
        return ERR_TAP_BAD_VOC;
      }
      #ifdef DEBUG_TAPE
      fprintf(pfoDebug, "%02x %d\r\n", *pbPtr, *(dword *)(pbPtr+0x01) & 0x00ffffff);
      #endif
      switch(*pbPtr) {
         case 0x0: // terminator
            bolDone = true;
            break;
         case 0x1: // sound data
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr+0x01) & 0x00ffffff) + 4;
            lSampleLength += iBlockLength - 6;
            if ((bSampleRate) && (bSampleRate != *(pbPtr+0x04))) { // no change in sample rate allowed
               LOG_ERROR("Reading VOC file: unsupported change in sample rate");
               return ERR_TAP_BAD_VOC;
            }
            bSampleRate = *(pbPtr+0x04);
            if (*(pbPtr+0x05) != 0) { // must be 8 bits wide
               LOG_ERROR("Reading VOC file: unsupported non-8 bits codec");
               return ERR_TAP_BAD_VOC;
            }
            break;
         case 0x2: // sound continue
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr+0x01) & 0x00ffffff) + 4;
            lSampleLength += iBlockLength - 4;
            break;
         case 0x3: // silence
            iBlockLength = 4;
            lSampleLength += *reinterpret_cast<word *>(pbPtr+0x01) + 1;
            if ((bSampleRate) && (bSampleRate != *(pbPtr+0x03))) { // no change in sample rate allowed
               LOG_ERROR("Reading VOC file: unsupported change in sample rate");
               return ERR_TAP_BAD_VOC;
            }
            bSampleRate = *(pbPtr+0x03);
            break;
         case 0x4: // marker
            iBlockLength = 3;
            break;
         case 0x5: // ascii
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr+0x01) & 0x00ffffff) + 4;
            break;
         case 0x6: // repeat
            LOG_ERROR("Reading VOC file: unsupported repeat block");
            return ERR_TAP_BAD_VOC;
         case 0x7: // end repeat
            LOG_ERROR("Reading VOC file: unsupported end repeat block");
            return ERR_TAP_BAD_VOC;
         case 0x8: // extended
            LOG_ERROR("Reading VOC file: unsupported extended block");
            return ERR_TAP_BAD_VOC;
         case 0x9: // sound data - new format
            LOG_ERROR("Reading VOC file: unsupported new format sound data block");
            return ERR_TAP_BAD_VOC;
         default:
            LOG_ERROR("Reading VOC file: unsupported unknown block type: " << static_cast<int>(*pbPtr));
            return ERR_TAP_BAD_VOC;
      }
      lOffset += iBlockLength;
   }
   #ifdef DEBUG_TAPE
   fprintf(pfoDebug, "--- %ld bytes\r\n", lSampleLength);
   #endif

   dword dwTapePulseCycles = 3500000L / (1000000L / (256 - bSampleRate)); // length of one pulse in ZX Spectrum T states
   dword dwCompressedSize = lSampleLength >> 3; // 8x data reduction
   if (dwCompressedSize > 0x00ffffff) { // we only support one direct recording block right now
      LOG_ERROR("Reading VOC file: tape size is too big");
      return ERR_TAP_BAD_VOC;
   }
   pbTapeImage.resize(dwCompressedSize+1+8+6);
   pbTapeImage[0] = 0x20; // start off with a pause block
   *reinterpret_cast<word *>(&pbTapeImage[1]) = 2000; // set the length to 2 seconds

   *(&pbTapeImage[3]) = 0x15; // direct recording block
   *reinterpret_cast<word *>(&pbTapeImage[4]) = static_cast<word>(dwTapePulseCycles); // number of T states per sample
   *reinterpret_cast<word *>(&pbTapeImage[6]) = 0; // pause after block
   pbTapeImage[8] = lSampleLength & 7 ? lSampleLength & 7 : 8; // bits used in last byte
   *reinterpret_cast<dword *>(&pbTapeImage[9]) = dwCompressedSize & 0x00ffffff; // data length
   pbTapeImagePtr = &pbTapeImage[12];

   lOffset = lInitialOffset;
   bolDone = false;
   dword dwBit = 8;
   byte bByte = 0;
   while ((!bolDone) && (lOffset < lFileSize)) {
      fseek(pfile, lOffset, SEEK_SET);
      if(fread(pbPtr, 1, 1, pfile) != 1) { // read block ID
        LOG_ERROR("Reading VOC file: error reading block ID");
        return ERR_TAP_BAD_VOC;
      }
      switch(*pbPtr) {
         case 0x0: // terminator
            bolDone = true;
            break;
         case 0x1: // sound data
            if(fread(pbPtr, 3+2, 1, pfile) != 1) { // get block size and sound info
              LOG_ERROR("Reading VOC file: error reading sound data");
              return ERR_TAP_BAD_VOC;
            }
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr) & 0x00ffffff) + 4;
            lSampleLength = iBlockLength - 6;
            pbVocDataBlock = new byte[lSampleLength];
            if(fread(pbVocDataBlock, lSampleLength, 1, pfile) != 1) {
              LOG_ERROR("Reading VOC file: error reading sound data");
              return ERR_TAP_BAD_VOC;
            }
            pbVocDataBlockPtr = pbVocDataBlock;
            for (int iBytePos = 0; iBytePos < lSampleLength; iBytePos++) {
               byte bVocSample = *pbVocDataBlockPtr++;
               dwBit--;
               if (bVocSample > VOC_THRESHOLD) {
                  bByte |= bit_values[dwBit];
               }
               if (!dwBit) { // got all 8 bits?
                  *pbTapeImagePtr++ = bByte;
                  dwBit = 8;
                  bByte = 0;
               }
            }
            delete [] pbVocDataBlock;
            break;
         case 0x2: // sound continue
            if(fread(pbPtr, 3, 1, pfile) != 1) { // get block size
              LOG_ERROR("Reading VOC file: error reading sound continue");
              return ERR_TAP_BAD_VOC;
            }
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr) & 0x00ffffff) + 4;
            lSampleLength = iBlockLength - 4;
            pbVocDataBlock = new byte[lSampleLength];
            if(fread(pbVocDataBlock, lSampleLength, 1, pfile) != 1) {
              LOG_ERROR("Reading VOC file: error reading sound continue");
              return ERR_TAP_BAD_VOC;
            }
            pbVocDataBlockPtr = pbVocDataBlock;
            for (int iBytePos = 0; iBytePos < lSampleLength; iBytePos++) {
               byte bVocSample = *pbVocDataBlockPtr++;
               dwBit--;
               if (bVocSample > VOC_THRESHOLD) {
                  bByte |= bit_values[dwBit];
               }
               if (!dwBit) { // got all 8 bits?
                  *pbTapeImagePtr++ = bByte;
                  dwBit = 8;
                  bByte = 0;
               }
            }
            delete [] pbVocDataBlock;
            break;
         case 0x3: // silence
            iBlockLength = 4;
            lSampleLength = *reinterpret_cast<word *>(pbPtr) + 1;
            for (int iBytePos = 0; iBytePos < lSampleLength; iBytePos++) {
               dwBit--;
               if (!dwBit) { // got all 8 bits?
                  *pbTapeImagePtr++ = bByte;
                  dwBit = 8;
                  bByte = 0;
               }
            }
            break;
         case 0x4: // marker
            iBlockLength = 3;
            break;
         case 0x5: // ascii
            iBlockLength = (*reinterpret_cast<dword *>(pbPtr) & 0x00ffffff) + 4;
            break;
         default:
            LOG_ERROR("Reading VOC file: unsupported unknown block type: " << static_cast<int>(*pbPtr));
            return ERR_TAP_BAD_VOC;
      }
      lOffset += iBlockLength;
   }

   *pbTapeImagePtr = 0x20; // end with a pause block
   *reinterpret_cast<word *>(pbTapeImagePtr+1) = 2000; // set the length to 2 seconds

   pbTapeImageEnd = pbTapeImagePtr + 3;

   Tape_Rewind();
   return 0;
}

void cartridge_load ()
{
  if (CPC.model >= 3) {
     if (file_load(CPC.cartridge)) {
        fprintf(stderr, "Load of cartridge failed. Aborting.\n");
        cleanExit(-1);
     }
  }
}

int cartridge_load (const std::string& filepath)
{
  if (CPC.model >= 3) {
    return cpr_load(filepath);
  }
  LOG_ERROR("Loading cartridge: not supported on the chosen CPC model: CPC.model=" << CPC.model);
  return ERR_FILE_UNSUPPORTED;
}

int cartridge_load (FILE *file) {
  if (CPC.model >= 3) {
    return cpr_load(file);
  }
  LOG_ERROR("Loading cartridge: not supported on the chosen CPC model: CPC.model=" << CPC.model);
  return ERR_FILE_UNSUPPORTED;
}

std::string drive_extensions(const DRIVE drive) {
  switch (drive) {
    case DRIVE::DSK_A:
    case DRIVE::DSK_B:
      return ".dsk.ipf.raw";
    case DRIVE::TAPE:
      return ".cdt.voc";
    case DRIVE::SNAPSHOT:
      return ".sna";
    case DRIVE::CARTRIDGE:
      return ".cpr";
  }
  LOG_ERROR("Unsupported drive type: " << static_cast<int>(drive))
  return "";
}

// Still some duplication there... but it cannot really be helped
int file_load(t_slot& slot)
{
  if (slot.file.empty()) {
    // Special casing because this is not an error if called from loadSlots
    LOG_VERBOSE("Ignoring empty filename passed to file_load.")
    return ERR_FILE_NOT_FOUND;
  }
  if (slot.file.length() < 4) {
    LOG_ERROR("File path is too short: '" << slot.file << "'");
    return ERR_FILE_NOT_FOUND;
  }
  int pos = slot.file.length() - 4;
  std::string extension = stringutils::lower(slot.file.substr(pos));

  FILE *file = nullptr;
  if (extension == ".zip") {
    zip::t_zip_info zip_info;
    zip_info.filename = slot.file;
    zip_info.extensions = drive_extensions(slot.drive);
    if (zip::dir(&zip_info)) {
      // error or nothing relevant found
      LOG_ERROR("Error opening or parsing zip file " << slot.file);
      return ERR_FILE_UNZIP_FAILED;
    }

    slot.zip_index = slot.zip_index % zip_info.filesOffsets.size();
    std::string filename = zip_info.filesOffsets[slot.zip_index].first;
    pos = filename.length() - 4;
    extension = stringutils::lower(filename.substr(pos)); // grab the extension in lowercases
    LOG_DEBUG("Extracting " << slot.file << ", " << filename << ", " << extension);
    file = extractFile(slot.file, filename, extension);
    if (zip_info.filesOffsets.size() > 1) {
      // Give 5s to the user to read the message.
      set_osd_message("Loaded '" + filename + "' - Press Shift+F5 for next file", 5000);
    }
  }

  for(const auto& loader : files_loader_list) {
    if (slot.drive == loader.drive && extension == loader.extension) {
      if (file) {
        return loader.load_from_file(file);
      }
      return loader.load_from_filename(slot.file);
    }
  }
  LOG_ERROR("File format unsupported for " << slot.file);
  return ERR_FILE_UNSUPPORTED;
}
