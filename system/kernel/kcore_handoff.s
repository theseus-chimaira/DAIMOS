; kcore_boot.s -- leave disposable KINIT and finish boot on a resident stack.

        .text
        .globl kcore_boot_handoff
        .globl kcore_boot_start

; void kcore_boot_handoff(stack_base, reclaim_base, reclaim_words)
; Never returns to KINIT.  AC17 is replaced before any reclaimed word can be
; reused, then resident C code publishes the reclaimed extent and loads INIT.
kcore_boot_handoff:
        move 17,1
        move 1,2
        move 2,3
        pushj 17,kcore_boot_start
kcore_boot_halt:
        halt
        jrst kcore_boot_halt
