#include "kinit.h"

#ifndef DAIMON_VERSION_MAJOR
#error DAIMON_VERSION_MAJOR must come from VERSION
#endif
#ifndef DAIMON_VERSION_MINOR
#error DAIMON_VERSION_MINOR must come from VERSION
#endif
#if DAIMON_VERSION_MAJOR > 9 || DAIMON_VERSION_MINOR > 9
#error KINIT banner supports one decimal digit per version component
#endif

static int
kinit_probe_word(volatile kword_t *addr)
{
        kword_t old;
        kword_t p1;
        kword_t p2;

        old = *addr;
        p1 = ((kword_t)(unsigned long)addr ^ 0525252525252UL) & KINIT_WORD_MASK;
        p2 = ((kword_t)(unsigned long)addr ^ 0252525252525UL) & KINIT_WORD_MASK;
        *addr = p1;
        if ((*addr & KINIT_WORD_MASK) != p1) {
                *addr = old;
                return 0;
        }
        *addr = p2;
        if ((*addr & KINIT_WORD_MASK) != p2) {
                *addr = old;
                return 0;
        }
        *addr = old;
        return 1;
}

static unsigned int
kinit_memory_kwords(void)
{
        unsigned int k;
        unsigned int found;
        kword_t words;

        found = 32U;
        for (k = 64U; k <= 256U; k += 32U) {
                words = ((kword_t)k) << 10;
                if (kinit_probe_word((volatile kword_t *)(unsigned long)(words - 1UL)))
                        found = k;
        }
        return found;
}

static void
kinit_put_blank_words(unsigned int words)
{
        while (words != 0U) {
                kinit_put6(PDP10_SIXBIT6(' ',' ',' ',' ',' ',' '));
                --words;
        }
}

static void
kinit_put_memory(unsigned int value)
{
        unsigned int h;
        unsigned int t;
        unsigned int o;

        h = (value / 100U) % 10U;
        t = (value / 10U) % 10U;
        o = value % 10U;

        if (h != 0U) {
                kinit_put6(PDP10_SIXBIT6(' ',' ',' ',' ',' ','0' + h));
        } else {
                kinit_put6(PDP10_SIXBIT6(' ',' ',' ',' ',' ',' '));
        }
        kinit_put6(PDP10_SIXBIT6('0' + t, '0' + o, ' ','K',' ',' '));
}

void
kinit_diag_banner(void)
{
        KINIT_TRACE(PDP10_SIXBIT6('K','B','A','N','N','R'));
        kinit_put6(PDP10_SIXBIT6('D','A','I','M','O','N'));
        kinit_put_blank_words(5U);
        kinit_put6(PDP10_SIXBIT6('V',
            '0' + DAIMON_VERSION_MAJOR, '.',
            '0' + DAIMON_VERSION_MINOR, ' ', ' '));
        kinit_newline();
}

void
kinit_diag_system(void)
{
        KINIT_TRACE(PDP10_SIXBIT6('K','D','I','A','G','S'));
        kinit_put6(PDP10_SIXBIT6('M','A','C','H',' ',' '));
        kinit_put_blank_words(5U);
        kinit_put6(KINIT_MACHINE_NAME);
        kinit_newline();
        kinit_put6(PDP10_SIXBIT6('M','E','M',' ',' ',' '));
        kinit_put_blank_words(4U);
        kinit_put_memory(kinit_memory_kwords());
        kinit_newline();
}

void
kinit_diag_finished(void)
{
        kinit_put6(PDP10_SIXBIT6('K','I','N','I','T',' '));
        kinit_put6(PDP10_SIXBIT6('F','I','N','I','S','H'));
        kinit_put6(PDP10_SIXBIT6('E','D',' ',' ',' ',' '));
        kinit_newline();
}
