; kcore_boot.s -- leave disposable KINIT and finish boot on a resident stack.

        .text
        .globl kcore_boot_handoff
        .globl kcore_boot_start
        .globl mach_kernel_stack_base

; void kcore_boot_handoff(stack_base, reclaim_base, reclaim_words)
; Never returns to KINIT.  AC17 is replaced before any reclaimed word can be
; reused, then resident C code publishes the reclaimed extent and loads INIT.
kcore_boot_handoff:
        movem 1,mach_kernel_stack_base
        addi 1,060
        move 17,1
        move 1,2
        move 2,3
        pushj 17,kcore_boot_start
kcore_boot_halt:
        halt
        jrst kcore_boot_halt
