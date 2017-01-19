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
#include <stdint.h>

struct dma_channel {
  unsigned int source_address;
  byte prescaler;
};

struct dma_t {
  dma_channel ch0;
  dma_channel ch1;
  dma_channel ch2;
  byte dcsr;
};

extern byte *pbRegisterPage;
extern bool asic_locked;
extern byte asic_sprites[16][16][16];
extern int16_t asic_sprites_x[16];
extern int16_t asic_sprites_y[16];
extern short int asic_sprites_mag_x[16];
extern short int asic_sprites_mag_y[16];
extern bool asic_extend_border;
extern int asic_hscroll;
extern int asic_vscroll;
extern dma_t dma;

void asic_poke_lock_sequence(byte val);
bool asic_register_page_write(word addr, byte val);
void asic_draw_sprites();

#endif
