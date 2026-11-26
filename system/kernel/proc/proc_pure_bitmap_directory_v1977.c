/*
 * A fixed mask directory makes the existing zero-presence representation
 * directly traversable in either direction, at identical encoded size.
 * No group buffer, index array, permanent state or extra stream word.
 * Preflight and execution share the exact same load/store schedule.
 * Caller retains stable exclusive physical spans across both passes.
 */
#include "proc_pure_bitmap_directory_v1977.h"

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PBMD_PTR(a) ((kword_t *)(unsigned long)((a) * sizeof(kword_t)))
#else
#define PBMD_PTR(a) ((kword_t *)(unsigned long)(a))
#endif

static int
pbmd_span(kword_t base, kword_t words)
{
        /* Low AC addresses are not MM-owned memory. */
        return base >= 020UL && base <= MM_HALF_MASK && words &&
            words <= MM_HALF_MASK + 1UL - base;
}

/* Native leaf uses four register arguments. Lengths fit in two halves
 * after span validation; the internal mode is never supplied by a caller.
 * Preflight and write use the same traversal, with stores disabled during
 * preflight. The validated spans must remain stable between the passes.
 */
#define PBMD_DECODE 1U
#define PBMD_WRITE  2U
#define PBMD_REVERSE 4U
extern int pbmd_walk_v1977(kword_t, kword_t, kword_t, unsigned int);

#define PBMD_WALK(r,n,p,c,d,m) pbmd_walk_v1977((r),(p), \
    (((kword_t)(n)) << 18U) | (kword_t)(c), \
    (m) | ((d) ? PBMD_REVERSE : 0U))

int
pbmd_plan_encode_v1977(kword_t source, kword_t raw_words,
    kword_t target, kword_t capacity, unsigned int direction,
    kword_t *encoded_words)
{
        long n, i;
        int rc;
        if (!encoded_words || direction > PBMD_REVERSE_V1977 ||
            !pbmd_span(source, raw_words) || !pbmd_span(target, capacity))
                return MM_ERR_INVAL;
        n = ((long)raw_words + 35L) / 36L;
        for (i = 0L; i < (long)raw_words; ++i)
                if (*PBMD_PTR(source + i))
                        ++n;
        if (n >= (long)raw_words)
                return PBM_NO_GAIN_V1975;
        if (n > (long)capacity)
                return MM_ERR_NOMEM;
        rc = PBMD_WALK(source, raw_words, target, n, direction, 0U);
        if (rc == MM_OK)
                *encoded_words = n;
        return rc;
}

int
pbmd_encode_v1977(kword_t source, kword_t raw_words,
    kword_t target, kword_t capacity, unsigned int direction,
    kword_t *encoded_words)
{
        int rc = pbmd_plan_encode_v1977(source, raw_words, target,
            capacity, direction, encoded_words);
        /* Under stable ownership the identical write schedule cannot
         * fail. Never report success or an ordinary RAW fallback if that
         * invariant is broken after the first store. */
        if (rc == MM_OK && PBMD_WALK(source, raw_words, target,
            *encoded_words, direction, PBMD_WRITE) != MM_OK)
                return PBMD_WRITE_FAULT_V1977;
        return rc;
}

int
pbmd_decode_v1977(kword_t source, kword_t encoded_words,
    kword_t target, kword_t raw_words, unsigned int direction)
{
        int rc;
        if (direction > PBMD_REVERSE_V1977 ||
            !pbmd_span(source, encoded_words) || !pbmd_span(target, raw_words) ||
            encoded_words >= raw_words ||
            encoded_words < (raw_words + 35UL) / 36UL)
                return MM_ERR_INVAL;
        rc = PBMD_WALK(target, raw_words, source, encoded_words,
            direction, PBMD_DECODE);
        if (rc == MM_OK && PBMD_WALK(target, raw_words, source, encoded_words,
            direction, PBMD_DECODE | PBMD_WRITE) != MM_OK)
                return PBMD_WRITE_FAULT_V1977;
        return rc;
}

/* Exact read-only equality, not just canonical shape. The same decode
 * walker checks masks, nonzero literals and exact consumption. */
int
pbmd_verify_v1979(kword_t raw_base, kword_t raw_words,
    kword_t packed_base, kword_t packed_words)
{
        if (!pbmd_span(raw_base, raw_words) ||
            !pbmd_span(packed_base, packed_words) || packed_words >= raw_words ||
            packed_words < (raw_words + 35UL) / 36UL)
                return MM_ERR_INVAL;
        return PBMD_WALK(raw_base, raw_words, packed_base, packed_words,
            PBMD_FORWARD_V1977, PBMD_DECODE | 020U);
}
