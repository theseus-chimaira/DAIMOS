/**
 * @file kinit_late_begin_pdp6.s
 * @brief Entry boundary and machine-level helpers for protected late KINIT.
 *
 * This object begins the KINIT text range which must remain unavailable to the
 * memory allocator after the ordinary bootstrap image has been released.
 * kinit_late_start() executes from this protected range while it performs the
 * final allocating/VFS work, then releases the range immediately before the
 * no-return transition to the initial user process.
 *
 * This is the PDP-6 baseline entry object because it installs the user-UUO
 * trap directly in PDP-6 low core at physical 000041.  Later PDP-10 memory
 * management and trap implementations may require a different entry path.
 * AC17 is the C pushdown pointer; C arguments arrive in AC1, AC2, ...
 * according to the kernel calling convention.
 */

        .text
        .globl __kinit_late_begin
        .globl kinit_late_handoff
        .globl kinit_late_start
        .globl mach_kernel_stack_base
        .globl kinit_user_trap_init
        .globl mach_syscall_save

__kinit_late_begin:

/**
 * @brief Install the private user monitor-UUO entry used during late boot.
 *
 * PDP-6 user UUO 041 vectors through physical location 041.  KINIT installs a
 * JSR to mach_syscall_save there before resident VFS/bootstrap code can issue
 * private monitor calls.  Stage1 handoff data formerly occupying this word has
 * already been copied to private KINIT storage.
 */
kinit_user_trap_init:
        move 1,[jsr mach_syscall_save]
        movem 1,000041
        popj 17,

/**
 * @brief Enter protected late KINIT without switching stacks prematurely.
 *
 * Keep the existing KINIT reserve stack until every allocating late-KINIT
 * operation is complete.  kinit_late_start() publishes stack_base for later
 * idle/exit use only immediately before releasing the KINIT stack reserve and
 * making the no-return user transition.
 *
 * A return from kinit_late_start() indicates bootstrap failure.  The handoff
 * therefore stops the processor instead of returning into already-partially-
 * reclaimed bootstrap state.
 *
 * @param stack_base AC1: base of the permanent idle/exit kernel stack.
 * @param reclaim_end AC2: first word after the transient KINIT stack reserve.
 */
kinit_late_handoff:
        pushj 17,kinit_late_start
kinit_late_halt:
        halt
        jrst kinit_late_halt
