; kcore_entry.s -- fixed KCORE V0.1 entry at relocated base 060.
;
; KINIT has already relocated KCORE and installed the MRES packages when this
; entry is reached.  The C bootstrap mounts INITFS/MEMFS and starts PID 1.

        .text
        .globl __kcore_image_start
        .globl kcore_entry
        .globl kcore_boot_v1
        .globl pdp10_halt

__kcore_image_start:
kcore_entry:
        movei 17,037777
        pushj 17,kcore_boot_v1
pdp10_halt:
        halt .
        jrst pdp10_halt

        .bss
        .globl kcore_boot_handoff
kcore_boot_handoff:
        .block 2
