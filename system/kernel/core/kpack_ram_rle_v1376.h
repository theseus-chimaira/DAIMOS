/**
 * @file kpack_ram_rle_v1376.h
 * @brief Lossless 36-bit-word RLE for optional swapless process retention.
 *
 * This is a staging codec, not a production KCORE dependency. It never
 * allocates memory and never writes to process disk swap. Source and
 * destination must be distinct allocations.
 */
#ifndef DAIMOS_KPACK_RAM_RLE_V1376_H
#define DAIMOS_KPACK_RAM_RLE_V1376_H

#include "kcore.h"

/* Return the exact encoded words, or -1 for invalid source or geometry.
 * The caller must compare the result with decoded_words and allocate only
 * when it produces a net RAM saving after all other storage overhead.
 */
int kpack_ram_measure_v1376(const kword_t *source,
    unsigned int decoded_words);

/* Return encoded words or -1 if CAPACITY is insufficient or input invalid.
 * A failure may leave partial output in the unpublished destination.
 */
int kpack_ram_encode_v1376(const kword_t *source,
    unsigned int decoded_words, kword_t *destination,
    unsigned int capacity);

/* Return exactly DECODED_WORDS or -1 for malformed/short/excess input.
 * A failed decode may modify unpublished destination words; the transaction
 * must discard them, never resume an incompletely reconstructed process.
 */
int kpack_ram_decode_v1376(const kword_t *source,
    unsigned int encoded_words, kword_t *destination,
    unsigned int decoded_words);

#endif
