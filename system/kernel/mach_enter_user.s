; mach_enter_user.s -- boot-only PDP-6/PDP-10 initial user transition.
;
; This routine is called exactly once by KINIT after /SYSTEM/INIT has been
; loaded.  The runtime syscall entry/return path remains in resident
; mach_user.s.  Keeping this transition in KINIT avoids retaining it after
; boot while preserving the existing ABI and register-save semantics.

        .text
        .globl mach_enter_user
        .globl mach_kernel_sp

; void mach_enter_user(base, entry, stack, ac1, ac2, ac3)
; GCC supplies arguments 1..4 in AC1..AC4 and arguments 5..6 on the C stack.
mach_enter_user:
        move 5,-1(17)
        move 6,-2(17)
        ; Preserve the callee-saved user-entry ACs as one contiguous block.
        movei 0,1(17)
        hrli 0,010
        blt 0,7(17)
        add 17,[7,,7]
        pushj 17,mach_enter_user_start
        ; Restore AC10..AC16 with one block transfer.
        movei 0,-6(17)
        hrl 0,0
        hrri 0,010
        blt 0,016
        sub 17,[7,,7]
        popj 17,

mach_enter_user_start:
        movem 17,mach_kernel_sp
        move 7,1
        add 7,2
        move 16,1
        move 17,1
        add 17,3
        move 1,4
        move 2,5
        move 3,6
        jrst 0(7)
