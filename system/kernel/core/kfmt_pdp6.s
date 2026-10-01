/**
 * @file kfmt_pdp6.s
 * @brief Compact resident unsigned 18-bit SIXBIT decimal formatter.
 *
 * MonitorFS pseudo-files are S6REC word streams.  Return the shortest decimal
 * form already packed as one SIXBIT word instead of exposing a character-at-
 * offset formatter.
 */
        .text
        .globl  kfmt_u18_sixbit

; Input AC1=value 0..0777777. Return AC1=left-aligned packed SIXBIT decimal,
; AC2=character count 1..6.
kfmt_u18_sixbit:
        jumpn   1,kfmt_u18_nonzero
        move    1,[0200000000000]      ; SIXBIT "0"
        movei   2,1
        popj    17,
kfmt_u18_nonzero:
        move    4,1
        setz    3,
        setz    2,
kfmt_u18_loop:
        move    5,4
        idivi   5,012                  ; quotient AC5, remainder AC6
        addi    6,020                  ; SIXBIT digit
        move    7,2
        imuli   7,6
        lsh     6,0(7)
        ior     3,6
        aoj     2,
        move    4,5
        jumpn   4,kfmt_u18_loop
        move    6,2
        imuli   6,6
        movei   7,044
        sub     7,6
        lsh     3,0(7)
        move    1,3
        popj    17,
