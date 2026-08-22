; wcnsls_io.s -- resident PDP-6 Spacewar console/scope primitives.
        .text
        .globl wcnsls_read
        .globl wcnsls_cono
        .globl wcnsls_plot
wcnsls_read:
        datai 0420,1
        popj 17,
wcnsls_cono:
        cono 0420,0(1)
        popj 17,
wcnsls_plot:
        datao 0420,1
        popj 17,
