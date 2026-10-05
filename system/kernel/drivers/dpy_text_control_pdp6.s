/**
 * @file dpy_text_control_pdp6.s
 * @brief Compact retained Type-342 terminal control for the PDP-6.
 *
 * The experimental 160x92 retained image is a lazily allocated native
 * Type-342 block store;
 * dpy_text_blocks_pdp6.s owns physical-row mapping and cell mutation.  This
 * file owns only allocation, cursor, scrolling, and terminal control bytes.
 *
 * dpy_text_active is intentionally separate from dpy_text_base.  Allocation
 * publishes the base before clearing the new extent, while PI refresh must not
 * consume it until every first block word is initialized.  active is therefore
 * the publication barrier and is set only after the initial clear completes.
 *
 * C ABI:
 *   dpy_text_putchar: AC1 = byte, AC1 return = 0 or -1
 *   AC1..AC7 caller-scratch; AC10..AC16 untouched.
 */

        .text
        .globl  dpy_text_putchar
        .globl  dpy_text_base
        .globl  dpy_text_top
        .globl  dpy_text_active
        .globl  dpy_text_rows_used
        .globl  mm_alloc
        .globl  dpy_phys_row
        .globl  dpy_cell_set
        .globl  dpy_clear_row

        .equ    DPY_TEXT_MM_OWNER,014
        .equ    MM_TYPE_KERNEL_DYNAMIC,3
        .equ    DPY_TEXT_ALLOC_WORDS,011550
        .equ    DPY_TEXT_COLS,0240
        .equ    DPY_TEXT_ROWS,0134

/** Clear all retained rows and reset ring/cursor state. */
dpy_text_clear_all:
        movei   5,0
dpy_text_clear_all_loop:
        move    1,5
        pushj   17,dpy_clear_row
        aoj     5,
        caige   5,DPY_TEXT_ROWS
        jrst    dpy_text_clear_all_loop
        setzm   dpy_text_top
        setzm   dpy_text_rows_used
        setzm   dpy_text_row
        setzm   dpy_text_col
        popj    17,

/** Allocate and initialize the retained image on first output. */
dpy_text_start:
        skipe   dpy_text_active
        jrst    dpy_text_zero
        push    17,[0]                 ; allocation result
        movei   1,0(17)
        push    17,1                   ; fifth mm_alloc argument
        movei   1,DPY_TEXT_ALLOC_WORDS
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,DPY_TEXT_MM_OWNER
        setz    4,                     ; MM_ALLOC_LOW
        pushj   17,mm_alloc
        sub     17,[1,,1]
        jumpn   1,dpy_text_start_fail
        move    1,0(17)
        movem   1,dpy_text_base
        pushj   17,dpy_text_clear_all
        movei   1,1
        movem   1,dpy_text_active
        sub     17,[1,,1]
dpy_text_zero:
        setz    1,
        popj    17,
dpy_text_start_fail:
        sub     17,[1,,1]
        seto    1,
        popj    17,

/** Scroll one row by rotating the logical/physical ring. */
dpy_text_scroll:
        move    5,dpy_text_top
        movei   1,1(5)
        caige   1,DPY_TEXT_ROWS
        jrst    dpy_text_scroll_top
        setz    1,
dpy_text_scroll_top:
        movem   1,dpy_text_top
        move    1,5
        pushj   17,dpy_clear_row
        movei   1,DPY_TEXT_ROWS-1
        movem   1,dpy_text_row
        movem   1,dpy_text_rows_used
        popj    17,

/** Advance to the next logical row, scrolling at the lower edge. */
dpy_text_newline:
        aos     1,dpy_text_row
        caige   1,DPY_TEXT_ROWS
        popj    17,
        jrst    dpy_text_scroll

/** Wrap a full 160-column row. */
dpy_text_wrap:
        move    1,dpy_text_col
        caige   1,DPY_TEXT_COLS
        popj    17,
        setzm   dpy_text_col
        jrst    dpy_text_newline

/**
 * @brief Consume one terminal byte into the retained native block image.
 * @param AC1 ASCII byte.
 * @return AC1=0, or -1 when lazy allocation fails.
 */
dpy_text_putchar:
        push    17,1                   ; preserve byte across lazy start
        pushj   17,dpy_text_start
        jumpn   1,dpy_text_putchar_fail
        pop     17,7
        andi    7,0377

        caie    7,015                  ; CR
        jrst    dpy_text_putchar_lf
        setzm   dpy_text_col
        jrst    dpy_text_zero

dpy_text_putchar_lf:
        caie    7,012                  ; LF
        jrst    dpy_text_putchar_bs
        pushj   17,dpy_text_newline
        jrst    dpy_text_zero

dpy_text_putchar_bs:
        caie    7,010                  ; BS
        jrst    dpy_text_putchar_ff
        skipn   dpy_text_col
        jrst    dpy_text_zero
        sos     dpy_text_col
        jrst    dpy_text_zero

dpy_text_putchar_ff:
        caie    7,014                  ; FF
        jrst    dpy_text_putchar_tab
        pushj   17,dpy_text_clear_all
        jrst    dpy_text_zero

dpy_text_putchar_tab:
        caie    7,011                  ; TAB
        jrst    dpy_text_putchar_printable
        move    6,dpy_text_col
        addi    6,010
        andi    6,0777770              ; next multiple of eight
        caile   6,DPY_TEXT_COLS
        movei   6,DPY_TEXT_COLS
        move    1,dpy_text_row
        pushj   17,dpy_phys_row
        push    17,1                   ; physical row
        push    17,6                   ; stop column
dpy_text_tab_loop:
        move    2,dpy_text_col
        caml    2,0(17)
        jrst    dpy_text_tab_done
        move    1,-1(17)
        setz    3,                     ; blank cell
        pushj   17,dpy_cell_set
        aos     dpy_text_col
        jrst    dpy_text_tab_loop
dpy_text_tab_done:
        sub     17,[2,,2]
        pushj   17,dpy_text_wrap
        jrst    dpy_text_zero

dpy_text_putchar_printable:
        caige   7,040
        jrst    dpy_text_zero
        caile   7,0176
        jrst    dpy_text_zero
        move    1,dpy_text_row
        pushj   17,dpy_phys_row
        move    2,dpy_text_col
        move    3,7                    ; ASCII; block backend normalizes/maps
        pushj   17,dpy_cell_set
        move    1,dpy_text_row
        addi    1,1
        camle   1,dpy_text_rows_used
        movem   1,dpy_text_rows_used
        aos     dpy_text_col
        pushj   17,dpy_text_wrap
        jrst    dpy_text_zero

dpy_text_putchar_fail:
        pop     17,0                   ; discard saved input
        seto    1,
        popj    17,

        .bss
dpy_text_base:
        .block  1
dpy_text_top:
        .block  1
dpy_text_active:
        .block  1
dpy_text_rows_used:
        .block  1
dpy_text_row:
        .block  1
dpy_text_col:
        .block  1
