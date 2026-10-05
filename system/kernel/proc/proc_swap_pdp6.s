/**
 * @file proc_swap_pdp6.s
 * @brief Compact PDP-6 swap service and reclaim policy leaves.
 *
 * The swap transfer transactions remain in C.  These two orchestration paths
 * need no compiler frame: the pending slot can live on the stack across the
 * swap-in call, while reclaim needs only the three callee-saved request
 * arguments.
 */

        .text
        .globl  proc_swap_service_one
        .globl  proc_swap_reclaim
        .globl  proc_sched_cursor
        .globl  proc_high_slot
        .globl  proc_table
        .globl  proc_swap_records
        .globl  proc_swap_in
        .globl  proc_swap_out
        .globl  proc_swap_victim
        .globl  proc_event_apply
        .globl  mm_compact

        .equ    PROC_WORDS,3
        .equ    PROC_STATE_RUN,2
        .equ    PROC_SCHED_SWAP_REQUEST,0400
        .equ    PROC_SLOT_MASK,0377
        .equ    PROC_TRANSITION_RH,0200000
        .equ    SYS_EVENT_TERM,1

/**
 * @brief Service one scheduler-requested swap-in from slot-0 context.
 * @return AC1 selected slot on success, zero if no valid request, -1 when the
 *         requested process cannot be restored and is terminated.
 */
proc_swap_service_one:
        move    1,proc_sched_cursor
        trnn    1,PROC_SCHED_SWAP_REQUEST
        jrst    kret_zero
        andi    1,PROC_SLOT_MASK
        jumpe   1,proc_swap_service_clear
        caml    1,proc_high_slot
        jrst    proc_swap_service_clear

        push    17,1
        move    2,1
        lsh     2,1
        add     2,1
        add     2,proc_table
        move    3,2(2)
        lsh     3,-041
        caie    3,PROC_STATE_RUN
        jrst    proc_swap_service_pop_clear
        move    3,1(2)
        trne    3,0777777
        jrst    proc_swap_service_pop_clear
        move    3,proc_swap_records
        add     3,(17)
        skipn   (3)
        jrst    proc_swap_service_pop_clear
        move    3,(2)
        trne    3,PROC_TRANSITION_RH
        jrst    proc_swap_service_pop_clear

        move    1,(17)
        pushj   17,proc_swap_in
        jumpn   1,proc_swap_service_failed
        pop     17,1
        movem   1,proc_sched_cursor
        popj    17,

proc_swap_service_failed:
        move    1,(17)
        movem   1,proc_sched_cursor
        movei   2,SYS_EVENT_TERM
        pushj   17,proc_event_apply
        sub     17,kconst_1_1
        jrst    kret_neg1

proc_swap_service_pop_clear:
        pop     17,1
proc_swap_service_clear:
        movem   1,proc_sched_cursor
        jrst    kret_zero

/**
 * @brief Reclaim swapped process VM until MM compaction can satisfy a request.
 * @param AC1 Required free words.
 * @param AC2 Required alignment.
 * @param AC3 Owner excluded from victim selection.
 * @return AC1 zero on success, -1 when no further victim can be reclaimed.
 */
proc_swap_reclaim:
        push    17,10
        push    17,11
        push    17,12
        move    10,1
        move    11,2
        move    12,3
proc_swap_reclaim_loop:
        move    1,12
        pushj   17,proc_swap_victim
        jumpl   1,proc_swap_reclaim_fail
        pushj   17,proc_swap_out
        jumpn   1,proc_swap_reclaim_fail
        move    1,10
        move    2,11
        pushj   17,mm_compact
        jumpn   1,proc_swap_reclaim_loop
        setz    1,
        jrst    proc_swap_reclaim_done
proc_swap_reclaim_fail:
        seto    1,
proc_swap_reclaim_done:
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,
