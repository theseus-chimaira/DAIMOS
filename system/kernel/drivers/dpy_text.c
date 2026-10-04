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
#define DPY_TEXT_LENGTH_OFF      DPY_TEXT_CHAR_WORDS
#define DPY_TEXT_PROG_OFF        (DPY_TEXT_LENGTH_OFF + DPY_TEXT_LENGTH_WORDS)
#define DPY_TEXT_ALLOC_WORDS     (DPY_TEXT_PROG_OFF + DPY_TEXT_PROG_TOTAL)

#define DPY_T342_SI              035U
#define DPY_T342_CR              034U
#define DPY_T342_LF              033U

/* Read directly by dpy_io.s. */
kword_t dpy_text_base;
kword_t dpy_text_top;
kword_t dpy_text_active;
kword_t dpy_text_rows_used;

static unsigned int dpy_text_row;
static unsigned int dpy_text_col;

/* Fixed-width Type-342 row compilation is substantially smaller and faster
 * in PDP-6 assembly than KCC's repeated /6 and %6 helper expansion. */
extern void dpy_compile_row(unsigned int prow);
extern unsigned int dpy_phys_row(unsigned int logical);
extern void dpy_cell_set(unsigned int prow, unsigned int col, unsigned int ch);
extern unsigned int dpy_sixbit(unsigned int ch);
extern void dpy_clear_row(unsigned int prow);

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
