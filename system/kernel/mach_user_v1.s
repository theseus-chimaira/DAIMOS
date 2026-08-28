; mach_user_v1.s -- PDP-6/PDP-10 user transition and syscall context.
;
; User ABI:
;   AC1  syscall number / startup AC1
;   AC2  arg0 / startup AC2
;   AC3  arg1 / startup AC3
;   AC4  arg2
;   AC16 DEX image base
;   AC17 user pushdown pointer
;
; The syscall boundary only snapshots AC1..AC5, which contain the complete
; native syscall argument set.  AC0..AC7 are caller-saved by the C ABI.
; AC10..AC16 are callee-saved and remain live through the kernel call chain.

        .text
        .globl mach_enter_user_v1
        .globl mach_syscall_v1
        .globl mach_syscall_trampoline_v1
        .globl mach_return_to_kernel_request_v1
        .globl exec_native_syscall_v1
        .globl mach_syscall_ac1_v1
        .globl mach_syscall_ac2_v1
        .globl mach_syscall_ac3_v1
        .globl mach_syscall_ac4_v1
        .globl mach_syscall_ac5_v1

; void mach_enter_user_v1(base, entry, stack, ac1, ac2, ac3)
; GCC supplies arguments 1..4 in AC1..AC4 and arguments 5..6 on the C stack.
mach_enter_user_v1:
        move 5,-1(17)
        move 6,-2(17)
        ; Preserve the callee-saved user-entry ACs as one contiguous block.
        movei 0,1(17)
        hrli 0,010
        blt 0,7(17)
        add 17,[7,,7]
        pushj 17,mach_enter_user_start_v1
        ; Restore AC10..AC16 with one block transfer.
        movei 0,-6(17)
        hrl 0,0
        hrri 0,010
        blt 0,016
        sub 17,[7,,7]
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
        move 0,[1,,mach_syscall_ac1_v1]
        blt 0,mach_syscall_ac5_v1
        movem 17,mach_user_sp_v1
        move 17,mach_kernel_sp_v1
        pushj 17,exec_native_syscall_v1
        movem 17,mach_kernel_sp_v1
        skipn mach_return_to_kernel_flag_v1
        jrst mach_syscall_user_return_v1
        setzm mach_return_to_kernel_flag_v1
        popj 17,

mach_syscall_user_return_v1:
        ; exec_native_syscall_v1 returns the user-visible result directly in AC1.
        move 17,mach_user_sp_v1
        popj 17,

mach_return_to_kernel_request_v1:
        setom mach_return_to_kernel_flag_v1
        popj 17,

        .bss
mach_syscall_ac1_v1:  .word 0
mach_syscall_ac2_v1:  .word 0
mach_syscall_ac3_v1:  .word 0
mach_syscall_ac4_v1:  .word 0
mach_syscall_ac5_v1:  .word 0
mach_user_sp_v1:      .word 0
mach_kernel_sp_v1:    .word 0
mach_return_to_kernel_flag_v1: .word 0
