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
        .globl mach_user_trap_init
        .globl mach_syscall
        .globl mach_syscall_save
        .globl mach_return_to_kernel_request
        .globl exec_native_syscall
        .globl mach_kernel_sp

mach_user_trap_init:
        move 1,[jsr mach_syscall_save]
        movem 1,000041
        popj 17,

mach_syscall_save:
        .word 0
mach_syscall:
        ; Materialize arg0 from the UUO's computed effective address.  AC2..AC4
        ; remain live until exec_native_syscall has arranged the target call.
        hrrz 1,000040
        movem 17,mach_user_sp
        move 17,mach_kernel_sp
        pushj 17,exec_native_syscall
        movem 17,mach_kernel_sp

        skipn mach_user_sp
        popj 17,

        move 17,mach_user_sp
        jrst 2,@mach_syscall_save

mach_return_to_kernel_request:
        setzm mach_user_sp
        popj 17,
