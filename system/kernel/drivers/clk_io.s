/**
 * @file clk_io.s
 * @brief Resident PDP-6 APR 60 Hz monotonic-clock interrupt service.
 *
 * KINIT probes the APR clock and copies this file's MRES package into permanent
 * memory only when the clock is usable.  The package then remains resident for
 * the life of the kernel: it owns the monotonic tick counter, clears each APR
 * clock flag, advances storage timeouts, and enters the scheduler once per
 * qualified tick.  Software PI6 requests use the same entry to reschedule a
 * sleeping process without fabricating elapsed time.
 *
 * APR and the line clock share one PDP-6 PIA.  The Type 340 display may attach
 * a post-service hook to the clock handler, but the CLK handler itself remains
 * the registered PI6 owner.  The generic PI dispatcher keeps its span cursor
 * in AC2 and its level-return address in AC3; clk_pi_service preserves both in
 * resident words because scheduler work may change the active kernel stack.
 */
        .text
        .globl clk_pi_handler
        .globl clk_pi_post_handler
        .globl clk_pi_service
        .globl clk_ticks
        .globl clk_tick_count
        .globl pdp10_pi_handler_return
        .globl proc_sched_pi_tick
        .globl proc_sched_pi_resched
        .globl proc_sched_kick
        .globl storage_clock_tick

/**
 * @brief PI6 handler entry registered by CLK MINIT.
 *
 * @return Does not return normally; control jumps to pdp10_pi_handler_return.
 *
 * AC1 may be clobbered.  AC2 and AC3 are preserved by clk_pi_service because
 * the common PI dispatcher still owns them.  AC17 is the active kernel stack.
 */
clk_pi_handler:
        pushj 017,clk_pi_service
clk_pi_post_handler:
        jrst pdp10_pi_handler_return

/**
 * @brief Service a possible APR clock event or software PI6 reschedule request.
 *
 * @return Returns to the caller through AC17 with AC2 and AC3 unchanged.
 *
 * The routine is also called by the optional DPY clock wrapper when a Type 340
 * is installed. A set APR clock flag denotes a real 60 Hz tick: increment
 * clk_tick_count, clear/re-enable the
 * hardware flag, advance storage timers, consume any pending software kick,
 * and run the scheduler tick path.  Without a clock flag, a nonzero
 * proc_sched_kick requests an immediate reschedule only; it must not increment
 * time, age sleepers, or charge a quantum.
 *
 * AC1 may be clobbered by the called services.  AC2/AC3 live in dedicated
 * resident save words because the generic PI dispatcher requires them to
 * survive every handler and the scheduler may switch kernel stacks.
 */
clk_pi_service:
        ; AC2/AC3 belong to the compact PI dispatcher, not to the scheduler.
        ; Do not preserve them on AC17: proc_sched_pi_tick may select another
        ; process/kernel-stack context before returning here.  PI6 cannot nest
        ; itself, so one resident save pair is sufficient and remains valid
        ; across any such scheduling decision.
        movem 2,clk_pi_saved_ac2
        movem 3,clk_pi_saved_ac3
        conso 0000,01000
        jrst clk_pi_kick
        aos clk_tick_count
        cono 0000,003006
        pushj 017,storage_clock_tick
        setzm proc_sched_kick
        pushj 017,proc_sched_pi_tick
        jrst clk_pi_service_done

; A real tick consumes a simultaneous software kick before entering the tick
; scheduler, so one PI6 event causes only one scheduling decision.
clk_pi_kick:
        skipn proc_sched_kick
        jrst clk_pi_service_done
        setzm proc_sched_kick
        pushj 017,proc_sched_pi_resched
clk_pi_service_done:
        move 3,clk_pi_saved_ac3
        move 2,clk_pi_saved_ac2
        popj 017,

/**
 * @brief Read the resident 60 Hz monotonic tick counter.
 *
 * @return AC1 = clk_tick_count; AC17 is unchanged after the return.
 *
 * The single-word read is intentionally lock-free.  PDP-6 memory-word loads
 * are atomic, and the interrupt path updates the counter with one AOS.
 */
clk_ticks:
        move 1,clk_tick_count
        popj 017,

        .bss
/** Resident 36-bit tick counter; initialized to zero when the MRES is installed. */
clk_tick_count:
        .block 1
/** PI6 dispatcher cursor saved across scheduler/storage service calls. */
clk_pi_saved_ac2:
        .block 1
/** PI6 level-return stub saved across scheduler/storage service calls. */
clk_pi_saved_ac3:
        .block 1
