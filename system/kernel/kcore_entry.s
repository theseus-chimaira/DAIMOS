; kcore_entry.s -- fixed KCORE V0.1 entry at relocated base 060.
;
; KINIT performs manifest processing, relocation, binding, and MINIT dispatch
; without borrowing device services from KCORE.

        .text
        .globl __kcore_image_start
        .globl kcore_entry
        .globl kcore_entry_impl
        .globl pdp10_halt

__kcore_image_start:
kcore_entry:      jrst kcore_entry_impl

pdp10_halt:
        halt .
        jrst pdp10_halt
