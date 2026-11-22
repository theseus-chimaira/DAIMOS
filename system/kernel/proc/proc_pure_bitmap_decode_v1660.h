/* v1660 test-only bounded bitmap restore admission. No mode3 publish. */
#ifndef PURE_BITMAP_DECODE_V1660_H
#define PURE_BITMAP_DECODE_V1660_H
#include "mm.h"
int proc_pure_bitmap_decode_v1660(kword_t source, kword_t encoded_words,
    kword_t target, kword_t raw_words);
#endif
