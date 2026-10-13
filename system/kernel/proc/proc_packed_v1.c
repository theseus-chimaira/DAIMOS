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
