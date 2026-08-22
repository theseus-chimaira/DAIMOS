#include "mres.h"

void
mres_load(void)
{
        const kword_t *src;
        kword_t *dst;
        kword_t *init_end;
        kword_t *end;

        KINIT_TRACE("KLOAD ");
        src = &__resident_load_begin;
        dst = (kword_t *)(unsigned long)KINIT_KCORE_BASE;
        init_end = &__resident_low_init_end;
        end = &__resident_low_end;

        while (dst < init_end)
                *dst++ = *src++ & KINIT_WORD_MASK;
        while (dst < end)
                *dst++ = 0;
}
