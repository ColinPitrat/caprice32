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
  bool locked;
  int lockSeqPos;

  bool extend_border;
  int hscroll;
  int vscroll;
  byte sprites[16][16][16];
  int16_t sprites_x[16];
  int16_t sprites_y[16];
  short int sprites_mag_x[16];
  short int sprites_mag_y[16];

  bool raster_interrupt;
  byte interrupt_vector;

  dma_t dma;
};

extern asic_t asic;
extern byte *pbRegisterPage;

void asic_reset();
void asic_poke_lock_sequence(byte val);
void asic_dma_cycle();
bool asic_register_page_write(word addr, byte val);
void asic_draw_sprites();

#endif
