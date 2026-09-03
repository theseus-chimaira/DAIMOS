; wcnsls_io.s -- resident PDP-6 Spacewar console/scope primitives.
        .text
        .globl wcnsls_read
        .globl wcnsls_cono
        .globl wcnsls_plot
wcnsls_read:
        datai 0420,1
        aos wcnsls_io_in
        popj 017,
wcnsls_cono:
        cono 0420,0(1)
        popj 017,
wcnsls_plot:
        datao 0420,1
        aos wcnsls_io_out
        popj 017,

; Device-local accounting state; absent devices consume no fixed KCORE.
        .bss
wcnsls_io_in: .block 1
wcnsls_io_out: .block 1
