/* One ownership validation and release path for KINIT-selected layouts.
 * A PACKED process always owns precisely one packed MM extent. No codec.
 * This is intentionally not linked into active KCORE pending RAM sizing.
 */
#include "proc_packed_release_v43.h"
#include "proc.h"
#include "vm_pdp6.h"

int
proc_packed_release_v43(unsigned int slot)
{
        const struct proc_packed_layout_v36 *r = &proc_packed_runtime_v38;
        kword_t *layout_word = 0;
        kword_t span = 0UL;
        unsigned int i;
        unsigned int matches = 0U;
        int rc;

        if (!r->words || !proc_table || !proc_storage_runtime_v29.words ||
            slot == 0U || slot >= r->slots ||
            r->slots != proc_storage_runtime_v29.slots ||
            !PROC_TRANSITION(&proc_table[slot]) ||
            VM_PDP6_BASE(&proc_table[slot]) != 0UL ||
            proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
                PROC_STORAGE_PACKED)
                return MM_ERR_INVAL;

        if (r->format == PROC_PACKED_LAYOUT_DENSE_V36) {
                layout_word = &r->words[slot];
        } else if (r->format == PROC_PACKED_LAYOUT_SPARSE_V36) {
                for (i = 0U; i < r->capacity; ++i)
                        if (r->words[2U * i] == slot) {
                                ++matches;
                                layout_word = &r->words[2U * i + 1U];
                        }
                if (matches != 1U)
                        return MM_ERR_INVAL;
        } else {
                return MM_ERR_INVAL;
        }
        if (!*layout_word || !(*layout_word & MM_HALF_MASK) ||
            ((*layout_word >> 18U) & MM_HALF_MASK) >
                MM_HALF_MASK - (*layout_word & MM_HALF_MASK))
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
        if (!span)
                return MM_ERR_INVAL;
        rc = mm_free(span & MM_HALF_MASK, MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                return rc;

        /* Successful MM release is the commit point. Caller holds exclusion;
         * the map and layout are now cleared without further fallible calls.
         */
        *layout_word = 0UL;
        if (r->format == PROC_PACKED_LAYOUT_SPARSE_V36)
                for (i = 0U; i < r->capacity; ++i)
                        if (r->words[2U * i] == slot) {
                                r->words[2U * i] = 0UL;
                                break;
                        }
        i = slot / PROC_STORAGE_MAP_SLOTS_PER_WORD_V28;
        proc_storage_runtime_v29.words[i] &= ~((kword_t)3UL <<
            ((slot % PROC_STORAGE_MAP_SLOTS_PER_WORD_V28) * 2U));
        return MM_OK;
}
