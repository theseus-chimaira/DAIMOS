/* MM-authoritative packed ownership and teardown shared by dense and sparse.
 * No compressor dependency, hidden allocation or process scheduler call.
 */
#include "proc_packed_owner_v37.h"
#include "proc.h"
#include "vm_pdp6.h"

/* There must be exactly one packed extent for SLOT, and it must agree with
 * the prepared span when publication is requested. MM's physical-base-sorted
 * descriptor table may move: copy its values, never retain an extent pointer.
 */
static int
proc_packed_owner_extent_v37(unsigned int slot, kword_t *span)
{
        int i;
        kword_t found = 0UL;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                if (MM_EXTENT_TYPE(e) != MM_TYPE_PACKED_PROCESS ||
                    MM_EXTENT_OWNER(e) != slot)
                        continue;
                if (found || !MM_EXTENT_BASE(e) || !MM_EXTENT_WORDS(e))
                        return MM_ERR_INVAL;
                found = e->span;
        }
        if (!found)
                return MM_ERR_INVAL;
        *span = found;
        return MM_OK;
}

static int
proc_packed_owner_valid_v37(struct proc_packed_layout_v36 *layout,
    unsigned int slot)
{
        if (!layout || !layout->words || !proc_table ||
            !proc_storage_runtime_v29.words || slot == 0U ||
            slot >= layout->slots ||
            layout->slots != proc_storage_runtime_v29.slots ||
            proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
                PROC_STORAGE_PACKED)
                return 0;
        return 1;
}

int
proc_packed_owner_lookup_v37(struct proc_packed_layout_v36 *layout,
    unsigned int slot, struct proc_packed_v1 *descriptor)
{
        kword_t *word;
        kword_t span;
        kword_t layout_word;
        if (!descriptor || !proc_packed_owner_valid_v37(layout, slot))
                return MM_ERR_INVAL;
        word = proc_packed_layout_find_v36(layout, slot);
        if (!word || !(layout_word = *word) ||
            !(layout_word & MM_HALF_MASK) ||
            ((layout_word >> 18U) & MM_HALF_MASK) >
                MM_HALF_MASK - (layout_word & MM_HALF_MASK) ||
            proc_packed_owner_extent_v37(slot, &span) != MM_OK)
                return MM_ERR_INVAL;
        descriptor->span = span;
        descriptor->layout = layout_word;
        return MM_OK;
}

int
proc_packed_owner_publish_v37(struct proc_packed_layout_v36 *layout,
    unsigned int slot, const struct proc_packed_v1 *prepared)
{
        kword_t *word;
        kword_t span;
        if (!prepared || !proc_packed_owner_valid_v37(layout, slot) ||
            !PROC_TRANSITION(&proc_table[slot]) ||
            VM_PDP6_BASE(&proc_table[slot]) ||
            !PROC_PACKED_DECODED(prepared) ||
            PROC_PACKED_OFFSET(prepared) >
                MM_HALF_MASK - PROC_PACKED_DECODED(prepared) ||
            proc_packed_owner_extent_v37(slot, &span) != MM_OK ||
            span != prepared->span)
                return MM_ERR_INVAL;
        word = proc_packed_layout_reserve_v36(layout, slot);
        if (!word)
                return MM_ERR_DESCRIPTORS;
        *word = prepared->layout;
        return MM_OK;
}

int
proc_packed_owner_discard_v37(struct proc_packed_layout_v36 *layout,
    unsigned int slot)
{
        struct proc_packed_v1 descriptor;
        unsigned int shift;
        kword_t mask;
        kword_t *map_word;
        int rc;
        if (proc_packed_owner_lookup_v37(layout, slot, &descriptor) !=
            MM_OK || !PROC_TRANSITION(&proc_table[slot]) ||
            VM_PDP6_BASE(&proc_table[slot]))
                return MM_ERR_INVAL;
        rc = mm_free(PROC_PACKED_BASE(&descriptor),
            MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                return rc;
        /* No fallible operations remain. The state is serialized by the
         * caller and the verified layout entry cannot disappear meanwhile. */
        (void)proc_packed_layout_clear_v36(layout, slot);
        shift = (slot % PROC_STORAGE_MAP_SLOTS_PER_WORD_V28) * 2U;
        map_word = &proc_storage_runtime_v29.words[
            slot / PROC_STORAGE_MAP_SLOTS_PER_WORD_V28];
        mask = (kword_t)3UL << shift;
        *map_word &= ~mask;
        return MM_OK;
}
