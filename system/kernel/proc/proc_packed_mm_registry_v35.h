/* MM-authoritative packed-process descriptor lookup (inactive experiment).
 * One layout word per occupied entry; MM provides extent base and length.
 */
#ifndef DAIMOS_PROC_PACKED_MM_REGISTRY_V35_H
#define DAIMOS_PROC_PACKED_MM_REGISTRY_V35_H
#include "proc_storage_map_v28.h"

struct proc_packed_mm_entry_v35 {
        kword_t slot;
        kword_t layout;
};

struct proc_packed_mm_registry_v35 {
        struct proc_packed_mm_entry_v35 *entries;
        unsigned int capacity;
};

int proc_packed_mm_init_v35(struct proc_packed_mm_registry_v35 *registry,
    struct proc_packed_mm_entry_v35 *entries, unsigned int capacity);
/* Returns a snapshot, not a pointer into MM's movable descriptor array. */
int proc_packed_mm_lookup_v35(const struct proc_packed_mm_registry_v35 *registry,
    unsigned int slot, struct proc_packed_v1 *result);
int proc_packed_mm_publish_v35(struct proc_packed_mm_registry_v35 *registry,
    unsigned int slot, const struct proc_packed_v1 *prepared);
int proc_packed_mm_discard_v35(struct proc_packed_mm_registry_v35 *registry,
    unsigned int slot);
#endif
