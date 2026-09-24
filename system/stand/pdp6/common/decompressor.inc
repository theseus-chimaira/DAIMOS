; Compact D6LZ36 memory decoder image for the fixed low-core service at 000060.
;
; The words in this include may be stored anywhere by Stage1, but every
; internal transfer is assembled for d6lz_fixed_base.  Copy the complete
; d6lz_image_start..d6lz_image_end range to d6lz_fixed_base before calling it.
;
; ABI is identical to d6lz36_decode():
;   AC1 = destination
;   AC2 = destination words
;   AC3 = compressed source
;   AC4 = compressed source words
;   -1(17) = optional unsigned-int src_used pointer
; Return AC1 = 0 on success, -1 on malformed/truncated input.
;
; AC1..AC7 and AC0 are caller-clobbered.  AC10 and AC11 are preserved.
; The image contains no literal-pool references and is therefore safe to copy.

        .equ d6lz_fixed_base,000060
        .globl d6lz36_decode

d6lz36_decode:
d6lz_image_start:
        push    17,10
        push    17,11
        push    17,4                    ; original src_words

        jumpn   3,d6lz_fixed_base+(d6lz_src_ok-d6lz_image_start)
        jumpn   4,d6lz_fixed_base+(d6lz_error-d6lz_image_start)
d6lz_src_ok:
        jumpe   2,d6lz_fixed_base+(d6lz_success-d6lz_image_start)
        jumpe   1,d6lz_fixed_base+(d6lz_error-d6lz_image_start)
        move    5,1                    ; output base
        setz    7,                     ; zero means load control

d6lz_loop:
        jumpn   7,d6lz_fixed_base+(d6lz_have_control-d6lz_image_start)
        jumpe   4,d6lz_fixed_base+(d6lz_error-d6lz_image_start)
        move    6,0(3)
        aoj     3,
        subi    4,1
        movsi   7,0400000              ; control bit 35

d6lz_have_control:
        jumpe   4,d6lz_fixed_base+(d6lz_error-d6lz_image_start)
        move    0,0(3)                 ; token
        aoj     3,
        subi    4,1
        move    10,6
        and     10,7
        jumpe   10,d6lz_fixed_base+(d6lz_literal-d6lz_image_start)

        move    11,0                   ; descriptor bits 14..35 must be zero
        lsh     11,-016
        jumpn   11,d6lz_fixed_base+(d6lz_error-d6lz_image_start)

        move    10,0                   ; match source = dst - distance
        andi    10,0177
        aoj     10,
        movn    10,10
        add     10,1
        camge   10,5
        jrst    d6lz_fixed_base+(d6lz_error-d6lz_image_start)

        lsh     0,-7                   ; length = next7 + 3
        andi    0,0177
        addi    0,3
        camge   2,0
        jrst    d6lz_fixed_base+(d6lz_error-d6lz_image_start)
        sub     2,0

d6lz_match_loop:
        move    11,0(10)
        movem   11,0(1)
        aoj     10,
        aoj     1,
        sojg    0,d6lz_fixed_base+(d6lz_match_loop-d6lz_image_start)
        jrst    d6lz_fixed_base+(d6lz_token_done-d6lz_image_start)

d6lz_literal:
        movem   0,0(1)
        aoj     1,
        subi    2,1

d6lz_token_done:
        lsh     7,-1
        jumpn   2,d6lz_fixed_base+(d6lz_loop-d6lz_image_start)

d6lz_success:
        move    10,-4(17)              ; src_usedp
        jumpe   10,d6lz_fixed_base+(d6lz_ok-d6lz_image_start)
        move    11,0(17)               ; original src_words - remaining
        sub     11,4
        movem   11,0(10)
d6lz_ok:
        setz    1,
        jrst    d6lz_fixed_base+(d6lz_return-d6lz_image_start)

d6lz_error:
        seto    1,
d6lz_return:
        pop     17,0                   ; discard saved original src_words
        pop     17,11
        pop     17,10
        popj    17,

d6lz_image_end:
