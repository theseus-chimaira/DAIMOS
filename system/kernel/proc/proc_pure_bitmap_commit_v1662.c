/*
 * @file proc_pure_bitmap_commit_v1662.c
 * @brief Prepared-only bitmap commit: retire RAW and publish mode3.
 *
 * This is NOT the full compressor, and is NOT linked to any live
 * scheduler/KCORE/MRES. A preparation phase must have pinned the
 * authoritative RAW suffix and allocated, encoded, and finally
 * unpinned a separate PACKED_PROCESS extent with an exact bitmap.
 * This routine independently revalidates contents and MM ownership.
 *
 * The new optional PDP-6 MM leaf validates both descriptors, then
 * deletes the once-pinned RAW without a separate fallible unpin.
 * It returns only after RAW has ceased to exist. The caller holds PI
 * disabled until it writes the existing two-bit mode3. No fallible
 * work follows that MM commit point, and TRANSITION is never cleared.
 */
#include "proc_pure_bitmap_commit_v1662.h"
#include "proc_pure_bitmap_terminal_v1661.h"
#include "proc_pure_lifecycle_v1604.h"
#include "proc_pure_admit_v1608.h"
#include "vm_pdp6_pure_range_v1604.h"
#include "vm_pdp6.h"

#if !defined(DAIMOS_PURE_BITMAP_COMMIT_V1662) || \
    DAIMOS_PURE_BITMAP_COMMIT_V1662 != 1
#error v1662_requires_explicit_test_only_bitmap_commit_build
#endif

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PBC_PTR_V1662(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PBC_PTR_V1662(x) ((kword_t *)(unsigned long)(x))
#endif

extern int proc_storage_expect_v30(unsigned int, unsigned int);
extern kword_t mach_pi_disable(void);
extern void mach_pi_restore(kword_t saved);
extern int mm_pure_bitmap_retire_v1662(unsigned int, kword_t, kword_t,
    kword_t, kword_t);

/* Pure bitmap is canonical: every present word is nonzero, and every
 * missing word decodes to zero. Read and compare without allocating a
 * second RAW-sized scratch buffer. Fail without any memory write.
 */
static int
pbc_exact_v1662(const kword_t *raw, kword_t raw_words,
    const kword_t *packed, kword_t packed_words)
{
        kword_t left = raw_words, src = 0UL, off = 0UL;

        while (left) {
                kword_t bits, bit = (kword_t)1UL << 35U;
                unsigned int i, count = left >= 36UL ? 36U :
                    (unsigned int)left;

                if (off >= packed_words)
                        return MM_ERR_INVAL;
                bits = packed[off++];
                for (i = 0U; i < count; ++i) {
                        if (bits & bit) {
                                if (off >= packed_words ||
                                    !packed[off] || packed[off] != raw[src])
                                        return MM_ERR_INVAL;
                                ++off;
                        } else if (raw[src]) {
                                return MM_ERR_INVAL;
                        }
                        ++src;
                        bit >>= 1U;
                }
                if (bit && (bits & (bit - 1UL)))
                        return MM_ERR_INVAL;
                left -= (kword_t)count;
        }
        return off == packed_words ? MM_OK : MM_ERR_INVAL;
}

/* There must be exactly TWO process extents while preparation is held:
 * a once-pinned RAW suffix and an unpinned compressed candidate.
 * Another numeric owner in MODULE/KERNEL_DYNAMIC is not this process.
 * No dereference of the physical payload precedes this owner proof.
 */
static int
pbc_prepared_v1662(unsigned int slot, kword_t raw_base,
    kword_t raw_words, kword_t encoded_base, kword_t encoded_words)
{
        unsigned int seen = 0U, i;

        if (mm_extent_count < 2 || mm_extent_count > MM_MAX_EXTENTS ||
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
                if (MM_EXTENT_BASE(e) == raw_base &&
                    MM_EXTENT_WORDS(e) == raw_words &&
                    MM_EXTENT_PINS(e) == 1U && !(seen & 1U))
                        seen |= 1U;
                else if (MM_EXTENT_BASE(e) == encoded_base &&
                    MM_EXTENT_WORDS(e) == encoded_words &&
                    !MM_EXTENT_PINS(e) && !(seen & 2U))
                        seen |= 2U;
                else
                        return MM_ERR_INVAL;
        }
        return seen == 3U ? MM_OK : MM_ERR_INVAL;
}

int
proc_pure_bitmap_commit_v1662(unsigned int slot, kword_t raw_base,
    kword_t encoded_base, kword_t encoded_words)
{
        struct proc *p;
        kword_t backing, words, skip, raw, masks, saved_pi;
        kword_t shift, mask, *modep;
        int rc;

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
            PBC_PTR_V1662(PROC_UAREA_BASE(p))[
                PROC_SWAP_BACKING_OFFSET] != backing)
                return MM_ERR_INVAL;

        raw = words - skip;
        masks = raw / 36UL + (raw % 36UL != 0UL);
        if (!raw_base || !encoded_base ||
            raw_base > MM_HALF_MASK || encoded_base > MM_HALF_MASK ||
            raw > MM_HALF_MASK + 1UL - raw_base ||
            !encoded_words || encoded_words >= raw ||
            encoded_words < masks ||
            encoded_words > MM_HALF_MASK + 1UL - encoded_base ||
            (raw_base < encoded_base + encoded_words &&
             encoded_base < raw_base + raw))
                return MM_ERR_INVAL;
        rc = pbc_prepared_v1662(slot, raw_base, raw,
            encoded_base, encoded_words);
        if (rc != MM_OK)
                return rc;
        rc = pbc_exact_v1662(PBC_PTR_V1662(raw_base), raw,
            PBC_PTR_V1662(encoded_base), encoded_words);
        if (rc != MM_OK)
                return rc;

        shift = (kword_t)(slot % PROC_STMAP_SLOTS_WORD_V28) * 2UL;
        mask = (kword_t)3UL << shift;
        modep = &proc_storage_runtime_v29.words[
            slot / PROC_STMAP_SLOTS_WORD_V28];

        saved_pi = mach_pi_disable();
        if (((*modep & mask) >> shift) != PROC_STORAGE_PACKED) {
                rc = MM_ERR_INVAL;
        } else {
                rc = mm_pure_bitmap_retire_v1662(slot, raw_base,
                    raw, encoded_base, encoded_words);
                if (rc == MM_OK) {
                        /* RAW has been deleted. These bit stores cannot
                         * fail; PI remains disabled until completion.
                         */
                        *modep = (*modep & ~mask) |
                            ((kword_t)PROC_STORAGE_BITMAP_V1661 << shift);
                }
        }
        mach_pi_restore(saved_pi);
        return rc;
}
