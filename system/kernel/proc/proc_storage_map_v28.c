/* Staged PDP-6 process storage map: no scheduler or swap-record modification. */
#include "proc_storage_map_v28.h"
#include "proc.h"

int
proc_storage_map_init_v28(struct proc_storage_map_v28 *map,
    kword_t *words, unsigned int slots)
{
        if (!map || !words || map->words || map->slots ||
            slots < PROC_MIN_SLOTS || slots > PROC_MAX_SLOTS)
                return -1;
        map->words = words;
        map->slots = slots;
        return 0;
}

int
proc_storage_get_v28(const struct proc_storage_map_v28 *map,
    unsigned int slot)
{
        unsigned int offset;
        unsigned int index;

        if (!map || !map->words || slot >= map->slots)
                return -1;
        index = slot / PROC_STORAGE_MAP_SLOTS_PER_WORD_V28;
        offset = (slot % PROC_STORAGE_MAP_SLOTS_PER_WORD_V28) * 2U;
        return (int)((map->words[index] >> offset) & 3UL);
}

int
proc_storage_change_v28(struct proc_storage_map_v28 *map,
    unsigned int slot, unsigned int old_state, unsigned int new_state)
{
        unsigned int offset;
        unsigned int index;
        kword_t mask;

        if (!map || !map->words || slot == 0U || slot >= map->slots ||
            old_state > PROC_STORAGE_PACKED ||
            new_state > PROC_STORAGE_PACKED ||
            proc_storage_get_v28(map, slot) != (int)old_state)
                return -1;
        index = slot / PROC_STORAGE_MAP_SLOTS_PER_WORD_V28;
        offset = (slot % PROC_STORAGE_MAP_SLOTS_PER_WORD_V28) * 2U;
        mask = (kword_t)3UL << offset;
        map->words[index] = (map->words[index] & ~mask) |
            ((kword_t)new_state << offset);
        return 0;
}
