/* Counted path packer, linked only by callers that accept C strings. */
#include "u.h"

int
u_s6_pack(kword_t *dst, unsigned int words, const char *src)
{
        unsigned int i;
        unsigned int wi;
        unsigned int sh;
        unsigned int ch;
        if (dst == 0 || src == 0 || words < 2U) return -1;
        for (i = 0U; i < words; ++i) dst[i] = 0;
        for (i = 0U; src[i] != 0; ++i) {
                wi = 1U + i / 6U;
                if (wi >= words) return -1;
                sh = 30U - (i % 6U) * 6U;
                ch = (unsigned int)(unsigned char)src[i];
                if (ch >= (unsigned int)'a' && ch <= (unsigned int)'z')
                        ch -= (unsigned int)('a' - 'A');
                if (ch < 040U || ch > 0137U)
                        return -1;
                dst[wi] |= ((kword_t)(ch - 040U)) << sh;
        }
        dst[0] = i;
        return 0;
}
