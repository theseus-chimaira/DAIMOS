/*
 * @file proc_packed_v1.c
 * @brief Isolated, unaligned physical allocation for staged process packing.
 *
 * No scheduler/process state changes occur here. Callers must provide their
 * own synchronization and may not publish a descriptor before encoding has
 * completed. Ownership is checked by MM when releasing an extent.
 */
#include "proc_packed_v1.h"
#include "mm_internal.h"
#include "proc.h"
#include "vm_pdp6.h"
#include "proc_storage_map_v28.h"
#include "proc_swap.h"

/* Target physical addresses are WORD-addressed. The host testkit builds
 * the same transaction against byte-addressed native memory and supplies
 * an explicit scale; the PDP-6 build retains direct physical word pointers.
 */
#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PROC_PACKED_WORDPTR_V31(addr) \
        ((kword_t *)(unsigned long)((addr) * sizeof(kword_t)))
#else
#define PROC_PACKED_WORDPTR_V31(addr) ((kword_t *)(unsigned long)(addr))
#endif

/* Check type, owner and exact length before trusting an MM physical address.
 * This is used only by the inactive packed-process transaction interface. */
static int
proc_packed_owned_v27(unsigned int slot, kword_t base, kword_t words,
    unsigned int type)
{
        int i;

        for (i = 0; i < mm_extent_count; ++i) {
                if (MM_EXTENT_BASE(&mm_extents[i]) == base)
                        return MM_EXTENT_TYPE(&mm_extents[i]) == type &&
                            MM_EXTENT_OWNER(&mm_extents[i]) == slot &&
                            MM_EXTENT_WORDS(&mm_extents[i]) == words;
        }
        return 0;
}

int
proc_packed_reserve_v1(unsigned int slot, kword_t encoded_words,
    kword_t logical_offset, kword_t decoded_words,
    struct proc_packed_v1 *descriptor)
{
        kword_t base;
        int rc;

        if (!descriptor || descriptor->span != 0UL ||
            descriptor->layout != 0UL || slot == 0U ||
            slot >= PROC_MAX_SLOTS || encoded_words == 0UL ||
            encoded_words > MM_HALF_MASK ||
            logical_offset > MM_HALF_MASK || decoded_words == 0UL ||
            decoded_words > MM_HALF_MASK ||
            logical_offset + decoded_words > MM_HALF_MASK)
                return MM_ERR_INVAL;
        /* No reclaim recursion during eviction; no APR alignment required. */
        rc = mm_alloc_aligned_noreclaim(encoded_words, 1UL,
            MM_TYPE_PACKED_PROCESS, slot, MM_ALLOC_HIGH, &base);
        if (rc != MM_OK)
                return rc;
        descriptor->span = PROC_PACKED_SPAN(base, encoded_words);
        descriptor->layout =
            (logical_offset << 18U) | decoded_words;
        return MM_OK;
}

int
proc_packed_release_v1(unsigned int slot,
    struct proc_packed_v1 *descriptor)
{
        int rc;

        if (!descriptor || slot == 0U || slot >= PROC_MAX_SLOTS ||
            PROC_PACKED_WORDS(descriptor) == 0UL)
                return MM_ERR_INVAL;
        rc = mm_free(PROC_PACKED_BASE(descriptor),
            MM_TYPE_PACKED_PROCESS, slot);
        if (rc == MM_OK) {
                descriptor->span = 0UL;
                descriptor->layout = 0UL;
        }
        return rc;
}

/*
 * Prepare the writable suffix in a fresh APR-aligned process allocation.
 * The packed input remains owned by SLOT; no process state is changed.
 * Calling code must finish PURE reload and validate the complete image
 * before releasing the packed source and publishing the new VM base.
 *
 * Avoid MM reclaim recursion: reclaim can choose the very process being
 * restored. A shortage is a retryable failure, not an excuse to evict it.
 */
int
proc_packed_prepare_restore_v26(unsigned int slot,
    const struct proc_packed_v1 *descriptor, kword_t process_words,
    proc_packed_decoder_v26 decoder, kword_t *restored_base)
{
        kword_t base;
        kword_t packed_base;
        unsigned int encoded;
        unsigned int decoded;
        unsigned int offset;
        int rc;

        if (!descriptor || !decoder || !restored_base ||
            slot == 0U || slot >= PROC_MAX_SLOTS ||
            process_words == 0UL || process_words > MM_HALF_MASK ||
            *restored_base != 0UL)
                return MM_ERR_INVAL;
        encoded = (unsigned int)PROC_PACKED_WORDS(descriptor);
        decoded = (unsigned int)PROC_PACKED_DECODED(descriptor);
        offset = (unsigned int)PROC_PACKED_OFFSET(descriptor);
        packed_base = PROC_PACKED_BASE(descriptor);
        if (!encoded || !decoded || !packed_base ||
            !proc_packed_owned_v27(slot, packed_base, (kword_t)encoded,
                MM_TYPE_PACKED_PROCESS) ||
            offset > process_words || decoded > process_words - offset)
                return MM_ERR_INVAL;

        /* Pin source before allocating: no MM movement during decoding. */
        rc = mm_pin(packed_base);
        if (rc != MM_OK)
                return rc;
        rc = mm_alloc_aligned_noreclaim(process_words,
            VM_PDP6_ALIGN_WORDS, MM_TYPE_PROCESS, slot, MM_ALLOC_HIGH,
            &base);
        if (rc != MM_OK)
                goto fail_source;
        rc = mm_pin(base);
        if (rc != MM_OK)
                goto fail_destination;
        if (decoder((const kword_t *)(unsigned long)packed_base, encoded,
            (kword_t *)(unsigned long)(base + offset), decoded) !=
            (int)decoded) {
                rc = MM_ERR_INVAL;
                goto fail_decode;
        }
        /* Remain pinned while unpublished: compaction has no VM base yet. */
        (void)mm_unpin(packed_base);
        *restored_base = base;
        return MM_OK;

fail_decode:
        (void)mm_unpin(base);
fail_destination:
        (void)mm_free(base, MM_TYPE_PROCESS, slot);
fail_source:
        (void)mm_unpin(packed_base);
        return rc;
}

/** Release the pin and allocation of an unpublished prepared image. */
int
proc_packed_abort_restore_v26(unsigned int slot, kword_t restored_base)
{
        int rc;

        if (!restored_base || slot == 0U || slot >= PROC_MAX_SLOTS)
                return MM_ERR_INVAL;
        rc = mm_unpin(restored_base);
        if (rc != MM_OK)
                return rc;
        return mm_free(restored_base, MM_TYPE_PROCESS, slot);
}

/*
 * Inactive complete restore transaction. Called only with process lifecycle
 * exclusion and PROC_F_TRANSITION held. No scheduler call may run meanwhile.
 * The stored packed descriptor is not exposed via the legacy swap record.
 *
 * PURE reload happens first: for compressed executables the loader rebuilds
 * the entire initialized image, and writable decoding must overwrite that.
 * Keep BOTH source and destination pinned until all fallible I/O finishes.
 */
int
proc_packed_restore_transaction_v31(unsigned int slot,
    struct proc_packed_v1 *descriptor, kword_t backing,
    proc_packed_decoder_v26 decoder,
    proc_packed_pure_reload_v31 reload_pure)
{
        struct proc *p;
        kword_t base;
        kword_t source;
        kword_t size;
        unsigned int encoded;
        unsigned int decoded;
        unsigned int offset;
        unsigned int shift;
        kword_t *mode_word;
        kword_t mode_mask;
        int rc;

        if (!descriptor || !decoder || !reload_pure || !backing ||
            slot == 0U || slot >= proc_storage_runtime_v29.slots ||
            !proc_storage_runtime_v29.words)
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        size = VM_SPACE_WORDS(p);
        source = PROC_PACKED_BASE(descriptor);
        encoded = (unsigned int)PROC_PACKED_WORDS(descriptor);
        decoded = (unsigned int)PROC_PACKED_DECODED(descriptor);
        offset = (unsigned int)PROC_PACKED_OFFSET(descriptor);
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
            VM_PDP6_BASE(p) != 0UL || size == 0UL ||
            proc_swap_records == 0 ||
            proc_swap_records[slot].state != backing ||
            proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
                PROC_STORAGE_PACKED ||
            source == 0UL || encoded == 0U || decoded == 0U ||
            offset > size || decoded > size - offset ||
            offset + decoded != size ||
            !proc_packed_owned_v27(slot, source, (kword_t)encoded,
                MM_TYPE_PACKED_PROCESS))
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

        /* Always initialize omitted words before reading executable backing. */
        {
                kword_t *mem = PROC_PACKED_WORDPTR_V31(base);
                kword_t i;
                for (i = 0UL; i < (kword_t)offset; ++i)
                        mem[i] = 0UL;
        }
        if (offset != 0U && reload_pure(backing, base) != 0) {
                rc = MM_ERR_INVAL;
                goto unpin_destination;
        }
        if (decoder(PROC_PACKED_WORDPTR_V31(source), encoded,
            PROC_PACKED_WORDPTR_V31(base + offset), decoded) !=
            (int)decoded) {
                rc = MM_ERR_INVAL;
                goto unpin_destination;
        }

        /* All I/O and decoding succeeded. The map changes under the same
         * lifecycle exclusion as the VM base. Freeing still may fail, so do
         * it before publication while the process remains inaccessible. */
        rc = mm_unpin(source);
        if (rc != MM_OK)
                goto unpin_destination_no_source;
        rc = mm_free(source, MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                goto unpin_destination_source_free;
        VM_PDP6_SET_BASE(p, base);
        /* This slot's map word was validated before the transaction and
         * cannot change while lifecycle exclusion and TRANSITION are held. */
        shift = (slot % PROC_STORAGE_MAP_SLOTS_PER_WORD_V28) * 2U;
        mode_word = &proc_storage_runtime_v29.words[
            slot / PROC_STORAGE_MAP_SLOTS_PER_WORD_V28];
        mode_mask = (kword_t)3UL << shift;
        *mode_word &= ~mode_mask;       /* PACKED -> RESIDENT (zero) */
        descriptor->span = 0UL;
        descriptor->layout = 0UL;
        rc = mm_unpin(base);
        if (rc != MM_OK)
                return MM_ERR_BUSY;    /* keep TRANSITION: fatal to caller */
        PROC_CLEAR_TRANSITION(p);
        return MM_OK;

unpin_destination_source_free:
        /* The source still exists; its unpin succeeded. */
        goto unpin_destination_no_source;
unpin_destination:
        (void)mm_unpin(source);
unpin_destination_no_source:
        (void)mm_unpin(base);
free_destination:
        (void)mm_free(base, MM_TYPE_PROCESS, slot);
unpin_source:
        if (rc != MM_OK)
                return rc;
        return MM_ERR_BUSY;
}
