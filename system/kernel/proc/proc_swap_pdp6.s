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
