; DTFS v1566: exclusive physical WRITE_INTENT -> CLEAN RBN0 transaction.
;
; AC1 = physical Type-551 unit, AC2/AC3 = exact saved 72-bit volume ID,
; AC4 = independent PINNED 0200-word MM scratch (never a DATA buffer).
; Return 0 only after a complete physical WRITE_INTENT record is read,
; the CLEAN record is written, and a separate raw read verifies CLEAN
; with BOTH exact original ID halves. Return -1 for any failure.
;
; Called solely from the mounted seal path with v1562 quiescence held,
; the complete provider lease held, and the mount already marked
; WRITE_UNCERTAIN *before* the physical write begins. The caller clears
; that fail-closed in-RAM bit only after this operation and MM release
; both succeed and the exact mounted owner is revalidated.
; Never read a cached RBN0 or call the recursive dtfs_dtc_write veneer.

        .text
        .globl dtfs_intent_clean_v1566
        .globl dtfs_intent_validate_v1550
        .globl dtfs_dtc_read_jump
        .globl dtfs_dtc_write_jump
        .globl dtfs_mark_unit_suspect_v1522
        .globl bcache_invalidate_all

dtfs_intent_clean_v1566:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                   ; physical unit
        move    011,2                   ; original high identity
        move    012,3                   ; original low identity
        move    013,4                   ; pinned private MM buffer
        jumpe   011,dtfs_clean_refuse_v1566
        jumpe   012,dtfs_clean_refuse_v1566
        jumpe   013,dtfs_clean_refuse_v1566

        ; This physical read must prove the exact previously DIRTY medium.
        ; A short raw read reporting success must not inherit prior scratch.
        pushj   17,dtfs_clean_wipe_v1566
        move    1,010
        setz    2,                      ; native reserved RBN0
        move    3,013
        pushj   17,dtfs_dtc_read_jump
        jumpn   1,dtfs_clean_uncertain_v1566
        move    1,013
        pushj   17,dtfs_intent_validate_v1550
        caie    1,1                     ; MUST be WRITE_INTENT, not CLEAN
        jrst    dtfs_clean_uncertain_v1566
        move    2,011
        came    2,1(013)
        jrst    dtfs_clean_uncertain_v1566
        move    2,012
        came    2,2(013)
        jrst    dtfs_clean_uncertain_v1566

        ; Preserve the same complete 0200-word physical record. Change
        ; ONLY state and salted seal, retaining both IDs and zero padding.
        setzm   4(013)                  ; exact DTVOL2 CLEAN = zero
        move    1,(013)
        xor     1,1(013)
        xor     1,2(013)
        xor     1,[0246134572610]
        setcm   1,1
        movem   1,5(013)

        ; No cached metadata survives a raw physical RBN0 transition.
        ; This path is reached only with the provider's exclusive lease.
        pushj   17,bcache_invalidate_all
        move    1,010
        setz    2,
        move    3,013
        pushj   17,dtfs_dtc_write_jump
        jumpn   1,dtfs_clean_uncertain_v1566

        ; New physical readback, not a comparison to the old MM buffer.
        pushj   17,dtfs_clean_wipe_v1566
        move    1,010
        setz    2,
        move    3,013
        pushj   17,dtfs_dtc_read_jump
        jumpn   1,dtfs_clean_uncertain_v1566
        move    1,013
        pushj   17,dtfs_intent_validate_v1550
        jumpn   1,dtfs_clean_uncertain_v1566 ; only full CLEAN is valid
        move    2,011
        came    2,1(013)
        jrst    dtfs_clean_uncertain_v1566
        move    2,012
        came    2,2(013)
        jrst    dtfs_clean_uncertain_v1566
        setz    1,
        jrst    dtfs_clean_done_v1566

dtfs_clean_uncertain_v1566:
        ; A changed identity, a wrong DIRTY state, a raw write error, or
        ; an incomplete readback must never be treated as successful
        ; CLEAN. Keep the original VFS mount and poison its physical unit.
        pushj   17,bcache_invalidate_all
        move    1,010
        movei   2,0400000              ; sticky WRITE_UNCERTAIN
        pushj   17,dtfs_mark_unit_suspect_v1522
dtfs_clean_refuse_v1566:
        seto    1,
dtfs_clean_done_v1566:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; Wipe EXACTLY 0200 words before each raw I/O. This detects transfers
; returning success after copying only an RBN0 prefix or no words.
dtfs_clean_wipe_v1566:
        move    2,013
        movei   3,0200
dtfs_clean_wipe_loop_v1566:
        setzm   (2)
        aoj     2,
        sojg    3,dtfs_clean_wipe_loop_v1566
        popj    17,
