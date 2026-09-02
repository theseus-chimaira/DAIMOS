        .text
        .globl _start
        .globl main
_start:
        pushj 17,main
        halt .
