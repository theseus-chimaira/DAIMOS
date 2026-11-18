/*
 * @file proc_pure_bitmap_terminal_v1661.c
 * @brief Staged mode3 PURE terminal release with original MM ownership.
 *
 * NOT linked to KCORE or the default/opt-in PURE MRES. Fixed KCORE's
 * mode3 refusal stays active until FULL restore and DROP commit exist.
 *
 * There is no persistent compressed-size descriptor: it is exactly the
 * span of the sole unpinned PACKED_PROCESS MM owner, which must be shorter
 * than the logical RAW suffix and at least one mask per 36 words.
 * The terminal path does not decode the dying process. In contrast to
 * restore, it is valid after immutable executable backing is unmounted.
 *
 * Caller holds TRANSITION across successful MM release and subsequent
 * swap-record/u-area retirement. The PI-excluded critical section checks
 * the current two-bit mode and ALL process MM descriptors, frees the
 * exact compressed owner, and ONLY THEN clears mode3 to mode0. No
 * fallible operation follows the MM-free commit point. On refusal the
 * old owner and mode remain authoritative for a later terminal retry.
 */
#include "proc_pure_bitmap_terminal_v1661.h"
#include "proc_pure_lifecycle_v1604.h"
#include "vm_pdp6_pure_range_v1604.h"
#include "vm_pdp6.h"
#include "mm.h"

#if !defined(DAIMOS_PURE_BITMAP_TERMINAL_V1661) || \
    DAIMOS_PURE_BITMAP_TERMINAL_V1661 != 1
#error v1661_requires_explicit_test_only_bitmap_terminal_build
#endif

extern kword_t mach_pi_disable(void);
extern void mach_pi_restore(kword_t saved);

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PBT_PTR_V1661(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PBT_PTR_V1661(x) ((kword_t *)(unsigned long)(x))
#endif

int
proc_pure_bitmap_terminal_v1661(unsigned int slot)
{
        struct proc *p;
        kword_t backing, words, skip, raw, masks, base, length;
        kword_t saved_pi, mask, shift;
        kword_t *modep;
        unsigned int found, i;
        int rc;

        rc = pure_life_prepare_v1604(slot, PURE_LIFE_TERMINAL_V1610,
            &p, &backing, &words);
        if (rc != MM_OK)
                return rc;
        if (!backing || !words ||
            (words & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            VM_PDP6_BASE(p) ||
            PBT_PTR_V1661(PROC_UAREA_BASE(p))[
                PROC_SWAP_BACKING_OFFSET] != backing ||
            pdp6_pure_skip_v1604(backing, words, &skip) != MM_OK)
                return MM_ERR_INVAL;

        raw = words - skip;
        masks = raw / 36UL + (raw % 36UL != 0UL);
        /* The existing map is two bits per slot; no new resident word. */
        shift = (kword_t)(slot % PROC_STMAP_SLOTS_WORD_V28) * 2UL;
        mask = (kword_t)3UL << shift;
        modep = &proc_storage_runtime_v29.words[
            slot / PROC_STMAP_SLOTS_WORD_V28];

        saved_pi = mach_pi_disable();
        if (((*modep & mask) >> shift) !=
            PROC_STORAGE_BITMAP_V1661) {
                rc = MM_ERR_INVAL;
                goto restore_pi_v1661;
        }
        if (mm_extent_count < 1 || mm_extent_count > MM_MAX_EXTENTS ||
            (unsigned int)mm_extent_count > mm_extent_capacity) {
                rc = MM_ERR_INVAL;
                goto restore_pi_v1661;
        }

        found = 0U;
        base = 0UL;
        length = 0UL;
        for (i = 0U; i < (unsigned int)mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int type = MM_EXTENT_TYPE(e);
                if (MM_EXTENT_OWNER(e) != slot)
                        continue;
                /* MODULE and KERNEL_DYNAMIC use independent owner
                 * namespaces. An invalid/free/reserved type carrying
                 * this process slot is NOT silently another module.
                 */
                if (type == MM_TYPE_MODULE ||
                    type == MM_TYPE_KERNEL_DYNAMIC)
                        continue;
                if (type != MM_TYPE_PROCESS &&
                    type != MM_TYPE_PACKED_PROCESS) {
                        rc = MM_ERR_INVAL;
                        goto restore_pi_v1661;
                }
                if (++found != 1U || type != MM_TYPE_PACKED_PROCESS ||
                    MM_EXTENT_PINS(e) ||
                    !(base = MM_EXTENT_BASE(e)) ||
                    !(length = MM_EXTENT_WORDS(e)) ||
                    length < masks || length >= raw ||
                    base + length > MM_HALF_MASK + 1UL) {
                        rc = MM_ERR_INVAL;
                        goto restore_pi_v1661;
                }
        }
        if (found != 1U) {
                rc = MM_ERR_INVAL;
                goto restore_pi_v1661;
        }

        /* Original MM free performs a second exact type/owner/pin check.
         * All prevalidation and mutation are serialized with PI off.
         * No changing of the executable backing or u-area is done here.
         */
        rc = mm_free(base, MM_TYPE_PACKED_PROCESS, slot);
        if (rc == MM_OK)
                *modep &= ~mask; /* mode3 -> RESIDENT; cannot fail */

restore_pi_v1661:
        mach_pi_restore(saved_pi);
        return rc;
}
