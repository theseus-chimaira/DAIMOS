; lowcore-v1.s -- the exact 16-word Stage1 SIXBIT helper at 000060.

        .text
        .globl start
        .globl __start

__start:
start:
        .include "../../common/sixbit-output.inc"
