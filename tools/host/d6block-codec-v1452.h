#ifndef DAIMOS_D6BLOCK_CODEC_V1452_H
#define DAIMOS_D6BLOCK_CODEC_V1452_H

/*
 * Host-side producer and reference decoder for the fixed v1421 36-bit
 * block LZ/RLE format. Files contain one 36-bit word per 64-bit slot.
 * The PDP-6 never runs the searching encoder; memory usage here is host-only.
 */
#include <stddef.h>
#include <stdint.h>

int d6block_compress_v1452(const uint64_t *source, size_t words,
    uint64_t **packed, size_t *packed_words);
int d6block_decompress_v1452(const uint64_t *packed, size_t packed_words,
    uint64_t *destination, size_t decoded_words);

#endif
