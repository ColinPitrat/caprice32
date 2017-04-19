/* Caprice32 - Amstrad CPC Emulator
   
   Loading of Plus range cartridge files (.cpr)
   The file format is RIFF (Resource Interchange File Format) as described
   here: 
    - http://www.cpcwiki.eu/index.php/Format:CPR_CPC_Plus_cartridge_file_format
    - https://en.wikipedia.org/wiki/Resource_Interchange_File_Format  
*/

#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <string>

void cpr_eject ();
int cpr_load (const std::string &filename);
int cpr_load (FILE *pfile);

#endif
