; cp_io.s -- resident PDP-6 card-punch I/O primitives.
        .text
        .globl cp_coni
        .globl cp_cono
        .globl cp_datao
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
