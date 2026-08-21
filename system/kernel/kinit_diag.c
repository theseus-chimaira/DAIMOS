#include "kinit.h"

#ifndef DAIMON_VERSION_MAJOR
#error DAIMON_VERSION_MAJOR must come from VERSION
#endif
#ifndef DAIMON_VERSION_MINOR
#error DAIMON_VERSION_MINOR must come from VERSION
#endif
#if DAIMON_VERSION_MAJOR > 9 || DAIMON_VERSION_MINOR > 9
#error KINIT V0.1 banner format supports one decimal digit per component
#endif

#define KD_VALUE_END_COLUMN 40U
#define KD_MIN_SPACES       3U

static void
kd_putc(int c)
{
        (void)kinit_cty_putchar(c);
}

static void
kd_nl(void)
{
        kd_putc(015);
        kd_putc(012);
}

static void
kd_spaces(unsigned int n)
{
        while (n-- != 0U)
                kd_putc(' ');
}

static unsigned int
kd_decimal_width(unsigned int value)
{
        unsigned int n;

        n = 1U;
        while (value >= 10U) {
                value /= 10U;
                n++;
        }
        return n;
}

static void
kd_decimal(unsigned int value)
{
        if (value >= 10U)
                kd_decimal(value / 10U);
        kd_putc((int)('0' + value % 10U));
}

static unsigned int
kd_octal_width(kword_t value)
{
        unsigned int n;

        n = 1U;
        while (value >= 8UL) {
                value >>= 3;
                n++;
        }
        return n + 1U;
}

static void
kd_octal_digits(kword_t value)
{
        if (value >= 8UL)
                kd_octal_digits(value >> 3);
        kd_putc((int)('0' + (int)(value & 07UL)));
}

static void
kd_octal(kword_t value)
{
        kd_putc('0');
        kd_octal_digits(value);
}

static void
kd_sixbit_word(kword_t word, unsigned int chars)
{
        unsigned int shift;

        shift = 30U;
        while (chars-- != 0U) {
                kd_putc((int)(((word >> shift) & 077UL) + 040UL));
                shift -= 6U;
        }
}

static void
kd_pad(unsigned int left_chars, unsigned int value_chars)
{
        if (left_chars + value_chars + KD_MIN_SPACES < KD_VALUE_END_COLUMN)
                kd_spaces(KD_VALUE_END_COLUMN - left_chars - value_chars);
        else
                kd_spaces(KD_MIN_SPACES);
}

static void
kd_text_line(kword_t label, unsigned int label_chars,
    kword_t value, unsigned int value_chars)
{
        kd_sixbit_word(label, label_chars);
        kd_pad(label_chars, value_chars);
        kd_sixbit_word(value, value_chars);
        kd_nl();
}

static void
kd_range_line(kword_t label, unsigned int label_chars,
    kword_t begin, unsigned int words)
{
        kword_t end;
        unsigned int value_chars;

        end = begin + (kword_t)words - 1UL;
        value_chars = kd_octal_width(begin) + 2U + kd_octal_width(end);
        kd_sixbit_word(label, label_chars);
        kd_pad(label_chars, value_chars);
        kd_octal(begin);
        kd_putc('.');
        kd_putc('.');
        kd_octal(end);
        kd_nl();
}

static void
kd_memory_line(unsigned int kwords)
{
        unsigned int value_chars;

        kd_sixbit_word(SIXBIT("MEM   "), 3U);
        value_chars = kd_decimal_width(kwords) + 7U;
        kd_pad(3U, value_chars);
        kd_decimal(kwords);
        kd_putc(' ');
        kd_sixbit_word(SIXBIT("K CORE"), 6U);
        kd_nl();
}

static int
kd_probe_word(volatile kword_t *addr)
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
kd_memory_kwords(void)
{
        unsigned int kwords;
        unsigned int found;
        kword_t words;

        found = 32U;
        for (kwords = 32U; kwords <= 256U; kwords += 32U) {
                words = ((kword_t)kwords) << 10;
                if (kd_probe_word((volatile kword_t *)(unsigned long)(words - 1UL)))
                        found = kwords;
        }
        return found;
}

void
kinit_diag_banner(void)
{
        kword_t version;

        version = KINIT_S6_W6(' ', 'V',
            '0' + DAIMON_VERSION_MAJOR, '.',
            '0' + DAIMON_VERSION_MINOR, ' ');
        kinit_poll_put6(SIXBIT("DAIMON"));
        kinit_poll_put6(version);
        kinit_poll_put6(SIXBIT("      "));
}

void
kinit_diag_system(void)
{
        struct kinit_manifest *mp;

        mp = kinit_manifest_get();
        kd_nl();
        kd_nl();
        kd_range_line(SIXBIT("KCORE "), 5U,
            KINIT_KCORE_BASE, mp->km_kcore_words);
        kd_range_line(SIXBIT("KINIT "), 5U,
            mp->km_kinit_begin, mp->km_kinit_words);
        kd_text_line(SIXBIT("MACH  "), 4U,
            SIXBIT("PDP-6 "), 5U);
        kd_memory_line(kd_memory_kwords());
        kd_text_line(SIXBIT("CONS  "), 4U,
            SIXBIT("CTY0  "), 4U);
        kd_text_line(SIXBIT("CTY0  "), 4U,
            SIXBIT("OK    "), 2U);
        kd_nl();
}

void
kinit_diag_failure_poll(void)
{
        kinit_poll_put6(SIXBIT("?KINIT"));
}

void
kinit_diag_failure(void)
{
        kd_sixbit_word(SIXBIT("?KINIT"), 6U);
        kd_nl();
}

void
kinit_diag_finished(void)
{
        kd_sixbit_word(SIXBIT("KINIT "), 6U);
        kd_sixbit_word(SIXBIT("FINISH"), 6U);
        kd_sixbit_word(SIXBIT("ED    "), 2U);
        kd_nl();
}
