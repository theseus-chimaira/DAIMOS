/**
 * @file kcore_io.s
 * @brief Minimal resident PDP-6 priority-interrupt runtime.
 *
 * The seven PDP-6 PI levels enter through fixed low-core JSR vectors installed
 * by MINIT.  Each entry saves AC1, AC2, AC3, and AC17 before dispatching the
 * registered resident handlers for that level.  AC1..AC3 are private to the
 * interrupt ABI; all other accumulators are left untouched by the dispatcher.
 * Separate save cells for every level make higher-priority nesting safe even
 * while a lower level is in its handler or return path.
 *
 * Low-core AC save layout:
 *   PI1: AC1=020 AC2=021 AC3=043
 *   PI2: AC1=022 AC2=023 AC3=045
 *   PI3: AC1=024 AC2=025 AC3=036
 *   PI4: AC1=026 AC2=027 AC3=051
 *   PI5: AC1=030 AC2=031 AC3=053
 *   PI6: AC1=032 AC2=033 AC3=055
 *   PI7: AC1=034 AC2=035 AC3=057
 *
 * AC17 is saved in pdp10_pi_sp_save at even offsets 0,2,...,014.  The unused
 * odd words of that table are deliberately overlaid with process globals to
 * avoid permanent BSS waste; the return calculation can only address even
 * offsets.  Handler spans and handler addresses remain resident because PI
 * dispatch must work after KINIT memory has been reclaimed.
 *
 * If an interrupt arrives while AC17 denotes a user stack, the level entry
 * switches to mach_kernel_sp before calling handlers.  Bit 010000 in the JSR
 * save word distinguishes that case.  Executive locations 040 and 041 remain
 * reserved for the PDP-6 user-UUO trap block and are not PI scratch storage.
 */

        .text
        .globl pdp10_pi_handler_return
        .globl pdp10_pi_dispatch
        .globl pdp10_pi_dispatch_done
        .globl pdp10_pi_level1
        .globl pdp10_pi_level1_dispatch_jump
        .globl pdp10_pi_level2
        .globl pdp10_pi_level2_dispatch_jump
        .globl pdp10_pi_level3
        .globl pdp10_pi_level3_dispatch_jump
        .globl pdp10_pi_level4
        .globl pdp10_pi_level4_dispatch_jump
        .globl pdp10_pi_level5
        .globl pdp10_pi_level5_dispatch_jump
        .globl pdp10_pi_level6
        .globl pdp10_pi_level6_dispatch_jump
        .globl pdp10_pi_level7
        .globl pdp10_pi_handlers
        .globl pdp10_pi_level_span
        .globl pdp10_pi_sp_save
        .globl mach_kernel_sp

/** @brief PI level 1 entry; saves AC1..AC3/AC17 and dispatches its span. */
pdp10_pi_level1:
        .word 0
        movem 1,000020
        movem 2,000021
        movem 3,000043
        movem 17,pdp10_pi_sp_save+0
        move 1,pdp10_pi_level1
        tlnn 1,010000
        jrst pdp10_pi_level1_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level1_stack_ready:
        move 2,pdp10_pi_level_span+0
        movei 3,pdp10_pi_return_level1
pdp10_pi_level1_dispatch_jump:
        jrst pdp10_pi_dispatch
/** @brief PI level 2 entry; saves AC1..AC3/AC17 and dispatches its span. */
pdp10_pi_level2:
        .word 0
        movem 1,000022
        movem 2,000023
        movem 3,000045
        movem 17,pdp10_pi_sp_save+2
        move 1,pdp10_pi_level2
        tlnn 1,010000
        jrst pdp10_pi_level2_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level2_stack_ready:
        move 2,pdp10_pi_level_span+1
        movei 3,pdp10_pi_return_level2
pdp10_pi_level2_dispatch_jump:
        jrst pdp10_pi_dispatch
/** @brief PI level 3 entry; shared completion path for the block-data channel. */
pdp10_pi_level3:
        .word 0
        movem 1,000024
        movem 2,000025
        movem 3,000036
        movem 17,pdp10_pi_sp_save+4
        move 1,pdp10_pi_level3
        tlnn 1,010000
        jrst pdp10_pi_level3_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level3_stack_ready:
        move 2,pdp10_pi_level_span+2
        movei 3,pdp10_pi_return_level3
pdp10_pi_level3_dispatch_jump:
        jrst pdp10_pi_dispatch
/** @brief PI level 4 entry; saves AC1..AC3/AC17 and dispatches its span. */
pdp10_pi_level4:
        .word 0
        movem 1,000026
        movem 2,000027
        movem 3,000051
        movem 17,pdp10_pi_sp_save+6
        move 1,pdp10_pi_level4
        tlnn 1,010000
        jrst pdp10_pi_level4_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level4_stack_ready:
        move 2,pdp10_pi_level_span+3
        movei 3,pdp10_pi_return_level4
pdp10_pi_level4_dispatch_jump:
        jrst pdp10_pi_dispatch
/** @brief PI level 5 entry; saves AC1..AC3/AC17 and dispatches its span. */
pdp10_pi_level5:
        .word 0
        movem 1,000030
        movem 2,000031
        movem 3,000053
        movem 17,pdp10_pi_sp_save+010
        move 1,pdp10_pi_level5
        tlnn 1,010000
        jrst pdp10_pi_level5_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level5_stack_ready:
        move 2,pdp10_pi_level_span+4
        movei 3,pdp10_pi_return_level5
pdp10_pi_level5_dispatch_jump:
        jrst pdp10_pi_dispatch
/** @brief PI level 6 entry; saves AC1..AC3/AC17 and dispatches its span. */
pdp10_pi_level6:
        .word 0
        movem 1,000032
        movem 2,000033
        movem 3,000055
        movem 17,pdp10_pi_sp_save+012
        move 1,pdp10_pi_level6
        tlnn 1,010000
        jrst pdp10_pi_level6_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level6_stack_ready:
        move 2,pdp10_pi_level_span+5
        movei 3,pdp10_pi_return_level6
pdp10_pi_level6_dispatch_jump:
        jrst pdp10_pi_dispatch
/** @brief PI level 7 entry; saves AC1..AC3/AC17 and dispatches its span. */
pdp10_pi_level7:
        .word 0
        movem 1,000034
        movem 2,000035
        movem 3,000057
        movem 17,pdp10_pi_sp_save+014
        move 1,pdp10_pi_level7
        tlnn 1,010000
        jrst pdp10_pi_level7_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level7_stack_ready:
        move 2,pdp10_pi_level_span+6
        movei 3,pdp10_pi_return_level7

/**
 * @brief Dispatch the compact handler span in AC2.
 *
 * @param AC2 Packed -count,,start cursor, or zero for no handlers.
 * @param AC3 Address of the level-specific return stub; preserved.
 * @return Does not return normally.  Control reaches pdp10_pi_handler_return
 *         from each handler and eventually branches through AC3.
 *
 * A nonzero span is stored as -count,,start.  The first handler is entered
 * without modifying the cursor; AOBJN in the common handler-return path then
 * advances both the negative count and handler-table index.
 */
pdp10_pi_dispatch:
        jumpe 2,pdp10_pi_dispatch_done
        jrst @pdp10_pi_handlers(2)

/**
 * @brief Common continuation for resident PI handlers.
 *
 * Handlers must preserve AC2 and AC3 and jump here when complete.  AC2 is
 * advanced to the next handler; exhaustion falls through to the restore path.
 */
pdp10_pi_handler_return:
        aobjn 2,pdp10_pi_dispatch
pdp10_pi_dispatch_done:
pdp10_pi_return_common:

        ; AC3 names a two-word-stride return stub.  Its offset therefore equals
        ; the even save-table offset for this level, allowing one common
        ; AC17/AC1/AC2 restore before the level-specific AC3/PI dismissal.
        move 2,3
        subi 2,pdp10_pi_return_level1
        move 17,pdp10_pi_sp_save(2)
        move 1,000020(2)
        move 2,000021(2)
        jrst (3)

pdp10_pi_return_level1:
        move 3,000043
        jrst 012,@pdp10_pi_level1
pdp10_pi_return_level2:
        move 3,000045
        jrst 012,@pdp10_pi_level2
pdp10_pi_return_level3:
        move 3,000036
        jrst 012,@pdp10_pi_level3
pdp10_pi_return_level4:
        move 3,000051
        jrst 012,@pdp10_pi_level4
pdp10_pi_return_level5:
        move 3,000053
        jrst 012,@pdp10_pi_level5
pdp10_pi_return_level6:
        move 3,000055
        jrst 012,@pdp10_pi_level6
pdp10_pi_return_level7:
        move 3,000057
        jrst 012,@pdp10_pi_level7

        .bss
/** Resident table of compact PI handler entry addresses. */
pdp10_pi_handlers:
        .block 015
/** Seven packed -count,,start dispatch spans, one per PI level. */
pdp10_pi_level_span:
        .block 07
/** AC17 save slots at even offsets; odd offsets are safely reusable globals. */
pdp10_pi_sp_save:
        .block 1                       ; PI1 stack slot, offset 0
        .globl proc_table
proc_table:
        .block 1                       ; unreachable PI offset 1
        .block 1                       ; PI2 stack slot, offset 2
        .globl proc_slots
proc_slots:
        .block 1                       ; unreachable PI offset 3
        .block 1                       ; PI3 stack slot, offset 4
        .globl proc_high_slot
proc_high_slot:
        .block 1                       ; unreachable PI offset 5
        .block 1                       ; PI4 stack slot, offset 6
        .globl proc_current_slot
proc_current_slot:
        .block 1                       ; unreachable PI offset 7
        .block 1                       ; PI5 stack slot, offset 010
        .globl proc_sched_cursor
proc_sched_cursor:
        .block 1                       ; unreachable PI offset 011
        .block 1                       ; PI6 stack slot, offset 012
        .globl proc_sched_kick
proc_sched_kick:
        .block 1                       ; unreachable PI offset 013
        .block 1                       ; PI7 stack slot, offset 014
        .globl proc_sched_deferred_ticks
proc_sched_deferred_ticks:
        .block 1
        .globl proc_runq_head
proc_runq_head:
        .block 1
        .globl proc_rt_owner
proc_rt_owner:
        .block 1

        .text
/**
 * @brief Save global PI state and disable priority interrupts.
 *
 * @return AC1 receives the CONI PI state word.
 * @clobber AC1.
 *
 * CONO PI,0400 clears only the global PI enable.  Individual level-enable
 * bits remain unchanged, allowing mach_pi_restore() to restore just the prior
 * global on/off state after a short kernel critical section.
 */
        .globl mach_pi_disable
        .globl mach_pi_restore
mach_pi_disable:
        coni 0004,1
        cono 0004,000400
        popj 017,

/**
 * @brief Restore the saved global PI enable state.
 *
 * @param AC1 CONI PI state returned by mach_pi_disable().
 * @return Preserves AC1 and all other accumulators.
 *
 * State bit 000200 means PI was globally enabled.  TRNN skips the CONO when
 * that bit is clear, so level-enable state is never rewritten.
 */
mach_pi_restore:
        trnn 1,000200
        cono 0004,000200
        popj 017,
