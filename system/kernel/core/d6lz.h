#ifndef DAIMON_D6LZ_H
#define DAIMON_D6LZ_H

#include "kcore.h"

/*
 * Kernel-internal D6LZ36 stream.
 *
 * A control word describes up to 36 tokens, most-significant bit first.
 * A zero bit selects a literal word.  A one bit selects a match descriptor:
 * bits 0..6 distance-1, bits 7..13 length-3, bits 14..35 reserved zero.
 * Output already produced is the history window; decoding allocates no memory.
 */
#define D6LZ36_WINDOW_WORDS 128U
#define D6LZ36_MIN_MATCH    3U
#define D6LZ36_MAX_MATCH    130U

int d6lz36_decode(kword_t *dst, unsigned int dst_words,
    const kword_t *src, unsigned int src_words);
int d6lz36_decode_vfs(kword_t node, kword_t file_offset,
    unsigned int src_words, kword_t dst_words_addr);

#endif
