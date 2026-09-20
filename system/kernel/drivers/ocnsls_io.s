; ocnsls_io.s -- resident PDP-6 old Spacewar console switch input.
        .globl mfsdev_io_in
        .text
        .globl ocnsls_read
ocnsls_read:
        datai 0724,1
        aos mfsdev_io_in+013
        popj 017,

; Device-local accounting state; absent devices consume no fixed KCORE.
        .bss
