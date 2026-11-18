/*
 * v1664 test-only bounded FULL restore from mode3 compressed PURE.
 * No fixed KCORE or production MRES linkage.
 *
 * Source MM owner is retained/pinned throughout allocation, bounded
 * immutable text reload and complete bitmap decode. Before decoding
 * a single word, the v1660 validator checks the entire stream.
 * The v1664 MM leaf consumes BOTH transaction pins and deletes the
 * source with no fallible operation after its final validation.
 * VM base, mode0, backing and TRANSITION are then published under
 * the SAME PI exclusion. On refusal, keep the sole source image.
 */
#include "proc_pure_bitmap_restore_v1664.h"
#include "proc_pure_bitmap_decode_v1660.h"
#include "proc_pure_bitmap_terminal_v1661.h"
#include "proc_pure_lifecycle_v1604.h"
#include "proc_pure_admit_v1608.h"
#include "vm_pdp6_pure_range_v1604.h"
#include "vm_pdp6.h"
#include "mm_internal.h"

#if !defined(DAIMOS_PBR_V1664) || DAIMOS_PBR_V1664 != 1
#error v1664_requires_opt_in_pure_bitmap_full_restore
#endif

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PBR_PTR_V1664(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PBR_PTR_V1664(x) ((kword_t *)(unsigned long)(x))
#endif

extern kword_t mach_pi_disable(void);
extern void mach_pi_restore(kword_t saved);
extern int mm_pure_bitmap_finish_v1664(unsigned int, kword_t, kword_t,
    kword_t, kword_t);

/* Read the existing two-bit map directly. The fixed mode expectation
 * leaf intentionally disallows mode3 and must remain unchanged.
 */
static int
pbr_mode_v1664(unsigned int slot)
{
        unsigned int index;
        kword_t shift;
        if (!proc_storage_runtime_v29.words ||
            slot >= proc_storage_runtime_v29.slots)
                return MM_ERR_INVAL;
        index = slot / PROC_STMAP_SLOTS_WORD_V28;
        shift = (kword_t)(slot % PROC_STMAP_SLOTS_WORD_V28) * 2UL;
        return ((proc_storage_runtime_v29.words[index] >> shift) & 3UL)
            == (kword_t)PROC_STORAGE_BITMAP_V1661 ? MM_OK : MM_ERR_INVAL;
}

/* Exact owned source and optional target, including pin counts.
 * A same-numeric-ID MODULE/KERNEL_DYNAMIC is independent. Refuse
 * any third process owner or illegal/reserved type with this ID.
 */
static int
pbr_owners_v1664(unsigned int slot, kword_t source, kword_t encoded,
    unsigned int source_pins, kword_t target, kword_t words,
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
                if (!(seen & 1U) && type == MM_TYPE_PACKED_PROCESS &&
                    MM_EXTENT_BASE(e) == source &&
                    MM_EXTENT_WORDS(e) == encoded &&
                    MM_EXTENT_PINS(e) == source_pins)
                        seen |= 1U;
                else if (words && !(seen & 2U) &&
                    type == MM_TYPE_PROCESS &&
                    MM_EXTENT_BASE(e) == target &&
                    MM_EXTENT_WORDS(e) == words &&
                    MM_EXTENT_PINS(e) == target_pins)
                        seen |= 2U;
                else
                        return MM_ERR_INVAL;
        }
        return seen == (words ? 3U : 1U) ? MM_OK : MM_ERR_INVAL;
}

/* The unique compressed source may be of variable length; it is
 * strictly shorter than the logical RAW suffix and must contain at
 * least one presence mask for every 36 RAW words. No physical read
 * is allowed until its exact unpinned MM descriptor is established.
 */
static int
pbr_source_v1664(unsigned int slot, kword_t raw,
    kword_t *sourcep, kword_t *encodedp)
{
        unsigned int seen = 0U, i;
        kword_t masks = raw / 36UL + (raw % 36UL != 0UL);

        if (mm_extent_count < 1 || mm_extent_count > MM_MAX_EXTENTS ||
            (unsigned int)mm_extent_count > mm_extent_capacity)
                return MM_ERR_INVAL;
        for (i = 0U; i < (unsigned int)mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int type = MM_EXTENT_TYPE(e);
                kword_t base = MM_EXTENT_BASE(e);
                kword_t length = MM_EXTENT_WORDS(e);
                if (MM_EXTENT_OWNER(e) != slot ||
                    type == MM_TYPE_MODULE ||
                    type == MM_TYPE_KERNEL_DYNAMIC)
                        continue;
                if (++seen != 1U || type != MM_TYPE_PACKED_PROCESS ||
                    MM_EXTENT_PINS(e) || !base ||
                    length < masks || length >= raw ||
                    base > MM_HALF_MASK ||
                    length > MM_HALF_MASK + 1UL - base)
                        return MM_ERR_INVAL;
                *sourcep = base;
                *encodedp = length;
        }
        return seen == 1U ? MM_OK : MM_ERR_INVAL;
}

/* Before attempting error cleanup, recheck the exact process whose
 * compressed MM image is retained. A changed/cleared TRANSITION,
 * unexpected APR base or altered saved executable identity cannot
 * authorize either target deletion or source unpin.
 */
static int
pbr_lifecycle_v1664(unsigned int slot)
{
        struct proc *p;
        kword_t backing, words;

        if (pure_life_prepare_v1604(slot, PURE_LIFE_RESTORE_V1610,
            &p, &backing, &words) != MM_OK ||
            !backing || !words || VM_PDP6_BASE(p) ||
            PBR_PTR_V1664(PROC_UAREA_BASE(p))[
                PROC_SWAP_BACKING_OFFSET] != backing)
                return MM_ERR_INVAL;
        return MM_OK;
}

/* Safe ordinary failure: destination unpin, then free; last unpin
 * source. Verify each MM mutation before allowing caller to retry.
 * A false-success/late free refusal leaves TRANSITION held (BUSY).
 */
static int
pbr_cancel_v1664(unsigned int slot, kword_t source,
    kword_t encoded, kword_t target, kword_t words,
    unsigned int target_pins, int reason)
{
        if (pbr_lifecycle_v1664(slot) != MM_OK ||
            pbr_mode_v1664(slot) != MM_OK ||
            pbr_owners_v1664(slot, source, encoded, 1U,
                target, words, target_pins) != MM_OK)
                return MM_ERR_BUSY;
        if (target_pins) {
                if (mm_unpin(target) != MM_OK ||
                    pbr_owners_v1664(slot, source, encoded, 1U,
                        target, words, 0U) != MM_OK)
                        return MM_ERR_BUSY;
        }
        if (mm_free(target, MM_TYPE_PROCESS, slot) != MM_OK ||
            pbr_owners_v1664(slot, source, encoded, 1U,
                0UL, 0UL, 0U) != MM_OK)
                return MM_ERR_BUSY;
        if (mm_unpin(source) != MM_OK ||
            pbr_owners_v1664(slot, source, encoded, 0U,
                0UL, 0UL, 0U) != MM_OK ||
            pbr_mode_v1664(slot) != MM_OK)
                return MM_ERR_BUSY;
        return reason;
}

static int
pbr_cancel_source_v1664(unsigned int slot, kword_t source,
    kword_t encoded, int reason)
{
        if (pbr_lifecycle_v1664(slot) != MM_OK ||
            pbr_mode_v1664(slot) != MM_OK ||
            pbr_owners_v1664(slot, source, encoded, 1U,
                0UL, 0UL, 0U) != MM_OK ||
            mm_unpin(source) != MM_OK ||
            pbr_owners_v1664(slot, source, encoded, 0U,
                0UL, 0UL, 0U) != MM_OK)
                return MM_ERR_BUSY;
        return reason;
}

int
proc_pure_bitmap_restore_v1664(unsigned int slot,
    proc_pure_ram_loader_v1391 loader)
{
        struct proc *p;
        kword_t backing, words, skip, raw, source = 0UL, encoded = 0UL;
        kword_t target = 0UL, pi_state, mask, shift;
        kword_t *u, *modep;
        unsigned int i;
        int rc;

        if (!loader)
                return MM_ERR_INVAL;
        rc = pure_life_prepare_v1604(slot, PURE_LIFE_RESTORE_V1610,
            &p, &backing, &words);
        if (rc != MM_OK)
                return rc;
        if (!backing || !words ||
            (words & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            pure_admit_source_v1608(p, backing) != MM_OK ||
            pdp6_pure_skip_v1604(backing, words, &skip) != MM_OK ||
            pbr_mode_v1664(slot) != MM_OK || VM_PDP6_BASE(p))
                return MM_ERR_INVAL;
        u = PBR_PTR_V1664(PROC_UAREA_BASE(p));
        if (u[PROC_SWAP_BACKING_OFFSET] != backing)
                return MM_ERR_INVAL;
        raw = words - skip;
        rc = pbr_source_v1664(slot, raw, &source, &encoded);
        if (rc != MM_OK)
                return rc;
        rc = mm_pin(source);
        if (rc != MM_OK)
                return rc;
        if (pbr_owners_v1664(slot, source, encoded, 1U,
            0UL, 0UL, 0U) != MM_OK)
                return MM_ERR_BUSY;

        rc = mm_alloc_aligned_noreclaim(words, VM_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, slot, MM_ALLOC_HIGH, &target);
        if (rc != MM_OK)
                return pbr_cancel_source_v1664(slot, source,
                    encoded, rc);
        if (!target || target > MM_HALF_MASK ||
            words > MM_HALF_MASK + 1UL - target ||
            (source < target + words && target < source + encoded) ||
            pbr_owners_v1664(slot, source, encoded, 1U,
                target, words, 0U) != MM_OK)
                return MM_ERR_BUSY; /* uncertain allocator witness */

        rc = mm_pin(target);
        if (rc != MM_OK)
                return pbr_cancel_v1664(slot, source, encoded,
                    target, words, 0U, rc);
        if (pbr_owners_v1664(slot, source, encoded, 1U,
            target, words, 1U) != MM_OK)
                return MM_ERR_BUSY;

        /* Avoid a second full RAW-sized buffer: the loader owns only
         * this exact APR extent. Zero omitted PURE words first.
         * The source is disjoint and remains pinned throughout I/O.
         */
        for (i = 0U; (kword_t)i < skip; ++i)
                PBR_PTR_V1664(target)[i] = 0UL;
        if (loader(backing, target, PROC_UAREA_BASE(p), words) != 0)
                return pbr_cancel_v1664(slot, source, encoded,
                    target, words, 1U, MM_ERR_INVAL);

        /* v1660 preflights the ENTIRE mask/literal stream before
         * writing a single decoded RAW word to target+skip.
         */
        rc = proc_pure_bitmap_decode_v1660(source, encoded,
            target + skip, raw);
        if (rc != MM_OK)
                return pbr_cancel_v1664(slot, source, encoded,
                    target, words, 1U, rc);

        shift = (kword_t)(slot % PROC_STMAP_SLOTS_WORD_V28) * 2UL;
        mask = (kword_t)3UL << shift;
        modep = &proc_storage_runtime_v29.words[
            slot / PROC_STMAP_SLOTS_WORD_V28];
        pi_state = mach_pi_disable();
        if (((*modep & mask) >> shift) !=
            PROC_STORAGE_BITMAP_V1661 ||
            !PROC_TRANSITION(p) || VM_PDP6_BASE(p) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing) {
                rc = MM_ERR_INVAL;
        } else {
                rc = mm_pure_bitmap_finish_v1664(slot, source,
                    encoded, target, words);
                if (rc == MM_OK) {
                        /* Source MM extent is gone and APR is unpinned.
                         * Every remaining operation is a nonfallible
                         * store while PI and TRANSITION are held.
                         */
                        VM_PDP6_SET_BASE(p, target);
                        u[PROC_SWAP_BACKING_OFFSET] = 0UL;
                        *modep &= ~mask; /* mode3 -> RESIDENT */
                        PROC_CLEAR_TRANSITION(p);
                }
        }
        mach_pi_restore(pi_state);
        if (rc != MM_OK)
                return pbr_cancel_v1664(slot, source, encoded,
                    target, words, 1U, rc);
        return MM_OK;
}
