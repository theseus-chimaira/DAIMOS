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
        .globl  proc_swap_mres_service_v481
        .globl  proc_swap_mres_reclaim_v481
        .globl  proc_swap_out
        .globl  proc_swap_exchange_sector_v771
        .globl  proc_swap_exchange_overlap_v772
        .globl  proc_swap_exchange_tails_v773
        .globl  proc_swap_exchange_transfer_v774
        .globl  proc_swap_exchange_checked_v783
        .globl  proc_swap_exchange_stage_v784
        .globl  proc_swap_exchange_pure_v785
        .globl  proc_swap_exchange_records_v786
        .globl  proc_swap_exchange_account_v787
        .globl  proc_swap_exchange_publish_v788
        .globl  proc_swap_exchange_commit_v789
        .globl  proc_swap_exchange_run_v790
        .globl  proc_swap_exchange_pair_v826
        .globl  proc_swap_exchange_lock_v829
        .globl  proc_swap_exchange_abort_v830
        .globl  proc_swap_exchange_build_v831
        .globl  proc_swap_exchange_execute_v832
        ; v840: exported diagnostic branch labels, no additional code/data.
        .globl  proc_swap_service_pressure
        .globl  proc_swap_service_pop_clear
        .globl  proc_swap_service_failed
        .globl  proc_swap_service_cooldown_v833
        .globl  proc_swap_service_candidate_v834
        .globl  proc_current_slot
        .globl  proc_swap_exchange_preflight_v791
        .globl  proc_swap_exchange_live_v794
        .globl  proc_swap_exchange_backing_v797
        .globl  backstore_verify_allocated_v795
        .globl  proc_storage_exchange_modes_v778
        .globl  proc_swap_exchange_mm_v779
        .globl  proc_swap_exchange_prepare_ram_v780
        .globl  proc_swap_exchange_prepare_v781
        .globl  proc_swap_exchange_cancel_v782
        .globl  backstore_extend
        .globl  mm_process_shrink_pinned
        .globl  mm_process_transfer_pinned
        .globl  mm_process_verify_pinned_v792
        .globl  mm_process_shrink_pinned
        .globl  mm_process_grow_pinned
        .globl  proc_swap_in
        .globl  proc_sched_cursor
        .globl  proc_swap_pressure_ticks
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
        .globl  vfs_read_words
        .globl  d6lz36_decode_vfs
        .globl  fs_zero_words
        .globl  fs_block_workspace
        .globl  bcache_workspace_invalidate
        .globl  proc_storage_expect_v30
        .globl  proc_storage_publish_v30

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
        .equ    EXEC_USER_ORIGIN,020
        .equ    EXEC_DXR_F_COMPRESSED,0100000
        .equ    PROC_SWAP_TEXT_MASK,037777
        .equ    PROC_SWAP_TEXT_SHIFT,026
        .equ    PROC_SWAP_MOUNT_SHIFT,022

; int proc_swap_exchange_pair_v826(a_slot,b_slot)
; Side-effect-free initial admission for equal-sized raw-image pairs.
; The caller must still lock both processes, pin A, and revalidate all
; live state before constructing/committing a transaction. No scheduling
; action is taken here. A larger or PURE image is deliberately deferred.
; AC1=resident candidate, AC2=swapped target. Return 0 or -1.
proc_swap_exchange_pair_v826:
        jumpe   1,proc_swap_pair_bad_v826
        jumpe   2,proc_swap_pair_bad_v826
        camn    1,2
        jrst    proc_swap_pair_bad_v826
        caml    1,proc_high_slot
        jrst    proc_swap_pair_bad_v826
        caml    2,proc_high_slot
        jrst    proc_swap_pair_bad_v826
        push    17,010
        push    17,011
        move    010,1
        move    011,2
        ; Never displace the process currently executing on the CPU.
        camn    010,proc_current_slot
        jrst    proc_swap_pair_fail_v826
        lsh     1,1
        add     1,010
        add     1,proc_table
        move    3,011
        lsh     3,1
        add     3,011
        add     3,proc_table
        ; Candidate must be schedulable or safely parked; the target
        ; must be RUN to satisfy the pending scheduler request.
        move    4,2(1)
        lsh     4,-041
        caie    4,2                    ; RUN
        cain    4,3                    ; SLEEP
        jrst    proc_swap_pair_a_state_v827
        caie    4,6                    ; STOP
        jrst    proc_swap_pair_fail_v826
proc_swap_pair_a_state_v827:
        move    4,2(3)
        lsh     4,-041
        caie    4,2                    ; incoming must be RUN
        jrst    proc_swap_pair_fail_v826
        move    4,(1)
        trne    4,PROC_TRANSITION_RH
        jrst    proc_swap_pair_fail_v826
        trnn    4,PROC_UAREA_RH
        jrst    proc_swap_pair_fail_v826
        hlrz    7,(1)
        move    5,PROC_FDCTL_OFFSET(7)
        trne    5,PROC_USER_MAP_BIT
        jrst    proc_swap_pair_fail_v826
        move    4,(3)
        trne    4,PROC_TRANSITION_RH
        jrst    proc_swap_pair_fail_v826
        trnn    4,PROC_UAREA_RH
        jrst    proc_swap_pair_fail_v826
        hrrz    4,1(1)
        jumpe   4,proc_swap_pair_fail_v826
        push    17,1
        push    17,3
        move    1,4
        pushj   17,mm_is_pinned
        move    5,1
        pop     17,3
        pop     17,1
        jumpn   5,proc_swap_pair_fail_v826
        hrrz    4,1(3)
        jumpn   4,proc_swap_pair_fail_v826
        hlrz    4,1(1)
        jumpe   4,proc_swap_pair_fail_v826
        trne    4,01777
        jrst    proc_swap_pair_fail_v826
        hlrz    5,1(3)
        came    4,5
        jrst    proc_swap_pair_fail_v826
        ; Only raw images: each saved disk record must span all words.
        move    6,4
        lsh     6,-7
        move    7,proc_swap_records
        add     7,010
        skipn   1,(7)
        jrst    proc_swap_pair_fail_v826
        push    17,3
        pushj   17,proc_swap_skip_words
        pop     17,3
        jumpn   1,proc_swap_pair_fail_v826
        hlrz    7,(3)
        jumpe   7,proc_swap_pair_fail_v826
        skipn   1,PROC_SWAP_BACKING_OFFSET(7)
        jrst    proc_swap_pair_fail_v826
        pushj   17,proc_swap_skip_words
        jumpn   1,proc_swap_pair_fail_v826
        move    7,proc_swap_records
        add     7,011
        hrrz    5,(7)
        came    5,6
        jrst    proc_swap_pair_fail_v826
        setz    1,
        jrst    proc_swap_pair_done_v826
proc_swap_pair_fail_v826:
        seto    1,
proc_swap_pair_done_v826:
        pop     17,011
        pop     17,010
        popj    17,
proc_swap_pair_bad_v826:
        seto    1,
        popj    17,

; int proc_swap_exchange_lock_v829(a_slot,b_slot)
; Slot-0-only preparation, under scheduler exclusion. On success both
; TRANSITION flags and exactly one A extent pin belong to the caller.
; AC2 returns the shared physical base. On failure no lock/pin remains.
; This helper does NOT submit the destructive v790 transaction.
proc_swap_exchange_lock_v829:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1
        move    011,2
        pushj   17,proc_swap_exchange_pair_v826
        jumpn   1,proc_swap_lock_fail_v829
        move    012,010
        lsh     012,1
        add     012,010
        add     012,proc_table
        move    013,011
        lsh     013,1
        add     013,011
        add     013,proc_table
        movei   1,PROC_TRANSITION_RH
        iorm    1,(012)
        iorm    1,(013)
        hrrz    1,1(012)
        push    17,1                  ; preserve base across MM call
        pushj   17,mm_pin
        jumpn   1,proc_swap_lock_rollback_v829
        move    1,(17)
        move    2,010
        hlrz    3,1(012)
        pushj   17,mm_process_verify_pinned_v792
        jumpe   1,proc_swap_lock_ok_v829
        move    1,(17)
        pushj   17,mm_unpin
        jumpn   1,proc_swap_storage_invariant_v30
proc_swap_lock_rollback_v829:
        move    1,(012)
        andcmi  1,PROC_TRANSITION_RH
        movem   1,(012)
        move    1,(013)
        andcmi  1,PROC_TRANSITION_RH
        movem   1,(013)
        sub     17,[1,,1]
        jrst    proc_swap_lock_fail_v829
proc_swap_lock_ok_v829:
        pop     17,2                  ; return pinned base
        setz    1,
        jrst    proc_swap_lock_done_v829
proc_swap_lock_fail_v829:
        seto    1,
proc_swap_lock_done_v829:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; void proc_swap_exchange_abort_v830(a_slot,b_slot,base)
; Release an acquired v829 pair before ANY destructive transfer.
; The caller must hold both transitions and the unique A MM pin.
; Unexpected ownership errors are fatal, not a reason to unlock.
proc_swap_exchange_abort_v830:
        push    17,010
        push    17,011
        push    17,012
        move    010,1
        move    011,2
        move    012,3
        jumpe   010,proc_swap_storage_invariant_v30
        jumpe   011,proc_swap_storage_invariant_v30
        camn    010,011
        jrst    proc_swap_storage_invariant_v30
        caml    010,proc_high_slot
        jrst    proc_swap_storage_invariant_v30
        caml    011,proc_high_slot
        jrst    proc_swap_storage_invariant_v30
        move    4,010
        lsh     4,1
        add     4,010
        add     4,proc_table
        move    5,011
        lsh     5,1
        add     5,011
        add     5,proc_table
        move    6,(4)
        trnn    6,PROC_TRANSITION_RH
        jrst    proc_swap_storage_invariant_v30
        move    6,(5)
        trnn    6,PROC_TRANSITION_RH
        jrst    proc_swap_storage_invariant_v30
        hrrz    6,1(4)
        came    6,012
        jrst    proc_swap_storage_invariant_v30
        hrrz    6,1(5)
        jumpn   6,proc_swap_storage_invariant_v30
        move    1,012
        move    2,010
        hlrz    3,1(4)
        pushj   17,mm_process_verify_pinned_v792
        jumpn   1,proc_swap_storage_invariant_v30
        move    1,012
        pushj   17,mm_unpin
        jumpn   1,proc_swap_storage_invariant_v30
        move    4,010
        lsh     4,1
        add     4,010
        add     4,proc_table
        move    5,011
        lsh     5,1
        add     5,011
        add     5,proc_table
        movei   6,PROC_TRANSITION_RH
        andcam  6,(4)
        andcam  6,(5)
        setz    1,
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_build_v831(a_slot,b_slot,base,graph)
; Construct the raw, equal-size v790 graph in 52 caller-owned words.
; Must follow successful v829 locking, before any destructive transfer.
; Graph layout: run[2], stage[2], commit[6], prepare[9], transfer[7],
; MM[5], PURE[4], records[8], account[4], publish[5].
; The v790 preflight/live checks remain authoritative.
proc_swap_exchange_build_v831:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1
        move    011,2
        move    012,3
        move    013,4
        jumpe   013,proc_swap_build_fail_v831
        ; Resolve current entries and incoming record.
        move    5,010
        lsh     5,1
        add     5,010
        add     5,proc_table
        move    6,011
        lsh     6,1
        add     6,011
        add     6,proc_table
        move    7,proc_swap_records
        add     7,011
        hlrz    1,(7)                 ; incoming first sector
        hrrz    2,(7)                 ; incoming sector count
        jumpe   2,proc_swap_build_fail_v831
        ; Build only the exact equal-size, zero-PURE subset.
        hlrz    3,1(5)
        hlrz    4,1(6)
        came    3,4
        jrst    proc_swap_build_fail_v831
        move    4,3
        lsh     4,-7
        came    2,4
        jrst    proc_swap_build_fail_v831
        ; Pointer graph. Constants are offsets from the caller base.
        move    4,013
        addi    4,2
        movem   4,(013)
        addi    4,2
        movem   4,1(013)
        move    4,013
        addi    4,012
        movem   4,2(013)
        addi    4,011
        movem   4,3(013)
        move    4,013
        addi    4,012                ; commit MM aliases prepare
        movem   4,4(013)
        addi    4,025                ; PURE at offset 31
        movem   4,5(013)
        addi    4,4
        movem   4,6(013)
        addi    4,010
        movem   4,7(013)
        addi    4,4
        movem   4,010(013)
        setzm   011(013)
        ; Prepare request (offset 10).
        movem   012,012(013)
        movem   010,013(013)
        movem   011,014(013)
        movem   3,015(013)
        movem   3,016(013)
        movem   1,017(013)
        movem   2,020(013)
        movem   2,021(013)
        setzm   022(013)
        ; Transfer request (offset 19).
        movem   1,023(013)
        movem   012,024(013)
        movem   012,025(013)
        movem   2,026(013)
        movem   2,027(013)
        setzm   030(013)
        setzm   031(013)
        ; MM request (offset 26).
        movem   012,032(013)
        movem   010,033(013)
        movem   011,034(013)
        movem   3,035(013)
        movem   3,036(013)
        ; PURE request: saved B backing, destination, B uarea, skip=0.
        hlrz    4,(6)
        move    4,PROC_SWAP_BACKING_OFFSET(4)
        movem   4,037(013)
        movem   012,040(013)
        hlrz    4,(6)
        movem   4,041(013)
        setzm   042(013)
        ; Records request (offset 35).
        movem   5,043(013)
        movem   6,044(013)
        move    4,proc_swap_records
        add     4,010
        movem   4,045(013)
        movem   7,046(013)
        movem   012,047(013)
        movem   1,050(013)
        movem   2,051(013)
        move    4,037(013)
        movem   4,052(013)
        ; Account and publish requests.
        movem   1,053(013)
        movem   2,054(013)
        movem   2,055(013)
        setzm   056(013)
        movem   5,057(013)
        movem   6,060(013)
        movem   010,061(013)
        movem   011,062(013)
        movem   012,063(013)
        setz    1,
        jrst    proc_swap_build_done_v831
proc_swap_build_fail_v831:
        seto    1,
proc_swap_build_done_v831:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_execute_v832(a_slot,b_slot,graph)
; Caller provides 064 octal (52 decimal) writable words for the graph.
; Slot-0 serialized context only. No permanent allocation or scheduler
; dispatch. A failed v790 leaves both locks and the sole pin intact and
; is followed by v830 abort; a successful v790 publishes and unlocks.
proc_swap_exchange_execute_v832:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1
        move    011,2
        move    012,3
        jumpe   012,proc_swap_execute_fail_v832
        move    1,010
        move    2,011
        pushj   17,proc_swap_exchange_lock_v829
        jumpn   1,proc_swap_execute_fail_v832
        move    013,2                   ; locked resident base
        move    1,010
        move    2,011
        move    3,013
        move    4,012
        pushj   17,proc_swap_exchange_build_v831
        jumpn   1,proc_swap_execute_abort_v832
        move    1,012
        pushj   17,proc_swap_exchange_run_v790
        jumpe   1,proc_swap_execute_done_v832
proc_swap_execute_abort_v832:
        move    1,010
        move    2,011
        move    3,013
        pushj   17,proc_swap_exchange_abort_v830
        jumpn   1,proc_swap_storage_invariant_v30
proc_swap_execute_fail_v832:
        seto    1,
proc_swap_execute_done_v832:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_sector_v771(disk_block, a_ram, b_ram)
; Exchange one writable sector: B disk -> scratch, A RAM -> disk,
; scratch -> B RAM. Caller excludes both processes and establishes the
; multi-sector dependency order. This leaf MUST NOT be used directly
; by the scheduler: it does not commit either process state.
;
; The backing gateway uses a caller buffer. Invalidate the shared VFS
; cache tag before using its 0200-word scratch workspace. The scratch
; is not shared across two concurrent calls; swap service is serialized.
proc_swap_exchange_sector_v771:
        push    17,010
        push    17,011
        push    17,012
        move    010,1                  ; disk block
        move    011,2                  ; source A RAM sector
        move    012,3                  ; destination B RAM sector
        pushj   17,bcache_workspace_invalidate
        move    1,010
        movei   2,1
        movei   3,fs_block_workspace
        pushj   17,backstore_read
        jumpn   1,proc_swap_exchange_sector_fail_v771
        move    1,010
        movei   2,1
        move    3,011
        pushj   17,backstore_write
        jumpn   1,proc_swap_exchange_sector_fail_v771
        movei   1,fs_block_workspace
        hrli    1,fs_block_workspace
        move    2,012
        hrr     1,2
        blt     1,0177(2)             ; copy exactly 0200 words
        setz    1,
        jrst    proc_swap_exchange_sector_done_v771
proc_swap_exchange_sector_fail_v771:
        seto    1,
proc_swap_exchange_sector_done_v771:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_overlap_v772(const kword_t *request)
; A six-word request: disk first, A writable first, B writable first,
; overlap block count, A writable sector offset, B writable sector offset.
; Offsets are in sectors from the common resident-image physical base.
; Caller has already saved A's disk-only tail and reserved disk growth,
; and will restore B's disk-only tail after this helper completes.
; No process records or MM ownership are published here.
proc_swap_exchange_overlap_v772:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        move    010,1
        move    011,3(010)           ; number of common sectors
        jumpe   011,proc_swap_overlap_ok_v772
        move    012,4(010)
        move    013,5(010)
        setz    014,                ; ascending index by default
        camg    013,012
        jrst    proc_swap_overlap_loop_v772
        ; Later B destination needs descending copy to preserve unread A.
        move    014,011
        subi    014,1
proc_swap_overlap_reverse_v772:
        move    1,(010)
        add     1,014
        move    2,1(010)
        move    3,014
        lsh     3,7
        add     2,3
        move    3,2(010)
        move    4,014
        lsh     4,7
        add     3,4
        pushj   17,proc_swap_exchange_sector_v771
        jumpn   1,proc_swap_overlap_fail_v772
        sojge   014,proc_swap_overlap_reverse_v772
        jrst    proc_swap_overlap_ok_v772
proc_swap_overlap_loop_v772:
        move    1,(010)
        add     1,014
        move    2,1(010)
        move    3,014
        lsh     3,7
        add     2,3
        move    3,2(010)
        move    4,014
        lsh     4,7
        add     3,4
        pushj   17,proc_swap_exchange_sector_v771
        jumpn   1,proc_swap_overlap_fail_v772
        aoj     014,
        camge   014,011
        jrst    proc_swap_overlap_loop_v772
proc_swap_overlap_ok_v772:
        setz    1,
        jrst    proc_swap_overlap_done_v772
proc_swap_overlap_fail_v772:
        seto    1,
proc_swap_overlap_done_v772:
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_tails_v773(request, phase)
; request: disk first, A writable RAM start, B writable RAM start,
;          A saved block count, B saved block count.
; phase=0: save A-only disk tail BEFORE overlap, phase=1: read B-only
; tail AFTER overlap. Caller must have reserved disk extension already.
; No scratch allocation and no process-state publication.
proc_swap_exchange_tails_v773:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1
        move    011,2
        move    012,3(010)          ; SA
        move    013,4(010)          ; SB
        jumpn   011,proc_swap_tail_in_v773
        ; Save only indices [SB, SA) to the reserved disk extension.
        camg    012,013
        jrst    proc_swap_tail_ok_v773
        move    011,013
proc_swap_tail_out_loop_v773:
        move    1,(010)
        add     1,011
        movei   2,1
        move    3,011
        lsh     3,7
        add     3,1(010)
        pushj   17,backstore_write
        jumpn   1,proc_swap_tail_fail_v773
        aoj     011,
        camge   011,012
        jrst    proc_swap_tail_out_loop_v773
        jrst    proc_swap_tail_ok_v773
proc_swap_tail_in_v773:
        ; Read indices [SA, SB) after all A sectors are preserved.
        camg    013,012
        jrst    proc_swap_tail_ok_v773
        move    011,012
proc_swap_tail_in_loop_v773:
        move    1,(010)
        add     1,011
        movei   2,1
        move    3,011
        lsh     3,7
        add     3,2(010)
        pushj   17,backstore_read
        jumpn   1,proc_swap_tail_fail_v773
        aoj     011,
        camge   011,013
        jrst    proc_swap_tail_in_loop_v773
proc_swap_tail_ok_v773:
        setz    1,
        jrst    proc_swap_tail_done_v773
proc_swap_tail_fail_v773:
        seto    1,
proc_swap_tail_done_v773:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_transfer_v774(request)
; Seven words: disk first, A writable RAM, B writable RAM,
; SA blocks, SB blocks, A RAM sector offset, B RAM sector offset.
; Caller has already reserved any contiguous disk extension and RAM
; growth, pinned both processes and excluded scheduling/compaction.
; Transfer is forward-only: failure after writes is NOT rollback-safe.
; Caller must halt on failure rather than resume either process.
proc_swap_exchange_transfer_v774:
        push    17,010
        push    17,011
        push    17,012
        move    010,1
        move    1,3(010)
        move    2,4(010)
        camg    1,2
        jrst    proc_swap_transfer_overlap_v774
        ; Outgoing A-only sectors must be saved first.
        move    1,010
        setz    2,
        pushj   17,proc_swap_exchange_tails_v773
        jumpn   1,proc_swap_transfer_fail_v774
proc_swap_transfer_overlap_v774:
        ; Build the six-word overlap request on the stack.
        move    011,3(010)
        move    012,4(010)
        camle   011,012
        move    011,012
        push    17,(010)
        push    17,1(010)
        push    17,2(010)
        push    17,011
        push    17,5(010)
        push    17,6(010)
        movei   1,-5(17)
        pushj   17,proc_swap_exchange_overlap_v772
        sub     17,[6,,6]
        jumpn   1,proc_swap_transfer_fail_v774
        move    1,3(010)
        move    2,4(010)
        camge   1,2
        jrst    proc_swap_transfer_in_v774
        setz    1,
        jrst    proc_swap_transfer_done_v774
proc_swap_transfer_in_v774:
        move    1,010
        movei   2,1
        pushj   17,proc_swap_exchange_tails_v773
        jumpn   1,proc_swap_transfer_fail_v774
        setz    1,
        jrst    proc_swap_transfer_done_v774
proc_swap_transfer_fail_v774:
        seto    1,
proc_swap_transfer_done_v774:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_checked_v783(request)
; Check sector counts and both RAM spans before destructive transfer.
; Caller must already have reserved disk/RAM and locked both slots.
; Invalid input returns -1 without I/O. Any failure after entering
; the transfer halts: the original B swap image may be overwritten.
proc_swap_exchange_checked_v783:
        move    2,3(1)                 ; A sectors
        move    3,4(1)                 ; B sectors
        jumple  2,proc_swap_checked_bad_v783
        jumple  3,proc_swap_checked_bad_v783
        skipge  (1)                   ; disk first must be nonnegative
        jrst    proc_swap_checked_bad_v783
        skipge  5(1)                  ; A offset in sectors
        jrst    proc_swap_checked_bad_v783
        skipge  6(1)                  ; B offset in sectors
        jrst    proc_swap_checked_bad_v783
        move    4,1(1)
        trne    4,0177                ; sector-aligned A start
        jrst    proc_swap_checked_bad_v783
        move    4,2(1)
        trne    4,0177                ; sector-aligned B start
        jrst    proc_swap_checked_bad_v783
        ; Reject physical end addresses outside the PDP-6 18-bit range.
        move    4,2
        lsh     4,7
        add     4,1(1)
        camle   4,[01000000]
        jrst    proc_swap_checked_bad_v783
        move    4,3
        lsh     4,7
        add     4,2(1)
        camle   4,[01000000]
        jrst    proc_swap_checked_bad_v783
        pushj   17,proc_swap_exchange_transfer_v774
        jumpn   1,proc_swap_storage_invariant_v30
        popj    17,
proc_swap_checked_bad_v783:
        seto    1,
        popj    17,

; int proc_swap_exchange_stage_v784(const kword_t *pair)
; pair[0] -> nine-word v781 reservation, pair[1] -> seven-word v783
; transfer. Validate matching disk spans and resident writable offsets
; before preparation; the caller still validates ownership, pins, PURE,
; and exclusion. No state is published here. An I/O failure in v783
; halts; only a rejected, untouched request can be canceled.
proc_swap_exchange_stage_v784:
        push    17,010
        push    17,011
        move    010,(1)
        move    011,1(1)
        jumpe   010,proc_swap_stage_bad_v784
        jumpe   011,proc_swap_stage_bad_v784
        move    1,5(010)
        came    1,(011)                ; same disk start
        jrst    proc_swap_stage_bad_v784
        move    1,6(010)
        came    1,4(011)               ; B blocks
        jrst    proc_swap_stage_bad_v784
        move    1,7(010)
        came    1,3(011)               ; A blocks
        jrst    proc_swap_stage_bad_v784
        ; Both writable starts must lie inside the reserved RAM span.
        move    1,1(011)
        caml    1,(010)
        jrst    proc_swap_stage_a_offset_v784
        jrst    proc_swap_stage_bad_v784
proc_swap_stage_a_offset_v784:
        move    2,1
        sub     2,(010)
        ; Do not truncate a misaligned byte/word displacement into a
        ; seemingly valid sector index before resource preparation.
        trne    2,0177
        jrst    proc_swap_stage_bad_v784
        lsh     2,-7
        came    2,5(011)
        jrst    proc_swap_stage_bad_v784
        ; A writable span must fit inside A's original complete image.
        move    3,3(011)              ; A saved sectors
        add     3,5(011)              ; A offset in sectors
        lsh     3,7
        camle   3,3(010)              ; A complete words
        jrst    proc_swap_stage_bad_v784
        move    1,2(011)
        caml    1,(010)
        jrst    proc_swap_stage_b_offset_v784
        jrst    proc_swap_stage_bad_v784
proc_swap_stage_b_offset_v784:
        move    2,1
        sub     2,(010)
        trne    2,0177
        jrst    proc_swap_stage_bad_v784
        lsh     2,-7
        came    2,6(011)
        jrst    proc_swap_stage_bad_v784
        ; Likewise B's restored writable span must fit its image.
        move    3,4(011)              ; B saved sectors
        add     3,6(011)              ; B offset in sectors
        lsh     3,7
        camle   3,4(010)              ; B complete words
        jrst    proc_swap_stage_bad_v784
        move    1,010
        pushj   17,proc_swap_exchange_prepare_v781
        jumpn   1,proc_swap_stage_bad_v784
        move    1,011
        pushj   17,proc_swap_exchange_checked_v783
        jumpe   1,proc_swap_stage_ok_v784
        move    1,010
        pushj   17,proc_swap_exchange_cancel_v782
        jumpn   1,proc_swap_storage_invariant_v30
proc_swap_stage_bad_v784:
        seto    1,
        jrst    proc_swap_stage_done_v784
proc_swap_stage_ok_v784:
        setz    1,
proc_swap_stage_done_v784:
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_pure_v785(const kword_t *request)
; Four words: incoming executable backing, shared RAM base, stable
; incoming u-area, PURE prefix words. Called ONLY after transfer and
; while both process transitions exclude scheduling. On reload failure
; the original incoming swap image is no longer recoverable: halt.
proc_swap_exchange_pure_v785:
        push    17,010
        move    010,1
        move    2,3(010)
        jumpe   2,proc_swap_pure_ok_v785
        trne    2,0177
        jrst    proc_swap_storage_invariant_v30
        move    1,1(010)
        pushj   17,fs_zero_words
        move    1,(010)
        move    2,1(010)
        move    3,2(010)
        pushj   17,proc_swap_reload_pure
        jumpn   1,proc_swap_storage_invariant_v30
proc_swap_pure_ok_v785:
        setz    1,
        pop     17,010
        popj    17,

; int proc_swap_exchange_records_v786(const kword_t *request)
; Eight words: A table entry, B table entry, A swap-record pointer,
; B swap-record pointer, shared RAM base, B first disk block,
; A saved block count, B executable backing state.
; Called only after both processes have TRANSITION set, sector exchange,
; PURE reload and MM ownership commit are complete. Does not change
; packed storage modes: caller publishes both through v778 afterward.
; No callbacks in this leaf: a record update cannot fail partway.
proc_swap_exchange_records_v786:
        push    17,010
        move    010,1
        move    2,(010)                ; outgoing A process entry
        move    3,1(010)               ; incoming B process entry
        move    4,2(010)               ; outgoing A swap record
        move    5,3(010)               ; incoming B swap record
        jumpe   2,proc_swap_records_bad_v786
        jumpe   3,proc_swap_records_bad_v786
        jumpe   4,proc_swap_records_bad_v786
        jumpe   5,proc_swap_records_bad_v786
        move    6,5(010)              ; disk first
        move    7,6(010)              ; outgoing saved sector count
        jumple  7,proc_swap_records_bad_v786
        trne    6,0777777000000      ; disk first must fit 18 bits
        jrst    proc_swap_records_bad_v786
        hrlz    6,6                   ; packed A record (first,,blocks)
        hrr     6,7
        hrrz    7,4(010)              ; resident RAM base
        jumpe   7,proc_swap_records_bad_v786
        ; Check all input before changing any process metadata.
        hrrz    1,1(2)                ; A must be resident
        jumpe   1,proc_swap_records_bad_v786
        hrrz    1,1(3)                ; B must be on disk
        jumpn   1,proc_swap_records_bad_v786
        hllzs   1(2)                  ; A: clear resident base
        hrrm    7,1(3)                ; B: set resident base
        movem   6,(4)                 ; A: now backed by disk
        move    1,7(010)
        movem   1,(5)                 ; B: now backed by executable
        setz    1,
        pop     17,010
        popj    17,
proc_swap_records_bad_v786:
        seto    1,
        pop     17,010
        popj    17,

; int proc_swap_exchange_account_v787(request)
; Four words: disk first, old incoming B block count, new outgoing A
; block count, reservation flag from v781. Run only after record/MM
; commit with both process transitions held. Disk extension from v781
; already belongs to A; release B's excess only when B > A.
; Adjust the process-only accounting delta (A-B) exactly once.
proc_swap_exchange_account_v787:
        push    17,010
        push    17,011
        move    010,1
        move    011,1(010)
        move    2,2(010)
        jumple  011,proc_swap_account_fail_v787
        jumple  2,proc_swap_account_fail_v787
        move    3,2
        sub     3,011
        jumpge  3,proc_swap_account_add_v787
        ; B has more saved blocks: trailing disk blocks are now free.
        move    1,(010)
        add     1,2
        movn    2,3
        push    17,3
        pushj   17,backstore_free
        pop     17,3
proc_swap_account_add_v787:
        addm    3,proc_swap_blocks_used
        setzm   3(010)
        setz    1,
        pop     17,011
        pop     17,010
        popj    17,
proc_swap_account_fail_v787:
        seto    1,
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_publish_v788(request)
; Five words: A entry, B entry, A slot, B slot, shared RAM base.
; Final commit after data transfer, PURE restore, MM ownership,
; records, and accounting. The caller holds both transition flags
; and precisely one physical MM pin. Release the pin first, then
; publish both storage modes, then clear both transition flags.
; Failure during finalization is not rollback-safe: halt.
proc_swap_exchange_publish_v788:
        push    17,010
        move    010,1
        skipn   (010)
        jrst    proc_swap_publish_bad_v788
        skipn   1(010)
        jrst    proc_swap_publish_bad_v788
        move    1,(010)
        move    2,(1)
        trnn    2,PROC_TRANSITION_RH
        jrst    proc_swap_publish_bad_v788
        move    1,1(010)
        move    2,(1)
        trnn    2,PROC_TRANSITION_RH
        jrst    proc_swap_publish_bad_v788
        move    1,4(010)
        pushj   17,mm_unpin
        jumpn   1,proc_swap_storage_invariant_v30
        move    1,2(010)
        move    2,3(010)
        pushj   17,proc_storage_exchange_modes_v778
        jumpn   1,proc_swap_storage_invariant_v30
        move    1,(010)
        move    2,(1)
        andcmi  2,PROC_TRANSITION_RH
        movem   2,(1)
        move    1,1(010)
        move    2,(1)
        andcmi  2,PROC_TRANSITION_RH
        movem   2,(1)
        setz    1,
        pop     17,010
        popj    17,
proc_swap_publish_bad_v788:
        seto    1,
        pop     17,010
        popj    17,

; Commit a prepared, already-transferred exchange. Six pointers:
; MM request, PURE request, record request, accounting request,
; publication request, and an optional result word (not used).
; Caller must have completed v784 and excluded both processes.
; Failure after destructive transfer is fatal; no rollback is possible.
proc_swap_exchange_commit_v789:
        push    17,010
        move    010,1
        move    1,1(010)
        pushj   17,proc_swap_exchange_pure_v785
        jumpn   1,proc_swap_storage_invariant_v30
        move    1,(010)
        pushj   17,proc_swap_exchange_mm_v779
        jumpn   1,proc_swap_storage_invariant_v30
        move    1,2(010)
        pushj   17,proc_swap_exchange_records_v786
        jumpn   1,proc_swap_storage_invariant_v30
        move    1,3(010)
        pushj   17,proc_swap_exchange_account_v787
        jumpn   1,proc_swap_storage_invariant_v30
        move    1,4(010)
        pushj   17,proc_swap_exchange_publish_v788
        jumpn   1,proc_swap_storage_invariant_v30
        setz    1,
        pop     17,010
        popj    17,

; int proc_swap_exchange_run_v790(const kword_t *request)
; Two words: v784 stage-pair pointer and v789 commit pointer.
; Caller holds both TRANSITION locks and the sole A RAM pin. All
; transaction requests must be prevalidated against live ownership.
; A failed stage returns -1 only before destructive transfer; v783
; halts on transfer failure. Commit failures also halt, never unwind.
proc_swap_exchange_run_v790:
        push    17,010
        move    010,1
        move    1,010
        pushj   17,proc_swap_exchange_preflight_v791
        jumpn   1,proc_swap_run_fail_v790
        ; Validate request graph before v794 dereferences nested records.
        move    1,010
        pushj   17,proc_swap_exchange_live_v794
        jumpn   1,proc_swap_run_fail_v790
        move    1,(010)
        pushj   17,proc_swap_exchange_stage_v784
        jumpn   1,proc_swap_run_fail_v790
        move    1,1(010)
        pushj   17,proc_swap_exchange_commit_v789
        jumpn   1,proc_swap_storage_invariant_v30
        setz    1,
        jrst    proc_swap_run_done_v790
proc_swap_run_fail_v790:
        seto    1,
proc_swap_run_done_v790:
        pop     17,010
        popj    17,

; Validate live process storage before any irreversible exchange.
; Input: v790 two-pointer request. Both TRANSITION bits must be set.
; A is resident, B is swapped, and the B disk record must match the
; preparation request. AC10..AC12 preserved across subroutine calls.
proc_swap_exchange_live_v794:
        push    17,010
        push    17,011
        push    17,012
        move    010,(1)              ; stage pair
        move    011,1(1)             ; commit array
        jumpe   010,proc_swap_live_bad_v794
        jumpe   011,proc_swap_live_bad_v794
        skipn   012,2(011)           ; record request
        jrst    proc_swap_live_bad_v794
        skipn   010,(010)           ; preparation request
        jrst    proc_swap_live_bad_v794
        move    4,(012)             ; A entry
        move    5,1(012)            ; B entry
        jumpe   4,proc_swap_live_bad_v794
        jumpe   5,proc_swap_live_bad_v794
        move    6,(4)
        trnn    6,PROC_TRANSITION_RH
        jrst    proc_swap_live_bad_v794
        move    6,(5)
        trnn    6,PROC_TRANSITION_RH
        jrst    proc_swap_live_bad_v794
        hrrz    6,1(4)
        came    6,(010)              ; A resident base
        jrst    proc_swap_live_bad_v794
        hrrz    6,1(5)
        jumpn   6,proc_swap_live_bad_v794
        hlrz    6,1(4)
        came    6,3(010)             ; outgoing RAM words
        jrst    proc_swap_live_bad_v794
        hlrz    6,1(5)
        came    6,4(010)             ; incoming RAM words
        jrst    proc_swap_live_bad_v794
        move    6,3(012)             ; B record address
        jumpe   6,proc_swap_live_bad_v794
        hlrz    7,(6)                ; incoming first disk block
        came    7,5(010)
        jrst    proc_swap_live_bad_v794
        hrrz    7,(6)                ; incoming disk blocks
        came    7,6(010)
        jrst    proc_swap_live_bad_v794
        ; v796: check the actual allocation bitmap before any disk write.
        move    1,5(010)
        move    2,6(010)
        pushj   17,backstore_verify_allocated_v795
        jumpn   1,proc_swap_live_bad_v794
        move    1,1(010)             ; A slot, RESIDENT
        setz    2,
        pushj   17,proc_storage_expect_v30
        jumpn   1,proc_swap_live_bad_v794
        move    1,2(010)             ; B slot, DISK
        movei   2,1
        pushj   17,proc_storage_expect_v30
        jumpn   1,proc_swap_live_bad_v794
        move    1,010
        move    2,011
        move    3,012
        pushj   17,proc_swap_exchange_backing_v797
        jumpn   1,proc_swap_live_bad_v794
        setz    1,
        jrst    proc_swap_live_done_v794
proc_swap_live_bad_v794:
        seto    1,
proc_swap_live_done_v794:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_backing_v797(prepare,commit,records)
; Require B's stable u-area backing to agree with the PURE request,
; and the complete saved-state sector count to match the same PURE
; truncation rule used by proc_swap_in. No storage is modified.
proc_swap_exchange_backing_v797:
        push    17,010
        push    17,011
        push    17,012
        move    010,1                ; nine-word preparation
        move    011,2                ; commit pointer array
        move    012,3                ; paired records
        jumpe   010,proc_swap_backing_bad_v797
        jumpe   011,proc_swap_backing_bad_v797
        jumpe   012,proc_swap_backing_bad_v797
        skipn   7,1(011)             ; PURE request
        jrst    proc_swap_backing_bad_v797
        skipn   5,1(012)             ; B process entry
        jrst    proc_swap_backing_bad_v797
        move    6,(5)
        trnn    6,PROC_UAREA_RH
        jrst    proc_swap_backing_bad_v797
        hlrz    6,6                 ; stable B u-area
        jumpe   6,proc_swap_backing_bad_v797
        move    4,PROC_SWAP_BACKING_OFFSET(6)
        jumpe   4,proc_swap_backing_bad_v797
        came    4,(7)               ; same executable backing
        jrst    proc_swap_backing_bad_v797
        move    4,2(7)              ; PURE owner u-area
        came    4,6
        jrst    proc_swap_backing_bad_v797
        move    4,1(7)              ; target RAM base
        came    4,(010)
        jrst    proc_swap_backing_bad_v797
        move    1,(7)
        pushj   17,proc_swap_skip_words
        move    7,1(011)
        came    1,3(7)              ; exact reconstructible prefix
        jrst    proc_swap_backing_bad_v797
        move    2,4(010)            ; complete B extent
        sub     2,1
        jumple  2,proc_swap_backing_bad_v797
        trne    2,0177
        jrst    proc_swap_backing_bad_v797
        lsh     2,-7
        came    2,6(010)            ; saved B blocks
        jrst    proc_swap_backing_bad_v797
        setz    1,
        jrst    proc_swap_backing_done_v797
proc_swap_backing_bad_v797:
        seto    1,
proc_swap_backing_done_v797:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_preflight_v791(request)
; Verify independent stage/commit requests describe the same exchange.
; The caller still establishes MM ownership, pin and process locks.
; All checks precede preparation, disk writes and record changes.
proc_swap_exchange_preflight_v791:
        push    17,010
        move    010,1
        skipn   1,(010)             ; stage pair
        jrst    proc_swap_preflight_bad_v791
        skipn   2,1(010)            ; commit pointer array
        jrst    proc_swap_preflight_bad_v791
        skipn   3,(1)               ; nine-word prepare request
        jrst    proc_swap_preflight_bad_v791
        skipn   4,1(1)              ; seven-word transfer request
        jrst    proc_swap_preflight_bad_v791
        ; v793: fail before reservation on impossible process identities
        ; or non-sector-sized process images. The stage helper validates
        ; writable offsets but must not be the first size gate.
        move    6,1(3)
        jumpe   6,proc_swap_preflight_bad_v791
        move    7,2(3)
        jumpe   7,proc_swap_preflight_bad_v791
        camn    6,7
        jrst    proc_swap_preflight_bad_v791
        move    6,3(3)
        jumple  6,proc_swap_preflight_bad_v791
        trne    6,0177
        jrst    proc_swap_preflight_bad_v791
        move    7,4(3)
        jumple  7,proc_swap_preflight_bad_v791
        trne    7,0177
        jrst    proc_swap_preflight_bad_v791
        skipg   6,6(3)              ; B has a nonempty disk record
        jrst    proc_swap_preflight_bad_v791
        skipg   7,7(3)              ; A has writable blocks
        jrst    proc_swap_preflight_bad_v791
        skipn   5,(2)               ; five-word MM request
        jrst    proc_swap_preflight_bad_v791
        came    3,5                 ; MM request aliases prepare header
        jrst    proc_swap_preflight_bad_v791
        skipn   6,1(2)              ; four-word PURE request
        jrst    proc_swap_preflight_bad_v791
        skipn   7,2(2)              ; eight-word record request
        jrst    proc_swap_preflight_bad_v791
        skipn   010,3(2)            ; four-word accounting request
        jrst    proc_swap_preflight_fail_v791
        move    1,5(3)              ; disk first
        came    1,5(7)
        jrst    proc_swap_preflight_fail_v791
        came    1,(010)
        jrst    proc_swap_preflight_fail_v791
        move    1,6(3)              ; B saved blocks
        came    1,1(010)
        jrst    proc_swap_preflight_fail_v791
        move    1,7(3)              ; A saved blocks
        came    1,6(7)
        jrst    proc_swap_preflight_fail_v791
        came    1,2(010)
        jrst    proc_swap_preflight_fail_v791
        move    1,(3)               ; shared RAM base
        came    1,4(7)
        jrst    proc_swap_preflight_fail_v791
        came    1,1(6)
        jrst    proc_swap_preflight_fail_v791
        skipn   1,4(2)              ; publication request
        jrst    proc_swap_preflight_fail_v791
        move    2,1(3)              ; A slot
        came    2,2(1)
        jrst    proc_swap_preflight_fail_v791
        move    2,2(3)              ; B slot
        came    2,3(1)
        jrst    proc_swap_preflight_fail_v791
        move    2,(3)               ; physical base
        came    2,4(1)
        jrst    proc_swap_preflight_fail_v791
        ; Verify the live MM descriptor before reserving backing or RAM.
        move    1,(3)
        move    2,1(3)
        move    3,3(3)
        pushj   17,mm_process_verify_pinned_v792
        jumpn   1,proc_swap_preflight_fail_v791
        setz    1,
        pop     17,010
        popj    17,
proc_swap_preflight_fail_v791:
proc_swap_preflight_bad_v791:
        seto    1,
        pop     17,010
        popj    17,

; int proc_swap_exchange_mm_v779(const kword_t *request)
; Five request words: base, old_slot, new_slot, old_words, new_words.
; Commit only. A larger B MUST already have been reserved via v780,
; before copying any incoming words. Retag then optionally shrink.
; After ownership changes, failure halts rather than unlocking either.
proc_swap_exchange_mm_v779:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,(1)
        move    011,1(1)
        move    012,2(1)
        move    013,3(1)
        move    4,4(1)
        push    17,4
proc_swap_mm_transfer_v779:
        move    1,010
        move    2,011
        move    3,012
        pushj   17,mm_process_transfer_pinned
        jumpn   1,proc_swap_mm_fail_v779
        move    4,(17)
        camge   4,013
        jrst    proc_swap_mm_shrink_v779
proc_swap_mm_ok_v779:
        setz    1,
        jrst    proc_swap_mm_done_v779
proc_swap_mm_shrink_v779:
        move    1,010
        move    2,012
        move    3,4
        pushj   17,mm_process_shrink_pinned
        jumpn   1,proc_swap_storage_invariant_v30
        jrst    proc_swap_mm_ok_v779
proc_swap_mm_fail_v779:
        seto    1,
proc_swap_mm_done_v779:
        sub     17,[1,,1]
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_prepare_ram_v780(request)
; Reserve B's entire RAM extent BEFORE any destructive sector transfer.
; The five-word request matches v779. A pinned A remains the MM owner.
; No resize is needed if B fits A's existing extent. On failure no
; process owner changes. Caller is responsible for canceling any grow
; if later *pre-destructive* preparation fails.
proc_swap_exchange_prepare_ram_v780:
        move    2,3(1)               ; A words
        move    3,4(1)               ; B words
        camg    3,2
        jrst    proc_swap_prepare_ram_ok_v780
        move    2,1(1)               ; A owner
        move    1,(1)                ; physical base
        pushj   17,mm_process_grow_pinned
        popj    17,
proc_swap_prepare_ram_ok_v780:
        setz    1,
        popj    17,

; int proc_swap_exchange_prepare_v781(const kword_t *request)
; Nine words: five-word MM request (base, A owner, B owner, A words,
; B words), followed by disk first, existing B blocks, A blocks,
; and reserved flag output (0 on entry, 1 after disk extension).
; Reserve disk extension FIRST, then any required RAM growth. If RAM
; growth fails, release ONLY the newly extended disk tail. B's original
; run remains allocated. No destructive transfer or publication occurs.
proc_swap_exchange_prepare_v781:
        push    17,010
        push    17,011
        push    17,012
        move    010,1
        setzm   010(010)
        move    011,6(010)
        move    012,7(010)
        camg    012,011
        jrst    proc_swap_prepare_ram_v781
        move    1,5(010)
        move    2,011
        move    3,012
        pushj   17,backstore_extend
        jumpn   1,proc_swap_prepare_fail_v781
        movei   1,1
        movem   1,010(010)
proc_swap_prepare_ram_v781:
        move    1,010
        pushj   17,proc_swap_exchange_prepare_ram_v780
        jumpe   1,proc_swap_prepare_ok_v781
        skipn   010(010)
        jrst    proc_swap_prepare_fail_v781
        ; Release [disk_first+B_blocks, disk_first+A_blocks).
        move    1,5(010)
        add     1,011
        move    2,012
        sub     2,011
        pushj   17,backstore_free
        setzm   010(010)
proc_swap_prepare_fail_v781:
        seto    1,
        jrst    proc_swap_prepare_done_v781
proc_swap_prepare_ok_v781:
        setz    1,
proc_swap_prepare_done_v781:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_swap_exchange_cancel_v782(request)
; Cancel completed v781 preparation ONLY before the first data write.
; A remains the single pinned owner. Restore its original MM footprint
; before freeing the optional disk extension. Caller must exclude both
; processes and must not invoke this after destructive transfer begins.
proc_swap_exchange_cancel_v782:
        push    17,010
        move    010,1
        move    2,3(010)              ; A original words
        move    3,4(010)              ; B desired words
        camg    3,2
        jrst    proc_swap_cancel_disk_v782
        move    1,(010)               ; physical base
        move    3,2                  ; restore A size
        move    2,1(010)              ; still A owner
        pushj   17,mm_process_shrink_pinned
        jumpn   1,proc_swap_storage_invariant_v30
proc_swap_cancel_disk_v782:
        skipn   010(010)
        jrst    proc_swap_cancel_ok_v782
        move    1,5(010)
        add     1,6(010)
        move    2,7(010)
        sub     2,6(010)
        pushj   17,backstore_free
        setzm   010(010)
proc_swap_cancel_ok_v782:
        setz    1,
        pop     17,010
        popj    17,

/* Return the complete 0200-word sectors covered by low user words + PURE text. */
proc_swap_skip_words:
        lsh     1,-PROC_SWAP_TEXT_SHIFT
        andi    1,PROC_SWAP_TEXT_MASK
        jumpe   1,proc_swap_skip_return
        addi    1,EXEC_USER_ORIGIN
        andi    1,0777600
proc_swap_skip_return:
        popj    17,

.if PROC_SWAP_VERIFY_PURE
/*
 * Expensive diagnostic assertion for the PURE executable contract.
 *
 * The verifier deliberately spends I/O and temporary backstore rather than
 * permanent RAM: save the complete process image, reconstruct the executable
 * exactly as swap-in would, compare each omitted 0200-word sector through the
 * shared filesystem workspace, then restore the original image.  A mismatch
 * means a supposedly PURE process modified memory the production kernel is
 * entitled to discard; restore the image, release scratch backing, and halt at
 * a named assertion site.  The complete body is absent from normal kernels.
 *
 * Input: AC1 backing state, AC2 base, AC3 full words, AC4 skipped words,
 *        AC5 owning stable u-area base (fifth mount-slot bit).
 * Return AC1 = 0 when verified, -1 when the verifier itself cannot run.
 */
proc_swap_verify_pure:
        add     17,[011,,011]
        movei   0,-010(17)
        hrli    0,010
        blt     0,-02(17)               ; preserve AC10..AC16
        move    10,1                    ; backing state
        move    11,2                    ; process base
        move    12,3                    ; complete extent words
        move    13,4                    ; omitted prefix words
        move    16,5                    ; stable u-area for PURE reload
        jumpe   13,proc_swap_verify_ok
        move    14,12
        lsh     14,-7                   ; complete temporary image blocks
        move    15,13
        lsh     15,-7                   ; blocks whose contents are discarded
        move    1,backstore_blocks
        movem   1,(17)                  ; scratch first block
        move    1,14
        setz    2,
        movei   3,(17)
        pushj   17,backstore_alloc
        jumpn   1,proc_swap_verify_fail
        move    1,(17)
        move    2,14
        move    3,11
        pushj   17,backstore_write
        jumpn   1,proc_swap_verify_free_fail

        move    1,11
        move    2,13
        pushj   17,fs_zero_words
        move    1,10
        move    2,11
        move    3,16
        pushj   17,proc_swap_reload_pure
        jumpn   1,proc_swap_verify_restore_fail

        setz    16,
proc_swap_verify_block:
        caml    16,15
        jrst    proc_swap_verify_restore_ok
        pushj   17,bcache_workspace_invalidate
        move    1,(17)
        add     1,16
        movei   2,1
        movei   3,fs_block_workspace
        pushj   17,backstore_read
        jumpn   1,proc_swap_verify_restore_fail
        move    5,16
        lsh     5,7
        add     5,11
        movei   6,fs_block_workspace
        movei   7,DSK_WORDS_PER_SECTOR
        jumpn   16,proc_swap_verify_words
        addi    5,EXEC_USER_ORIGIN      ; low 0..017 are PDP-6 fast ACs,
        addi    6,EXEC_USER_ORIGIN      ; not reconstructible user text
        subi    7,EXEC_USER_ORIGIN
proc_swap_verify_words:
        move    1,(6)
        came    1,(5)
        jrst    proc_swap_verify_mismatch
        aoj     5,
        aoj     6,
        sojg    7,proc_swap_verify_words
        aoja    16,proc_swap_verify_block

proc_swap_verify_mismatch:
        setom   -1(17)                  ; remember contract violation
        jrst    proc_swap_verify_restore
proc_swap_verify_restore_ok:
        setzm   -1(17)
proc_swap_verify_restore:
        move    1,(17)
        move    2,14
        move    3,11
        pushj   17,backstore_read
        jumpn   1,proc_swap_verify_free_fail
        move    1,(17)
        move    2,14
        pushj   17,backstore_free
        skipn   -1(17)
        jrst    proc_swap_verify_ok
proc_swap_pure_contract_violation:
        halt    .                       ; diagnostic kernel assertion

proc_swap_verify_restore_fail:
        move    1,(17)
        move    2,14
        move    3,11
        pushj   17,backstore_read       ; best-effort original-image restore
proc_swap_verify_free_fail:
        move    1,(17)
        caml    1,backstore_blocks
        jrst    proc_swap_verify_fail
        move    2,14
        pushj   17,backstore_free
proc_swap_verify_fail:
        seto    1,
        jrst    proc_swap_verify_return
proc_swap_verify_ok:
        setz    1,
proc_swap_verify_return:
        movei   0,010
        hrli    0,-010(17)
        blt     0,016
        sub     17,[011,,011]
        popj    17,
.endif

/**
 * @brief Write one resident process extent to backing store and release VM.
 * @param AC1 Process slot.
 * @return AC1 zero on success, +1 on resource pressure, -1 on invalid
 *         state, I/O, or failed transaction cleanup.
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

        move    1,10
        movei   2,0                    ; must still be RESIDENT
        pushj   17,proc_storage_expect_v30
        jumpn   1,proc_swap_out_fail

        movei   1,PROC_TRANSITION_RH
        iorm    1,(11)
        move    1,13
        pushj   17,mm_pin
        jumpn   1,proc_swap_out_clear

        move    1,16
        pushj   17,proc_swap_skip_words
.if PROC_SWAP_VERIFY_PURE
        jumpe   1,proc_swap_out_verified
        move    4,1
        move    1,16
        move    2,13
        move    3,14
        hlrz    5,(11)
        pushj   17,proc_swap_verify_pure
        jumpn   1,proc_swap_out_verify_fail
        move    1,16
        pushj   17,proc_swap_skip_words
proc_swap_out_verified:
.endif
        push    17,1                    ; discarded executable prefix
        move    15,14
        sub     15,1
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
        add     3,-1(17)
        pushj   17,backstore_write
        jumpn   1,proc_swap_out_unpin

        hlrz    1,(11)
        movem   16,PROC_SWAP_BACKING_OFFSET(1)
        move    1,13
        pushj   17,mm_unpin
        ; A failed unpin after a successful write leaves MM ownership
        ; uncertain.  Never unlock a possibly pinned process.
        jumpn   1,proc_swap_storage_invariant_v30
        move    1,13
        movei   2,MM_TYPE_PROCESS
        move    3,10
        pushj   17,mm_free
        jumpn   1,proc_swap_out_record_fail

        hllzs   1(11)                  ; clear VM base, preserve extent LH
        hrlz    1,(17)
        hrr     1,15
        movem   1,(12)
        addm    15,proc_swap_blocks_used
        move    1,10
        movei   2,0                    ; RESIDENT -> DISK
        movei   3,1
        pushj   17,proc_storage_publish_v30
        jumpn   1,proc_swap_storage_invariant_v30
        move    1,(11)
        andcmi  1,PROC_TRANSITION_RH
        movem   1,(11)
        sub     17,kconst_2_2
        setz    1,
        jrst    proc_swap_out_restore

proc_swap_out_unpin:
        move    1,13
        pushj   17,mm_unpin
        ; Rollback cannot safely return to the scheduler if the pin was
        ; not released.  Treat this as an MM invariant violation.
        jumpn   1,proc_swap_storage_invariant_v30
proc_swap_out_record_fail:
        move    1,(17)
        caml    1,backstore_blocks
        jrst    proc_swap_out_no_free
        move    2,15
        pushj   17,backstore_free
proc_swap_out_no_free:
        hlrz    1,(11)
        setzm   PROC_SWAP_BACKING_OFFSET(1)
        sub     17,kconst_2_2
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

.if PROC_SWAP_VERIFY_PURE
proc_swap_out_verify_fail:
        move    1,13
        pushj   17,mm_unpin
        jrst    proc_swap_out_clear
.endif

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
        move    1,14
        pushj   17,proc_swap_skip_words
        move    2,13
        sub     2,1
        lsh     2,-7
        came    16,2
        jrst    proc_swap_in_fail

        move    1,10
        movei   2,1                    ; must still be DISK
        pushj   17,proc_storage_expect_v30
        jumpn   1,proc_swap_in_fail

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
        jumpn   1,proc_swap_in_alloc_status

        move    1,(17)
        pushj   17,mm_pin
        jumpn   1,proc_swap_in_pin_fail
        move    1,14
        pushj   17,proc_swap_skip_words
        move    3,1
        jumpe   3,proc_swap_in_read
        push    17,3                    ; preserve skip across reload
        move    1,-1(17)                ; allocated base
        move    2,(17)                  ; skipped prefix words
        pushj   17,fs_zero_words
        move    1,14
        move    2,-1(17)
        hlrz    3,(11)                 ; owning stable u-area
        pushj   17,proc_swap_reload_pure
        jumpn   1,proc_swap_in_reload_fail
        pop     17,3
        jrst    proc_swap_in_read_offset
proc_swap_in_reload_fail:
        sub     17,kconst_1_1
        jrst    proc_swap_in_read_fail
proc_swap_in_read:
        setz    3,
proc_swap_in_read_offset:
        move    1,15
        move    2,16
        add     3,(17)
        pushj   17,backstore_read
        jumpn   1,proc_swap_in_read_fail
        move    1,(17)
        pushj   17,mm_unpin
        jumpn   1,proc_swap_in_free_fail

        move    1,(17)
        hrrm    1,1(11)
        movn    1,16
        addm    1,proc_swap_blocks_used
        move    1,15
        move    2,16
        pushj   17,backstore_free
        movem   14,(12)
        hlrz    1,(11)
        setzm   PROC_SWAP_BACKING_OFFSET(1)
        move    1,10
        movei   2,1                    ; DISK -> RESIDENT
        movei   3,0
        pushj   17,proc_storage_publish_v30
        jumpn   1,proc_swap_storage_invariant_v30
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
        ; Both allocation failure and post-allocation rollback retain the
        ; one-word result local after discarding the two call arguments.
        ; Drop it before restoring AC10..AC16 and the saved return address.
        sub     17,kconst_1_1
        move    1,(11)
        andcmi  1,PROC_TRANSITION_RH
        movem   1,(11)
        jrst    proc_swap_in_fail
proc_swap_in_alloc_status:
        ; Only allocator resource results (-1..-3) are retryable.
        ; No image has been allocated or published on this path.
        setz    7,
        camge   1,[-3]
        jrst    proc_swap_in_alloc_clear
        movei   7,1
proc_swap_in_alloc_clear:
        sub     17,kconst_1_1
        move    1,(11)
        andcmi  1,PROC_TRANSITION_RH
        movem   1,(11)
        jumpn   7,proc_swap_in_pressure
proc_swap_in_fail:
        seto    1,
        jrst    proc_swap_in_restore
proc_swap_in_pressure:
        movei   1,1
proc_swap_in_restore:
        movei   0,10
        hrli    0,-6(17)
        blt     0,16
        sub     17,kconst_7_7
        popj    17,

/* An impossible late mode mismatch must not release a partially committed
 * process to the scheduler: retain TRANSITION and halt, never fake success. */
        .globl  proc_swap_storage_invariant_v30

/**
 * @brief Service one scheduler-requested swap-in from slot-0 context.
 * @return AC1 selected slot on success, zero if no valid request, -1 when the
 *         requested process cannot be restored and is terminated.
 */
proc_swap_mres_service_v481:
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
        jumpg   1,proc_swap_service_pressure
        jumpn   1,proc_swap_service_failed
        pop     17,1
        movem   1,proc_sched_cursor
        popj    17,

proc_swap_service_pressure:
        ; v834: walk all resident candidates. The reclaim-policy winner
        ; need not be exchange-eligible; one rejected slot must not stall
        ; a feasible equal-sized exchange. Slot 0 is never a victim.
        ; Stack: pending B, candidate A, then 064 graph words.
        push    17,[1]
proc_swap_service_candidate_v834:
        move    1,(17)
        caml    1,proc_high_slot
        jrst    proc_swap_service_no_pair_v834
        move    2,-1(17)
        pushj   17,proc_swap_exchange_pair_v826
        jumpn   1,proc_swap_service_next_v834
        add     17,[064,,064]
        move    1,-064(17)             ; A candidate
        move    2,-065(17)             ; pending B
        movei   3,-063(17)             ; graph words -063..0
        pushj   17,proc_swap_exchange_execute_v832
        sub     17,[064,,064]
        jumpe   1,proc_swap_service_exchanged_v834
proc_swap_service_next_v834:
        aos     (17)
        jrst    proc_swap_service_candidate_v834
proc_swap_service_exchanged_v834:
        pop     17,2                   ; discard candidate
        ; v790 published B resident and released both transitions.
        pop     17,1
        movem   1,proc_sched_cursor
        popj    17,
proc_swap_service_no_pair_v834:
        pop     17,2                   ; discard candidate
proc_swap_service_cooldown_v833:
        ; The swapped image is intact. Defer another restore attempt long
        ; enough for resident jobs to run and free resources. Clock PI6
        ; ages the delay; the scheduler ignores swapped jobs during it.
        movei   1,0100
        movem   1,proc_swap_pressure_ticks
        pop     17,1
        movem   1,proc_sched_cursor
        jrst    kret_zero

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
proc_swap_mres_reclaim_v481:
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
        ; mm_compact returned zero: preserve it as the success result.
        jrst    proc_swap_reclaim_done
proc_swap_reclaim_fail:
        seto    1,
proc_swap_reclaim_done:
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,
