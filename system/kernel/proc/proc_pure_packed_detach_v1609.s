; PDP-6 terminal RAW PURE owner release.
; AC1=slot, AC2=process descriptor, AC3=resident executable backing.
; Return AC1=0 only after exact MM owner free and PACKED -> RESIDENT.
; A failure leaves the original PACKED MM/mode/record retryable, except
; an impossible post-MM-free mode mismatch which fails closed (HALT).
; No executable vnode or mounted provider is needed on the EXIT path.
; This leaf is not a generic segmented/paged memory interface.

        .text
        .globl pdp6_pure_detach_v1609
        .globl mm_extents
        .globl mm_extent_count
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
        jumpe   010,ppure_detach_bad_v1609
        jumpe   011,ppure_detach_bad_v1609
        jumpe   012,ppure_detach_bad_v1609
        ; This leaf is now installed only as the PURE terminal MRES
        ; export. Never free RAW MM ownership when mode != PACKED.
        move    1,010
        movei   2,2
        pushj   17,proc_storage_expect_v30
        jumpn   1,ppure_detach_bad_v1609
        move    4,(011)
        trnn    4,PPURE_META_TRANSITION_V1609
        jrst    ppure_detach_bad_v1609
        trnn    4,PPURE_META_UAREA_V1609
        jrst    ppure_detach_bad_v1609
        hlrz    5,4                     ; stable u-area
        jumpe   5,ppure_detach_bad_v1609
        move    6,PROC_SWAP_BACKING_OFFSET(5)
        came    6,012
        jrst    ppure_detach_bad_v1609
        hrrz    6,1(011)                ; VM base must remain absent
        jumpn   6,ppure_detach_bad_v1609
        hlrz    013,1(011)              ; original APR words
        jumpe   013,ppure_detach_bad_v1609
        trne    013,01777               ; full APR alignment
        jrst    ppure_detach_bad_v1609
        move    014,012
        lsh     014,-PROC_SWAP_TEXT_SHIFT
        andi    014,PROC_SWAP_TEXT_MASK
        jumpe   014,ppure_detach_bad_v1609
        addi    014,EXEC_USER_ORIGIN
        andi    014,0777600             ; omitted complete 0200 sectors
        jumpe   014,ppure_detach_bad_v1609
        camge   014,013
        jrst    ppure_detach_span_v1609
        jrst    ppure_detach_bad_v1609
ppure_detach_span_v1609:
        sub     013,014                 ; required writable suffix size
        setz    015,                    ; MM descriptor index
        setz    016,                    ; one matching physical base

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
        jrst    ppure_detach_next_v1609
ppure_detach_owned_v1609:
        jumpn   016,ppure_detach_bad_v1609
        caie    7,PPURE_TYPE_PACKED_V1609
        jrst    ppure_detach_bad_v1609
        hlrz    7,5
        came    7,013
        jrst    ppure_detach_bad_v1609
        move    7,6
        and     7,[PPURE_META_PIN_MASK_V1609]
        jumpn   7,ppure_detach_bad_v1609
        hrrz    016,5
        jumpe   016,ppure_detach_bad_v1609
ppure_detach_next_v1609:
        aoja    015,ppure_detach_scan_v1609

ppure_detach_found_v1609:
        jumpe   016,ppure_detach_bad_v1609
        move    1,016
        movei   2,PPURE_TYPE_PACKED_V1609
        move    3,010
        pushj   17,mm_free
        jumpn   1,ppure_detach_bad_v1609
        move    1,010
        movei   2,2                     ; PACKED
        setz    3,                      ; RESIDENT
        pushj   17,proc_storage_publish_v30
        jumpn   1,proc_swap_storage_invariant_v30
        setz    1,
        jrst    ppure_detach_return_v1609
ppure_detach_bad_v1609:
        seto    1,
ppure_detach_return_v1609:
        movei   0,010
        hrli    0,-7(17)
        blt     0,016
        sub     17,[010,,010]
        popj    17,
