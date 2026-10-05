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
        .globl  proc_swap_out
        .globl  proc_swap_in
        .globl  proc_sched_cursor
        .globl  proc_high_slot
        .globl  proc_table
        .globl  proc_swap_records
        .globl  proc_swap_in
        .globl  proc_swap_out
        .globl  proc_swap_victim
        .globl  proc_event_apply
        .globl  mm_compact
        .globl  mm_is_pinned
        .globl  mm_pin
        .globl  mm_unpin
        .globl  mm_free
        .globl  mm_alloc_aligned
        .globl  backstore_alloc
        .globl  backstore_free
        .globl  backstore_read
        .globl  backstore_write
        .globl  backstore_blocks
        .globl  proc_swap_blocks_used

        .equ    PROC_WORDS,3
        .equ    PROC_STATE_RUN,2
        .equ    PROC_STATE_SLEEP,3
        .equ    PROC_STATE_STOP,6
        .equ    PROC_SCHED_SWAP_REQUEST,0400
        .equ    PROC_SLOT_MASK,0377
        .equ    PROC_TRANSITION_RH,0200000
        .equ    PROC_UAREA_RH,0400000
        .equ    PROC_USER_MAP_BIT,02
        .equ    PROC_FDCTL_OFFSET,024
        .equ    PROC_SWAP_BACKING_OFFSET,0406
        .equ    MM_TYPE_PROCESS,1
        .equ    MM_ALLOC_HIGH,1
        .equ    VM_PDP6_ALIGN_WORDS,02000
        .equ    DSK_WORDS_PER_SECTOR,0200
        .equ    SYS_EVENT_TERM,1

/**
 * @brief Write one resident process extent to backing store and release VM.
 * @param AC1 Process slot.
 * @return AC1 zero on success, -1 on validation or transaction failure.
 *
 * AC10..AC16 hold the complete transaction: slot, descriptor, swap record,
 * physical base, extent words, block count, and resident executable-backing
 * word.  Only the newly allocated first swap block needs one stack local.
 */
proc_swap_out:
        add     17,kconst_7_7
        movei   0,-6(17)
        hrli    0,10
        blt     0,(17)
        move    10,1
        move    11,1
        lsh     11,1
        add     11,10
        add     11,proc_table

        move    1,2(11)
        lsh     1,-041
        caie    1,PROC_STATE_RUN
        cain    1,PROC_STATE_SLEEP
        jrst    proc_swap_out_state_ok
        caie    1,PROC_STATE_STOP
        jrst    proc_swap_out_fail
proc_swap_out_state_ok:
        move    1,(11)
        trne    1,PROC_TRANSITION_RH
        jrst    proc_swap_out_fail
        trnn    1,PROC_UAREA_RH
        jrst    proc_swap_out_fail
        hlrz    2,1
        move    3,PROC_FDCTL_OFFSET(2)
        trne    3,PROC_USER_MAP_BIT
        jrst    proc_swap_out_fail

        move    12,proc_swap_records
        add     12,10
        skipn   16,(12)
        jrst    proc_swap_out_fail
        hrrz    13,1(11)
        hlrz    14,1(11)
        jumpe   13,proc_swap_out_fail
        jumpe   14,proc_swap_out_fail
        move    1,13
        pushj   17,mm_is_pinned
        jumpn   1,proc_swap_out_fail
        trne    14,DSK_WORDS_PER_SECTOR-1
        jrst    proc_swap_out_fail

        movei   1,PROC_TRANSITION_RH
        iorm    1,(11)
        move    1,13
        pushj   17,mm_pin
        jumpn   1,proc_swap_out_clear

        move    15,14
        lsh     15,-7
        push    17,backstore_blocks
        move    1,15
        setz    2,
        movei   3,(17)
        pushj   17,backstore_alloc
        jumpn   1,proc_swap_out_unpin

        move    1,(17)
        move    2,15
        move    3,13
        pushj   17,backstore_write
        jumpn   1,proc_swap_out_unpin

        hlrz    1,(11)
        movem   16,PROC_SWAP_BACKING_OFFSET(1)
        move    1,13
        pushj   17,mm_unpin
        jumpn   1,proc_swap_out_record_fail
        move    1,13
        movei   2,MM_TYPE_PROCESS
        move    3,10
        pushj   17,mm_free
        jumpn   1,proc_swap_out_record_fail

        setz    1,
        hrrm    1,1(11)
        hrlz    1,(17)
        hrr     1,15
        movem   1,(12)
        add     15,proc_swap_blocks_used
        move    1,(11)
        andcmi  1,PROC_TRANSITION_RH
        movem   1,(11)
        sub     17,kconst_1_1
        setz    1,
        jrst    proc_swap_out_restore

proc_swap_out_unpin:
        move    1,13
        pushj   17,mm_unpin
proc_swap_out_record_fail:
        move    1,(17)
        caml    1,backstore_blocks
        jrst    proc_swap_out_no_free
        move    2,15
        pushj   17,backstore_free
proc_swap_out_no_free:
        hlrz    1,(11)
        setzm   PROC_SWAP_BACKING_OFFSET(1)
        sub     17,kconst_1_1
proc_swap_out_clear:
        move    1,(11)
        andcmi  1,PROC_TRANSITION_RH
        movem   1,(11)
proc_swap_out_fail:
        seto    1,
proc_swap_out_restore:
        movei   0,10
        hrli    0,-6(17)
        blt     0,16
        sub     17,kconst_7_7
        popj    17,

/**
 * @brief Restore one swapped process into a new aligned resident extent.
 * @param AC1 Process slot.
 * @return AC1 zero on success, -1 on validation or transaction failure.
 */
proc_swap_in:
        add     17,kconst_7_7
        movei   0,-6(17)
        hrli    0,10
        blt     0,(17)
        move    10,1
        move    11,1
        lsh     11,1
        add     11,10
        add     11,proc_table
        hrrz    1,1(11)
        jumpn   1,proc_swap_in_fail
        move    1,2(11)
        lsh     1,-041
        jumpe   1,proc_swap_in_fail
        move    1,(11)
        trne    1,PROC_TRANSITION_RH
        jrst    proc_swap_in_fail
        trnn    1,PROC_UAREA_RH
        jrst    proc_swap_in_fail

        move    12,proc_swap_records
        add     12,10
        skipn   1,(12)
        jrst    proc_swap_in_fail
        hlrz    13,1(11)
        jumpe   13,proc_swap_in_fail
        hlrz    2,(11)
        skipn   14,PROC_SWAP_BACKING_OFFSET(2)
        jrst    proc_swap_in_fail
        trne    13,DSK_WORDS_PER_SECTOR-1
        jrst    proc_swap_in_fail
        hlrz    15,(12)
        hrrz    16,(12)
        jumpe   16,proc_swap_in_fail
        move    2,13
        lsh     2,-7
        came    16,2
        jrst    proc_swap_in_fail

        movei   1,PROC_TRANSITION_RH
        iorm    1,(11)
        push    17,[0]
        movei   1,(17)
        push    17,1
        push    17,[MM_ALLOC_HIGH]
        move    1,13
        movei   2,VM_PDP6_ALIGN_WORDS
        movei   3,MM_TYPE_PROCESS
        move    4,10
        pushj   17,mm_alloc_aligned
        sub     17,kconst_2_2
        jumpn   1,proc_swap_in_alloc_fail

        move    1,(17)
        pushj   17,mm_pin
        jumpn   1,proc_swap_in_pin_fail
        move    1,15
        move    2,16
        move    3,(17)
        pushj   17,backstore_read
        jumpn   1,proc_swap_in_read_fail
        move    1,(17)
        pushj   17,mm_unpin
        jumpn   1,proc_swap_in_free_fail

        move    1,(17)
        hrrm    1,1(11)
        movn    1,16
        add     1,proc_swap_blocks_used
        move    1,15
        move    2,16
        pushj   17,backstore_free
        movem   14,(12)
        hlrz    1,(11)
        setzm   PROC_SWAP_BACKING_OFFSET(1)
        move    1,(11)
        andcmi  1,PROC_TRANSITION_RH
        movem   1,(11)
        sub     17,kconst_1_1
        setz    1,
        jrst    proc_swap_in_restore

proc_swap_in_read_fail:
        move    1,(17)
        pushj   17,mm_unpin
proc_swap_in_free_fail:
        move    1,(17)
        movei   2,MM_TYPE_PROCESS
        move    3,10
        pushj   17,mm_free
        jrst    proc_swap_in_drop_local
proc_swap_in_pin_fail:
        move    1,(17)
        movei   2,MM_TYPE_PROCESS
        move    3,10
        pushj   17,mm_free
proc_swap_in_drop_local:
        sub     17,kconst_1_1
proc_swap_in_alloc_fail:
        move    1,(11)
        andcmi  1,PROC_TRANSITION_RH
        movem   1,(11)
proc_swap_in_fail:
        seto    1,
proc_swap_in_restore:
        movei   0,10
        hrli    0,-6(17)
        blt     0,16
        sub     17,kconst_7_7
        popj    17,

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
