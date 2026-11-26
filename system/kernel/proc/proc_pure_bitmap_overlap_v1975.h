/* Staged K11 bounded forward bitmap codec; no MM publication. */
#ifndef PURE_BITMAP_OVERLAP_V1975_H
#define PURE_BITMAP_OVERLAP_V1975_H
#include "mm.h"

#define PBM_NO_GAIN_V1975 1
#define PBM_OVERLAP_V1975 2

/* Caller owns stable physical spans and an independent output count cell.
 * Plan never writes either span and publishes exact count only on OK.
 * Encode rechecks the plan before any output; its count has the same ABI.
 * Unsafe overlap is a normal refusal, not permission to change ownership.
 */
int pbm_plan_encode_v1975(kword_t source, kword_t raw_words,
    kword_t target, kword_t capacity, kword_t *encoded_words);
int pbm_encode_v1975(kword_t source, kword_t raw_words,
    kword_t target, kword_t capacity, kword_t *encoded_words);
int pbm_decode_v1975(kword_t source, kword_t encoded_words,
    kword_t target, kword_t raw_words);
#endif
