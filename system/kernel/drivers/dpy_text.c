/**
 * @file dpy_text.c
 * @brief Retained Type-342 text terminal state for the optional Type 340.
 *
 * The large screen/cache allocation is managed core, not MRES BSS.  It is
 * allocated lazily on the first terminal byte sent to DPY.  The resident
 * module keeps only a base pointer and cursor/ring state.
 *
 * Each logical row has 17 words of packed seven-bit terminal characters
 * (five characters per word) and a fixed 29-word native Type-342 cache slot.
 * 84 columns are therefore preserved without case loss while still keeping
 * rows independently movable.  The native slots contain only character-mode
 * words and are position independent; the refresh driver supplies the frame
 * setup and then streams logical rows in ring order.
 */

#include "dpy.h"
#include "../mm/mm.h"

#define DPY_TEXT_MM_OWNER        014U
#define DPY_TEXT_COLS            84U
#define DPY_TEXT_ROWS            42U
#define DPY_TEXT_ROW_WORDS       17U
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

        word = dpy_words()[prow * DPY_TEXT_ROW_WORDS + col / 5U];
        shift = 29U - (col % 5U) * 7U;
        return (unsigned int)((word >> shift) & 0177UL);
}

static void
dpy_cell_set(unsigned int prow, unsigned int col, unsigned int ch)
{
        kword_t *word;
        kword_t mask;
        unsigned int shift;

        word = &dpy_words()[prow * DPY_TEXT_ROW_WORDS + col / 5U];
        shift = 29U - (col % 5U) * 7U;
        mask = (kword_t)0177UL << shift;
        *word = (*word & ~mask) | (((kword_t)(ch & 0177U)) << shift);
}

static unsigned int
dpy_code(unsigned int ch, unsigned int *lowerp)
{
        *lowerp = 0U;
        if (ch >= 040U && ch <= 077U)
                return ch;
        if (ch >= 0101U && ch <= 0132U)
                return ch & 077U;
        if (ch >= 0141U && ch <= 0172U) {
                *lowerp = 1U;
                return ch - 0140U;
        }

        *lowerp = 1U;
        if (ch == 0134U) return 052U;
        if (ch == 0133U) return 053U;
        if (ch == 0135U) return 054U;
        if (ch == 0173U) return 055U;
        if (ch == 0175U) return 056U;
        if (ch == 0137U) return 060U;
        if (ch == 0174U) return 062U;
        if (ch == 0140U) return 066U;
        if (ch == 0136U) return 067U;
        if (ch == 0176U) return 043U;
        *lowerp = 0U;
        return DPY_T342_BAD;
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
        unsigned int code;
        unsigned int lower;
        unsigned int shifted;
        unsigned int ncode;
        unsigned int nwords;
        unsigned int i;

        words = dpy_words();
        prog = &words[DPY_TEXT_PROG_OFF + prow * DPY_TEXT_PROG_WORDS];
        for (i = 0U; i < DPY_TEXT_PROG_WORDS; ++i)
                prog[i] = 0UL;

        ncode = 0U;
        shifted = 0U;
        for (col = 0U; col < DPY_TEXT_COLS; ++col) {
                code = dpy_code(dpy_cell_get(prow, col), &lower);
                if (lower != shifted) {
                        dpy_emit_code(prog, &ncode,
                            lower != 0U ? DPY_T342_SO : DPY_T342_SI);
                        shifted = lower;
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
                dpy_cell_set(prow, col, 040U);
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
                        dpy_cell_set(prow, dpy_text_col++, 040U);
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
        dpy_cell_set(prow, dpy_text_col, ch);
        ++dpy_text_col;
        dpy_compile_row(prow);
        if (dpy_text_col >= DPY_TEXT_COLS) {
                dpy_text_col = 0U;
                dpy_newline();
        }
        return 0;
}
