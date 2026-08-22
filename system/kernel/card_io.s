; card_io.s -- resident PDP-6 card reader/punch I/O primitives.
        .text
        .globl cr_coni
        .globl cr_cono
        .globl cr_datai
        .globl cp_coni
        .globl cp_cono
        .globl cp_datao
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
cp_coni:
        coni 0110,1
        popj 17,
cp_cono:
        move 2,[cono 0110,0]
        hrr 2,1
        xct 2
        popj 17,
cp_datao:
        datao 0110,1
        popj 17,
