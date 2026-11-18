/* v1664 opt-in FULL APR restore from an exact mode3 bitmap MM owner. */
#ifndef DAIMOS_PBR1664_H
#define DAIMOS_PBR1664_H
#include "proc_pure_ram_restore_v1391.h"

/* Loader must reject images beyond allocated_words before writing.
 * Caller holds lifecycle TRANSITION; only success clears it.
 * No production mode3 dispatcher is enabled by this source.
 */
int proc_pure_bitmap_restore_v1664(unsigned int slot,
    proc_pure_ram_loader_v1391 loader);
#endif
