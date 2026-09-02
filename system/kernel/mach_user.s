; mach_user.s -- PDP-6/PDP-10 user transition and syscall context.
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
        .globl mach_enter_user
        .globl mach_syscall
        .globl mach_syscall_trampoline
        .globl mach_return_to_kernel_request
        .globl exec_native_syscall
        .globl mach_syscall_ac1
        .globl mach_syscall_ac2
        .globl mach_syscall_ac3
        .globl mach_syscall_ac4
        .globl mach_syscall_ac5

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

mach_syscall_trampoline:
        jrst mach_syscall

mach_syscall:
        move 0,[1,,mach_syscall_ac1]
        blt 0,mach_syscall_ac5
        movem 17,mach_user_sp
        move 17,mach_kernel_sp
        pushj 17,exec_native_syscall
        movem 17,mach_kernel_sp
        skipn mach_return_to_kernel_flag
        jrst mach_syscall_user_return
        setzm mach_return_to_kernel_flag
        popj 17,

mach_syscall_user_return:
        ; exec_native_syscall returns the user-visible result directly in AC1.
        move 17,mach_user_sp
        popj 17,

mach_return_to_kernel_request:
        setom mach_return_to_kernel_flag
        popj 17,

        .bss
mach_syscall_ac1:  .word 0
mach_syscall_ac2:  .word 0
mach_syscall_ac3:  .word 0
mach_syscall_ac4:  .word 0
mach_syscall_ac5:  .word 0
mach_user_sp:      .word 0
mach_kernel_sp:    .word 0
mach_return_to_kernel_flag: .word 0
