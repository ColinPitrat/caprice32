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

#include "disk.h"

typedef enum {
  DSK_A,
  DSK_B,
  OTHER,
} DRIVE;

FILE *extractFile(const std::string& zipfile, const std::string& filename, const std::string& ext);
int snapshot_load (FILE *pfile);
int snapshot_load (const std::string& filename);
int snapshot_save (const std::string& filename);
int dsk_load (FILE *pfile, t_drive *drive);
int dsk_load (const std::string& filename, t_drive *drive);
int dsk_save (const std::string& filename, t_drive *drive);
void dsk_eject (t_drive* drive);
int dsk_format (t_drive* drive, int iFormat);
int tape_insert (FILE *pfile);
int tape_insert (const std::string& filename);
int tape_insert_cdt (FILE *pfile);
int tape_insert_voc (FILE *pfile);
void tape_eject ();
void cartridge_load ();
int cartridge_load (const std::string& filepath);
int cartridge_load (FILE *file);
// Smart load: support loading DSK, SNA, CDT, VOC, CPR or a zip containing one of these.
// drive must be DSK_A or DSK_B for DSK, OTHER otherwise.
int file_load(const std::string& filepath, const DRIVE drive);
// Retrieve files that are passed as argument and update CPC fields so that they will be loaded properly
void fillSlots (std::vector<std::string> slot_list, t_CPC& CPC);
// Loads slot content in memory
void loadSlots();

#define MAX_DISK_FORMAT          8
#define DEFAULT_DISK_FORMAT      0
#define FIRST_CUSTOM_DISK_FORMAT 2

t_disk_format parseDiskFormat(const std::string& format);
std::string serializeDiskFormat(const t_disk_format& format);

