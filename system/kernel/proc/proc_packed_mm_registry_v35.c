/* MM extent descriptors are authoritative for physical packed storage.
 * Every call requires exclusive access to process lifecycle / MM metadata.
 */
#include "proc_packed_mm_registry_v35.h"
#include "proc.h"
#include "vm_pdp6.h"

static int
packed_extent_v35(unsigned int slot, struct proc_packed_v1 *d)
{
        int i;
        int found = 0;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                if (MM_EXTENT_TYPE(e) == MM_TYPE_PACKED_PROCESS &&
                    MM_EXTENT_OWNER(e) == slot) {
                        if (found || !MM_EXTENT_WORDS(e) ||
                            !MM_EXTENT_BASE(e))
                                return MM_ERR_INVAL;
                        d->span = e->span;
                        found = 1;
                }
        }
        return found ? MM_OK : MM_ERR_INVAL;
}

int
proc_packed_mm_init_v35(struct proc_packed_mm_registry_v35 *registry,
    struct proc_packed_mm_entry_v35 *entries, unsigned int capacity)
{
        unsigned int i;
        if (!registry || !entries || registry->entries || registry->capacity ||
            !capacity || capacity >= PROC_MAX_SLOTS)
                return MM_ERR_INVAL;
        for (i = 0; i < capacity; ++i)
                if (entries[i].slot || entries[i].layout)
                        return MM_ERR_INVAL;
        registry->entries = entries;
        registry->capacity = capacity;
        return MM_OK;
}

int
proc_packed_mm_lookup_v35(const struct proc_packed_mm_registry_v35 *registry,
    unsigned int slot, struct proc_packed_v1 *result)
{
        unsigned int i;
        struct proc_packed_v1 d;
        int found = 0;
        if (!result || !registry || !registry->entries || !proc_table ||
            slot == 0U || slot >= proc_storage_runtime_v29.slots ||
            proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
                PROC_STORAGE_PACKED)
                return MM_ERR_INVAL;
        for (i = 0U; i < registry->capacity; ++i) {
                if (registry->entries[i].slot != slot)
                        continue;
                if (found)
                        return MM_ERR_INVAL;
                found = 1;
                d.span = 0UL;
                d.layout = registry->entries[i].layout;
                if (!PROC_PACKED_DECODED(&d) ||
                    PROC_PACKED_OFFSET(&d) >
                    MM_HALF_MASK - PROC_PACKED_DECODED(&d) ||
                    packed_extent_v35(slot, &d) != MM_OK)
                        return MM_ERR_INVAL;
                *result = d;
        }
        return found ? MM_OK : MM_ERR_INVAL;
}

int
proc_packed_mm_publish_v35(struct proc_packed_mm_registry_v35 *registry,
    unsigned int slot, const struct proc_packed_v1 *prepared)
{
        unsigned int i;
        struct proc_packed_v1 d;
        struct proc_packed_mm_entry_v35 *free_entry = 0;
        if (!registry || !registry->entries || !prepared || !proc_table ||
            slot == 0U || slot >= proc_storage_runtime_v29.slots ||
            proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
                PROC_STORAGE_PACKED ||
            !PROC_TRANSITION(&proc_table[slot]) ||
            VM_PDP6_BASE(&proc_table[slot]) ||
            !PROC_PACKED_DECODED(prepared) ||
            PROC_PACKED_OFFSET(prepared) >
                MM_HALF_MASK - PROC_PACKED_DECODED(prepared) ||
            packed_extent_v35(slot, &d) != MM_OK ||
            d.span != prepared->span)
                return MM_ERR_INVAL;
        for (i = 0U; i < registry->capacity; ++i) {
                if (registry->entries[i].slot == slot)
                        return MM_ERR_INVAL;
                if (!registry->entries[i].slot && !free_entry)
                        free_entry = &registry->entries[i];
        }
        if (!free_entry)
                return MM_ERR_DESCRIPTORS;
        free_entry->layout = prepared->layout;
        free_entry->slot = slot;
        return MM_OK;
}

int
proc_packed_mm_discard_v35(struct proc_packed_mm_registry_v35 *registry,
    unsigned int slot)
{
        struct proc_packed_v1 d;
        struct proc_packed_mm_entry_v35 *entry = 0;
        unsigned int i;
        unsigned int shift;
        kword_t mask;
        kword_t *word;
        int rc;
        if (!registry || proc_packed_mm_lookup_v35(registry, slot, &d) !=
            MM_OK || !PROC_TRANSITION(&proc_table[slot]) ||
            VM_PDP6_BASE(&proc_table[slot]))
                return MM_ERR_INVAL;
        for (i = 0U; i < registry->capacity; ++i)
                if (registry->entries[i].slot == slot) {
                        entry = &registry->entries[i];
                        break;
                }
        if (!entry)
                return MM_ERR_INVAL;
        rc = mm_free(PROC_PACKED_BASE(&d), MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                return rc;
        shift = (slot % PROC_STORAGE_MAP_SLOTS_PER_WORD_V28) * 2U;
        word = &proc_storage_runtime_v29.words[
            slot / PROC_STORAGE_MAP_SLOTS_PER_WORD_V28];
        mask = (kword_t)3UL << shift;
        *word &= ~mask;
        entry->layout = 0UL;
        entry->slot = 0UL;
        return MM_OK;
}
