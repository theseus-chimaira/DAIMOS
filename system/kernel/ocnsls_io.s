; ocnsls_io.s -- resident PDP-6 old Spacewar console switch input.
        .text
        .globl ocnsls_read
ocnsls_read:
        datai 0724,1
        popj 17,
