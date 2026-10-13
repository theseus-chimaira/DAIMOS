/* Compact MM-authoritative publication; paired with v43 release.
 * This is not linked to the active kernel until scheduler transaction
 * ordering and restore consumption have been reconciled.
 */
#include "proc_packed_publish_v44.h"
#include "proc.h"
#include "vm_pdp6.h"

int
proc_packed_publish_v44(unsigned int slot,
    const struct proc_packed_v1 *prepared)
{
        struct proc_packed_layout_v36 *r = &proc_packed_runtime_v38;
        kword_t *word = 0;
        kword_t span = 0UL;
        unsigned int i;
        unsigned int free_index = 0U;
        int have_free = 0;

        if (!prepared || !r->words || !proc_table ||
            !proc_storage_runtime_v29.words || !slot || slot >= r->slots ||
            r->slots != proc_storage_runtime_v29.slots ||
            !PROC_TRANSITION(&proc_table[slot]) ||
            VM_PDP6_BASE(&proc_table[slot]) ||
            proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
                PROC_STORAGE_PACKED ||
            !PROC_PACKED_DECODED(prepared) ||
            PROC_PACKED_OFFSET(prepared) >
                MM_HALF_MASK - PROC_PACKED_DECODED(prepared))
                return MM_ERR_INVAL;

        for (i = 0U; i < (unsigned int)mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                if (MM_EXTENT_TYPE(e) != MM_TYPE_PACKED_PROCESS ||
                    MM_EXTENT_OWNER(e) != slot)
                        continue;
                if (span || !MM_EXTENT_BASE(e) || !MM_EXTENT_WORDS(e))
                        return MM_ERR_INVAL;
                span = e->span;
        }
        if (!span || span != prepared->span)
                return MM_ERR_INVAL;

        if (r->format == PROC_PACKED_LAYOUT_DENSE_V36) {
                word = &r->words[slot];
                if (*word)
                        return MM_ERR_INVAL;
        } else if (r->format == PROC_PACKED_LAYOUT_SPARSE_V36) {
                for (i = 0U; i < r->capacity; ++i) {
                        if (r->words[2U * i] == slot)
                                return MM_ERR_INVAL;
                        if (!r->words[2U * i] && !have_free) {
                                have_free = 1;
                                free_index = i;
                        }
                }
                if (!have_free)
                        return MM_ERR_DESCRIPTORS;
                word = &r->words[2U * free_index + 1U];
                if (*word)
                        return MM_ERR_INVAL;
        } else {
                return MM_ERR_INVAL;
        }

        *word = prepared->layout;
        if (r->format == PROC_PACKED_LAYOUT_SPARSE_V36)
                r->words[2U * free_index] = slot;
        return MM_OK;
}
