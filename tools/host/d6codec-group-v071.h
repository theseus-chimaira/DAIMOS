#ifndef DAIMOS_D6CODEC_GROUP_V071_H
#define DAIMOS_D6CODEC_GROUP_V071_H
#include <stddef.h>
#include <stdint.h>
/* Staged grouped-control codec, not the production D6LZ36 ABI. */
int d6codec_group_v071_decode(const uint64_t *in, size_t in_words,
    uint64_t *out, size_t out_words);
/* Encode grouped 36-bit words into caller-provided storage.
 * Capacity is measured in 36-bit words; no allocation is performed.
 * Returns zero and writes *written on success, -1 on invalid geometry.
 */
int d6codec_group_v072_encode(const uint64_t *in, size_t in_words,
    uint64_t *out, size_t out_capacity, size_t *written);

#endif
