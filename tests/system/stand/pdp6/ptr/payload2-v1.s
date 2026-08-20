; payload2-v1.s -- Tape 2 test payload at 040014.
;
; Prints DRIVERS through the same low-core helper, emits CR/LF, then halts.

        .text
        .globl start
        .globl __start

__start:
start:
        movei 017,050000
        move 01,msg_driv0
        pushj 017,000060
        move 01,msg_driv1
        pushj 017,000060
        movei 03,015
        datao 0120,03
drivers_cr_wait:
        coni 0120,04
        trne 04,0020
        jrst drivers_cr_wait
        movei 03,012
        datao 0120,03
        halt .
msg_driv0:
        .word 0446251664562
msg_driv1:
        .word 0630000000000
