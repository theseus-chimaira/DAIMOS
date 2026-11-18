/*
 * v1663: bounded RAW->bitmap preparation, original MM allocation,
 * and v1662 failure-atomic commit. Explicitly NOT installed in the
 * scheduler, KCORE, KINIT or opt-in PURE MRES. No extra resident data.
 *
 * The ORIGINAL RAW suffix is always retained until v1662 has verified
 * all encoded data and atomically retired the exact once-pinned RAW
 * descriptor. Ordinary failures free the candidate first, then unpin
 * RAW. No cleanup is attempted after an uncertain pin/free result.
 * A suspect allocator witness never authorizes writing to an owner.
 */
#include "proc_pure_bitmap_compress_v1663.h"
#include "proc_pure_bitmap_commit_v1662.h"
#include "proc_pure_lifecycle_v1604.h"
#include "proc_pure_admit_v1608.h"
#include "vm_pdp6_pure_range_v1604.h"
#include "vm_pdp6.h"
#include "mm_internal.h"

#if !defined(DAIMOS_PBF_V1663) || DAIMOS_PBF_V1663 != 1
#error v1663_requires_explicit_opt_in_pure_bitmap_compress
#endif

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PBF_PTR_V1663(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PBF_PTR_V1663(x) ((kword_t *)(unsigned long)(x))
#endif

extern int proc_storage_expect_v30(unsigned int, unsigned int);
extern void bitmap_encode_v1451(kword_t, kword_t, kword_t);

/* Verify the only PROCESS/PACKED_PROCESS owners of SLOT. The MM table
 * also has separate MODULE/KERNEL_DYNAMIC namespaces with overlapping
 * numeric IDs; malformed/reserved types with our owner are refused.
 * A target length of zero means only RAW is permitted. The caller
 * must test physical disjointness BEFORE accepting the target.
 */
static int
pbf_owners_v1663(unsigned int slot, kword_t source, kword_t raw,
    unsigned int source_pins, kword_t target, kword_t encoded,
    unsigned int target_pins)
{
        unsigned int seen = 0U, i;

        if (mm_extent_count < 1 || mm_extent_count > MM_MAX_EXTENTS ||
            (unsigned int)mm_extent_count > mm_extent_capacity)
                return MM_ERR_INVAL;
        for (i = 0U; i < (unsigned int)mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int type = MM_EXTENT_TYPE(e);

                if (MM_EXTENT_OWNER(e) != slot ||
                    type == MM_TYPE_MODULE ||
                    type == MM_TYPE_KERNEL_DYNAMIC)
                        continue;
                if (type != MM_TYPE_PACKED_PROCESS)
                        return MM_ERR_INVAL;
                if (!(seen & 1U) && MM_EXTENT_BASE(e) == source &&
                    MM_EXTENT_WORDS(e) == raw &&
                    MM_EXTENT_PINS(e) == source_pins)
                        seen |= 1U;
                else if (encoded && !(seen & 2U) &&
                    MM_EXTENT_BASE(e) == target &&
                    MM_EXTENT_WORDS(e) == encoded &&
                    MM_EXTENT_PINS(e) == target_pins)
                        seen |= 2U;
                else
                        return MM_ERR_INVAL;
        }
        return seen == (encoded ? 3U : 1U) ? MM_OK : MM_ERR_INVAL;
}

/* On ordinary refusal from v1662, its invariant is: source still has
 * exactly one pin; candidate has zero. No release if this cannot be
 * proved. Any failed free leaves RAW pinned, TRANSITION and mode2.
 */
static int
pbf_cancel_v1663(unsigned int slot, kword_t source, kword_t raw,
    kword_t target, kword_t encoded, int original_rc)
{
        /* A refused commit must not have published mode3. If the
         * storage map is uncertain, destroying the candidate could
         * destroy the only recoverable image. Quarantine instead.
         */
        if (proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0 ||
            pbf_owners_v1663(slot, source, raw, 1U,
            target, encoded, 0U) != MM_OK)
                return MM_ERR_BUSY;
        if (mm_free(target, MM_TYPE_PACKED_PROCESS, slot) != MM_OK)
                return MM_ERR_BUSY;
        if (pbf_owners_v1663(slot, source, raw, 1U,
            0UL, 0UL, 0U) != MM_OK || mm_unpin(source) != MM_OK ||
            pbf_owners_v1663(slot, source, raw, 0U,
                0UL, 0UL, 0U) != MM_OK)
                return MM_ERR_BUSY;
        return original_rc;
}

int
proc_pure_bitmap_compress_v1663(unsigned int slot,
    kword_t *encoded_wordsp)
{
        struct proc *p;
        kword_t backing, words, skip, raw, source, target = 0UL;
        kword_t encoded, masks, nonzero, i;
        int rc;

        if (!encoded_wordsp || *encoded_wordsp)
                return MM_ERR_INVAL;
        rc = pure_life_prepare_v1604(slot, PURE_LIFE_DROP_V1610,
            &p, &backing, &words);
        if (rc != MM_OK)
                return rc;
        if (!backing || !words ||
            (words & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            pure_admit_source_v1608(p, backing) != MM_OK ||
            pdp6_pure_skip_v1604(backing, words, &skip) != MM_OK ||
            proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0 ||
            VM_PDP6_BASE(p) ||
            PBF_PTR_V1663(PROC_UAREA_BASE(p))[
                PROC_SWAP_BACKING_OFFSET] != backing)
                return MM_ERR_INVAL;

        raw = words - skip;
        source = 0UL;
        /* Locate the SINGLE unpinned RAW owner before its base is used
         * as a physical address. We cannot borrow a process APR base.
         */
        {
                unsigned int seen = 0U, j;
                if (mm_extent_count < 1 ||
                    mm_extent_count > MM_MAX_EXTENTS ||
                    (unsigned int)mm_extent_count > mm_extent_capacity)
                        return MM_ERR_INVAL;
                for (j = 0U; j < (unsigned int)mm_extent_count; ++j) {
                        const struct mm_extent *e = &mm_extents[j];
                        unsigned int kind = MM_EXTENT_TYPE(e);
                        if (MM_EXTENT_OWNER(e) != slot ||
                            kind == MM_TYPE_MODULE ||
                            kind == MM_TYPE_KERNEL_DYNAMIC)
                                continue;
                        if (++seen != 1U ||
                            kind != MM_TYPE_PACKED_PROCESS ||
                            MM_EXTENT_WORDS(e) != raw ||
                            MM_EXTENT_PINS(e) ||
                            !(source = MM_EXTENT_BASE(e)) ||
                            source > MM_HALF_MASK + 1UL - raw)
                                return MM_ERR_INVAL;
                }
                if (seen != 1U)
                        return MM_ERR_INVAL;
        }
        rc = mm_pin(source);
        if (rc != MM_OK)
                return rc;
        if (pbf_owners_v1663(slot, source, raw, 1U,
            0UL, 0UL, 0U) != MM_OK)
                return MM_ERR_BUSY;

        masks = raw / 36UL + (raw % 36UL != 0UL);
        nonzero = 0UL;
        for (i = 0UL; i < raw; ++i)
                if (PBF_PTR_V1663(source)[i])
                        ++nonzero;
        encoded = masks + nonzero;
        if (encoded >= raw) {
                if (mm_unpin(source) != MM_OK ||
                    pbf_owners_v1663(slot, source, raw, 0U,
                        0UL, 0UL, 0U) != MM_OK)
                        return MM_ERR_BUSY;
                return PURE_BITMAP_NO_GAIN_V1663;
        }

        rc = mm_alloc_aligned_noreclaim(encoded, 1UL,
            MM_TYPE_PACKED_PROCESS, slot, MM_ALLOC_HIGH, &target);
        if (rc != MM_OK) {
                if (pbf_owners_v1663(slot, source, raw, 1U,
                    0UL, 0UL, 0U) != MM_OK ||
                    mm_unpin(source) != MM_OK ||
                    pbf_owners_v1663(slot, source, raw, 0U,
                        0UL, 0UL, 0U) != MM_OK)
                        return MM_ERR_BUSY;
                return rc;
        }
        /* An invalid allocator witness cannot authorize writes or MM
         * repair. Leave TRANSITION and all owners quarantined.
         */
        if (!target || target > MM_HALF_MASK ||
            encoded > MM_HALF_MASK + 1UL - target ||
            (source < target + encoded && target < source + raw) ||
            pbf_owners_v1663(slot, source, raw, 1U,
                target, encoded, 0U) != MM_OK)
                return MM_ERR_BUSY;

        rc = mm_pin(target);
        if (rc != MM_OK) {
                if (pbf_owners_v1663(slot, source, raw, 1U,
                    target, encoded, 0U) != MM_OK)
                        return MM_ERR_BUSY;
                return pbf_cancel_v1663(slot, source, raw, target,
                    encoded, rc);
        }
        if (pbf_owners_v1663(slot, source, raw, 1U,
            target, encoded, 1U) != MM_OK)
                return MM_ERR_BUSY;

        bitmap_encode_v1451(source, raw, target);
        if (mm_unpin(target) != MM_OK)
                return MM_ERR_BUSY;
        if (pbf_owners_v1663(slot, source, raw, 1U,
            target, encoded, 0U) != MM_OK)
                return MM_ERR_BUSY;

        /* v1662 revalidates the COMPLETE canonical stream and both MM
         * owners BEFORE retiring RAW; no redundant full pass here.
         */
        rc = proc_pure_bitmap_commit_v1662(slot, source,
            target, encoded);
        if (rc != MM_OK)
                return pbf_cancel_v1663(slot, source, raw, target,
                    encoded, rc);

        *encoded_wordsp = encoded;
        return MM_OK;
}
