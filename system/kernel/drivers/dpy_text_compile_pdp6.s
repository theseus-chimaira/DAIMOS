/**
 * @file dpy_text_compile_pdp6.s
 * @brief Compact PDP-6 Type-342 row compiler for the retained DPY terminal.
 *
 * KCC's generic implementation repeatedly divides and takes remainders by
 * six while scanning packed SIXBIT cells and constructing packed Type-342
 * output.  On a PDP-6 that expands into large resident helper/save-frame
 * sequences.  This routine scans the fixed representation sequentially and
 * keeps packing state in registers.
 *
 * C ABI:
 *   AC1 = physical row 0..41
 *   AC10..AC15 are preserved
 *   AC1..AC7 are caller-saved scratch
 */

        .text
        .globl dpy_compile_row
        .globl dpy_phys_row
        .globl dpy_cell_set
        .globl dpy_sixbit
        .globl dpy_clear_row
        .globl dpy_text_base
        .globl dpy_text_top

        .equ DPY_TEXT_COLS,0124
        .equ DPY_TEXT_ROW_WORDS,016
        .equ DPY_TEXT_LENGTH_OFF,01114
        .equ DPY_TEXT_PROG_OFF,01166
        .equ DPY_TEXT_PROG_WORDS,035

        .equ DPY_T342_SI,035
        .equ DPY_T342_SO,036
        .equ DPY_T342_CR,034
        .equ DPY_T342_LF,033
        .equ DPY_T342_BAD,077

dpy_phys_row:
        add     1,dpy_text_top
        cail    1,052
        subi    1,052
        popj    17,

dpy_cell_set:
        move    4,3
        divi    2,6
        imuli   1,DPY_TEXT_ROW_WORDS
        add     1,dpy_text_base
        add     1,2
        imuli   3,6
        movn    3,3
        addi    3,036
        movei   5,077
        lsh     5,0(3)
        move    6,(1)
        setcm   7,5
        and     6,7
        andi    4,077
        lsh     4,0(3)
        ior     6,4
        movem   6,(1)
        popj    17,

dpy_sixbit:
        caige   1,0141
        jrst    dpy_sixbit_range
        caile   1,0172
        jrst    dpy_sixbit_range
        subi    1,040
dpy_sixbit_range:
        caige   1,040
        jrst    dpy_sixbit_bad
        caile   1,0137
        jrst    dpy_sixbit_bad
        subi    1,040
        popj    17,
dpy_sixbit_bad:
        movei   1,037
        popj    17,

/**
 * @brief Clear one packed row and install its one-word blank refresh stream.
 * @param AC1 Physical row.
 */
dpy_clear_row:
        move    4,1
        imuli   1,DPY_TEXT_ROW_WORDS
        add     1,dpy_text_base
        movei   2,DPY_TEXT_ROW_WORDS
dpy_clear_row_cells:
        setzm   (1)
        addi    1,1
        sojg    2,dpy_clear_row_cells

        move    1,4
        imuli   1,DPY_TEXT_PROG_WORDS
        add     1,dpy_text_base
        addi    1,DPY_TEXT_PROG_OFF
        move    2,[0353535353433]
        movem   2,(1)

        move    1,dpy_text_base
        addi    1,DPY_TEXT_LENGTH_OFF
        add     1,4
        movei   2,1
        movem   2,(1)
        popj    17,

/**
 * @brief Map one stored SIXBIT cell to Type-342 code and shift state.
 * @param AC1 SIXBIT cell.
 * @return AC1 Type-342 code, AC2 zero for primary or one for shifted set.
 */
dpy_compile_code:
        setz    2,
        jumpe   1,dpy_compile_code_space
        caile   1,037
        jrst    dpy_compile_code_alpha
        addi    1,040
        popj    17,

dpy_compile_code_alpha:
        caige   1,041
        jrst    dpy_compile_code_bad   ; SIXBIT '@'
        caile   1,072
        jrst    dpy_compile_code_shift
        subi    1,040
        popj    17,

dpy_compile_code_shift:
        movei   2,1
        caie    1,073
        jrst    dpy_compile_code_074
        movei   1,053                  ; [
        popj    17,
dpy_compile_code_074:
        caie    1,074
        jrst    dpy_compile_code_075
        movei   1,052                  ; backslash
        popj    17,
dpy_compile_code_075:
        caie    1,075
        jrst    dpy_compile_code_076
        movei   1,054                  ; ]
        popj    17,
dpy_compile_code_076:
        caie    1,076
        jrst    dpy_compile_code_077
        movei   1,067                  ; ^
        popj    17,
dpy_compile_code_077:
        caie    1,077
        jrst    dpy_compile_code_bad
        movei   1,060                  ; _
        popj    17,

dpy_compile_code_space:
        movei   1,040
        popj    17,
dpy_compile_code_bad:
        setz    2,
        movei   1,DPY_T342_BAD
        popj    17,

/**
 * @brief Append AC1 to the packed Type-342 output stream.
 *
 * Private row-compiler state:
 *   AC6  completed output words
 *   AC7  slot 0..5 in current output word
 *   AC14 output pointer
 *   AC15 partial output word
 */
dpy_compile_emit:
        move    2,7
        imuli   2,6
        movn    2,2
        addi    2,036
        andi    1,077
        lsh     1,0(2)
        ior     15,1
        addi    7,1
        caie    7,6
        popj    17,
        movem   15,(14)
        addi    14,1
        addi    6,1
        setz    15,
        setz    7,
        popj    17,

/**
 * @brief Compile one physical packed-SIXBIT row.
 * @param AC1 Physical row.
 */
dpy_compile_row:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        push    17,14
        push    17,15

        move    5,1                    ; physical row for length result
        imuli   1,DPY_TEXT_ROW_WORDS
        add     1,dpy_text_base
        move    10,1                   ; packed source row
        addi    1,DPY_TEXT_ROW_WORDS
        movei   11,DPY_TEXT_COLS       ; cells through current scan point

        ; Find the final nonblank cell a packed word at a time.
dpy_compile_find_word:
        jumpe   11,dpy_compile_found
        subi    1,1
        move    12,(1)
        jumpn   12,dpy_compile_find_cell
        subi    11,6
        jrst    dpy_compile_find_word
dpy_compile_find_cell:
        move    2,12
        andi    2,077
        jumpn   2,dpy_compile_found
        lsh     12,-6
        soja    11,dpy_compile_find_cell

dpy_compile_found:
        move    14,5
        imuli   14,DPY_TEXT_PROG_WORDS
        add     14,dpy_text_base
        addi    14,DPY_TEXT_PROG_OFF
        setz    15,                    ; partial output word
        setz    13,                    ; current Type-342 shift state
        setz    7,                     ; output slot
        setz    6,                     ; completed output words
        setz    4,                     ; source cells left in AC12

dpy_compile_loop:
        jumpe   11,dpy_compile_tail
        jumpn   4,dpy_compile_have_word
        move    12,(10)
        addi    10,1
        movei   4,6
dpy_compile_have_word:
        move    1,12
        lsh     1,-036
        andi    1,077
        lsh     12,6
        subi    4,1
        subi    11,1
        pushj   17,dpy_compile_code
        camn    2,13
        jrst    dpy_compile_emit_char
        move    13,2
        push    17,1
        jumpe   13,dpy_compile_shift_in
        movei   1,DPY_T342_SO
        jrst    dpy_compile_emit_shift
dpy_compile_shift_in:
        movei   1,DPY_T342_SI
dpy_compile_emit_shift:
        pushj   17,dpy_compile_emit
        pop     17,1
dpy_compile_emit_char:
        pushj   17,dpy_compile_emit
        jrst    dpy_compile_loop

dpy_compile_tail:
        jumpe   13,dpy_compile_pad
        movei   1,DPY_T342_SI
        pushj   17,dpy_compile_emit
dpy_compile_pad:
        caie    7,4
        jrst    dpy_compile_pad_one
        movei   1,DPY_T342_CR
        pushj   17,dpy_compile_emit
        movei   1,DPY_T342_LF
        pushj   17,dpy_compile_emit

        move    1,dpy_text_base
        addi    1,DPY_TEXT_LENGTH_OFF
        add     1,5
        movem   6,(1)

        pop     17,15
        pop     17,14
        pop     17,13
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,

dpy_compile_pad_one:
        movei   1,DPY_T342_SI
        pushj   17,dpy_compile_emit
        jrst    dpy_compile_pad
