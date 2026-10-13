/* Slot-indexed packed descriptor ownership; not yet enabled at boot. */
#ifndef DAIMOS_PROC_PACKED_LIFETIME_V33_H
#define DAIMOS_PROC_PACKED_LIFETIME_V33_H
#include "proc_storage_map_v28.h"

struct proc_packed_registry_v33 {
        struct proc_packed_v1 *entries;
        unsigned int slots;
};

/* Entries must be zeroed by the caller and remain stable through KINIT. */
int proc_packed_registry_init_v33(struct proc_packed_registry_v33 *registry,
    struct proc_packed_v1 *entries, unsigned int slots);

/* Claim a fully prepared packed extent for a slot already in PACKED mode.
 * This API never copies or takes ownership of an unvalidated MM extent.
 */
int proc_packed_registry_publish_v33(struct proc_packed_registry_v33 *registry,
    unsigned int slot, const struct proc_packed_v1 *prepared);

/* Look up the stable slot-indexed descriptor (only when mode is PACKED). */
struct proc_packed_v1 *proc_packed_registry_get_v33(
    struct proc_packed_registry_v33 *registry, unsigned int slot);

/* Called during slot teardown with lifecycle exclusion and TRANSITION held.
 * On MM_BUSY / bad ownership, keep descriptor+mode unchanged for retry.
 * A successful release clears packed metadata and returns mode to RESIDENT.
 */
int proc_packed_registry_discard_v33(struct proc_packed_registry_v33 *registry,
    unsigned int slot);

#endif
