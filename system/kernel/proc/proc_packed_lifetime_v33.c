/*
 * @file proc_packed_lifetime_v33.c
 * @brief Checked ownership and teardown of per-slot packed MM descriptors.
 *
 * The packed registry is allocated only when the packed lifecycle is enabled.
 * No global 256-slot descriptor array or reclaim-time allocation is used.
 */
#include "proc_packed_lifetime_v33.h"
#include "proc.h"
#include "vm_pdp6.h"

static int
proc_packed_owned_v33(unsigned int slot, const struct proc_packed_v1 *d)
{
        int i;
        kword_t base;
        kword_t words;

        if (!d || !d->span || !d->layout)
                return 0;
        base = PROC_PACKED_BASE(d);
        words = PROC_PACKED_WORDS(d);
        if (!base || !words || !PROC_PACKED_DECODED(d) ||
            PROC_PACKED_OFFSET(d) > MM_HALF_MASK - PROC_PACKED_DECODED(d))
                return 0;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                if (MM_EXTENT_BASE(e) == base)
                        return MM_EXTENT_WORDS(e) == words &&
                            MM_EXTENT_TYPE(e) == MM_TYPE_PACKED_PROCESS &&
                            MM_EXTENT_OWNER(e) == slot;
        }
        return 0;
}

int
proc_packed_registry_init_v33(struct proc_packed_registry_v33 *registry,
    struct proc_packed_v1 *entries, unsigned int slots)
{
        unsigned int i;

        if (!registry || !entries || registry->entries || registry->slots ||
            slots < PROC_MIN_SLOTS || slots > PROC_MAX_SLOTS)
                return MM_ERR_INVAL;
        for (i = 0U; i < slots; ++i) {
                if (entries[i].span || entries[i].layout)
                        return MM_ERR_INVAL;
        }
        registry->entries = entries;
        registry->slots = slots;
        return MM_OK;
}

struct proc_packed_v1 *
proc_packed_registry_get_v33(struct proc_packed_registry_v33 *registry,
    unsigned int slot)
{
        if (!registry || !registry->entries || !proc_table || slot == 0U ||
            slot >= registry->slots ||
            proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
                PROC_STORAGE_PACKED)
                return 0;
        return &registry->entries[slot];
}

int
proc_packed_registry_publish_v33(struct proc_packed_registry_v33 *registry,
    unsigned int slot, const struct proc_packed_v1 *prepared)
{
        struct proc_packed_v1 *entry;
        struct proc *p;

        entry = proc_packed_registry_get_v33(registry, slot);
        if (!entry || !prepared || entry->span || entry->layout ||
            !proc_packed_owned_v33(slot, prepared))
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        if (!PROC_TRANSITION(p) || VM_PDP6_BASE(p) != 0UL)
                return MM_ERR_BUSY;
        *entry = *prepared;
        return MM_OK;
}

int
proc_packed_registry_discard_v33(struct proc_packed_registry_v33 *registry,
    unsigned int slot)
{
        struct proc_packed_v1 *entry;
        struct proc *p;
        int rc;

        entry = proc_packed_registry_get_v33(registry, slot);
        if (!entry || !proc_packed_owned_v33(slot, entry))
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        if (!PROC_TRANSITION(p) || VM_PDP6_BASE(p) != 0UL)
                return MM_ERR_BUSY;
        rc = mm_free(PROC_PACKED_BASE(entry), MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                return rc;
        /* After a successful MM free, no fallible operation remains. */
        entry->span = 0UL;
        entry->layout = 0UL;
        return proc_storage_change_v28(&proc_storage_runtime_v29, slot,
            PROC_STORAGE_PACKED, PROC_STORAGE_RESIDENT) == 0 ?
            MM_OK : MM_ERR_BUSY;
}
