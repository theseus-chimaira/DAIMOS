#include "kernel.h"
#include "kinit.h"
#include "bootinfo_hdd_v1.h"
#include "dboot_v1.h"
#include "module_abi.h"
#include "daimod_ids.h"
#include "dsk270.h"
#include "dtc551.h"
#include "mtc516.h"
#include "machine.h"
#include "errno.h"
#include "architecture/dec36/pdp10/io.h"

/*
 * Reclaimable boot diagnostics.
 *
 * The display is deliberately kept in KINIT.  It restores the detailed V001
 * boot report without spending permanent KCORE or fixed-MRES words.  Text is
 * packed SIXBIT and every status value ends at column 40, matching the older
 * DAIMON boot display.
 */
struct kinit_diag_text {
        kword_t word;
};

#define KD_VALUE_END_COLUMN 40U
#define KD_MIN_SPACES        3U

#define KD_TEXT(name, a, b, c, d, e, f) \
        static const struct kinit_diag_text name = { \
                S6_W6(a, b, c, d, e, f) \
        }

KD_TEXT(KD_DAIMON_KERNEL_V001, 'D','A','I','M','O','N');
KD_TEXT(KD_BAD_BOOTINFO,       'B','A','D',' ','B','I');
KD_TEXT(KD_BAD_BADMAP,         'B','A','D',' ','D','B');
KD_TEXT(KD_BAD_PAYLOAD,        'B','A','D',' ','P','L');
KD_TEXT(KD_BAD_DMANIF,         'B','A','D',' ','D','M');
KD_TEXT(KD_BOOTINFO,           'B','O','O','T','I','N');
KD_TEXT(KD_OK,                 'O','K',' ',' ',' ',' ');
KD_TEXT(KD_BOOTSET,            'B','O','O','T','S','E');
KD_TEXT(KD_GEN,                'G','E','N',' ',' ',' ');
KD_TEXT(KD_MEMBERS,            'M','E','M','B','E','R');
KD_TEXT(KD_KCORE_RANGE,        'K','C','O','R','E',' ');
KD_TEXT(KD_KINIT_RANGE,        'K','I','N','I','T',' ');
KD_TEXT(KD_WARNINGS,           'W','A','R','N','S',' ');
KD_TEXT(KD_WARN_DBOOT_BADS,    'D','B','O','O','T','!');
KD_TEXT(KD_INFO_NONBOOT_DISKS,  'N','O','N','B','O','T');
KD_TEXT(KD_WARN_INCOMP_OTHER,   'I','N','C','O','M','P');
KD_TEXT(KD_INFO_EXTRA_BOOTABLE, 'X','B','O','O','T',' ');
KD_TEXT(KD_INFO_SINGLE_DISK,    '1','D','I','S','K',' ');
KD_TEXT(KD_WARN_GEN_MISMATCH,   'G','E','N','M','I','X');
KD_TEXT(KD_MACHINE,            'M','A','C','H',' ',' ');
KD_TEXT(KD_MACHINE_VALUE,      'P','D','P','-','6',' ');
KD_TEXT(KD_CONSOLE,            'C','O','N','S',' ',' ');
KD_TEXT(KD_CONSOLE_VALUE,      'C','T','Y','0',' ',' ');
KD_TEXT(KD_MEM,                'M','E','M',' ',' ',' ');
KD_TEXT(KD_MEM_SUFFIX,         'K',' ','C','O','R','E');
KD_TEXT(KD_HZ,                 'H','Z',' ',' ',' ',' ');
KD_TEXT(KD_HZ_SUFFIX,          'L','I','N','E',' ',' ');
KD_TEXT(KD_HANDOFF,            'K','S','T','A','R','T');

static const kword_t KD_CAP_W[] = {
        S6_W6('C','A','P',':',' ',' ')
};
static const kword_t KD_BLOCK_W[] = {
        S6_W6('B','L','O','C','K',' ')
};
static const kword_t KD_DS_W[] = {
        S6_W6('D','S',':',' ',' ',' ')
};
static const kword_t KD_TS_W[] = {
        S6_W6('T','S',':',' ',' ',' ')
};
static const kword_t KD_NO_TAPE_W[] = {
        S6_W6('N','O',' ','T','A','P'),
        S6_W6('E',' ',' ',' ',' ',' ')
};
static const kword_t KD_TAPE_LOADED_W[] = {
        S6_W6('T','A','P','E',' ','L'),
        S6_W6('O','A','D','E','D',' ')
};

static kword_t
kd_word_addr_add(kword_t base, kword_t words)
{
#ifdef __PDP10__
        return base + words;
#else
        return (kword_t)(((kword_t *)base) + words);
#endif
}

static void
kd_putc(int c)
{
        (void)pdp10_cty_putc(c);
}

static void
kd_sixbit(const kword_t *words, unsigned int nchars)
{
        kword_t word;
        unsigned int n;
        unsigned int shift;

        while (nchars != 0U) {
                word = *words++;
                n = nchars < 6U ? nchars : 6U;
                shift = 30U;
                while (n-- != 0U) {
                        kd_putc((int)(((word >> shift) & 077UL) + 040UL));
                        shift -= 6U;
                        nchars--;
                }
        }
}

static unsigned int
kd_text_chars(const struct kinit_diag_text *text)
{
        kword_t word;
        unsigned int n;

        word = text->word;
        n = 6U;
        while (n != 0U && (word & 077UL) == 0UL) {
                word >>= 6;
                n--;
        }
        return n;
}

static unsigned int
kd_text(const struct kinit_diag_text *text)
{
        unsigned int n;

        n = kd_text_chars(text);
        kd_sixbit(&text->word, n);
        return n;
}

static void
kd_spaces(unsigned int n)
{
        while (n-- != 0U)
                kd_putc(' ');
}

static void
kd_nl(void)
{
        kd_putc(015);
        kd_putc(012);
}

static void
kd_line(const struct kinit_diag_text *text)
{
        kd_text(text);
        kd_nl();
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
kd_octal_width(kword_t value, int prefix_zero)
{
        unsigned int n;

        n = 1U;
        while (value >= 8UL) {
                value >>= 3;
                n++;
        }
        return n + (prefix_zero ? 1U : 0U);
}

static void
kd_octal_digits(kword_t value)
{
        if (value >= 8UL)
                kd_octal_digits(value >> 3);
        kd_putc((int)('0' + (int)(value & 07UL)));
}

static void
kd_octal(kword_t value, int prefix_zero)
{
        if (prefix_zero)
                kd_putc('0');
        kd_octal_digits(value);
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
kd_status_line(const struct kinit_diag_text *label, const struct kinit_diag_text *value)
{
        unsigned int left_chars;
        unsigned int value_chars;

        left_chars = kd_text(label);
        value_chars = kd_text_chars(value);
        kd_pad(left_chars, value_chars);
        kd_sixbit(&value->word, value_chars);
        kd_nl();
}

static void
kd_text_value_line(const struct kinit_diag_text *label, const struct kinit_diag_text *value)
{
        kd_status_line(label, value);
}

static void
kd_number_line(const struct kinit_diag_text *label, unsigned int value, const struct kinit_diag_text *suffix)
{
        unsigned int value_chars;

        value_chars = kd_decimal_width(value);
        if (suffix != 0)
                value_chars += 1U + kd_text_chars(suffix);
        kd_pad(kd_text(label), value_chars);
        kd_decimal(value);
        if (suffix != 0) {
                kd_putc(' ');
                kd_text(suffix);
        }
        kd_nl();
}

static void
kd_range_line(const struct kinit_diag_text *label, kword_t start, kword_t words)
{
        kword_t end;
        unsigned int value_chars;

        end = words == 0 ? start : start + words - 1UL;
        value_chars = kd_octal_width(start, 1) + 2U +
            kd_octal_width(end, 1);
        kd_pad(kd_text(label), value_chars);
        kd_octal(start, 1);
        kd_putc('.');
        kd_putc('.');
        kd_octal(end, 1);
        kd_nl();
}

static void
kd_bootset_line(kword_t generation)
{
        unsigned int value_chars;

        value_chars = kd_text_chars(&KD_GEN) + 1U +
            kd_decimal_width((unsigned int)generation);
        kd_pad(kd_text(&KD_BOOTSET), value_chars);
        kd_text(&KD_GEN);
        kd_putc(' ');
        kd_decimal((unsigned int)generation);
        kd_nl();
}

static void
kd_member_slot(unsigned int i)
{
        kd_putc('[');
        if (i < 10U)
                kd_putc((int)('0' + i));
        else
                kd_putc((int)('A' + i - 10U));
        kd_putc(']');
}

static void
kd_members_line(kword_t member_mask)
{
        unsigned int i;
        unsigned int value_chars;
        int any;

        value_chars = 0U;
        for (i = 0U; i < 16U; i++) {
                if ((member_mask & (1UL << i)) == 0)
                        continue;
                if (value_chars != 0U)
                        value_chars++;
                value_chars += 3U;
        }
        if (value_chars == 0U)
                value_chars = kd_text_chars(&KD_BAD_BOOTINFO);
        kd_pad(kd_text(&KD_MEMBERS), value_chars);
        any = 0;
        for (i = 0U; i < 16U; i++) {
                if ((member_mask & (1UL << i)) == 0)
                        continue;
                if (any)
                        kd_putc(' ');
                kd_member_slot(i);
                any = 1;
        }
        if (!any)
                kd_text(&KD_BAD_BOOTINFO);
        kd_nl();
}

static void
kd_warning_lines(kword_t warnings)
{
        kword_t warning_bits;

        warning_bits = warnings & ~(DBOOT_INFO_NONBOOT_DISKS |
            DBOOT_INFO_SINGLE_DISK | DBOOT_INFO_EXTRA_BOOTABLE);
        if (warning_bits != 0) {
                kd_text(&KD_WARNINGS);
                kd_octal(warning_bits, 1);
                kd_nl();
                if ((warning_bits & (DBOOT_WARN_DB0_NONDEFAULT |
                    DBOOT_WARN_DBX_SKIP | DBOOT_WARN_SOME_BADS)) != 0)
                        kd_text(&KD_WARN_DBOOT_BADS);
                        kd_nl();
                if ((warning_bits & DBOOT_WARN_INCOMP_OTHER) != 0)
                        kd_text(&KD_WARN_INCOMP_OTHER);
                        kd_nl();
                if ((warning_bits & DBOOT_WARN_GEN_MISMATCH) != 0)
                        kd_text(&KD_WARN_GEN_MISMATCH);
                        kd_nl();
        }
        if ((warnings & DBOOT_INFO_NONBOOT_DISKS) != 0)
                kd_text(&KD_INFO_NONBOOT_DISKS);
                        kd_nl();
        if ((warnings & DBOOT_INFO_SINGLE_DISK) != 0)
                kd_text(&KD_INFO_SINGLE_DISK);
                        kd_nl();
        if ((warnings & DBOOT_INFO_EXTRA_BOOTABLE) != 0)
                kd_text(&KD_INFO_EXTRA_BOOTABLE);
                        kd_nl();
}

static void
kd_module_line(unsigned int id, const kword_t *name, unsigned int name_chars, const kword_t *status, unsigned int status_chars)
{
        unsigned int indent;

        indent = (id == DAIMOD_ID_DSK270 || id == DAIMOD_ID_DTC551 ||
            id == DAIMOD_ID_MTC516) ? 2U : 0U;
        kd_spaces(indent);
        kd_sixbit(name, name_chars);
        kd_pad(indent + name_chars, status_chars);
        kd_sixbit(status, status_chars);
        kd_nl();
}

static void
kd_unit_label(int a, int b, int c, unsigned int unit)
{
        kd_spaces(4U);
        kd_putc(a);
        kd_putc(b);
        kd_putc(c);
        kd_putc((int)('0' + unit));
}

static void
kd_dsk_units(kword_t member_mask, kword_t present_mask)
{
        unsigned int i;
        unsigned int value_chars;

        value_chars = 4U +
            kd_decimal_width((unsigned int)DSK270_SECTORS_PER_UNIT) +
            1U + 5U + 1U + 4U;
        for (i = 0U; i < DSK270_UNIT_COUNT; i++) {
                if ((present_mask & (1UL << i)) == 0)
                        continue;
                kd_unit_label('D', 'S', 'K', i);
                kd_pad(8U, value_chars);
                kd_sixbit(KD_CAP_W, 4U);
                kd_decimal((unsigned int)DSK270_SECTORS_PER_UNIT);
                kd_putc(' ');
                kd_sixbit(KD_BLOCK_W, 5U);
                kd_putc(' ');
                kd_sixbit(KD_DS_W, 3U);
                kd_putc((member_mask & (1UL << i)) != 0 ? 'A' : '-');
                kd_nl();
        }
}

static int
kd_dtc_probe(unsigned int unit)
{
        kword_t status;

        pdp10_cono(DTC551_DEVICE, DTC551_CTL_SELECT |
            (((kword_t)unit) << DTC551_CTL_UNIT_SHIFT));
        status = pdp10_coni(DTC551_STATUSB_DEVICE);
        pdp10_cono(DTC551_DEVICE, 0);
        if ((status & DTC551_STB_REQ) != 0)
                return 1;
        if ((status & DTC551_STB_ILL) != 0)
                return -1;
        return 0;
}

static void
kd_dtc_units(void)
{
        unsigned int i;
        unsigned int value_chars;
        int probe;

        value_chars = 4U +
            kd_decimal_width((unsigned int)DTC551_BLOCKS_PER_TAPE) +
            1U + 5U + 1U + 4U;
        for (i = 0U; i < DTC551_UNIT_COUNT; i++) {
                probe = kd_dtc_probe(i);
                kd_unit_label('D', 'T', 'C', i);
                if (probe <= 0) {
                        kd_pad(8U, 7U);
                        kd_sixbit(KD_NO_TAPE_W, 7U);
                        kd_nl();
                        continue;
                }
                kd_pad(8U, value_chars);
                kd_sixbit(KD_CAP_W, 4U);
                kd_decimal((unsigned int)DTC551_BLOCKS_PER_TAPE);
                kd_putc(' ');
                kd_sixbit(KD_BLOCK_W, 5U);
                kd_putc(' ');
                kd_sixbit(KD_TS_W, 3U);
                kd_putc('-');
                kd_nl();
        }
}

static int
kd_mtc_probe(unsigned int unit)
{
        kword_t status;

        pdp10_cono(MTC516_CTL_DEVICE,
            ((kword_t)unit) << MTC516_CMD_UNIT_SHIFT);
        pdp10_cono(MTC516_STA_DEVICE, MTC516_ST_TAPE_RDY);
        status = pdp10_coni(MTC516_STA_DEVICE);
        return (status & MTC516_ST_TAPE_RDY) != 0 ? 1 : 0;
}

static void
kd_mtc_units(void)
{
        unsigned int i;
        int probe;

        for (i = 0U; i < MTC516_UNIT_COUNT; i++) {
                probe = kd_mtc_probe(i);
                kd_unit_label('M', 'T', 'C', i);
                if (probe == 0) {
                        kd_pad(8U, 7U);
                        kd_sixbit(KD_NO_TAPE_W, 7U);
                        kd_nl();
                        continue;
                }
                kd_pad(8U, 11U);
                kd_sixbit(KD_TAPE_LOADED_W, 11U);
                kd_nl();
        }
}

void
kinit_diag_banner(void)
{
        kd_line(&KD_DAIMON_KERNEL_V001);
}

void
kinit_diag_error(unsigned int which, int error)
{
        switch (which) {
        case KINIT_DIAG_BAD_BOOTINFO:
                kd_text(&KD_BAD_BOOTINFO);
                break;
        case KINIT_DIAG_BAD_BADMAP:
                kd_text(&KD_BAD_BADMAP);
                break;
        case KINIT_DIAG_BAD_DMANIF:
                kd_text(&KD_BAD_DMANIF);
                break;
        default:
                kd_text(&KD_BAD_PAYLOAD);
                break;
        }
        kd_putc(' ');
        if (error < 0) {
                kd_putc('-');
                error = -error;
        }
        kd_octal((kword_t)(unsigned int)error, 0);
        kd_nl();
}

void
kinit_diag_bootinfo(const kword_t *bi)
{
        kword_t member_summary;
        unsigned int memory_kwords;

        member_summary = bi[BOOTINFO_WORD_MEMBER_SUMMARY] & DBOOT_WORD_MASK;
        kd_status_line(&KD_BOOTINFO, &KD_OK);
        if ((bi[BOOTINFO_WORD_FLAGS] & BOOTINFO_F_PAPER_SOURCE) == 0) {
                kd_bootset_line(bi[BOOTINFO_WORD_GENERATION] &
                    DBOOT_WORD_MASK);
                kd_members_line(BOOTINFO_MEMBER_MASK(member_summary));
        }
        kd_range_line(&KD_KCORE_RANGE,
            bi[BOOTINFO_WORD_KCORE_START] & DBOOT_HALF_MASK,
            bi[BOOTINFO_WORD_KCORE_MEMORY] & DBOOT_HALF_MASK);
        kd_range_line(&KD_KINIT_RANGE,
            bi[BOOTINFO_WORD_KINIT_START] & DBOOT_HALF_MASK,
            bi[BOOTINFO_WORD_KINIT_MEMORY] & DBOOT_HALF_MASK);
        kd_warning_lines(bi[BOOTINFO_WORD_WARNINGS] & DBOOT_WORD_MASK);

        memory_kwords = machine_probe_memory_kwords();
        kd_text_value_line(&KD_MACHINE, &KD_MACHINE_VALUE);
        kd_text_value_line(&KD_CONSOLE, &KD_CONSOLE_VALUE);
        kd_number_line(&KD_MEM, memory_kwords, &KD_MEM_SUFFIX);
        kd_number_line(&KD_HZ, (unsigned int)HZ_DEFAULT, &KD_HZ_SUFFIX);
}

int
kinit_diag_modules(kword_t init_base, kword_t init_words, kword_t member_mask, kword_t present_mask)
{
        const kword_t *manifest;
        const kword_t *minit;
        const kword_t *section;
        const kword_t *name;
        const kword_t *status;
        kword_t control;
        kword_t sizes;
        kword_t entry;
        kword_t kinit_words;
        unsigned int version;
        unsigned int module_count;
        unsigned int manifest_words;
        unsigned int minit_words;
        unsigned int id;
        unsigned int format;
        unsigned int name_chars;
        unsigned int status_chars;
        unsigned int name_words;
        unsigned int status_words;
        unsigned int i;

        manifest = (const kword_t *)init_base;
        if (init_words < DMANIF_HEADER_WORDS_V1)
                return -EINVAL;
        control = manifest[DMANIF_WORD_CONTROL] & DBOOT_WORD_MASK;
        version = (unsigned int)DMANIF_CTL_VERSION(control);
        module_count = DMANIF_CTL_MOD_COUNT(control);
        if (version == DMANIF_VERSION_V1 &&
            DMANIF_CTL_HDR_WORDS(control) == DMANIF_HEADER_WORDS_V1)
                manifest_words = DMANIF_HEADER_WORDS_V1 + module_count;
        else if (version == DMANIF_VERSION_V2 &&
            DMANIF_CTL_HDR_WORDS(control) == DMANIF_HEADER_WORDS_V2)
                manifest_words = DMANIF_HEADER_WORDS_V2 +
                    DMANIF_V2_ENTRY_WORDS * module_count;
        else
                return -EINVAL;
        if ((manifest[DMANIF_WORD_MAGIC] & DBOOT_WORD_MASK) != DMANIF_MAGIC ||
            module_count == 0U || init_words < (kword_t)manifest_words)
                return -EINVAL;
        sizes = manifest[DMANIF_WORD_SIZES] & DBOOT_WORD_MASK;
        if (init_words < (kword_t)manifest_words +
            DMANIF_SIZES_MINIT(sizes) + DMANIF_SIZES_MRES(sizes))
                return -EINVAL;
        kinit_words = init_words - (kword_t)manifest_words -
            DMANIF_SIZES_MINIT(sizes) - DMANIF_SIZES_MRES(sizes);
        minit = (const kword_t *)kd_word_addr_add(init_base,
            (kword_t)manifest_words + kinit_words);

        for (i = 0U; i < module_count; i++) {
                if (version == DMANIF_VERSION_V1) {
                        entry = manifest[DMANIF_WORD_MODULE0 + i] &
                            DBOOT_WORD_MASK;
                        minit_words =
                            (unsigned int)DMANIF_ENTRY_MINIT_WORDS(entry);
                        id = (unsigned int)DMANIF_ENTRY_ID(entry);
                        format = DMANIF_FMT_RAW;
                } else {
                        entry = manifest[DMANIF_WORD_MODULE0 +
                            DMANIF_V2_ENTRY_WORDS * i] & DBOOT_WORD_MASK;
                        minit_words =
                            (unsigned int)DMANIF_V2_ENTRY_MINIT_WORDS(entry);
                        id = (unsigned int)DMANIF_V2_ENTRY_ID(entry);
                        format = (unsigned int)DMANIF_V2_ENTRY_FORMAT(entry);
                }
                if (minit_words == 0U)
                        continue;
                section = minit;
                minit += minit_words;
                if (format != DMANIF_FMT_RAW)
                        continue;
                if (minit_words <= KINIT_MINIT_WORD_TEXT0 ||
                    section[KINIT_MINIT_WORD_MAGIC] != KINIT_MINIT_MAGIC_V1 ||
                    (unsigned int)(section[KINIT_MINIT_WORD_ID] & 0777777UL) !=
                    id)
                        return -EINVAL;
                name_chars = (unsigned int)((section[KINIT_MINIT_WORD_COUNTS] >>
                    18) & 0777777UL);
                status_chars = (unsigned int)(section[KINIT_MINIT_WORD_COUNTS] &
                    0777777UL);
                name_words = (name_chars + 5U) / 6U;
                status_words = (status_chars + 5U) / 6U;
                if (name_chars == 0U || status_chars == 0U ||
                    KINIT_MINIT_WORD_TEXT0 + name_words + status_words >
                    minit_words)
                        return -EINVAL;
                name = section + KINIT_MINIT_WORD_TEXT0;
                status = name + name_words;
                kd_module_line(id, name, name_chars, status, status_chars);
                if (id == DAIMOD_ID_DSK270)
                        kd_dsk_units(member_mask, present_mask);
                else if (id == DAIMOD_ID_DTC551)
                        kd_dtc_units();
                else if (id == DAIMOD_ID_MTC516)
                        kd_mtc_units();
        }
        return 0;
}

void
kinit_diag_handoff(void)
{
        kd_pad(kd_text(&KD_HANDOFF), kd_text_chars(&KD_OK));
}
