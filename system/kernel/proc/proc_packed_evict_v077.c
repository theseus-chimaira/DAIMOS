/* Staged exact-size codec-backed registry eviction v077. Not in the production kernel link.
 * Every error before freeing the resident extent preserves its ownership.
 * Once resident MM is freed, TRANSITION must never clear until the registry
 * and PACKED mode have been published and destination unpinned.
 */
#include "proc_packed_publish_v44.h"
#include "proc_packed_v1.h"
#include "proc_packed_evict_v077.h"
#include "proc.h"
#include "proc_swap.h"
#include "vm_pdp6.h"
#include "mm_internal.h"

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PACKED_PTR_V077(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PACKED_PTR_V077(x) ((kword_t *)(unsigned long)(x))
#endif

static int
packed_source_v077(unsigned int slot, kword_t base, kword_t words)
{
        int i;
        for (i = 0; i < mm_extent_count; ++i)
                if (MM_EXTENT_BASE(&mm_extents[i]) == base)
                        return MM_EXTENT_TYPE(&mm_extents[i]) ==
                            MM_TYPE_PROCESS &&
                            MM_EXTENT_OWNER(&mm_extents[i]) == slot &&
                            MM_EXTENT_WORDS(&mm_extents[i]) == words;
        return 0;
}

static int
packed_registry_room_v077(unsigned int slot)
{
        const struct proc_packed_layout_v36 *r = &proc_packed_runtime_v38;
        unsigned int i;
        int free_entry = 0;
        if (!r->words || r->slots != proc_storage_runtime_v29.slots)
                return 0;
        /* Publication requires exactly one packed extent. Reject an
         * already-owned extent before releasing the resident source. */
        for (i = 0; i < (unsigned int)mm_extent_count; ++i)
                if (MM_EXTENT_TYPE(&mm_extents[i]) ==
                        MM_TYPE_PACKED_PROCESS &&
                    MM_EXTENT_OWNER(&mm_extents[i]) == slot)
                        return 0;
        if (r->format == PROC_PACKED_LAYOUT_DENSE_V36)
                return r->words[slot] == 0UL;
        if (r->format != PROC_PACKED_LAYOUT_SPARSE_V36)
                return 0;
        for (i = 0; i < r->capacity; ++i) {
                if (r->words[2U * i] == slot)
                        return 0;
                if (!r->words[2U * i] && !r->words[2U * i + 1U])
                        free_entry = 1;
        }
        return free_entry;
}

int
proc_packed_evict_transaction_v077(unsigned int slot,
    kword_t retained_offset, proc_packed_sizer_v077 sizer,
    proc_packed_encoder_v32 encoder)
{
        struct proc *p;
        struct proc_packed_v1 prepared;
        kword_t base, packed_base, words, retained, *mode_word, mask;
        unsigned int shift;
        int rc;
        int encoded;
        unsigned int encoded_capacity;
        if (!sizer || !encoder || !proc_table || !proc_swap_records ||
            !proc_storage_runtime_v29.words || !slot ||
            slot >= proc_storage_runtime_v29.slots)
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        base = VM_PDP6_BASE(p);
        words = VM_SPACE_WORDS(p);
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) || !base ||
            !words || retained_offset >= words ||
            (retained_offset && !proc_swap_records[slot].state) ||
            !packed_source_v077(slot, base, words) ||
            proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
                PROC_STORAGE_RESIDENT || !packed_registry_room_v077(slot))
                return MM_ERR_INVAL;
        retained = words - retained_offset;
        if (retained > MM_HALF_MASK || retained_offset > MM_HALF_MASK)
                return MM_ERR_INVAL;
        rc = mm_pin(base);
        if (rc != MM_OK)
                return rc;
        /* Size the actual codec stream while MM cannot move the source.
         * The measured size, not a caller-supplied upper bound, owns RAM. */
        encoded = sizer(PACKED_PTR_V077(base + retained_offset),
            (unsigned int)retained);
        if (encoded <= 0 || (unsigned int)encoded >= retained) {
                rc = MM_ERR_INVAL;
                goto unpin_source;
        }
        encoded_capacity = (unsigned int)encoded;
        rc = mm_alloc_aligned_noreclaim((kword_t)encoded_capacity, 1UL,
            MM_TYPE_PACKED_PROCESS, slot, MM_ALLOC_HIGH, &packed_base);
        if (rc != MM_OK)
                goto unpin_source;
        rc = mm_pin(packed_base);
        if (rc != MM_OK)
                goto free_packed;
        encoded = encoder(PACKED_PTR_V077(base + retained_offset),
            (unsigned int)retained, PACKED_PTR_V077(packed_base),
            encoded_capacity);
        if (encoded != (int)encoded_capacity) {
                rc = MM_ERR_INVAL;
                goto unpin_both;
        }
        rc = mm_unpin(base);
        if (rc != MM_OK)
                goto unpin_both;
        rc = mm_free(base, MM_TYPE_PROCESS, slot);
        if (rc != MM_OK)
                goto unpin_packed;

        prepared.span = PROC_PACKED_SPAN(packed_base, encoded_capacity);
        prepared.layout = (retained_offset << 18U) | retained;
        VM_PDP6_SET_BASE(p, 0UL);
        shift = (slot % PROC_STORAGE_MAP_SLOTS_PER_WORD_V28) * 2U;
        mode_word = &proc_storage_runtime_v29.words[
            slot / PROC_STORAGE_MAP_SLOTS_PER_WORD_V28];
        mask = (kword_t)3UL << shift;
        *mode_word = (*mode_word & ~mask) |
            ((kword_t)PROC_STORAGE_PACKED << shift);
        rc = proc_packed_publish_v44(slot, &prepared);
        if (rc != MM_OK)
                return MM_ERR_BUSY; /* resident source gone: fatal */
        rc = mm_unpin(packed_base);
        if (rc != MM_OK)
                return MM_ERR_BUSY; /* valid descriptor, transition held */
        PROC_CLEAR_TRANSITION(p);
        return MM_OK;

unpin_both:
        (void)mm_unpin(base);
unpin_packed:
        (void)mm_unpin(packed_base);
        (void)mm_free(packed_base, MM_TYPE_PACKED_PROCESS, slot);
        return rc;
free_packed:
        (void)mm_free(packed_base, MM_TYPE_PACKED_PROCESS, slot);
unpin_source:
        /* Failure to allocate or pin the destination must not leak the
         * already-acquired source pin. The paths above which already
         * released the source return before reaching this label. */
        (void)mm_unpin(base);
        return rc;
}
