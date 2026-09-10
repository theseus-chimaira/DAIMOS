; kinit_late_begin.s -- protected late-bootstrap hole inside disposable KINIT.

        .text
        .globl __kinit_late_begin
        .globl kinit_late_handoff
        .globl kinit_late_start
        .globl mach_kernel_stack_base
        .globl kinit_user_trap_init
        .globl mach_syscall_save

__kinit_late_begin:

kinit_user_trap_init:
        move 1,[jsr mach_syscall_save]
        movem 1,000041
        popj 17,

; void kinit_late_handoff(stack_base, reclaim_end)
; Keep the existing KINIT reserve stack until all allocating late-KINIT work
; is complete.  kinit_late_start() publishes stack_base for later idle/exit
; use immediately before the no-return user transition.
kinit_late_handoff:
        pushj 17,kinit_late_start
kinit_late_halt:
        halt
        jrst kinit_late_halt
