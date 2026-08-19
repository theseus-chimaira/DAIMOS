; payload2-v1.s -- Tape 2 test payload at 040005.
;
; Prints DRIVERS through the same low-core helper, then halts.

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
        halt .
msg_driv0:
        .word 0446251664562
msg_driv1:
        .word 0630000000000
