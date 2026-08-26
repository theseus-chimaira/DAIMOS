; mach_user_v1.s -- PDP-6/PDP-10 user transition and syscall context.
;
; Kept separate from the compact resident PI runtime.  The syscall dispatcher
; is supplied by the higher-level syscall layer when userland is linked.
;
; User ABI:
;   AC1  syscall number / startup AC1
;   AC2  arg0 / startup AC2
;   AC3  arg1 / startup AC3
;   AC4  arg2
;   AC16 DEX image base
;   AC17 user pushdown pointer

        .text
        .globl mach_enter_user_v1
        .globl mach_syscall_v1
        .globl mach_syscall_trampoline_v1
        .globl mach_syscall_context_save_v1
        .globl mach_syscall_context_restore_v1
        .globl mach_return_to_kernel_request_v1
        .globl mach_syscall_ac1_v1
        .globl mach_syscall_ac2_v1
        .globl mach_syscall_ac3_v1
        .globl mach_syscall_ac4_v1
        .globl mach_syscall_ac5_v1
        .globl mach_dispatch_syscall_v1

; void mach_enter_user_v1(base, entry, stack, ac1, ac2, ac3)
; GCC supplies arguments 1..4 in AC1..AC4 and arguments 5..6 on the C stack.
mach_enter_user_v1:
        move 5,-1(17)
        move 6,-2(17)
        push 17,010
        push 17,011
        push 17,012
        push 17,013
        push 17,014
        push 17,015
        push 17,016
        pushj 17,mach_enter_user_start_v1
        pop 17,016
        pop 17,015
        pop 17,014
        pop 17,013
        pop 17,012
        pop 17,011
        pop 17,010
        popj 17,

mach_enter_user_start_v1:
        movem 17,mach_kernel_sp_v1
        move 7,1
        add 7,2
        move 16,1
        move 17,1
        add 17,3
        move 1,4
        move 2,5
        move 3,6
        jrst 0(7)

mach_syscall_trampoline_v1:
        jrst mach_syscall_v1

mach_syscall_v1:
        movem 0,mach_syscall_ac0_v1
        move 0,[1,,mach_syscall_ac1_v1]
        blt 0,mach_syscall_ac16_v1
        movem 17,mach_user_sp_v1
        move 17,mach_kernel_sp_v1
        setz 16,
        movei 1,mach_syscall_ac0_v1
        pushj 17,mach_dispatch_syscall_v1
        movem 17,mach_kernel_sp_v1
        skipn mach_return_to_kernel_flag_v1
        jrst mach_syscall_user_return_v1
        setzm mach_return_to_kernel_flag_v1
        popj 17,

mach_syscall_user_return_v1:
        move 1,mach_syscall_ac1_v1
        move 2,mach_syscall_ac2_v1
        move 3,mach_syscall_ac3_v1
        move 4,mach_syscall_ac4_v1
        move 5,mach_syscall_ac5_v1
        move 6,mach_syscall_ac6_v1
        move 7,mach_syscall_ac7_v1
        move 010,mach_syscall_ac10_v1
        move 011,mach_syscall_ac11_v1
        move 012,mach_syscall_ac12_v1
        move 013,mach_syscall_ac13_v1
        move 014,mach_syscall_ac14_v1
        move 015,mach_syscall_ac15_v1
        move 016,mach_syscall_ac16_v1
        move 0,mach_syscall_ac0_v1
        move 17,mach_user_sp_v1
        popj 17,

; Save/restore the complete AC0..AC16 plus user SP context used by nested
; process execution and wait-style kernel control paths.
mach_syscall_context_save_v1:
        move 0,1
        hrli 0,mach_syscall_ac0_v1
        blt 0,017(1)
        popj 17,

mach_syscall_context_restore_v1:
        movei 0,mach_syscall_ac0_v1
        hrl 0,1
        blt 0,mach_user_sp_v1
        popj 17,

mach_return_to_kernel_request_v1:
        setom mach_return_to_kernel_flag_v1
        popj 17,

        .bss
mach_syscall_ac0_v1:  .word 0
mach_syscall_ac1_v1:  .word 0
mach_syscall_ac2_v1:  .word 0
mach_syscall_ac3_v1:  .word 0
mach_syscall_ac4_v1:  .word 0
mach_syscall_ac5_v1:  .word 0
mach_syscall_ac6_v1:  .word 0
mach_syscall_ac7_v1:  .word 0
mach_syscall_ac10_v1: .word 0
mach_syscall_ac11_v1: .word 0
mach_syscall_ac12_v1: .word 0
mach_syscall_ac13_v1: .word 0
mach_syscall_ac14_v1: .word 0
mach_syscall_ac15_v1: .word 0
mach_syscall_ac16_v1: .word 0
mach_user_sp_v1:      .word 0
mach_kernel_sp_v1:    .word 0
mach_return_to_kernel_flag_v1: .word 0
