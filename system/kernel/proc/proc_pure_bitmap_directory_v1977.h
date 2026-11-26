/* Staged K11 directory codec. NOT the interleaved v1451 stream. */
#ifndef PURE_BITMAP_DIRECTORY_V1977_H
#define PURE_BITMAP_DIRECTORY_V1977_H
#include "proc_pure_bitmap_overlap_v1975.h"

#define PBMD_FORWARD_V1977 0U
#define PBMD_REVERSE_V1977 1U
/* Unexpected write-pass failure: source may be altered, NOT RAW fallback. */
#define PBMD_WRITE_FAULT_V1977 (-6)

/* [ceil(raw/36) masks][nonzero literals in original order].
 * Both directions produce the identical canonical stream. A successful
 * plan publishes the exact shrinking count, never writes either span.
 * Normal preflight refusals leave both spans and the independent count
 * cell intact. A write-pass fault is a hard invariant failure: the spans
 * and count may have changed; the caller must quarantine the owner.
 * Caller must own stable spans throughout plan and execution. This is
 * not an MM lease or a failure-atomic ownership transaction.
 */
int pbmd_plan_encode_v1977(kword_t source, kword_t raw_words,
    kword_t target, kword_t capacity, unsigned int direction,
    kword_t *encoded_words);
int pbmd_encode_v1977(kword_t source, kword_t raw_words,
    kword_t target, kword_t capacity, unsigned int direction,
    kword_t *encoded_words);
int pbmd_decode_v1977(kword_t source, kword_t encoded_words,
    kword_t target, kword_t raw_words, unsigned int direction);
#endif
