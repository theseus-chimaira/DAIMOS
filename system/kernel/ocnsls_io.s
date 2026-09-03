; ocnsls_io.s -- resident PDP-6 old Spacewar console switch input.
        .text
        .globl ocnsls_read
ocnsls_read:
        datai 0724,1
        aos ocnsls_io_in
        popj 017,

; Device-local accounting state; absent devices consume no fixed KCORE.
        .bss
ocnsls_io_in: .block 1
