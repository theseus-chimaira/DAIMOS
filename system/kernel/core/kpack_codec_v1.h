/* Provisional lossless process-state codec ABI, version 1. */
#ifndef DAIMOS_KPACK_CODEC_V1_H
#define DAIMOS_KPACK_CODEC_V1_H
#include "kcore.h"
/* Returns words written or -1. Source and destination must not overlap. */
int kpack_encode_v1(const kword_t *src, unsigned int words,
    kword_t *dst, unsigned int capacity);
int kpack_decode_v1(const kword_t *src, unsigned int words,
    kword_t *dst, unsigned int expected_words);
#endif
