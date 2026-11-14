; DTFS v1554: physical RBN0 verification for an already-armed volume.
;
; v1572: live production V2 RW executes this physical check before
; each subsequent write. An A/B omission exposed an actual Type-136
; reverse-write overrun, independently fixed at the driver in v1574.
; This check is retained pending separate physical fault/recovery
; acceptance of a smaller fast path; operator trust alone does not
; authorize removing I/O correctness checks.
;
; AC1=physical DTC unit, AC2/AC3=expected 72-bit nonzero volume ID,
; AC4=independent PINNED 0200-word scratch owned by the caller.
; Return 0 ONLY when a fresh physical RBN0 read validates a complete
; DTVOL2/WRITE_INTENT record matching both saved ID halves.
; On any I/O/validation/identity failure return -1, invalidate BCACHE
; and mark the mounted unit sticky WRITE_UNCERTAIN. Never rewrite RBN0.
;
; The caller holds the serialized DTFS provider lease from verification
; through the corresponding DATA/DIR write, preventing other filesystem
; actors from changing this unit via the provider while it is checked.
; The v1572 operator-trust contract prohibits physical reel exchange
; while the mount is writable. There is no hardware-enforced lock.

        .text
        .globl dtfs_intent_verify_v1554
        .globl dtfs_intent_validate_v1550
        .globl dtfs_dtc_read_jump
        .globl dtfs_mark_unit_suspect_v1522
        .globl bcache_invalidate_all

dtfs_intent_verify_v1554:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                   ; physical Type-551 unit
        move    011,2                   ; admitted high ID
        move    012,3                   ; admitted low ID
        move    013,4                   ; independently pinned scratch
        jumpe   011,dtfs_verify_refuse_v1554
        jumpe   012,dtfs_verify_refuse_v1554
        jumpe   013,dtfs_verify_refuse_v1554

        ; Zero all words before raw I/O to avoid false successes from a
        ; stale former DIRTY header if the raw transfer was incomplete.
        move    2,013
        movei   3,0200
dtfs_verify_clear_v1554:
        setzm   (2)
        aoj     2,
        sojg    3,dtfs_verify_clear_v1554

        move    1,010
        setz    2,                      ; reserved physical RBN0
        move    3,013
        pushj   17,dtfs_dtc_read_jump  ; never use BCACHE identity
        jumpn   1,dtfs_verify_suspect_v1554
        move    1,013
        pushj   17,dtfs_intent_validate_v1550
        caie    1,1                     ; exact WRITE_INTENT, NOT CLEAN
        jrst    dtfs_verify_suspect_v1554
        move    2,011
        came    2,1(013)
        jrst    dtfs_verify_suspect_v1554
        move    2,012
        came    2,2(013)
        jrst    dtfs_verify_suspect_v1554
        setz    1,                      ; only success path
        jrst    dtfs_verify_done_v1554

dtfs_verify_suspect_v1554:
        ; A missing or changed DIRTY record after prior successful arming
        ; is not a recoverable identity-only READ_SUSPECT. Do not let an
        ; existing VFS identity-revalidation path clear this condition.
        pushj   17,bcache_invalidate_all
        move    1,010
        movei   2,0400000              ; sticky WRITE_UNCERTAIN
        pushj   17,dtfs_mark_unit_suspect_v1522
dtfs_verify_refuse_v1554:
        seto    1,
dtfs_verify_done_v1554:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
