/* Compact, separately owned process-storage lifecycle map (staged API).
 * Two bits per slot fit 18 slots per PDP-6 word. No permanent 256-slot table.
 */
#ifndef DAIMOS_PROC_STORAGE_MAP_V28_H
#define DAIMOS_PROC_STORAGE_MAP_V28_H

#include "proc_packed_v1.h"

#define PROC_STORAGE_MAP_SLOTS_PER_WORD_V28 18U
#define PROC_STORAGE_MAP_WORDS_V28(slots) (((slots) + 17U) / 18U)

struct proc_storage_map_v28 {
        kword_t *words;
        unsigned int slots;
};

/* Caller owns an initially zero-filled WORDS array of the required size.
 * A zero word represents 18 RESIDENT states. No physical allocation here.
 */
int proc_storage_map_init_v28(struct proc_storage_map_v28 *map,
    kword_t *words, unsigned int slots);
int proc_storage_get_v28(const struct proc_storage_map_v28 *map,
    unsigned int slot);
/* Compare-and-exchange state, valid only under process lifecycle locking.
 * Return 0 on successful transition, -1 for stale or illegal state.
 */
int proc_storage_change_v28(struct proc_storage_map_v28 *map,
    unsigned int slot, unsigned int old_state, unsigned int new_state);

#endif
