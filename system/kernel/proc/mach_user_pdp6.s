/**
 * @file mach_user_pdp6.s
 * @brief Resident PDP-6 monitor-UUO trap and user-return boundary.
 *
 * PDP-6 user UUOs trap through executive locations 040/041. Hardware stores
 * the trapped instruction with its computed effective address at 040 and
 * executes 041 in executive mode. KINIT installs a JSR there which lands at
 * mach_syscall_save. JRST 2,@mach_syscall_save restores user mode and logical
 * PC after exec_native_syscall() returns.
 *
 * User ABI: UUO opcode selects monitor call 040..077; the UUO effective address
 * is arg0; AC2..AC4 are arg1..arg3; AC1 is the result; AC17 is the user stack.
 * The stable process-private kernel stack is substituted while the syscall is
 * active so sleeping calls survive process rescheduling.
 */

        .text
        .globl mach_syscall
        .globl mach_syscall_save
        .globl mach_return_to_kernel_request
        .globl exec_native_syscall
        .globl mach_kernel_sp

/** Saved PDP-6 hardware JSR return word for the current monitor UUO. */
mach_syscall_save:
        .word 0
/** @brief Enter native syscall dispatch from a PDP-6 user monitor UUO. */
mach_syscall:
        ; All programmed operators reaching this entry are user syscalls.
        ; Resident kernel code calls movable filesystem providers through the
        ; direct register bridge instead of taking a second executive trap.
        ; User LUUO selectors 001..037 never enter this monitor dispatcher.

        ; Materialize arg0 from the UUO's computed effective address.  AC2..AC4
        ; remain live until exec_native_syscall has arranged the target call.
        hrrz 1,000040
        movem 17,mach_user_sp
        move 17,mach_kernel_sp
        ; Keep the user return on the process kernel stack so sleeping
        ; syscalls are safe across scheduler save/restore.
        push 17,mach_syscall_save
        pushj 17,exec_native_syscall
        pop 17,mach_syscall_save
        movem 17,mach_kernel_sp

        skipn mach_user_sp
        popj 17,

        move 17,mach_user_sp
        jrst 2,@mach_syscall_save

/** @brief Suppress user-mode return by clearing the saved user stack pointer. */
mach_return_to_kernel_request:
        setzm mach_user_sp
        popj 17,
