; Allocation-free D6LZ36 decoder for PDP-6/PDP-10.
; AC1=dst, AC2=dst_words, AC3=src, AC4=src_words,
; fifth C argument src_usedp is at -1(17) on entry.

        .text
        .globl d6lz36_decode

d6lz36_decode:
        move    0,-1(17)               ; preserve stack-passed fifth arg
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        push    17,14
        push    17,15
        push    17,16
        move    10,1                   ; current output
        move    11,2                   ; output words remaining
        move    12,3                   ; current input
        move    13,4                   ; input words remaining
        move    16,0                   ; src_usedp
        move    7,1                    ; output base
        move    6,3                    ; immutable input base

        jumpe   10,d6lz_null_dst
        jrst    d6lz_check_src
 d6lz_null_dst:
        jumpn   11,d6lz_error
 d6lz_check_src:
        jumpe   12,d6lz_null_src
        jrst    d6lz_start
 d6lz_null_src:
        jumpn   13,d6lz_error
 d6lz_start:
        setz    14,                    ; control word
        setz    15,                    ; zero means load a control word
        jumpe   11,d6lz_success

 d6lz_token_loop:
        jumpn   15,d6lz_have_control
        jumpe   13,d6lz_error
        move    14,0(12)
        addi    12,1
        subi    13,1
        movei   15,1
        lsh     15,043                 ; bit 35
 d6lz_have_control:
        jumpe   13,d6lz_error
        move    5,0(12)                ; token
        addi    12,1
        subi    13,1
        move    0,14
        and     0,15
        jumpe   0,d6lz_literal

        ; Descriptor bits 14..35 must be zero.
        move    0,5
        lsh     0,-016
        jumpn   0,d6lz_error

        ; distance = (token & 0177) + 1; it cannot exceed produced words.
        move    3,5
        andi    3,0177
        addi    3,1
        move    0,10
        sub     0,7
        sub     0,3
        jumpl   0,d6lz_error

        ; length = ((token >> 7) & 0177) + 3; it must fit output remainder.
        move    4,5
        lsh     4,-7
        andi    4,0177
        addi    4,3
        move    0,11
        sub     0,4
        jumpl   0,d6lz_error

        ; Copy forward so overlapping distance-one runs work naturally.
        move    5,10
        sub     5,3
 d6lz_match_loop:
        move    0,0(5)
        movem   0,0(10)
        addi    5,1
        addi    10,1
        subi    11,1
        sojg    4,d6lz_match_loop
        jrst    d6lz_token_done

 d6lz_literal:
        movem   5,0(10)
        addi    10,1
        subi    11,1
 d6lz_token_done:
        lsh     15,-1
        jumpn   11,d6lz_token_loop

 d6lz_success:
        jumpe   16,d6lz_ok
        move    0,12
        sub     0,6
        movem   0,0(16)
 d6lz_ok:
        setz    1,
        jrst    d6lz_return
 d6lz_error:
        seto    1,
 d6lz_return:
        pop     17,16
        pop     17,15
        pop     17,14
        pop     17,13
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,
