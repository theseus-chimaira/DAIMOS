; ocnsls_io.s -- resident PDP-6 old Spacewar console switch input.
        .text
        .globl devicefs_v1_io_in
        .globl devicefs_v1_io_out
        .globl ocnsls_read
ocnsls_read:
        datai 0724,1
        aos devicefs_v1_io_in+11
        popj 017,
