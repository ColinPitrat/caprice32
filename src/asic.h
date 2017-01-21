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
  bool enabled;
  bool interrupt;
};

struct dma_t {
  dma_channel ch0;
  dma_channel ch1;
  dma_channel ch2;
};

struct asic_t {
  // TODO: Use the DMA info to feed PSG from RAM
  // Basically: read one 16bits instruction for each channel X (if chX.enabled only?) at each scan line and execute it.
  // More precisely: after leading edge of HSYNC, one dead cycle followed by a fetch cycle for each active channel (enabled and not executing a pause) followed by an execution cycle for each active channel.
  // All instructions last 1 cycle except LOAD that lasts 8 (up to 10 if CPU is also accessing the PSG).
  //  - LOAD R,DD will write DD to PSG.Index[R] - The ASIC should actually be blocking the CPU if it tries to access the PSG, to determine if it's important to emulate.
  //  - PAUSE N set a counter to wait N*(chX.prescaler+1) cycles
  //  - REPEAT NNN keep address of loop start (next instruction) and a counter of loops
  //  - NOP does nothing
  //  - LOOP jump to address of loop stat if counter of loops is >0 and decrement it (yes, code is actually executed NNN+1 times)
  //  - INT generates an interruption for chX by setting chX.interrupt to true (code for CPU to detect it must also be done !)
  //  - STOP set chX.enabled to false ? (still increment address for when processing will restart)
  //  The last 4 can be OR-ed to be combined
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
bool asic_register_page_write(word addr, byte val);
void asic_draw_sprites();

#endif
