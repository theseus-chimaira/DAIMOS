; cr_io.s -- resident PDP-6 card-reader I/O primitives.
        .text
        .globl cr_coni
        .globl cr_cono
        .globl cr_datai
cr_coni:
        coni 0150,1
        popj 17,
cr_cono:
        move 2,[cono 0150,0]
        hrr 2,1
        xct 2
        popj 17,
cr_datai:
        datai 0150,1
        popj 17,
