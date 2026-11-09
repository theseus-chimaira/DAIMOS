/**
 * @file proc_pure_ram_v1379.h
 * @brief Experimental registry-free swapless PURE image transactions.
 *
 * Not linked into KCORE. All calls require lifecycle exclusion and a
 * PROC_TRANSITION-held process with immutable executable backing.
 * The only inactive descriptor is the existing MM_TYPE_PACKED_PROCESS
 * extent, with its slot owner and exact raw suffix length.
 */
#ifndef DAIMOS_PROC_PURE_RAM_V1379_H
#define DAIMOS_PROC_PURE_RAM_V1379_H

#include "proc.h"

typedef int (*proc_pure_ram_reload_v1379)(kword_t backing,
    kword_t base, kword_t uarea);

int proc_pure_ram_drop_v1379(unsigned int slot);
int proc_pure_ram_restore_v1379(unsigned int slot,
    proc_pure_ram_reload_v1379 reload);
int proc_pure_ram_discard_v1379(unsigned int slot);

#endif
