; clk_io.s -- PDP-6 APR line-clock primitives for the resident clock module.

        .text
        .globl clk_coni
        .globl clk_cono

clk_coni:
        coni 0000,clk_ioword
        move 1,clk_ioword
        popj 17,

clk_cono:
        cono 0000,0(1)
        popj 17,

        .data
clk_ioword: .word 0
