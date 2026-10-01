/**
 * @file s6rec.s
 * @brief Compact resident S6REC text-frame validation shared by word devices.
 *
 * TTY and LPT are independently movable MRES packages, but both must validate
 * exactly one complete S6REC TEXT record before emitting device output.  Keep
 * that format check once in KCORE only because the measured whole-image result
 * is smaller than carrying both copies in the MRES packages.
 */

        .text
        .globl  s6rec_text_validate
        .globl  kret_neg1

; AC1=record base, AC2=supplied word count.
; Return AC1=character count and AC2=POINT 6 pointer at first payload word.
; Return AC1=-1 on malformed type/length.  AC3..AC6 are scratch.
s6rec_text_validate:
        move    6,1
        move    3,(1)
        ldb     4,[POINT 6,3,5]
        caie    4,1
        jrst    kret_neg1
        and     3,[077777777]
        move    4,3
        addi    4,5
        idivi   4,6
        addi    4,1
        came    4,2
        jrst    kret_neg1
        move    2,[POINT 6,0]
        movei   5,1(6)
        hrr     2,5
        move    1,3
        popj    17,
