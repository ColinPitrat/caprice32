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

#define NB_DMA_CHANNELS 3

struct dma_channel {
  unsigned int source_address;
  unsigned int loop_address;
  byte prescaler;
  // TODO: This must be set to false at CPC reset
  bool enabled;
  bool interrupt;
  int pause_ticks;
  byte tick_cycles;
  int loops;
};

struct dma_t {
  dma_channel ch[NB_DMA_CHANNELS];
};

struct asic_t {
  bool locked = true;

  bool extend_border = false;
  int hscroll = 0;
  int vscroll = 0;
  byte sprites[16][16][16];
  int16_t sprites_x[16];
  int16_t sprites_y[16];
  short int sprites_mag_x[16];
  short int sprites_mag_y[16];

  dma_t dma;

  bool raster_interrupt;
};

extern asic_t asic;
extern byte *pbRegisterPage;

void asic_poke_lock_sequence(byte val);
void asic_dma_cycle();
bool asic_register_page_write(word addr, byte val);
void asic_draw_sprites();

#endif
