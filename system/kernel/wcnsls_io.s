; wcnsls_io.s -- resident PDP-6 Spacewar console/scope primitives.
        .text
        .globl devicefs_io_in
        .globl devicefs_io_out
        .globl wcnsls_read
        .globl wcnsls_cono
        .globl wcnsls_plot
wcnsls_read:
        datai 0420,1
        aos devicefs_io_in+5
        popj 017,
wcnsls_cono:
        cono 0420,0(1)
        popj 017,
wcnsls_plot:
        datao 0420,1
        aos devicefs_io_out+6
        popj 017,
