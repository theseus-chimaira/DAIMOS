; ocnsls_io.s -- resident PDP-6 old Spacewar console switch input.
        .text
        .globl devicefs_io_in
        .globl devicefs_io_out
        .globl ocnsls_read
ocnsls_read:
        datai 0724,1
        aos devicefs_io_in+6
        popj 017,
