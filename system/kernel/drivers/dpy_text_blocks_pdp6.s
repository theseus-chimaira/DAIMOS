/**
 * @file dpy_text_blocks_pdp6.s
 * @brief Native six-cell Type-342 block store for the retained DPY terminal.
 *
 * Each 84-column row has fourteen independently replayable two-word blocks.
 * A zero first word means six blank cells.  A zero second word means the first
 * word is six primary-set Type-342 glyphs and can be sent directly.  A nonzero
 * second word marks a complex block: both words contain three fixed
 * (shift-control,glyph) pairs, so every cell occupies exactly twelve bits.
 *
 * The fixed-pair form deliberately favors resident-code size over reclaiming a
 * complex block after its shifted character is overwritten.  Such a block
 * remains two words until row clear/scroll.  It is still directly replayable,
 * bounded at the historical two-word maximum, and needs no shadow SIXBIT plane
 * or row compiler.
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
        .equ DPY_BLOCK_BLANK,0404040404040

/** @return AC1 physical ring row for logical row AC1. */
dpy_phys_row:
        add     1,dpy_text_top
        cail    1,DPY_TEXT_ROWS
        subi    1,DPY_TEXT_ROWS
        popj    17,

/** Clear one physical 84-column row with fourteen stores. */
dpy_clear_row:
        imuli   1,DPY_TEXT_ROW_WORDS
        add     1,dpy_text_base
        movei   2,DPY_TEXT_BLOCKS
dpy_clear_row_loop:
        setzm   (1)                    ; word 1 is ignored while word 0 is zero
        addi    1,2
        sojg    2,dpy_clear_row_loop
        popj    17,

/**
 * Map ASCII to canonical Type-342 glyph + optional DPY_CELL_SHIFT flag.
 * Zero is the TAB-fill request and maps to a primary-set space.
 */
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
        jrst    dpy_text_code_bad      ; @ is absent from the primary set
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
 * Expand a simple six-glyph word at AC6 to fixed SI,glyph pairs in its two
 * native block words.  AC1..AC5 scratch; AC6 and AC7 survive.
 */
dpy_block_expand:
        move    4,(6)
        jumpn   4,dpy_block_expand_have
        move    4,[DPY_BLOCK_BLANK]
dpy_block_expand_have:
        setz    5,
        movei   2,3
dpy_block_expand_first:
        move    1,4
        lsh     1,-036
        andi    1,077
        iori    1,DPY_PAIR_SI
        lsh     5,014
        ior     5,1
        lsh     4,6
        sojg    2,dpy_block_expand_first
        movem   5,(6)
        setz    5,
        movei   2,3
dpy_block_expand_second:
        move    1,4
        lsh     1,-036
        andi    1,077
        iori    1,DPY_PAIR_SI
        lsh     5,014
        ior     5,1
        lsh     4,6
        sojg    2,dpy_block_expand_second
        movem   5,1(6)
        popj    17,

/**
 * Patch one fixed 12-bit (SI/SO,glyph) pair in an already-complex block.
 * AC6=block, AC3=cell 0..5, AC7=canonical token.
 */
dpy_block_patch_complex:
        move    1,6                    ; selected native word
        move    2,3                    ; pair slot within that word
        caige   2,3
        jrst    dpy_block_patch_word
        subi    2,3
        addi    1,1
dpy_block_patch_word:
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

/**
 * @brief Replace one retained terminal cell.
 * @param AC1 Physical row.
 * @param AC2 Column 0..83.
 * @param AC3 ASCII byte; zero means blank space for TAB fill.
 */
dpy_cell_set:
        push    17,1
        push    17,2
        move    1,3
        pushj   17,dpy_text_code
        move    7,1                    ; canonical new token
        pop     17,2
        pop     17,1

        imuli   1,DPY_TEXT_ROW_WORDS
        add     1,dpy_text_base
        move    3,2                    ; DIVI consumes the AC2:AC3 pair
        setz    2,                     ; unsigned column lives in low AC3
        divi    2,6                    ; AC2 block, AC3 cell slot
        lsh     2,1
        add     1,2
        move    6,1                    ; block address (AC0 cannot index)

        ; A zero first word is authoritative even if row clear left a stale
        ; second word.  Otherwise an existing complex block stays complex.
        skipn   (6)
        jrst    dpy_cell_set_empty
        skipe   1(6)
        jrst    dpy_cell_set_complex
dpy_cell_set_empty:
        trne    7,DPY_CELL_SHIFT
        jrst    dpy_cell_set_make_complex
        setzm   1(6)                   ; discard stale complex word on reuse

        ; Simple primary-set block.  Zero expands logically to six spaces.
        move    4,(6)
        jumpn   4,dpy_cell_set_simple_have
        move    4,[DPY_BLOCK_BLANK]
dpy_cell_set_simple_have:
        move    5,3
        imuli   5,6
        movn    5,5
        addi    5,036
        movei   1,077
        lsh     1,0(5)
        setcm   2,1
        and     4,2
        move    2,7
        andi    2,077
        lsh     2,0(5)
        ior     4,2
        camn    4,[DPY_BLOCK_BLANK]
        setz    4,
        movem   4,(6)
        popj    17,

dpy_cell_set_make_complex:
        pushj   17,dpy_block_expand
dpy_cell_set_complex:
        jrst    dpy_block_patch_complex
