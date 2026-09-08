; mach_user.s -- resident PDP-6 user UUO syscall boundary.
;
; PDP-6 user UUOs trap through executive locations 040/041.  Hardware writes
; the offending instruction to 040 and executes 041 in executive mode.  KINIT
; installs a JSR in 041 which lands at mach_syscall_save below.  The JSR save
; word retains the user flags and logical return PC, so JRST 2,@save restores
; user mode without exposing any physical kernel address to the user image.
;
; User ABI:
;   AC1  syscall number / return value
;   AC2  arg0
;   AC3  arg1
;   AC4  arg2
;   AC5  arg3
;   AC17 user pushdown pointer

        .text
        .globl mach_user_trap_init
        .globl mach_syscall
        .globl mach_return_to_kernel_request
        .globl exec_native_syscall
        .globl mach_syscall_ac2
        .globl mach_syscall_ac3
        .globl mach_syscall_ac4
        .globl mach_syscall_ac5
        .globl mach_kernel_sp

; Install the permanent PDP-6 user UUO vector after Stage1's 040/041 handoff
; has been copied out and after PI setup has stopped borrowing those words.
mach_user_trap_init:
        move 1,[jsr mach_syscall_save]
        movem 1,000041
        popj 17,

; JSR deposits user flags and the logical return PC here and starts at +1.
mach_syscall_save:
        .word 0
mach_syscall:
        ; Snapshot caller-saved syscall argument ACs before switching stacks.
        move 0,[2,,mach_syscall_ac2]
        blt 0,mach_syscall_ac5
        movem 17,mach_user_sp
        move 17,mach_kernel_sp
        pushj 17,exec_native_syscall
        movem 17,mach_kernel_sp

        ; EXIT requests return to the KINIT caller instead of user mode.
        skipn mach_user_sp
        popj 17,

        ; Restore only the user stack.  AC1 is the syscall return value;
        ; AC2..AC5 are caller-saved by the native ABI.
        move 17,mach_user_sp
        jrst 2,@mach_syscall_save

mach_return_to_kernel_request:
        setzm mach_user_sp
        popj 17,

        .bss
mach_syscall_ac2:  .word 0
mach_syscall_ac3:  .word 0
mach_syscall_ac4:  .word 0
mach_syscall_ac5:  .word 0
