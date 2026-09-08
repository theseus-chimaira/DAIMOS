#include "kinit.h"

#ifndef DAIMON_VERSION_TEXT
#error DAIMON_VERSION_TEXT must come from VERSION
#endif

static int
kinit_probe_word(volatile kword_t *addr)
{
        kword_t old;

        old = *addr;
        *addr = 0525252525252UL;
        if (*addr != 0525252525252UL) {
                *addr = old;
                return 0;
        }
        *addr = 0252525252525UL;
        if (*addr != 0252525252525UL) {
                *addr = old;
                return 0;
        }
        *addr = old;
        return 1;
}

unsigned int
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
                /* A failed probe is expected to set the PDP-6 APR NXM flag.
                 * Do not leave it pending until PI/clock initialization. */
                kinit_apr_clear();
        }
        return found;
}

void
kinit_put6_spaces(unsigned int words)
{
        while (words-- != 0U)
                kinit_put6(0);
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

        if (h != 0U)
                kinit_put6((kword_t)(020U + h));
        else
                kinit_put6(0);
        kinit_put6(((kword_t)(020U + t) << 30) |
            ((kword_t)(020U + o) << 24) |
            ((kword_t)053 << 12));
}

void
kinit_diag_banner(void)
{
#ifdef KINIT_DEBUG
        KINIT_TRACE(KINIT_DIAG_BANNER);
#endif
        kinit_put6((kword_t)SIXBIT("DAIMON"));
        kinit_put6_spaces(5U);
        kinit_put6((kword_t)SIXBIT(DAIMON_VERSION_TEXT));
        kinit_newline();
}

void
kinit_diag_system(unsigned int memory_kwords)
{
#ifdef KINIT_DEBUG
        KINIT_TRACE(KINIT_DIAG_SYSTEM);
#endif
        kinit_put6((kword_t)SIXBIT("MACH  "));
        kinit_put6_spaces(5U);
        kinit_put6((kword_t)SIXBIT(KINIT_MACHINE_NAME));
        kinit_newline();

        kinit_put6((kword_t)SIXBIT("MEM   "));
        kinit_put6_spaces(4U);
        kinit_put_memory(memory_kwords);
        kinit_newline();
}

#ifdef KINIT_DEBUG
void
kinit_diag_finished(void)
{
        kinit_put6((kword_t)SIXBIT("KINIT "));
        kinit_put6((kword_t)SIXBIT("FINISH"));
        kinit_put6((kword_t)SIXBIT("ED    "));
        kinit_newline();
}
#endif
