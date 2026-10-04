/**
 * @file dpy_text.c
 * @brief Retained Type-342 text terminal state for the optional Type 340.
 *
 * The large screen/cache allocation is managed core, not MRES BSS.  It is
 * allocated lazily on the first terminal byte sent to DPY.  The resident
 * module keeps only a base pointer and cursor/ring state.
 *
 * Each logical row has 14 words of packed SIXBIT terminal characters
 * (six characters per word) and a fixed 29-word native Type-342 cache slot.
 * Ordinary DPY TTY text is deliberately uppercase-only: lowercase input is
 * normalized before storage.  The Type-342 shifted set is used only for the
 * few SIXBIT punctuation glyphs that are not present in the primary set.
 * The native slots contain only the visible prefix of each row followed by
 * CR/LF.  Trailing spaces are never refreshed.  dpy_text_rows_used bounds the
 * highest logical row that can contain visible text, so the refresh driver
 * also omits all trailing blank rows.
 */

#include "dpy.h"
#include "../mm/mm.h"

#define DPY_TEXT_MM_OWNER        014U
#define DPY_TEXT_COLS            84U
#define DPY_TEXT_ROWS            42U
#define DPY_TEXT_ROW_WORDS       14U
#define DPY_TEXT_CHAR_WORDS      (DPY_TEXT_ROWS * DPY_TEXT_ROW_WORDS)
#define DPY_TEXT_LENGTH_WORDS    DPY_TEXT_ROWS
#define DPY_TEXT_PROG_WORDS      29U
#define DPY_TEXT_PROG_TOTAL      (DPY_TEXT_ROWS * DPY_TEXT_PROG_WORDS)
#define DPY_TEXT_INTENSITY_WORDS 32U
#define DPY_TEXT_LENGTH_OFF      DPY_TEXT_CHAR_WORDS
#define DPY_TEXT_PROG_OFF        (DPY_TEXT_LENGTH_OFF + DPY_TEXT_LENGTH_WORDS)
#define DPY_TEXT_INTENSITY_OFF   (DPY_TEXT_PROG_OFF + DPY_TEXT_PROG_TOTAL)
#define DPY_TEXT_ALLOC_WORDS     (DPY_TEXT_INTENSITY_OFF + DPY_TEXT_INTENSITY_WORDS)

#define DPY_T342_SI              035U
#define DPY_T342_SO              036U
#define DPY_T342_CR              034U
#define DPY_T342_LF              033U
#define DPY_T342_BAD             077U

/* Read directly by dpy_io.s. */
kword_t dpy_text_base;
kword_t dpy_text_top;
kword_t dpy_text_active;
kword_t dpy_text_rows_used;

static unsigned int dpy_text_row;
static unsigned int dpy_text_col;

static kword_t *
dpy_words(void)
{
        return (kword_t *)(unsigned long)dpy_text_base;
}

static unsigned int
dpy_phys_row(unsigned int logical)
{
        unsigned int row;

        row = (unsigned int)dpy_text_top + logical;
        if (row >= DPY_TEXT_ROWS)
                row -= DPY_TEXT_ROWS;
        return row;
}

static unsigned int
dpy_cell_get(unsigned int prow, unsigned int col)
{
        kword_t word;
        unsigned int shift;

        word = dpy_words()[prow * DPY_TEXT_ROW_WORDS + col / 6U];
        shift = 30U - (col % 6U) * 6U;
        return (unsigned int)((word >> shift) & 077UL);
}

static void
dpy_cell_set(unsigned int prow, unsigned int col, unsigned int ch)
{
        kword_t *word;
        kword_t mask;
        unsigned int shift;

        word = &dpy_words()[prow * DPY_TEXT_ROW_WORDS + col / 6U];
        shift = 30U - (col % 6U) * 6U;
        mask = (kword_t)077UL << shift;
        *word = (*word & ~mask) | (((kword_t)(ch & 077U)) << shift);
}

static unsigned int
dpy_code(unsigned int ch, unsigned int *shiftp)
{
        *shiftp = 0U;
        if (ch == 0U)
                return 040U;            /* SIXBIT space */
        if (ch <= 037U)
                return ch + 040U;       /* ! through ? */
        if (ch >= 041U && ch <= 072U)
                return ch - 040U;       /* A through Z */

        /* These SIXBIT punctuation characters live only in the Type-342
         * shifted set.  '@' (040) has no useful native Type-342 glyph. */
        *shiftp = 1U;
        if (ch == 073U) return 053U;    /* [ */
        if (ch == 074U) return 052U;    /* \ */
        if (ch == 075U) return 054U;    /* ] */
        if (ch == 076U) return 067U;    /* ^ */
        if (ch == 077U) return 060U;    /* _ */
        *shiftp = 0U;
        return DPY_T342_BAD;
}

static unsigned int
dpy_sixbit(unsigned int ch)
{
        if (ch >= 0141U && ch <= 0172U)
                ch -= 040U;             /* stream TTY is uppercase-only */
        if (ch >= 040U && ch <= 0137U)
                return ch - 040U;
        return 037U;                    /* unsupported printable -> '?' */
}

static void
dpy_emit_code(kword_t *prog, unsigned int *ncode, unsigned int code)
{
        unsigned int word;
        unsigned int slot;
        unsigned int shift;

        word = *ncode / 6U;
        slot = *ncode % 6U;
        shift = 30U - slot * 6U;
        prog[word] |= ((kword_t)(code & 077U)) << shift;
        ++*ncode;
}

static void
dpy_compile_row(unsigned int prow)
{
        kword_t *words;
        kword_t *prog;
        unsigned int col;
        unsigned int last;
        unsigned int code;
        unsigned int shift;
        unsigned int shifted;
        unsigned int ncode;
        unsigned int nwords;
        unsigned int i;

        words = dpy_words();
        prog = &words[DPY_TEXT_PROG_OFF + prow * DPY_TEXT_PROG_WORDS];
        for (i = 0U; i < DPY_TEXT_PROG_WORDS; ++i)
                prog[i] = 0UL;

        last = DPY_TEXT_COLS;
        while (last != 0U && dpy_cell_get(prow, last - 1U) == 0U)
                --last;

        ncode = 0U;
        shifted = 0U;
        for (col = 0U; col < last; ++col) {
                code = dpy_code(dpy_cell_get(prow, col), &shift);
                if (shift != shifted) {
                        dpy_emit_code(prog, &ncode,
                            shift != 0U ? DPY_T342_SO : DPY_T342_SI);
                        shifted = shift;
                }
                dpy_emit_code(prog, &ncode, code);
        }
        if (shifted != 0U)
                dpy_emit_code(prog, &ncode, DPY_T342_SI);
        while (((ncode + 2U) % 6U) != 0U)
                dpy_emit_code(prog, &ncode, DPY_T342_SI);
        dpy_emit_code(prog, &ncode, DPY_T342_CR);
        dpy_emit_code(prog, &ncode, DPY_T342_LF);

        nwords = ncode / 6U;
        words[DPY_TEXT_LENGTH_OFF + prow] = (kword_t)nwords;
}

static void
dpy_clear_row(unsigned int prow)
{
        unsigned int col;

        for (col = 0U; col < DPY_TEXT_COLS; ++col)
                dpy_cell_set(prow, col, 0U);
        dpy_compile_row(prow);
}

static int
dpy_text_start(void)
{
        kword_t base;
        unsigned int row;

        if (dpy_text_active != 0UL)
                return 0;
        base = 0UL;
        if (mm_alloc((kword_t)DPY_TEXT_ALLOC_WORDS, MM_TYPE_KERNEL_DYNAMIC,
            DPY_TEXT_MM_OWNER, MM_ALLOC_LOW, &base) != MM_OK)
                return -1;

        dpy_text_base = base;
        dpy_text_top = 0UL;
        dpy_text_rows_used = 0UL;
        dpy_text_row = 0U;
        dpy_text_col = 0U;
        for (row = 0U; row < DPY_TEXT_ROWS; ++row)
                dpy_clear_row(row);
        dpy_text_active = 1UL;
        return 0;
}

static void
dpy_scroll(void)
{
        unsigned int old_top;

        old_top = (unsigned int)dpy_text_top;
        dpy_text_top = (kword_t)(old_top + 1U);
        if (dpy_text_top >= (kword_t)DPY_TEXT_ROWS)
                dpy_text_top = 0UL;
        dpy_clear_row(old_top);
        dpy_text_row = DPY_TEXT_ROWS - 1U;
        dpy_text_rows_used = (kword_t)(DPY_TEXT_ROWS - 1U);
}

static void
dpy_newline(void)
{
        ++dpy_text_row;
        if (dpy_text_row >= DPY_TEXT_ROWS)
                dpy_scroll();
}

int
dpy_text_putchar(unsigned int ch)
{
        unsigned int prow;
        unsigned int stop;

        if (dpy_text_start() != 0)
                return -1;
        ch &= 0377U;
        if (ch == 015U) {
                dpy_text_col = 0U;
                return 0;
        }
        if (ch == 012U) {
                dpy_newline();
                return 0;
        }
        if (ch == 010U) {
                if (dpy_text_col != 0U)
                        --dpy_text_col;
                return 0;
        }
        if (ch == 014U) {
                unsigned int row;
                for (row = 0U; row < DPY_TEXT_ROWS; ++row)
                        dpy_clear_row(row);
                dpy_text_top = 0UL;
                dpy_text_rows_used = 0UL;
                dpy_text_row = 0U;
                dpy_text_col = 0U;
                return 0;
        }
        if (ch == 011U) {
                stop = (dpy_text_col + 8U) & ~7U;
                if (stop > DPY_TEXT_COLS)
                        stop = DPY_TEXT_COLS;
                prow = dpy_phys_row(dpy_text_row);
                while (dpy_text_col < stop)
                        dpy_cell_set(prow, dpy_text_col++, 0U);
                dpy_compile_row(prow);
                if (dpy_text_col >= DPY_TEXT_COLS) {
                        dpy_text_col = 0U;
                        dpy_newline();
                }
                return 0;
        }
        if (ch < 040U || ch > 0176U)
                return 0;

        prow = dpy_phys_row(dpy_text_row);
        dpy_cell_set(prow, dpy_text_col, dpy_sixbit(ch));
        if (dpy_text_rows_used < (kword_t)(dpy_text_row + 1U))
                dpy_text_rows_used = (kword_t)(dpy_text_row + 1U);
        ++dpy_text_col;
        dpy_compile_row(prow);
        if (dpy_text_col >= DPY_TEXT_COLS) {
                dpy_text_col = 0U;
                dpy_newline();
        }
        return 0;
}
