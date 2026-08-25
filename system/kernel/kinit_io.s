; kinit_io.s -- disposable KINIT bootstrap helpers.

        .text
        .globl kinit_put6
        .globl kinit_newline
        .globl kinit_call18
        .globl kinit_call18_0
        .globl kinit_call18_1
        .globl kinit_call18_2
        .globl kinit_halt

; void kinit_put6(kword_t word)
kinit_put6:
        pushj 017,077760
        popj 017,

; Polling CR/LF, deliberately independent of CTY module state.
kinit_newline:
        movei 03,015
        pushj 017,knl_putc
        movei 03,012
knl_putc:
        coni 0120,04
        trne 04,0020
        jrst knl_putc
        datao 0120,03
        popj 017,

; void kinit_call18(unsigned int address)
kinit_call18:
        andi 01,0777777
        pushj 017,(01)
        popj 017,

; kword_t kinit_call18_0(unsigned int address)
kinit_call18_0:
        andi 01,0777777
        pushj 017,(01)
        popj 017,

; kword_t kinit_call18_1(unsigned int address, kword_t arg)
kinit_call18_1:
        move 03,01
        move 01,02
        andi 03,0777777
        pushj 017,(03)
        popj 017,

; kword_t kinit_call18_2(unsigned int address, kword_t arg1, kword_t arg2)
kinit_call18_2:
        move 04,01
        move 01,02
        move 02,03
        andi 04,0777777
        pushj 017,(04)
        popj 017,

kinit_halt:
        halt .
        jrst kinit_halt
