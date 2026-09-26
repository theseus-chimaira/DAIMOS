; Common fatal Stage1 diagnostic.
;
; Print the three-character SIXBIT halfword ?B1 and halt.  The packed
; constant is reversed so each character is available in the low six bits;
; this avoids a full-word SIXBIT helper and keeps every Stage1 small.
; No CR/LF is emitted: HALT immediately follows the diagnostic.
;
; Clobbers AC2, AC3, AC4, AC6.
stage1_fail_b1:
        movei 02,0214237              ; reverse-packed SIXBIT ?B1
        movei 06,03
stage1_fail_b1_loop:
        move 03,02
        andi 03,077
        addi 03,040
stage1_fail_b1_wait:
        coni 0120,04
        trne 04,0020
        jrst stage1_fail_b1_wait
        datao 0120,03
        lsh 02,-06
        sojg 06,stage1_fail_b1_loop
stage1_fail_b1_halt:
        halt .
        jrst stage1_fail_b1_halt
