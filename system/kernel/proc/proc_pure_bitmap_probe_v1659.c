/*
 * @file proc_pure_bitmap_probe_v1659.c
 * @brief Bounded, disjoint MM-backed bitmap trial, NOT compressed storage.
 *
 * This one-shot opt-in PDP-6 probe is deliberately not linked into KCORE,
 * KINIT or the v1612 service. The current two-bit storage map permits only
 * RESIDENT/DISK/RAW-PACKED; the permanent terminal teardown assumes a RAW
 * suffix of exact size. Never publish a compressed owner with that ABI.
 *
 * Count EXACT nonzero RAW words while the source is pinned; allocate the
 * smallest disjoint packed extent without reclaim, encode with the v1451
 * 30-word assembler, then verify EVERY source word against the mask/literal
 * stream and the exact consumed capacity. Finally release the temporary
 * owner before releasing the source pin. Nothing is committed. This
 * measures real transient MM cost and exercises failure atomicity without
 * fabricating mode3, a registry or any new permanent RAM allocation.
 *
 * Any uncertain unpin or free intentionally leaves the caller's existing
 * process TRANSITION held. No blind retries or synthetic owner cleanup.
 */
#include "proc_pure_bitmap_probe_v1659.h"
#include "proc_pure_lifecycle_v1604.h"
#include "proc_pure_admit_v1608.h"
#include "vm_pdp6_pure_range_v1604.h"
#include "vm_pdp6.h"
#include "proc_swap.h"
#include "proc_storage_map_v28.h"
#include "mm_internal.h"

#if !defined(DAIMOS_PURE_BITMAP_PROBE_V1659) || \
    DAIMOS_PURE_BITMAP_PROBE_V1659 != 1
#error v1659_requires_explicit_test_only_bitmap_probe_build
#endif

extern int proc_storage_expect_v30(unsigned int, unsigned int);
extern void bitmap_encode_v1451(kword_t, kword_t, kword_t);

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PBMP_PTR_V1659(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PBMP_PTR_V1659(x) ((kword_t *)(unsigned long)(x))
#endif

/* Only the original RAW suffix may belong to this process at admission.
 * A second PROCESS or PACKED_PROCESS owner is a failed transaction, not a
 * choice of one physical base. Do not accept even an unpinned duplicate.
 */
static int
pbmp_source_v1659(unsigned int slot, kword_t raw, kword_t *basep)
{
        int i;
        unsigned int found = 0U;
        kword_t base = 0UL;

        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int type = MM_EXTENT_TYPE(e);
                if (MM_EXTENT_OWNER(e) != slot ||
                    (type != MM_TYPE_PROCESS &&
                     type != MM_TYPE_PACKED_PROCESS))
                        continue;
                if (++found != 1U || type != MM_TYPE_PACKED_PROCESS ||
                    MM_EXTENT_WORDS(e) != raw ||
                    MM_EXTENT_PINNED(e) ||
                    !(base = MM_EXTENT_BASE(e)))
                        return MM_ERR_INVAL;
        }
        if (found != 1U)
                return MM_ERR_INVAL;
        *basep = base;
        return MM_OK;
}

/* Exact validation of the trusted stream, including short trailing group.
 * The encoder cannot outrun the precomputed capacity unless it is faulty.
 * No second RAW-sized scratch image is required for this comparison.
 */
static int
pbmp_verify_v1659(const kword_t *raw, kword_t raw_words,
    const kword_t *packed, kword_t packed_words)
{
        kword_t remaining = raw_words, offset = 0UL, src = 0UL;

        /* Work group by group. There is no 36-bit DIV/REM in the hot
         * per-word loop; only a single bound check for each mask.
         */
        while (remaining) {
                kword_t bits, bit = (kword_t)1UL << 35U;
                unsigned int count = remaining >= 36UL ? 36U :
                    (unsigned int)remaining;
                unsigned int i;

                if (offset == packed_words)
                        return MM_ERR_INVAL;
                bits = packed[offset++];
                for (i = 0U; i < count; ++i) {
                        kword_t restored = 0UL;
                        if (bits & bit) {
                                if (offset == packed_words)
                                        return MM_ERR_INVAL;
                                restored = packed[offset++];
                        }
                        if (restored != raw[src++])
                                return MM_ERR_INVAL;
                        bit >>= 1U;
                }
                /* Unused low bits in the last partial bitmap must be
                 * zero. With a full group BIT has shifted down to 0.
                 */
                if (bit && (bits & (bit - 1UL)))
                        return MM_ERR_INVAL;
                remaining -= (kword_t)count;
        }
        return offset == packed_words ? MM_OK : MM_ERR_INVAL;
}

int
proc_pure_bitmap_probe_v1659(unsigned int slot, kword_t *packed_words)
{
        struct proc *p;
        kword_t backing, words, skip, raw, source, target;
        kword_t nonzero, encoded, masks, i;
        const kword_t *data;
        kword_t *u;
        int rc;

        if (!packed_words || *packed_words != 0UL)
                return MM_ERR_INVAL;
        rc = pure_life_prepare_v1604(slot, PURE_LIFE_DROP_V1610,
            &p, &backing, &words);
        if (rc != MM_OK)
                return rc;
        if (pure_admit_source_v1608(p, backing) != MM_OK ||
            pdp6_pure_skip_v1604(backing, words, &skip) != MM_OK ||
            proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0 ||
            VM_PDP6_BASE(p))
                return MM_ERR_INVAL;
        u = PBMP_PTR_V1659(PROC_UAREA_BASE(p));
        if (u[PROC_SWAP_BACKING_OFFSET] != backing)
                return MM_ERR_INVAL;
        raw = words - skip;
        rc = pbmp_source_v1659(slot, raw, &source);
        if (rc != MM_OK)
                return rc;
        rc = mm_pin(source);
        if (rc != MM_OK)
                return rc;

        masks = raw / 36UL + (raw % 36UL != 0UL);
        nonzero = 0UL;
        data = PBMP_PTR_V1659(source);
        for (i = 0UL; i < raw; ++i)
                if (data[i])
                        ++nonzero;
        encoded = masks + nonzero;
        if (!encoded || encoded >= raw) {
                rc = PURE_BITMAP_NO_GAIN_V1659;
                goto unpin_source_v1659;
        }

        rc = mm_alloc_aligned_noreclaim(encoded, 1UL,
            MM_TYPE_PACKED_PROCESS, slot, MM_ALLOC_HIGH, &target);
        if (rc != MM_OK)
                goto unpin_source_v1659;
        /* Reject invalid allocator geometry BEFORE writing a word. The
         * authoritative MM allocator is still required to reserve the
         * exact disjoint destination descriptor in the managed arena.
         */
        if (!target || target + encoded > 01000000UL ||
            (target < source + raw && source < target + encoded)) {
                rc = MM_ERR_INVAL;
                goto free_target_v1659;
        }
        rc = mm_pin(target);
        if (rc != MM_OK)
                goto free_target_v1659;

        bitmap_encode_v1451(source, raw, target);
        rc = pbmp_verify_v1659(data, raw, PBMP_PTR_V1659(target),
            encoded);
        if (rc != MM_OK)
                goto unpin_target_v1659;

        /* Destination is not published or source freed. On any cleanup
         * uncertainty leave TRANSITION held; the two MM owners cannot be
         * mistaken for one valid runnable/process-packed owner.
         */
        if (mm_unpin(target) != MM_OK)
                return MM_ERR_BUSY;
        if (mm_free(target, MM_TYPE_PACKED_PROCESS, slot) != MM_OK)
                return MM_ERR_BUSY;
        if (mm_unpin(source) != MM_OK)
                return MM_ERR_BUSY;
        if (pbmp_source_v1659(slot, raw, &source) != MM_OK)
                return MM_ERR_BUSY;
        *packed_words = encoded;
        return MM_OK;

unpin_target_v1659:
        if (mm_unpin(target) != MM_OK)
                return MM_ERR_BUSY;
free_target_v1659:
        if (mm_free(target, MM_TYPE_PACKED_PROCESS, slot) != MM_OK)
                return MM_ERR_BUSY;
unpin_source_v1659:
        if (mm_unpin(source) != MM_OK)
                return MM_ERR_BUSY;
        return rc;
}
