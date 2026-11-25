; PDP-6 terminal RAW PURE owner release.
; AC1=slot, AC2=process descriptor, AC3=resident executable backing.
; Return AC1=0 only after exact MM owner free and PACKED -> RESIDENT.
; v1920: reconcile uncertain FULL/DROP/PREFIX in the idle service
; BEFORE terminal cleanup. Never discover a speculative scratch owner.
; Validate the sorted MM table, the sole RAW extent and its alignment.
; Escrow the original RAW base across PREVM retries; prove complete
; physical absence after any free reply under PI-off.
; No executable vnode or mounted provider is needed on the EXIT path.

        .text
        .globl pdp6_pure_detach_v1609
        .globl mm_extents
        .globl mm_extent_count
        .globl mm_extent_capacity
        .globl mm_free
        .globl mm_pure_range_v1922
        .globl mm_pure_table_v1922
        .globl proc_storage_publish_v30
        .globl proc_swap_storage_invariant_v30
        .equ PPURE_TYPE_PACKED_V1609,4
        .equ PPURE_META_TRANSITION_V1609,0200000
        .equ PPURE_META_UAREA_V1609,0400000
        .equ PPURE_TERM_WORD_V1920,067
        .equ PPURE_TERM_TAG_V1920,020000
        .equ PPURE_TERM_MASK_V1920,023777

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
        ; A live transaction's escrow is not a terminal retry marker.
        ; Ordinary FINALIZE+PREVM uses the same bit32 for its timer.
        move    6,PPURE_TERM_WORD_V1920(5)
        tlne    6,014000               ; DROP/PREFIX kind bits
        jrst    ppure_detach_bad_early_v1900
        tlne    6,0400000              ; FINALIZE
        jrst    ppure_detach_marker_ok_v1920
        tlne    6,040000               ; nonterminal retry/escrow
        jrst    ppure_detach_bad_early_v1900
ppure_detach_marker_ok_v1920:
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
        ; Keep validation, free and result proof in one PI window.
        coni    0004,1
        push    17,1
        cono    0004,000400
        hlrz    5,(011)
        hlrz    016,PPURE_TERM_WORD_V1920(5)
        move    0,016
        andi    016,03777
        trnn    0,PPURE_TERM_TAG_V1920
        jrst    ppure_detach_discover_v1920
        ; An earlier terminal attempt already established the source.
        lsh     016,7
        move    4,016
        sub     4,014
        jumple  4,ppure_detach_bad_v1609
        trne    4,01777
        jrst    ppure_detach_bad_v1609
        move    4,016
        add     4,013
        camle   4,[01000000]
        jrst    ppure_detach_bad_v1609
        movei   0,1                    ; exact retry, not discovery
        pushj   17,ppure_detach_scan_v1920
        jumpn   1,ppure_detach_bad_v1609
        jumpe   2,ppure_detach_publish_v1900 ; original range is absent
        jrst    ppure_detach_free_v1920
ppure_detach_discover_v1920:
        jumpn   016,ppure_detach_bad_v1609 ; stray identity without tag
        setz    0,
        pushj   17,ppure_detach_scan_v1920
        jumpn   1,ppure_detach_bad_v1609
        jumpe   2,ppure_detach_bad_v1609
        ; Record the sector identity BEFORE the first fallible free.
        hlrz    5,(011)
        move    4,016
        lsh     4,-7
        iori    4,PPURE_TERM_TAG_V1920
        movs    4,4
        iorm    4,PPURE_TERM_WORD_V1920(5)
ppure_detach_free_v1920:
        ; Even a pinned exact owner must have been escrowed. Do not
        ; adopt a replacement on a later PREVM retry after pin refusal.
        caie    2,1
        jrst    ppure_detach_bad_v1609
        move    1,016
        movei   2,PPURE_TYPE_PACKED_V1609
        move    3,010
        pushj   17,mm_free
        ; A return code is not proof of deletion. The same scanner
        ; checks all geometry and absence of the original full range.
        movei   0,2
        pushj   17,ppure_detach_scan_v1920
        jumpn   1,ppure_detach_bad_v1609
        jrst    ppure_detach_publish_v1900

; AC0 mode: 0 discover, 1 exact retry (may be absent), 2 prove absence.
; AC16 holds the exact source; AC2=0 absent, 1 unpinned, 2 pinned.
; AC12 is reused for previous extent end; AC15 is the index.
; No new frame or static state. Caller holds PI disabled.
ppure_detach_scan_v1920:
        push    17,0                   ; scan mode across shared proof
        pushj   17,mm_pure_table_v1922
        pop     17,0
        jumpn   1,ppure_detach_scan_bad_v1920
        jumpe   0,ppure_detach_table_ok_v1922
        push    17,0
        move    1,016
        move    2,013
        pushj   17,mm_pure_range_v1922
        pop     17,0
        jumpn   1,ppure_detach_scan_bad_v1920
ppure_detach_table_ok_v1922:
        setz    015,
        setz    2,                     ; matching owner seen
ppure_detach_scan_loop_v1920:
        caml    015,mm_extent_count
        jrst    ppure_detach_scan_done_v1920
        move    4,015
        lsh     4,1
        move    5,mm_extents(4)
        move    6,mm_extents+1(4)
        hrrz    7,5                     ; base
        hlrz    4,5                     ; span (shared table proof)
        move    012,7
        add     012,4                   ; exclusive physical end
        hrrz    4,6
        came    4,010
        jrst    ppure_detach_foreign_v1920
        hlrz    4,6
        andi    4,7
        caie    4,2                     ; independent MODULE namespace
        cain    4,3                     ; independent KERNEL_DYNAMIC
        jrst    ppure_detach_foreign_v1920
        caie    0,2
        jumpe   2,ppure_detach_single_v1920
        jrst    ppure_detach_scan_bad_v1920 ; extra user owner
ppure_detach_single_v1920:
        caie    4,PPURE_TYPE_PACKED_V1609
        jrst    ppure_detach_scan_bad_v1920
        hlrz    4,5
        came    4,013
        jrst    ppure_detach_scan_bad_v1920
        move    4,7
        sub     4,014                   ; original full APR base
        jumple  4,ppure_detach_scan_bad_v1920
        trne    4,01777
        jrst    ppure_detach_scan_bad_v1920
        jumpe   0,ppure_detach_record_v1920
        came    7,016                   ; never adopt a replacement
        jrst    ppure_detach_scan_bad_v1920
ppure_detach_record_v1920:
        move    016,7
        movei   2,1
        tlne    6,03770                 ; record exact owner, refuse pins
        movei   2,2
        jrst    ppure_detach_next_v1920
ppure_detach_foreign_v1920:
        jumpe   0,ppure_detach_next_v1920
        ; Exact retry/absence: reject any foreign intersection with
        ; the escrowed source, even a claim starting inside it.
        move    4,016
        add     4,013
        caml    7,4
        jrst    ppure_detach_next_v1920
        camle   012,016
        jrst    ppure_detach_scan_bad_v1920
ppure_detach_next_v1920:
        aoja    015,ppure_detach_scan_loop_v1920
ppure_detach_scan_done_v1920:
        setz    1,
        popj    17,
ppure_detach_scan_bad_v1920:
        seto    1,
        popj    17,
ppure_detach_publish_v1900:
        move    1,010
        movei   2,2                     ; PACKED
        setz    3,                      ; RESIDENT
        pushj   17,proc_storage_publish_v30
        jumpn   1,proc_swap_storage_invariant_v30
        hlrz    5,(011)
        movsi   4,PPURE_TERM_MASK_V1920
        andcam  4,PPURE_TERM_WORD_V1920(5)
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
