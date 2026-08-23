; kcore_entry.s -- fixed KCORE V0.1 entry at relocated base 060.
;
; KINIT performs manifest processing, relocation, binding, and MINIT dispatch
; without borrowing device services from KCORE.  Reaching KCORE currently
; means the bootstrap checkpoint is complete, so the entry halts directly.

        .text
        .globl __kcore_image_start
        .globl kcore_entry
        .globl pdp10_halt

__kcore_image_start:
kcore_entry:
pdp10_halt:
        halt .
        jrst pdp10_halt

        .bss
        .globl kcore_boot_handoff
kcore_boot_handoff:
        .block 2
