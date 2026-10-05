/**
 * @file dpy_text_blocks_pdp6.s
 * @brief Direct-BLKO Type-342 retained blocks for the PDP-6 display.
 *
 * Each of 46 84-column rows owns fourteen two-word blocks.  An untouched block has a
 * zero first word and marks trailing blank storage.  Once touched, both words
 * become directly executable Type-342 character data: three fixed
 * (SI/SO,glyph) pairs per word.  Every cell therefore carries its own character
 * set selection and rows may be streamed as one contiguous BLKO span without a
 * compiler, shadow text plane, or inter-block shift state.
 *
 * The representation uses the same 28 words per row as the previous native
 * block store, so lazy text RAM is 1288 words.  A block may remain
 * initialized after being overwritten with spaces; row clear/scroll restores
 * zero trailing markers.
 */

        .text
        .globl dpy_phys_row
        .globl dpy_cell_set
        .globl dpy_clear_row
        .globl dpy_text_base
        .globl dpy_text_top

        .equ DPY_TEXT_BLOCKS,016
        .equ DPY_TEXT_ROW_WORDS,034
        .equ DPY_T342_SI,035
        .equ DPY_T342_SO,036
        .equ DPY_T342_SPACE,040
        .equ DPY_T342_BAD,077
        .equ DPY_CELL_SHIFT,0100
        .equ DPY_PAIR_SI,03500
        .equ DPY_PAIR_SO,03600
        .equ DPY_BLOCK_BLANK_PAIRS,0354035403540

/** @return AC1 physical ring row for logical row AC1. */
dpy_phys_row:
        add     1,dpy_text_top
        cail    1,DPY_TEXT_ROWS
        subi    1,DPY_TEXT_ROWS
        popj    17,

/** Clear one physical row; zero first words are authoritative trailing blanks. */
dpy_clear_row:
        imuli   1,DPY_TEXT_ROW_WORDS
        add     1,dpy_text_base
        movei   2,DPY_TEXT_BLOCKS
dpy_clear_row_loop:
        setzm   (1)                    ; second word ignored while first is zero
        addi    1,2
        sojg    2,dpy_clear_row_loop
        popj    17,

/** Map ASCII to Type-342 glyph plus optional DPY_CELL_SHIFT flag. */
dpy_text_code:
        jumpe   1,dpy_text_code_space
        caige   1,0141
        jrst    dpy_text_code_upper
        caile   1,0172
        jrst    dpy_text_code_upper
        subi    1,040                  ; lowercase -> uppercase

dpy_text_code_upper:
        cain    1,040
        jrst    dpy_text_code_space
        caige   1,041
        jrst    dpy_text_code_bad
        caile   1,077
        jrst    dpy_text_code_alpha
        popj    17,                    ; ! through ? map directly

dpy_text_code_alpha:
        caige   1,0101
        jrst    dpy_text_code_bad      ; @ absent from primary set
        caile   1,0132
        jrst    dpy_text_code_shift
        subi    1,0100                 ; A..Z -> Type-342 1..32
        popj    17,

dpy_text_code_shift:
        caie    1,0133
        jrst    dpy_text_code_134
        movei   1,DPY_CELL_SHIFT+053  ; [
        popj    17,
dpy_text_code_134:
        caie    1,0134
        jrst    dpy_text_code_135
        movei   1,DPY_CELL_SHIFT+052  ; backslash
        popj    17,
dpy_text_code_135:
        caie    1,0135
        jrst    dpy_text_code_136
        movei   1,DPY_CELL_SHIFT+054  ; ]
        popj    17,
dpy_text_code_136:
        caie    1,0136
        jrst    dpy_text_code_137
        movei   1,DPY_CELL_SHIFT+067  ; ^
        popj    17,
dpy_text_code_137:
        caie    1,0137
        jrst    dpy_text_code_bad
        movei   1,DPY_CELL_SHIFT+060  ; _
        popj    17,

dpy_text_code_space:
        movei   1,DPY_T342_SPACE
        popj    17,
dpy_text_code_bad:
        movei   1,DPY_T342_BAD
        popj    17,

/**
 * @brief Replace one retained terminal cell.
 * @param AC1 Physical row.
 * @param AC2 Column 0..83.
 * @param AC3 ASCII byte; zero means primary-set blank for TAB fill.
 *
 * A newly touched block is initialized to six explicit SI+SPACE pairs.  This
 * makes every initialized prefix of a row safe for direct contiguous BLKO.
 */
dpy_cell_set:
        push    17,1
        push    17,2
        move    1,3
        pushj   17,dpy_text_code
        move    7,1                    ; canonical token
        pop     17,2
        pop     17,1

        imuli   1,DPY_TEXT_ROW_WORDS
        add     1,dpy_text_base
        move    3,2                    ; DIVI consumes AC2:AC3 pair
        setz    2,
        divi    2,6                    ; AC2 block, AC3 cell 0..5
        lsh     2,1
        add     1,2
        move    6,1                    ; selected block address

        ; LF preserves the terminal column, so the first character on a new row
        ; may land in a later block.  A contiguous BLKO row may never contain a
        ; zero/stale hole.  Only on first use of this block, initialize every
        ; still-zero predecessor through the selected block to explicit spaces.
        ; Once a block is initialized it never returns to zero until row clear,
        ; so ordinary character updates remain O(1).
        skipe   (6)
        jrst    dpy_cell_set_patch
        move    5,6
        sub     5,2                    ; physical row base
        move    4,[DPY_BLOCK_BLANK_PAIRS]
dpy_cell_fill_prefix:
        skipe   (5)
        jrst    dpy_cell_fill_next
        movem   4,(5)
        movem   4,1(5)
dpy_cell_fill_next:
        camn    5,6
        jrst    dpy_cell_set_patch
        addi    5,2
        jrst    dpy_cell_fill_prefix

dpy_cell_set_patch:
        ; Select one of the two three-pair words and pair slot 0..2.
        move    1,6
        move    2,3
        caige   2,3
        jrst    dpy_cell_set_word
        subi    2,3
        addi    1,1
dpy_cell_set_word:
        move    4,7
        andi    4,077
        trne    7,DPY_CELL_SHIFT
        iori    4,DPY_PAIR_SO
        trnn    7,DPY_CELL_SHIFT
        iori    4,DPY_PAIR_SI
        move    5,2
        movn    5,5
        addi    5,2
        imuli   5,014                  ; shifts 24,12,0 bits
        movei   2,07777
        lsh     2,0(5)
        setcm   3,2
        and     3,(1)
        lsh     4,0(5)
        ior     3,4
        movem   3,(1)
        popj    17,
