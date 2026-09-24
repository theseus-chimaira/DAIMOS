#include "d6lz.h"

#ifndef __PDP10__
#define D6LZ36_DISTANCE_MASK    0177UL
#define D6LZ36_LENGTH_MASK      0177UL
#define D6LZ36_LENGTH_SHIFT     7U
#define D6LZ36_DESCRIPTOR_MASK  037777UL
#define D6LZ36_CONTROL_TOP      (((kword_t)1UL) << 35)

int
d6lz36_decode(kword_t *dst, unsigned int dst_words,
    const kword_t *src, unsigned int src_words)
{
        kword_t control;
        kword_t control_mask;
        kword_t token;
        unsigned int sp;
        unsigned int dp;
        unsigned int distance;
        unsigned int length;
        unsigned int i;

        if (dst_words == 0U)
                return 0;
        if (dst == 0 || src == 0)
                return -1;

        sp = 0U;
        dp = 0U;
        control = 0;
        control_mask = 0;
        while (dp < dst_words) {
                if (control_mask == 0) {
                        if (sp >= src_words)
                                return -1;
                        control = src[sp++];
                        control_mask = D6LZ36_CONTROL_TOP;
                }
                if (sp >= src_words)
                        return -1;
                token = src[sp++];
                if ((control & control_mask) == 0) {
                        dst[dp++] = token;
                } else {
                        if ((token & ~((kword_t)D6LZ36_DESCRIPTOR_MASK)) != 0)
                                return -1;
                        distance = (unsigned int)(token &
                            (kword_t)D6LZ36_DISTANCE_MASK) + 1U;
                        length = (unsigned int)((token >> D6LZ36_LENGTH_SHIFT) &
                            (kword_t)D6LZ36_LENGTH_MASK) + D6LZ36_MIN_MATCH;
                        if (distance > dp || length > dst_words - dp)
                                return -1;
                        for (i = 0U; i < length; ++i) {
                                dst[dp] = dst[dp - distance];
                                ++dp;
                        }
                }
                control_mask >>= 1;
        }
        return 0;
}
#endif
