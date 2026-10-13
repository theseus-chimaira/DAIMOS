/*
 * Temporary identity codec for eventual process-packing integration.
 * The codec uses no allocation, hidden scratch buffer or global state.
 * It does NOT enable eviction or claim to compress the writable state.
 */
#include "kpack_codec_v1.h"
int
kpack_encode_v1(const kword_t *src, unsigned int words,
    kword_t *dst, unsigned int capacity)
{
        unsigned int i;
        if (words > capacity || (words && (!src || !dst)))
                return -1;
        for (i = 0U; i < words; ++i)
                dst[i] = src[i];
        return (int)words;
}
int
kpack_decode_v1(const kword_t *src, unsigned int words,
    kword_t *dst, unsigned int expected_words)
{
        unsigned int i;
        if (words != expected_words || (words && (!src || !dst)))
                return -1;
        for (i = 0U; i < words; ++i)
                dst[i] = src[i];
        return (int)words;
}
