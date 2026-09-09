; mach_user.s -- resident PDP-6 user UUO syscall boundary.
;
; PDP-6 user UUOs trap through executive locations 040/041.  Hardware writes
; the trapped instruction, with its computed effective address, to 040 and
; executes 041 in executive mode.  KINIT installs a JSR in 041 which lands at
; mach_syscall_save below.  JRST 2,@save restores user mode and logical PC.
;
; User ABI:
;   UUO opcode  syscall selector 040..073
;   UUO EA      arg0 (18-bit pointer/scalar)
;   AC2..AC4    arg1..arg3
;   AC1         return value
;   AC17        user pushdown pointer

        .text
        .globl mach_syscall
        .globl mach_syscall_save
        .globl mach_return_to_kernel_request
        .globl exec_native_syscall
        .globl fs_provider_reg_call
        .globl mach_kernel_sp

mach_syscall_save:
        .word 0
mach_syscall:
        ; PDP-6 programmed operators are also useful as compact executive
        ; calls.  A JSR from user mode records USER in bit 5 of this save
        ; word; executive KUUOs do not.  User selectors, including 000..037,
        ; always stay on the syscall path and therefore cannot reach private
        ; kernel services.
        move 6,mach_syscall_save
        tlnn 6,010000
        jrst mach_kernel_uuo

        ; Materialize arg0 from the UUO's computed effective address.  AC2..AC4
        ; remain live until exec_native_syscall has arranged the target call.
        hrrz 1,000040
        movem 17,mach_user_sp
        move 17,mach_kernel_sp
        ; Private KUUOs reuse mach_syscall_save.  Keep the outer user return
        ; on the process kernel stack so nested calls and sleeping syscalls are
        ; safe across scheduler save/restore.
        push 17,mach_syscall_save
        pushj 17,exec_native_syscall
        pop 17,mach_syscall_save
        movem 17,mach_kernel_sp

        skipn mach_user_sp
        popj 17,

        move 17,mach_user_sp
        jrst 2,@mach_syscall_save

; Private filesystem programmed-operator ABI:
;   UUO opcode  FS_MRES_OP_* (001..024 currently)
;   UUO EA      provider number, normally computed as 0(7)
;   AC1..AC5    request a..e
;
; fs_provider_reg_call validates both the provider and provider-local operation
; vector.  Preserve the JSR return on the current executive stack because a
; provider may sleep and the scheduler can save this executive continuation.
mach_kernel_uuo:
        push 17,mach_syscall_save
        hlrz 6,000040
        lsh 6,-011
        hrrz 7,000040
        pushj 17,fs_provider_reg_call
        pop 17,mach_syscall_save
        jrst 2,@mach_syscall_save

mach_return_to_kernel_request:
        setzm mach_user_sp
        popj 17,
