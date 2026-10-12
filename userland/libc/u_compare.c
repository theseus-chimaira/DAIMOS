/* Counted SIXBIT/text equality is separate so shells with native
 * dsh_s6 comparisons do not link an unused compatibility helper. */
#include "u.h"

int
u_s6_eq(const kword_t *s, const char *text)
{
        unsigned int i;
        unsigned int n;
        unsigned int wi;
        unsigned int sh;
        if (s == 0 || text == 0) return 0;
        n = (unsigned int)(s[0] & 0777777UL);
        for (i = 0U; text[i] != 0; ++i) {
                if (i >= n) return 0;
                wi = 1U + i / 6U;
                sh = 30U - (i % 6U) * 6U;
                if ((unsigned int)text[i] != (unsigned int)(((s[wi] >> sh) & 077UL) + 040UL)) return 0;
        }
        return i == n;
}
