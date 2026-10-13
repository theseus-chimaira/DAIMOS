/* D6 inline-token 36-bit word codec, host library v068.
 * Inactive until the complete DAIMOS format cutover.
 */
#ifndef DAIMOS_D6CODEC_CODEC_V068_H
#define DAIMOS_D6CODEC_CODEC_V068_H

#include <stddef.h>
#include <stdint.h>

/* Allocate the encoded stream; caller owns *outp. Empty input yields NULL. */
int d6codec_encode_v068(const uint64_t *source, size_t source_words,
    uint64_t **outp, size_t *out_words);

/* Decode exact input and output word counts into a disjoint caller buffer.
 * On failure the output is invalid even if partially written.
 */
int d6codec_decode_v068(const uint64_t *source, size_t compressed_words,
    uint64_t *output, size_t output_words);

#endif
