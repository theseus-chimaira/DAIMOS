/* Inactive registry-backed restore transaction for dense or sparse layouts.
 * The packed source and new aligned process destination stay pinned during
 * PURE reload and decode. The packed descriptor is reconstructed from MM.
 * Only after source MM free can the registry and storage mode be retired.
 */
#include "proc_packed_owner_v37.h"
#include "proc_packed_boot_v38.h"
#include "proc_packed_v1.h"
#include "proc.h"
#include "proc_swap.h"
#include "vm_pdp6.h"
#include "mm_internal.h"

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PACKED_PTR_V46(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PACKED_PTR_V46(x) ((kword_t *)(unsigned long)(x))
#endif

int
proc_packed_restore_transaction_v46(unsigned int slot, kword_t backing,
    proc_packed_decoder_v26 decoder,
    proc_packed_pure_reload_v31 reload_pure)
{
        struct proc_packed_v1 d;
        struct proc *p;
        struct proc_packed_layout_v36 *r = &proc_packed_runtime_v38;
        kword_t *entry;
        kword_t source, base, size;
        unsigned int offset, decoded, encoded, index;
        int rc;

        if (!decoder || !reload_pure || !backing || !proc_table ||
            !proc_swap_records || !proc_storage_runtime_v29.words ||
            !slot || slot >= proc_storage_runtime_v29.slots)
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
            VM_PDP6_BASE(p) ||
            proc_swap_records[slot].state != backing ||
            proc_packed_owner_lookup_v37(r, slot, &d) != MM_OK)
                return MM_ERR_INVAL;
        size = VM_SPACE_WORDS(p);
        source = PROC_PACKED_BASE(&d);
        encoded = (unsigned int)PROC_PACKED_WORDS(&d);
        offset = (unsigned int)PROC_PACKED_OFFSET(&d);
        decoded = (unsigned int)PROC_PACKED_DECODED(&d);
        if (!size || !encoded || !decoded || offset > size ||
            decoded > size - offset || offset + decoded != size)
                return MM_ERR_INVAL;

        rc = mm_pin(source);
        if (rc != MM_OK)
                return rc;
        rc = mm_alloc_aligned_noreclaim(size, VM_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, slot, MM_ALLOC_HIGH, &base);
        if (rc != MM_OK)
                goto unpin_source;
        rc = mm_pin(base);
        if (rc != MM_OK)
                goto free_destination;
        {
                kword_t *dst = PACKED_PTR_V46(base);
                kword_t i;
                for (i = 0UL; i < (kword_t)offset; ++i)
                        dst[i] = 0UL;
        }
        if (offset && reload_pure(backing, base)) {
                rc = MM_ERR_INVAL;
                goto unpin_destination;
        }
        if (decoder(PACKED_PTR_V46(source), encoded,
            PACKED_PTR_V46(base + offset), decoded) != (int)decoded) {
                rc = MM_ERR_INVAL;
                goto unpin_destination;
        }

        rc = mm_unpin(source);
        if (rc != MM_OK)
                goto unpin_destination_source;
        rc = mm_free(source, MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                goto unpin_destination_source;

        /* MM free is the commit point: with lifecycle exclusion, the
         * previously verified registry entry cannot change here. */
        entry = proc_packed_layout_find_v36(r, slot);
        *entry = 0UL;
        if (r->format == PROC_PACKED_LAYOUT_SPARSE_V36) {
                for (index = 0U; index < r->capacity; ++index)
                        if (r->words[2U * index] == slot) {
                                r->words[2U * index] = 0UL;
                                break;
                        }
        }
        index = slot / PROC_STORAGE_MAP_SLOTS_PER_WORD_V28;
        proc_storage_runtime_v29.words[index] &= ~((kword_t)3UL <<
            ((slot % PROC_STORAGE_MAP_SLOTS_PER_WORD_V28) * 2U));
        VM_PDP6_SET_BASE(p, base);
        rc = mm_unpin(base);
        if (rc != MM_OK)
                return MM_ERR_BUSY; /* committed, keep TRANSITION */
        PROC_CLEAR_TRANSITION(p);
        return MM_OK;

unpin_destination:
        (void)mm_unpin(source);
unpin_destination_source:
        (void)mm_unpin(base);
free_destination:
        (void)mm_free(base, MM_TYPE_PROCESS, slot);
unpin_source:
        return rc;
}
