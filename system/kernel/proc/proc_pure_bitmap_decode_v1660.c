/*
 * v1660: preflight an entire untrusted compressed PURE bitmap BEFORE
 * the original v1451 unbounded decoder may write a destination word.
 * No second decoded buffer, no permanent metadata, no publication.
 * Caller owns disjoint, pinned, exact-length source and destination MM
 * extents under TRANSITION. This is an explicitly test-only leaf.
 */
#include "proc_pure_bitmap_decode_v1660.h"

#if !defined(DAIMOS_PURE_BITMAP_DECODE_V1660) || \
    DAIMOS_PURE_BITMAP_DECODE_V1660 != 1
#error v1660_requires_explicit_test_only_decoder_build
#endif

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PBD_PTR_V1660(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PBD_PTR_V1660(x) ((kword_t *)(unsigned long)(x))
#endif

/* Original assembly ABI: AC1=source, AC3=destination, AC4=RAW count.
 * AC2 is intentionally unused by the v1451 decoder.
 */
extern void bitmap_decode_v1451(kword_t, kword_t, kword_t, kword_t);

int
proc_pure_bitmap_decode_v1660(kword_t source, kword_t encoded_words,
    kword_t target, kword_t raw_words)
{
        const kword_t *packed;
        kword_t remaining, offset;

        if (!source || !target || !raw_words || !encoded_words ||
            source > MM_HALF_MASK || target > MM_HALF_MASK ||
            raw_words > MM_HALF_MASK - target ||
            encoded_words > MM_HALF_MASK - source ||
            encoded_words >= raw_words ||
            (source < target + raw_words &&
             target < source + encoded_words))
                return MM_ERR_INVAL;

        packed = PBD_PTR_V1660(source);
        remaining = raw_words;
        offset = 0UL;
        while (remaining) {
                kword_t mask, bit = (kword_t)1UL << 35U;
                unsigned int count, i;

                if (offset >= encoded_words)
                        return MM_ERR_INVAL;
                mask = packed[offset++];
                count = remaining >= 36UL ? 36U :
                    (unsigned int)remaining;
                for (i = 0U; i < count; ++i) {
                        if (mask & bit) {
                                /* Canonical v1451: every literal is
                                 * nonzero; reject malformed encodings.
                                 */
                                if (offset >= encoded_words ||
                                    !packed[offset])
                                        return MM_ERR_INVAL;
                                ++offset;
                        }
                        bit >>= 1U;
                }
                if (bit && (mask & (bit - 1UL)))
                        return MM_ERR_INVAL;
                remaining -= (kword_t)count;
        }
        if (offset != encoded_words)
                return MM_ERR_INVAL;

        /* The 30-word original decoder is safe ONLY after all masks,
         * literals, output geometry and overlap have been checked.
         * A failed preflight has written no destination words.
         */
        bitmap_decode_v1451(source, 0UL, target, raw_words);
        return MM_OK;
}
