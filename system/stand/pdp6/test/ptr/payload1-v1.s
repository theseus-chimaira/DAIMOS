; payload1-v1.s -- Tape 1 test payload at 040000.
;
; Prints KERNEL through the low-core helper and jumps directly to the first
; word of the Tape 2 payload at 040005.

        .text
        .globl start
        .globl __start

__start:
start:
        movei 017,050000
        move 01,msg_kernel
        pushj 017,000060
        jrst 040005
msg_kernel:
        .word 0534562564554
