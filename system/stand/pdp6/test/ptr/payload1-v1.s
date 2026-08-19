; payload1-v1.s -- Tape 1 test payload at 040000.
;
; Prints KERNEL through the low-core helper, emits CR/LF, and jumps directly
; to the first word of the Tape 2 payload at 040014.

        .text
        .globl start
        .globl __start

__start:
start:
        movei 017,050000
        move 01,msg_kernel
        pushj 017,000060
        movei 03,015
        datao 0120,03
kernel_cr_wait:
        coni 0120,04
        trne 04,0020
        jrst kernel_cr_wait
        movei 03,012
        datao 0120,03
        jrst 040014
msg_kernel:
        .word 0534562564554
