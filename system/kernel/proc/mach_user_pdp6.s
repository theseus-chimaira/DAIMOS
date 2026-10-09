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

        ; Materialize arg0 and the syscall selector immediately.  Low-core 040
        ; is hardware scratch for the trapped UUO, not process-private state;
        ; do not leave the dispatcher dependent on it across stack setup or a
        ; nested interrupt.  AC2..AC4 remain live until exec_native_syscall
        ; has arranged the target call; AC5 carries opcode-040.
        hrrz 1,000040
        hlrz 5,000040
        lsh 5,-011
        subi 5,040
        movem 17,mach_user_sp
        move 17,mach_kernel_sp
        ; A malformed monitor trap must never index the packed native
        ; dispatcher outside its 040..077 opcode range.  In particular,
        ; opcode zero would otherwise use a negative table index and XCT
        ; arbitrary resident words.  Terminate the offending user process
        ; through the normal EXIT service instead.
        caige 5,0
        jrst mach_syscall_invalid
        caile 5,037
        jrst mach_syscall_invalid
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


        .globl mach_invalid_uuo_word
        .globl mach_invalid_uuo_pc
mach_syscall_invalid:
        ; Record the actual low-core trap and the hardware JSR return word
        ; before output can invoke a PI handler or clobber AC1..AC3.
        move 1,000040
        movem 1,mach_invalid_uuo_word
        move 1,mach_syscall_save
        movem 1,mach_invalid_uuo_pc
        ; TEMPORARY diagnostic: direct CTY output avoids user buffers and
        ; syscalls.  CTY is permanent/resident; AC17 is the kernel stack.
        movei 1,012                 ; newline begins error line
        pushj 17,mach_invalid_putchar
        movei 1,0125                ; U
        pushj 17,mach_invalid_putchar
        movei 1,0125                ; U
        pushj 17,mach_invalid_putchar
        movei 1,0117                ; O
        pushj 17,mach_invalid_putchar
        movei 1,040                 ; space
        pushj 17,mach_invalid_putchar
        move 4,mach_invalid_uuo_word
        pushj 17,mach_invalid_oct36
        movei 1,040                 ; separator
        pushj 17,mach_invalid_putchar
        move 4,mach_invalid_uuo_pc
        pushj 17,mach_invalid_oct36
        movei 1,015
        pushj 17,mach_invalid_putchar
        movei 1,012
        pushj 17,mach_invalid_putchar
        movei 1,0176                ; exit status 126
        setz 5,                     ; selector zero is native EXIT
        jrst exec_native_syscall

 ; Temporary diagnostic output: poll the PDP-6 console directly so no
; link dependency on the separately loaded CTY MRES is introduced.
; AC2 is scratch; preserves AC4 (the octal shift register).
mach_invalid_putchar:
        movei 2,0200000
mach_invalid_put_wait:
        conso 0120,0020
        jrst mach_invalid_put_ready
        sojg 2,mach_invalid_put_wait
        popj 17,
mach_invalid_put_ready:
        andi 1,0177
        datao 0120,1
        popj 17,

; Display one saved 36-bit word in twelve octal digits.  AC4 holds the
; shifting value, AC7 counts digits; cty_putchar clobbers AC1..AC3.
mach_invalid_oct36:
        movei 7,014
mach_invalid_oct36_next:
        move 1,4
        lsh 1,-041                   ; right shift 33 decimal bits
        andi 1,07
        addi 1,060
        pushj 17,mach_invalid_putchar
        lsh 4,03
        sojg 7,mach_invalid_oct36_next
        popj 17,

        .data
mach_invalid_uuo_word:
        .word 0
mach_invalid_uuo_pc:
        .word 0
        .text

/** @brief Suppress user-mode return by clearing the saved user stack pointer. */
mach_return_to_kernel_request:
        setzm mach_user_sp
        popj 17,
