/**
 * @file proc_pure_ram_v1381.h
 * @brief Experimental zero-copy swapless PURE image transactions.
 *
 * Not linked into KCORE. All calls require lifecycle exclusion and a
 * PROC_TRANSITION-held process with immutable executable backing.
 * The only inactive descriptor is the existing MM_TYPE_PACKED_PROCESS
 * extent, with its slot owner and exact raw suffix length. Drop rebases
 * the original physical descriptor without copying or allocating RAM.
 */
#ifndef DAIMOS_PROC_PURE_RAM_V1381_H
#define DAIMOS_PROC_PURE_RAM_V1381_H

#include "proc.h"

typedef int (*proc_pure_ram_reload_v1381)(kword_t backing,
    kword_t base, kword_t uarea);

/* PI-excluded exact single-owner proof. Base zero discovers an owner;
 * nonzero requires the original base. Zero return means uncertainty.
 */
kword_t proc_pure_owner_v1918(unsigned int slot, unsigned int type,
    kword_t base, kword_t words, unsigned int pins);

int proc_pure_ram_drop_v1381(unsigned int slot);
int proc_pure_ram_restore_v1381(unsigned int slot,
    proc_pure_ram_reload_v1381 reload);
int proc_pure_ram_discard_v1381(unsigned int slot);

#endif
