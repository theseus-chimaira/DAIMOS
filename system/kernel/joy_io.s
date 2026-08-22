; joy_io.s -- resident PDP-6 Spacewar console I/O primitives.
        .text
        .globl wcnsls_read
        .globl ocnsls_read
        .globl wcnsls_cono
wcnsls_read:
        datai 0420,1
        popj 17,
ocnsls_read:
        datai 0724,1
        popj 17,
wcnsls_cono:
        cono 0420,0(1)
        popj 17,
