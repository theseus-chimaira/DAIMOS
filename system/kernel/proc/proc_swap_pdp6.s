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
        .globl  vfs_read_words
        .globl  d6lz36_decode_vfs
        .globl  fs_zero_words
        .globl  fs_block_workspace
        .globl  bcache_workspace_invalidate

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
        .equ    PROC_SWAP_PROVIDER_SHIFT,024

/*
 * Reconstruct the canonical executable vnode encoded by the one-word swap
 * backing record.
 *
 * Input:  AC1 = packed resident backing state.
 * Output: AC1 = vnode, AC2 = PURE text words.
 * Clobbers AC3..AC5.
 */
proc_swap_unpack_backing:
        move    5,1
        move    4,1
        lsh     4,-PROC_SWAP_TEXT_SHIFT
        andi    4,PROC_SWAP_TEXT_MASK
        hrrz    3,5                     ; vnode index
        move    2,5
        lsh     2,-PROC_SWAP_PROVIDER_SHIFT
        andi    2,03
        addi    2,4                     ; MEMFS..TSFS provider
        move    1,5
        lsh     1,-PROC_SWAP_MOUNT_SHIFT
        andi    1,03
        addi    1,1                     ; public mount id
        move    5,2
        andi    5,1
        addi    5,1                     ; provider-local executable kind
        lsh     1,6
        add     1,5
        lsh     1,022                   ; VFS kind/mount field
        ior     3,1
        lsh     2,036                   ; VFS provider field
        ior     3,2
        move    1,3
        move    2,4
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

/*
 * Return nonzero while a nonresident PURE process depends on NODE as its
 * executable backing.  Resident processes keep their text in core and do not
 * constrain later vnode mutation.
 */
        .globl  proc_swap_backing_busy
proc_swap_backing_busy:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        hrrz    10,1                    ; packed key: index + mount/provider
        move    11,1
        lsh     11,-036                 ; provider
        subi    11,4
        andi    11,03
        lsh     11,PROC_SWAP_PROVIDER_SHIFT
        ior     10,11
        move    11,1
        lsh     11,-030                 ; public mount id
        andi    11,077
        subi    11,1
        andi    11,03
        lsh     11,PROC_SWAP_MOUNT_SHIFT
        ior     10,11

        movei   11,1
        move    12,proc_high_slot
proc_swap_backing_busy_loop:
        caml    11,12
        jrst    proc_swap_backing_not_busy
        move    13,11
        lsh     13,1
        add     13,11
        add     13,proc_table
        hlrz    1,(13)
        jumpe   1,proc_swap_backing_busy_next
        move    1,PROC_SWAP_BACKING_OFFSET(1)
        move    2,1
        lsh     2,-PROC_SWAP_TEXT_SHIFT
        andi    2,PROC_SWAP_TEXT_MASK
        jumpe   2,proc_swap_backing_busy_next
        and     1,[017,,0777777]
        camn    1,10
        jrst    proc_swap_backing_is_busy
proc_swap_backing_busy_next:
        aoja    11,proc_swap_backing_busy_loop
proc_swap_backing_is_busy:
        movei   1,1
        jrst    proc_swap_backing_busy_return
proc_swap_backing_not_busy:
        setz    1,
proc_swap_backing_busy_return:
        pop     17,13
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,

/* Return nonzero while any swapped PURE process depends on public MOUNT id. */
        .globl  proc_swap_mount_busy
proc_swap_mount_busy:
        push    17,10
        push    17,11
        push    17,12
        move    10,1
        movei   11,1
        move    12,proc_high_slot
proc_swap_mount_busy_loop:
        caml    11,12
        jrst    proc_swap_mount_not_busy
        move    2,11
        lsh     2,1
        add     2,11
        add     2,proc_table
        hlrz    1,(2)
        jumpe   1,proc_swap_mount_busy_next
        move    1,PROC_SWAP_BACKING_OFFSET(1)
        move    3,1
        lsh     3,-PROC_SWAP_TEXT_SHIFT
        andi    3,PROC_SWAP_TEXT_MASK
        jumpe   3,proc_swap_mount_busy_next
        lsh     1,-PROC_SWAP_MOUNT_SHIFT
        andi    1,03
        addi    1,1
        camn    1,10
        jrst    proc_swap_mount_is_busy
proc_swap_mount_busy_next:
        aoja    11,proc_swap_mount_busy_loop
proc_swap_mount_is_busy:
        movei   1,1
        jrst    proc_swap_mount_busy_return
proc_swap_mount_not_busy:
        setz    1,
proc_swap_mount_busy_return:
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,

/*
 * Recreate PURE executable text in a newly allocated process extent.
 *
 * Plain DXR2 needs only the text prefix.  Compressed DXR2 is decoded as a
 * complete initialized image and the subsequently restored swap suffix
 * overwrites all writable state, avoiding a second compressed-text decoder.
 *
 * Input: AC1 = packed backing state, AC2 = physical process base.
 * Return AC1 = 0 on success, -1 on backing/read/decode failure.
 */
proc_swap_reload_pure:
        add     17,[010,,010]
        movei   0,-7(17)
        hrli    0,010
        blt     0,-1(17)                ; preserve AC10..AC16
        move    10,1                    ; packed backing
        move    11,2                    ; physical base
        pushj   17,proc_swap_unpack_backing
        move    12,1                    ; executable vnode
        move    13,2                    ; text words
        jumpe   13,proc_swap_reload_fail

        move    1,12
        movei   2,1                     ; DXR flags/image word
        movei   3,(17)
        movei   4,1
        pushj   17,vfs_read_words
        caie    1,1
        jrst    proc_swap_reload_fail
        move    14,(17)
        trne    14,EXEC_DXR_F_COMPRESSED
        jrst    proc_swap_reload_compressed

        move    1,12
        movei   2,3                     ; DXR2 payload offset
        move    3,11
        addi    3,EXEC_USER_ORIGIN
        move    4,13
        pushj   17,vfs_read_words
        came    1,13
        jrst    proc_swap_reload_fail
        jrst    proc_swap_reload_ok

proc_swap_reload_compressed:
        hlrz    15,14                   ; uncompressed image words
        jumpe   15,proc_swap_reload_fail
        hrlz    4,15
        move    6,11
        addi    6,EXEC_USER_ORIGIN
        hrr     4,6
        move    1,12
        movei   2,3
        setz    3,                      ; trusted immutable backing
        pushj   17,d6lz36_decode_vfs
        jumpn   1,proc_swap_reload_fail
proc_swap_reload_ok:
        setz    1,
        jrst    proc_swap_reload_return
proc_swap_reload_fail:
        seto    1,
proc_swap_reload_return:
        movei   0,010
        hrli    0,-7(17)
        blt     0,016
        sub     17,[010,,010]
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
 * Input: AC1 backing state, AC2 base, AC3 full words, AC4 skipped words.
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

        move    1,16
        pushj   17,proc_swap_skip_words
.if PROC_SWAP_VERIFY_PURE
        jumpe   1,proc_swap_out_verified
        move    4,1
        move    1,16
        move    2,13
        move    3,14
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
        addm    15,proc_swap_blocks_used
        move    1,(11)
        andcmi  1,PROC_TRANSITION_RH
        movem   1,(11)
        sub     17,kconst_2_2
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
