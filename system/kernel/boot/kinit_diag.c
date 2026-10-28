/**
 * @file kinit_diag.c
 * @brief Early KINIT memory probing and polling-console diagnostics.
 */

#include "kinit.h"

#ifndef DAIMON_VERSION_TEXT
#error DAIMON_VERSION_TEXT must come from VERSION
#endif

/*
 * Alternating probe patterns exercise both polarities of every bit in a
 * 36-bit core word.  A location counts as installed memory only if both
 * patterns can be written and read back, after which its original contents
 * are restored.
 */
/*
 * Probe one physical word without changing its final contents.
 *
 * Accessing nonexistent core raises the PDP-6 APR NXM condition.  The caller
 * clears APR state after each probe so this helper only reports whether the
 * tested location behaved like writable memory.
 */
extern int kinit_probe_word(volatile kword_t *addr);

/**
 * @brief Detect installed core in the supported 32K..256K range.
 *
 * PDP-6 core can be expanded in 8K-word modules.  Probe the last word of
 * each 8K boundary beyond the 32K boot minimum.  Installed core is a
 * contiguous prefix: stop at the first absent bank rather than counting
 * higher addresses that might alias or be independently mapped.  Clear APR
 * NXM state after each attempt, including the first unsuccessful probe.
 *
 * The returned value is in Kwords (1024 PDP-10 words), matching the boot
 * diagnostics and the conversion performed by kinit_enter().
 *
 * @return Installed memory in 1024-word units.
 */
unsigned int
kinit_memory_kwords(void)
{
        unsigned int k;
        unsigned int found;
        kword_t words;

        found = 32U;
        for (k = 40U; k <= 256U; k += 8U) {
                words = ((kword_t)k) << 10;
                if (!kinit_probe_word((volatile kword_t *)(unsigned long)(words - 1UL))) {
                        kinit_apr_clear();
                        break;
                }
                found = k;
                /* A failed probe is expected to set the PDP-6 APR NXM flag.
                 * Do not leave it pending until PI/clock initialization. */
                kinit_apr_clear();
        }
        return found;
}

/**
 * @brief Emit whole blank SIXBIT words for fixed-column diagnostics.
 * @param words Number of six-character blank fields to emit.
 */
extern void kinit_put6_spaces(unsigned int words);

/* The native PDP-6 implementation in kinit_io_pdp6.s formats 32K..256K
 * as three decimal SIXBIT digits using two IDIVI instructions. */
extern void kinit_put_memory(unsigned int value);

/**
 * @brief Print the KINIT identification banner.
 *
 *     DAIMON                              V<version>
 *
 * Output uses only the polling SIXBIT console path, so it is available before
 * the resident terminal subsystem and normal formatted I/O exist.
 */
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

/**
 * @brief Print the detected machine type and installed core size.
 *
 * These lines are deliberately simple fixed-column diagnostics suitable for
 * the earliest boot phase; they do not depend on KCORE terminal services.
 *
 * @param memory_kwords Installed memory in 1024-word units.
 */
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
/**
 * @brief Mark successful completion of the debug-visible KINIT path.
 */
void
kinit_diag_finished(void)
{
        kinit_put6((kword_t)SIXBIT("KINIT "));
        kinit_put6((kword_t)SIXBIT("FINISH"));
        kinit_put6((kword_t)SIXBIT("ED    "));
        kinit_newline();
}
#endif
