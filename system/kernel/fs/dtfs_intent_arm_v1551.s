; dtfs_intent_arm_v1551.s -- raw on-media DTFS write-ahead barrier.
;
; v1572: production DTVOL2 may be mounted RW under the site/operator
; contract that the physical reel is NEVER changed while mounted.
; Every first DATA/DIR write must complete this physical RBN0 marker
; before touching data, regardless of the trusted-operator policy.
;
; int dtfs_intent_arm_v1551(unit, id_hi, id_lo, scratch)
; AC1=physical DTC unit, AC2/AC3=expected nonzero volume ID halves,
; AC4=independent writable 0200-word scratch (NOT caller write buffer).
; Returns AC1=0 ONLY after successful raw RBN0 write and fresh readback.
; Returns -1 on any error; a failed/uncertain header write also poisons
; the already mounted DTC unit through the existing v1522 path.
;
; The caller holds the serialized DTFS provider lease across the entire
; transaction. The caller must have physically admitted DTVOL2/CLEAN,
; and must not issue DATA/DIR writes before this routine returns zero.
; It may not be used to upgrade a dirty, legacy or unrecognized tape.
; There are no permanent BSS words and only four stack saved registers.

        .text
        .globl dtfs_intent_arm_v1551
        .globl dtfs_intent_validate_v1550
        .globl dtfs_dtc_read_jump
        .globl dtfs_dtc_write_jump
        .globl dtfs_mark_unit_suspect_v1522
        .globl bcache_invalidate_all

dtfs_intent_arm_v1551:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                   ; physical unit
        move    011,2                   ; expected high ID
        move    012,3                   ; expected low ID
        move    013,4                   ; independent scratch pointer
        jumpe   011,dtfs_arm_reject_v1551
        jumpe   012,dtfs_arm_reject_v1551
        jumpe   013,dtfs_arm_reject_v1551

        ; Direct physical RBN0 read, never BCACHE and never the pending
        ; filesystem data buffer. Only a pristine DTVOL2/CLEAN record
        ; with BOTH exact already-admitted volume-ID halves may arm.
        ; v1558: MM may return a reused scratch block. A raw Type-551
        ; read with partial/zero DMA but false success must not inherit
        ; a complete older CLEAN header and write DIRTY to wrong media.
        pushj   17,dtfs_arm_clear_scratch_v1551
        move    1,010
        setz    2,                      ; reserved physical block 0
        move    3,013
        pushj   17,dtfs_dtc_read_jump
        jumpn   1,dtfs_arm_reject_v1551
        move    1,013
        pushj   17,dtfs_intent_validate_v1550
        jumpn   1,dtfs_arm_reject_v1551
        move    2,011
        came    2,1(013)
        jrst    dtfs_arm_reject_v1551
        move    2,012
        came    2,2(013)
        jrst    dtfs_arm_reject_v1551

        ; RBN0 is already checked. Only words 4 and 5 change; full 0200
        ; words are written, retaining the physical 72-bit ID and zeros.
        move    2,[0552255225522]
        movem   2,4(013)
        move    1,(013)
        xor     1,1(013)
        xor     1,2(013)
        xor     1,4(013)
        xor     1,[0246134572610]
        setcm   1,1
        movem   1,5(013)

        ; Do NOT call the high-level dtfs_dtc_write veneer here: once
        ; integrated, that veneer will itself invoke the dirty barrier.
        ; A direct raw write avoids recursion but is never a clean cache
        ; commit. Evict all clean BCACHE identities before that raw write;
        ; its normal veneer would have done this for a DATA/DIR block.
        ; Provider serialization forbids races on the same unit.
        pushj   17,bcache_invalidate_all
        move    1,010
        setz    2,
        move    3,013
        pushj   17,dtfs_dtc_write_jump
        jumpn   1,dtfs_arm_uncertain_v1551

        ; Clear scratch BEFORE the independent physical read. A stale
        ; record in RAM must not be mistaken for verified on-media intent.
        pushj   17,dtfs_arm_clear_scratch_v1551
        move    1,010
        setz    2,
        move    3,013
        pushj   17,dtfs_dtc_read_jump
        jumpn   1,dtfs_arm_uncertain_v1551
        move    1,013
        pushj   17,dtfs_intent_validate_v1550
        caie    1,1                     ; exact WRITE_INTENT state
        jrst    dtfs_arm_uncertain_v1551
        move    2,011
        came    2,1(013)
        jrst    dtfs_arm_uncertain_v1551
        move    2,012
        came    2,2(013)
        jrst    dtfs_arm_uncertain_v1551

        setz    1,                      ; ONLY success path
        jrst    dtfs_arm_done_v1551

dtfs_arm_uncertain_v1551:
        ; The raw header write may have changed any prefix of RBN0.
        ; Invalidate mounted ownership but never discard it here; the
        ; existing v1522 WRITE_UNCERTAIN bit refuses future operations.
        move    1,010
        movei   2,0400000
        pushj   17,dtfs_mark_unit_suspect_v1522
dtfs_arm_reject_v1551:
        seto    1,
dtfs_arm_done_v1551:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; One exact 0200-word wipe shared by initial read and final readback.
; AC2/AC3 are scratch, AC13 remains the stable pinned MM buffer base.
dtfs_arm_clear_scratch_v1551:
        move    2,013
        movei   3,0200
dtfs_arm_clear_loop_v1558:
        setzm   (2)
        aoj     2,
        sojg    3,dtfs_arm_clear_loop_v1558
        popj    17,
