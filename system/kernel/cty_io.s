; cty_io.s -- PDP-6 console device primitives for the resident CTY module.

        .text
        .globl cty_coni
        .globl cty_cono
        .globl cty_datai
        .globl cty_datao

cty_coni:
        coni 0120,1
        popj 17,

cty_cono:
        cono 0120,0(1)
        popj 17,

cty_datai:
        datai 0120,1
        popj 17,

cty_datao:
        datao 0120,1
        popj 17,
