; ptr_io.s -- resident PDP-6 paper-tape reader I/O primitives.
;

        .text
        .globl ptr_coni
        .globl ptr_cono
        .globl ptr_datai

ptr_coni:
        coni 0104,1
        popj 17,

ptr_cono:
        move 2,[cono 0104,0]
        hrr 2,1
        xct 2
        popj 17,

ptr_datai:
        datai 0104,1
        popj 17,
