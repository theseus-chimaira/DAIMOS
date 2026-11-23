; PDP-6 terminal RAW PURE owner release.
; AC1=slot, AC2=process descriptor, AC3=resident executable backing.
; Return AC1=0 only after exact MM owner free and PACKED -> RESIDENT.
; v1900: terminal may inherit one complete provisional PROCESS owner
; after failed FULL rollback, in addition to the authoritative RAW
; suffix. Verify both exact extents before deleting either. Release
; PROCESS first; a failure leaves RAW authoritative and retryable.
; A lost successful source-free reply is resolved only by an exact
; post-free MM scan while PI remains excluded. No new resident words.
; An impossible post-MM-free mode mismatch fails closed (HALT).
; No executable vnode or mounted provider is needed on the EXIT path.
; This leaf is not a generic segmented/paged memory interface.

        .text
        .globl pdp6_pure_detach_v1609
        .globl mm_extents
        .globl mm_extent_count
        .globl mm_extent_capacity
        .globl mm_free
        .globl proc_storage_publish_v30
        .globl proc_swap_storage_invariant_v30
        .equ PPURE_TYPE_PROCESS_V1609,1
        .equ PPURE_TYPE_PACKED_V1609,4
        .equ PPURE_META_TRANSITION_V1609,0200000
        .equ PPURE_META_UAREA_V1609,0400000
        .equ PPURE_META_PIN_MASK_V1609,03770000000

pdp6_pure_detach_v1609:
        add     17,[010,,010]
        movei   0,-7(17)
        hrli    0,010
        blt     0,-1(17)               ; AC10..AC16
        move    010,1                   ; slot
        move    011,2                   ; descriptor
        move    012,3                   ; saved immutable backing
        jumpe   010,ppure_detach_bad_early_v1900
        jumpe   011,ppure_detach_bad_early_v1900
        jumpe   012,ppure_detach_bad_early_v1900
        ; This leaf is now installed only as the PURE terminal MRES
        ; export. Never free RAW MM ownership when mode != PACKED.
        move    1,010
        movei   2,2
        pushj   17,proc_storage_expect_v30
        jumpn   1,ppure_detach_bad_early_v1900
        move    4,(011)
        trnn    4,PPURE_META_TRANSITION_V1609
        jrst    ppure_detach_bad_early_v1900
        trnn    4,PPURE_META_UAREA_V1609
        jrst    ppure_detach_bad_early_v1900
        hlrz    5,4                     ; stable u-area
        jumpe   5,ppure_detach_bad_early_v1900
        move    6,PROC_SWAP_BACKING_OFFSET(5)
        came    6,012
        jrst    ppure_detach_bad_early_v1900
        hrrz    6,1(011)                ; VM base must remain absent
        jumpn   6,ppure_detach_bad_early_v1900
        hlrz    013,1(011)              ; original APR words
        jumpe   013,ppure_detach_bad_early_v1900
        trne    013,01777               ; full APR alignment
        jrst    ppure_detach_bad_early_v1900
        move    014,012
        lsh     014,-PROC_SWAP_TEXT_SHIFT
        andi    014,PROC_SWAP_TEXT_MASK
        jumpe   014,ppure_detach_bad_early_v1900
        addi    014,EXEC_USER_ORIGIN
        andi    014,0777600             ; omitted complete 0200 sectors
        jumpe   014,ppure_detach_bad_early_v1900
        camge   014,013
        jrst    ppure_detach_span_v1609
        jrst    ppure_detach_bad_early_v1900
ppure_detach_span_v1609:
        sub     013,014                 ; required writable suffix size
        ; MM owner proof and BOTH possible deletion calls must use a
        ; single PI-excluded transaction. Nested mm_free preserves PI-off.
        coni    0004,1
        push    17,1                   ; saved PI state on private stack
        cono    0004,000400             ; disable global PI
        ; A corrupt MM count cannot authorize reads beyond the
        ; permanent 136-entry descriptor reservation.
        move    4,mm_extent_count
        jumple  4,ppure_detach_bad_v1609
        caile   4,0210                 ; MM_MAX_EXTENTS, decimal 136
        jrst    ppure_detach_bad_v1609
        camle   4,mm_extent_capacity   ; runtime reserved capacity
        jrst    ppure_detach_bad_v1609
        setz    015,                    ; MM descriptor index
        setz    016,                    ; one matching physical base
        setz    014,                    ; optional full PROCESS target

ppure_detach_scan_v1609:
        caml    015,mm_extent_count
        jrst    ppure_detach_found_v1609
        move    4,015
        lsh     4,1
        move    5,mm_extents(4)         ; LH span, RH physical base
        move    6,mm_extents+1(4)       ; LH pins/type, RH owner
        hrrz    7,6
        came    7,010
        jrst    ppure_detach_next_v1609
        hlrz    7,6
        andi    7,7                     ; MM type
        caie    7,PPURE_TYPE_PROCESS_V1609
        cain    7,PPURE_TYPE_PACKED_V1609
        jrst    ppure_detach_owned_v1609
        ; Types MODULE=2 and KERNEL_DYNAMIC=3 have independent
        ; owner namespaces. Reserved/free types with this slot ID
        ; instead indicate an uncertain process owner: fail closed.
        caie    7,2
        cain    7,3
        jrst    ppure_detach_next_v1609
        jrst    ppure_detach_bad_v1609
ppure_detach_owned_v1609:
        caie    7,PPURE_TYPE_PACKED_V1609
        jrst    ppure_detach_target_v1900
        jumpn   016,ppure_detach_bad_v1609
        hlrz    7,5
        came    7,013
        jrst    ppure_detach_bad_v1609
        move    7,6
        and     7,[PPURE_META_PIN_MASK_V1609]
        jumpn   7,ppure_detach_bad_v1609
        hrrz    016,5
        jumpe   016,ppure_detach_bad_v1609
        jrst    ppure_detach_next_v1609
ppure_detach_target_v1900:
        ; Only one exact full-size process APR may survive a failed
        ; FULL rollback. It is never the runnable VM (VMbase=0).
        jumpn   014,ppure_detach_bad_v1609
        hlrz    7,5
        hlrz    4,1(011)               ; original complete APR length
        came    7,4
        jrst    ppure_detach_bad_v1609
        move    7,6
        and     7,[PPURE_META_PIN_MASK_V1609]
        jumpn   7,ppure_detach_bad_v1609
        hrrz    7,5
        jumpe   7,ppure_detach_bad_v1609
        trne    7,01777                ; exact APR alignment
        jrst    ppure_detach_bad_v1609
        move    014,7
ppure_detach_next_v1609:
        aoja    015,ppure_detach_scan_v1609

ppure_detach_found_v1609:
        jumpe   016,ppure_detach_bad_v1609
        jumpe   014,ppure_detach_free_source_v1900
        ; On an original failed FULL rollback, dispose of scratch
        ; first. A refused/ambiguous target-free reply leaves RAW
        ; intact for the existing PREVM finalizer retry/backoff.
        move    1,014
        movei   2,PPURE_TYPE_PROCESS_V1609
        move    3,010
        pushj   17,mm_free
        jumpn   1,ppure_detach_bad_v1609
ppure_detach_free_source_v1900:
        move    1,016
        movei   2,PPURE_TYPE_PACKED_V1609
        move    3,010
        pushj   17,mm_free
        jumpe   1,ppure_detach_publish_v1900
        ; The native free might have committed with a false error
        ; reply. Do not accept that claim without checking that the
        ; just-validated source base and ALL slot-owned user extents
        ; are truly gone. Unknown owners remain quarantined.
        setz    015,
ppure_detach_postfree_scan_v1900:
        caml    015,mm_extent_count
        jrst    ppure_detach_publish_v1900
        move    4,015
        lsh     4,1
        move    5,mm_extents(4)
        move    6,mm_extents+1(4)
        hrrz    7,5
        came    7,016                  ; original source base still used?
        jrst    ppure_detach_postfree_owner_v1900
        jrst    ppure_detach_bad_v1609
ppure_detach_postfree_owner_v1900:
        hrrz    7,6
        came    7,010
        jrst    ppure_detach_postfree_next_v1900
        hlrz    7,6
        andi    7,7
        caie    7,PPURE_TYPE_PROCESS_V1609
        cain    7,PPURE_TYPE_PACKED_V1609
        jrst    ppure_detach_bad_v1609
        ; Only MODULE and KERNEL_DYNAMIC have an independent owner
        ; namespace. A reserved/free type carrying this process slot
        ; is not proof that the last user extent has disappeared.
        caie    7,2
        cain    7,3
        jrst    ppure_detach_postfree_next_v1900
        jrst    ppure_detach_bad_v1609
ppure_detach_postfree_next_v1900:
        aoja    015,ppure_detach_postfree_scan_v1900
ppure_detach_publish_v1900:
        move    1,010
        movei   2,2                     ; PACKED
        setz    3,                      ; RESIDENT
        pushj   17,proc_storage_publish_v30
        jumpn   1,proc_swap_storage_invariant_v30
        setz    1,
        jrst    ppure_detach_unlock_v1900
ppure_detach_bad_v1609:
        seto    1,
ppure_detach_unlock_v1900:
        push    17,1                   ; original terminal typed result
        move    1,-1(17)               ; original prior global PI state
        trne    1,000200               ; was global PI enabled?
        cono    0004,000200             ; restore only the global gate
        pop     17,1
        sub     17,[1,,1]              ; discard saved PI snapshot
        jrst    ppure_detach_return_v1609
ppure_detach_bad_early_v1900:
        seto    1,
ppure_detach_return_v1609:
        movei   0,010
        hrli    0,-7(17)
        blt     0,016
        sub     17,[010,,010]
        popj    17,
