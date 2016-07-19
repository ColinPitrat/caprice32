/* Caprice32 - Amstrad CPC Emulator

   The ASIC of the Plus range replaces multiple parts of the old CPC and
   provides new additional features.
   This file only concern the new features.
   More details on what those features are is available here:
    - http://www.cpcwiki.eu/index.php/Arnold_V_Specs_Revised
*/

#ifndef ASIC_H
#define ASIC_H

#include "types.h"

extern byte *pbRegisterPage;
extern bool asic_locked;

void asic_poke_lock_sequence(byte val);
void asic_register_page_write(word addr, byte val);

#endif
