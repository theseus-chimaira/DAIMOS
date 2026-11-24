/**
 * @file proc_pure_ram_restore_v1391.h
 * @brief Capacity-bounded FULL restore from inactive PURE RAM.
 *
 * Staged, not linked in production. Replaces the unbounded v1381
 * three-argument reload contract with the actual pinned APR extent
 * length. The callback must reject any initialized image larger than
 * that extent BEFORE the first payload read or decompression write.
 */
#ifndef DAIMOS_PURE_RAM_RESTORE_V1391_H
#define DAIMOS_PURE_RAM_RESTORE_V1391_H

#include "proc.h"

typedef int (*proc_pure_ram_loader_v1391)(kword_t backing,
    kword_t physical_base, kword_t stable_uarea, kword_t allocated_words);

int proc_pure_ram_restore_v1391(unsigned int slot,
    proc_pure_ram_loader_v1391 loader);

/* PI-excluded original FULL owner proof, shared with idle rollback.
 * Returns 1 (RAW), 2 (PROCESS), 3 (both), or -1 for ambiguity.
 * Expected pin count 2 permits zero/one only for an uncertain reply.
 */
int proc_pure_full_owners_v1917(unsigned int slot, kword_t source,
    kword_t raw, kword_t target, kword_t words,
    unsigned int source_pins, unsigned int target_pins);

#endif
