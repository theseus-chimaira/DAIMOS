/*
 * K11: validate the exact write order of the existing v1451 codec before
 * allowing overlapping physical spans. No scratch array or resident state.
 * Neither entry owns MM or changes process storage mode. The caller must
 * retain exclusive, stable ownership from preflight through the last store.
 */
#include "proc_pure_bitmap_overlap_v1975.h"

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PBM_PTR(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PBM_PTR(x) ((kword_t *)(unsigned long)(x))
#endif

extern void bitmap_encode_v1451(kword_t, kword_t, kword_t);
extern void bitmap_decode_v1451(kword_t, kword_t, kword_t, kword_t);

static int
pbm_span(kword_t base, kword_t words)
{
        return base && base <= MM_HALF_MASK && words &&
            words <= MM_HALF_MASK + 1UL - base;
}

/* A store may only hit memory outside the still-unread input interval. */
static int
pbm_hazard(kword_t store, kword_t unread, kword_t end)
{
        return store >= unread && store < end;
}

int
pbm_plan_encode_v1975(kword_t source, kword_t raw_words,
    kword_t target, kword_t capacity, kword_t *encoded_words)
{
        const kword_t *raw;
        kword_t in = 0UL, out = 0UL, end;
        unsigned int group = 0U;
        int hazard = 0;

        if (!encoded_words || !pbm_span(source, raw_words) ||
            !pbm_span(target, capacity))
                return MM_ERR_INVAL;
        raw = PBM_PTR(source);
        end = source + raw_words;
        while (in < raw_words) {
                /* v1451 clears each mask BEFORE reading its first word. */
                if (!group) {
                        if (pbm_hazard(target + out, source + in, end))
                                hazard = 1;
                        ++out;
                }
                if (raw[in++]) {
                        /* Literal store follows the input load. Later mask
                         * updates only touch the already validated address.
                         */
                        if (pbm_hazard(target + out, source + in, end))
                                hazard = 1;
                        ++out;
                }
                if (++group == 36U)
                        group = 0U;
        }
        if (out >= raw_words)
                return PBM_NO_GAIN_V1975;
        if (out > capacity)
                return MM_ERR_NOMEM;
        if (hazard)
                return PBM_OVERLAP_V1975;
        *encoded_words = out;
        return MM_OK;
}

int
pbm_encode_v1975(kword_t source, kword_t raw_words,
    kword_t target, kword_t capacity, kword_t *encoded_words)
{
        int rc = pbm_plan_encode_v1975(source, raw_words, target,
            capacity, encoded_words);

        if (rc == MM_OK)
                bitmap_encode_v1451(source, raw_words, target);
        return rc;
}

int
pbm_decode_v1975(kword_t source, kword_t encoded_words,
    kword_t target, kword_t raw_words)
{
        const kword_t *packed;
        kword_t in = 0UL, out = 0UL, end, mask = 0UL, bit = 0UL;
        int hazard = 0;

        if (!pbm_span(source, encoded_words) ||
            !pbm_span(target, raw_words) || encoded_words >= raw_words)
                return MM_ERR_INVAL;
        packed = PBM_PTR(source);
        end = source + encoded_words;
        while (out < raw_words) {
                if (!bit) {
                        if (in == encoded_words)
                                return MM_ERR_INVAL;
                        mask = packed[in++];
                        bit = (kword_t)1UL << 35U;
                }
                /* v1451 clears the destination BEFORE reading even a
                 * nonzero literal. Check against the current unread word.
                 */
                if (pbm_hazard(target + out, source + in, end))
                        hazard = 1;
                if (mask & bit) {
                        if (in == encoded_words || !packed[in])
                                return MM_ERR_INVAL;
                        ++in;
                }
                ++out;
                bit >>= 1U;
        }
        /* Any unused mask bits or trailing literals are noncanonical. */
        if (in != encoded_words || (bit && (mask & (bit | (bit - 1UL)))))
                return MM_ERR_INVAL;
        if (hazard)
                return PBM_OVERLAP_V1975;
        bitmap_decode_v1451(source, 0UL, target, raw_words);
        return MM_OK;
}
