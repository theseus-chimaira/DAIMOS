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
#define KINIT_PROBE_PATTERN_A    0525252525252UL
#define KINIT_PROBE_PATTERN_B    0252525252525UL

/*
 * Probe one physical word without changing its final contents.
 *
 * Accessing nonexistent core raises the PDP-6 APR NXM condition.  The caller
 * clears APR state after each probe so this helper only reports whether the
 * tested location behaved like writable memory.
 */
static int
kinit_probe_word(volatile kword_t *addr)
{
        kword_t old;

        old = *addr;
        *addr = KINIT_PROBE_PATTERN_A;
        if (*addr != KINIT_PROBE_PATTERN_A) {
                *addr = old;
                return 0;
        }
        *addr = KINIT_PROBE_PATTERN_B;
        if (*addr != KINIT_PROBE_PATTERN_B) {
                *addr = old;
                return 0;
        }
        *addr = old;
        return 1;
}

/**
 * @brief Detect installed core in the supported 32K..256K range.
 *
 * PDP-6 memory is tested at the last word of each 32K boundary.  The minimum
 * supported configuration is 32K, so probing starts at 64K and records the
 * highest boundary that responds as writable core.  APR state is cleared
 * after every attempt because an absent boundary normally raises NXM.
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

/**
 * @brief Emit whole blank SIXBIT words for fixed-column diagnostics.
 * @param words Number of six-character blank fields to emit.
 */
void
kinit_put6_spaces(unsigned int words)
{
        while (words-- != 0U)
                kinit_put6(0);
}

/*
 * Memory probing returns only 32K multiples from 32K through 256K.  The
 * corresponding display field is therefore a fixed eight-entry table rather
 * than a general decimal formatter.  This keeps KINIT small and avoids integer
 * division on the PDP-6 merely to print one boot-time value.
 *
 * Each row is two six-character SIXBIT words.  The leading word right-aligns
 * the hundreds digit; the second word supplies the remaining digits and "K".
 */
static const kword_t kinit_memory_text[8][2] = {
        { KINIT_SIX6(' ', ' ', ' ', ' ', ' ', ' '),
            KINIT_SIX6('3', '2', ' ', 'K', ' ', ' ') },
        { KINIT_SIX6(' ', ' ', ' ', ' ', ' ', ' '),
            KINIT_SIX6('6', '4', ' ', 'K', ' ', ' ') },
        { KINIT_SIX6(' ', ' ', ' ', ' ', ' ', ' '),
            KINIT_SIX6('9', '6', ' ', 'K', ' ', ' ') },
        { KINIT_SIX6(' ', ' ', ' ', ' ', ' ', '1'),
            KINIT_SIX6('2', '8', ' ', 'K', ' ', ' ') },
        { KINIT_SIX6(' ', ' ', ' ', ' ', ' ', '1'),
            KINIT_SIX6('6', '0', ' ', 'K', ' ', ' ') },
        { KINIT_SIX6(' ', ' ', ' ', ' ', ' ', '1'),
            KINIT_SIX6('9', '2', ' ', 'K', ' ', ' ') },
        { KINIT_SIX6(' ', ' ', ' ', ' ', ' ', '2'),
            KINIT_SIX6('2', '4', ' ', 'K', ' ', ' ') },
        { KINIT_SIX6(' ', ' ', ' ', ' ', ' ', '2'),
            KINIT_SIX6('5', '6', ' ', 'K', ' ', ' ') }
};

/*
 * Print the fixed-width memory-size field for a detected 32K multiple.
 *
 * Shifting by five divides the Kword count by 32; subtracting one maps
 * 32K..256K to table indices 0..7.
 */
static void
kinit_put_memory(unsigned int value)
{
        unsigned int index;

        index = (value >> 5) - 1U;
        kinit_put6(kinit_memory_text[index][0]);
        kinit_put6(kinit_memory_text[index][1]);
}

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
