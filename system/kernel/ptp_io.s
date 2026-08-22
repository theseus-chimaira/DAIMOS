; ptp_io.s -- resident PDP-6 paper-tape punch I/O primitives.
        .text
        .globl ptp_coni
        .globl ptp_cono
        .globl ptp_datao
ptp_coni:
        coni 0100,1
        popj 17,
ptp_cono:
        move 2,[cono 0100,0]
        hrr 2,1
        xct 2
        popj 17,
ptp_datao:
        datao 0100,1
        popj 17,
