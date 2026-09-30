; INIT-only MEMFS mount veneer.
;
; Keep this out of the common libc syscall object: normal programs do not
; mount MEMFS, and charging every executable for this administrative call is
; especially harmful on the 32K PDP-6 profile.
        .text
        .globl dsys_memfs_mount
dsys_memfs_mount:
        move    3,2
        move    2,1
        movei   1,046
        uuo     077,0(1)
        popj    17,
