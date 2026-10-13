/* No allocation or compaction occurs inside the registry. Its capacity is
 * fixed by the owner at initialization; absence of free entries is retryable.
 */
#include "proc_packed_sparse_v34.h"
#include "proc.h"
#include "vm_pdp6.h"

static struct proc_packed_sparse_entry_v34 *
proc_packed_sparse_find_v34(const struct proc_packed_sparse_v34 *table,
    unsigned int slot)
{
        unsigned int i;
        if (!table || !table->entries || slot == 0U ||
            slot >= proc_storage_runtime_v29.slots)
                return 0;
        for (i = 0U; i < table->capacity; ++i)
                if (table->entries[i].slot == (kword_t)slot)
                        return &table->entries[i];
        return 0;
}

static int
proc_packed_sparse_owned_v34(unsigned int slot,
    const struct proc_packed_v1 *d)
{
        int i;
        kword_t base;
        kword_t length;
        if (!d || !d->span || !d->layout ||
            !PROC_PACKED_DECODED(d) ||
            PROC_PACKED_OFFSET(d) > MM_HALF_MASK - PROC_PACKED_DECODED(d))
                return 0;
        base = PROC_PACKED_BASE(d);
        length = PROC_PACKED_WORDS(d);
        if (!base || !length)
                return 0;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                if (MM_EXTENT_BASE(e) == base)
                        return MM_EXTENT_WORDS(e) == length &&
                            MM_EXTENT_OWNER(e) == slot &&
                            MM_EXTENT_TYPE(e) == MM_TYPE_PACKED_PROCESS;
        }
        return 0;
}

int
proc_packed_sparse_init_v34(struct proc_packed_sparse_v34 *table,
    struct proc_packed_sparse_entry_v34 *entries, unsigned int capacity)
{
        unsigned int i;
        if (!table || !entries || table->entries || table->capacity ||
            capacity == 0U || capacity >= PROC_MAX_SLOTS)
                return MM_ERR_INVAL;
        for (i = 0U; i < capacity; ++i)
                if (entries[i].slot || entries[i].descriptor.span ||
                    entries[i].descriptor.layout)
                        return MM_ERR_INVAL;
        table->entries = entries;
        table->capacity = capacity;
        return MM_OK;
}

struct proc_packed_v1 *
proc_packed_sparse_get_v34(const struct proc_packed_sparse_v34 *table,
    unsigned int slot)
{
        struct proc_packed_sparse_entry_v34 *e;
        if (!proc_table || proc_storage_get_v28(&proc_storage_runtime_v29,
            slot) != PROC_STORAGE_PACKED)
                return 0;
        e = proc_packed_sparse_find_v34(table, slot);
        return e ? &e->descriptor : 0;
}

int
proc_packed_sparse_publish_v34(struct proc_packed_sparse_v34 *table,
    unsigned int slot, const struct proc_packed_v1 *prepared)
{
        struct proc_packed_sparse_entry_v34 *free_entry;
        struct proc *p;
        unsigned int i;
        if (!table || !table->entries || !proc_table ||
            slot == 0U || slot >= proc_storage_runtime_v29.slots ||
            proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
                PROC_STORAGE_PACKED ||
            !proc_packed_sparse_owned_v34(slot, prepared))
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        if (!PROC_TRANSITION(p) || VM_PDP6_BASE(p))
                return MM_ERR_BUSY;
        free_entry = 0;
        for (i = 0U; i < table->capacity; ++i) {
                if (table->entries[i].slot == slot)
                        return MM_ERR_INVAL;
                if (!table->entries[i].slot && !free_entry)
                        free_entry = &table->entries[i];
        }
        if (!free_entry)
                return MM_ERR_DESCRIPTORS;
        free_entry->descriptor = *prepared;
        free_entry->slot = slot;
        return MM_OK;
}

int
proc_packed_sparse_discard_v34(struct proc_packed_sparse_v34 *table,
    unsigned int slot)
{
        struct proc_packed_sparse_entry_v34 *e;
        struct proc *p;
        int rc;
        e = proc_packed_sparse_find_v34(table, slot);
        if (!e || !proc_table ||
            proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
                PROC_STORAGE_PACKED ||
            !proc_packed_sparse_owned_v34(slot, &e->descriptor))
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        if (!PROC_TRANSITION(p) || VM_PDP6_BASE(p))
                return MM_ERR_BUSY;
        rc = mm_free(PROC_PACKED_BASE(&e->descriptor),
            MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                return rc;
        /* No fallible operation after freeing the owned extent. The state
         * transition is serialized by the caller's lifecycle exclusion. */
        {
                unsigned int shift = (slot % 18U) * 2U;
                kword_t mask = (kword_t)3UL << shift;
                kword_t *word = &proc_storage_runtime_v29.words[slot / 18U];
                *word &= ~mask;
        }
        e->descriptor.span = 0UL;
        e->descriptor.layout = 0UL;
        e->slot = 0UL;
        return MM_OK;
}
