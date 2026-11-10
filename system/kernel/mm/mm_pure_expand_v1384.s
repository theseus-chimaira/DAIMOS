; Staged, not production linked.
; Claim a free PURE prefix beside an inactive raw suffix, without copying it.
; The claim stays typed PACKED_PROCESS for the whole potentially fallible
; VFS read, and therefore cannot be scheduled. A failed read reverses the
; exact MM claim; successful reads atomically retype to PROCESS.
;
; C ABI: AC1 = suffix base (claim), or original base (release/commit).
;        AC2 = process owner, AC3 = PURE prefix, AC4 = full APR words.
;        AC1 = 0, MM_ERR_INVAL (-4), MM_ERR_BUSY (-5), or
;              MM_ERR_FRAGMENTED (-2) if claim gap is not available.
; Caller holds PROCESS TRANSITION, one pinned extent, and PI-disabled MM
; serialization during each operation (not during the VFS read).
;
; All failure checks happen before descriptor writes. The MM extent array
; stays sorted because both source and destination keep the same high end.
        .text
        .globl mm_pure_claim_prefix_v1384
        .globl mm_pure_release_prefix_v1384
        .globl mm_pure_commit_prefix_v1384
        .globl mm_extents
        .globl mm_extent_count
        .globl mm_arenas
        .globl mm_arena_count

mm_pure_claim_prefix_v1384:
        setz    0,                     ; mode 0: suffix -> full PACKED
        jrst    mpure_enter_v1384
mm_pure_release_prefix_v1384:
        movei   0,1                    ; mode 1: full -> suffix PACKED
        jrst    mpure_enter_v1384
mm_pure_commit_prefix_v1384:
        movei   0,2                    ; mode 2: full PACKED -> PROCESS
mpure_enter_v1384:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        move    014,0                  ; selected operation
        move    5,1                    ; caller's physical extent base
        move    6,2                    ; owner slot
        move    7,3                    ; omitted complete PURE sectors
        move    010,4                  ; full APR process size
        jumpe   5,mpure_bad_v1384
        jumpe   6,mpure_bad_v1384
        caile   6,0377
        jrst    mpure_bad_v1384
        jumpe   7,mpure_bad_v1384
        trne    7,0177
        jrst    mpure_bad_v1384
        jumpe   010,mpure_bad_v1384
        tlne    010,0777777
        jrst    mpure_bad_v1384
        trne    010,01777
        jrst    mpure_bad_v1384
        caml    7,010
        jrst    mpure_bad_v1384
        move    012,010
        sub     012,7                  ; exact retained suffix words
        move    011,5                  ; original APR base
        jumpn   014,mpure_original_v1384
        camge   5,7
        jrst    mpure_bad_v1384
        sub     011,7
        jrst    mpure_bounds_v1384
mpure_original_v1384:
        add     5,7                     ; expected suffix physical base
mpure_bounds_v1384:
        trne    011,01777               ; original APR alignment
        jrst    mpure_bad_v1384
        move    2,011
        add     2,010
        camle   2,[01000000]           ; 18-bit physical address limit
        jrst    mpure_bad_v1384

        ; Scan the existing sorted extent table for the exact base.
        setz    1,
mpure_find_v1384:
        caml    1,mm_extent_count
        jrst    mpure_bad_v1384
        move    013,1
        lsh     013,1
        hrrz    3,mm_extents(013)
        jumpn   014,mpure_find_original_v1384
        camn    3,5                     ; claim: suffix physical base
        jrst    mpure_found_v1384
        aoja    1,mpure_find_v1384
mpure_find_original_v1384:
        camn    3,011                   ; release/commit: full base
        jrst    mpure_found_v1384
        aoja    1,mpure_find_v1384

mpure_found_v1384:
        hlrz    3,mm_extents(013)
        jumpn   014,mpure_check_full_v1384
        came    3,012                   ; claim: exact raw suffix size
        jrst    mpure_bad_v1384
        jrst    mpure_check_meta_v1384
mpure_check_full_v1384:
        came    3,010                   ; full raw reserved span
        jrst    mpure_bad_v1384
mpure_check_meta_v1384:
        move    4,mm_extents+1(013)
        hlrz    3,4
        andi    3,7
        caie    3,4                     ; MM_TYPE_PACKED_PROCESS
        jrst    mpure_bad_v1384
        hrrz    3,4
        came    3,6
        jrst    mpure_bad_v1384
        move    3,4
        and     3,[03770000000]        ; exactly one MM pin
        came    3,[010000000]
        jrst    mpure_busy_v1384

        jumpn   014,mpure_release_or_commit_v1384
        ; Claim only if the preceding descriptor ends at/before the
        ; original aligned base. A live neighbor must never be overwritten.
        jumpe   013,mpure_check_arena_v1384
        move    0,013
        subi    0,2
        hrrz    1,mm_extents(0)
        hlrz    2,mm_extents(0)
        add     1,2
        camle   1,011
        jrst    mpure_fragmented_v1384

mpure_check_arena_v1384:
        ; Both the original base and the full high end must lie in ONE
        ; actual managed MM arena; free gaps outside an arena are not RAM.
        setz    0,
mpure_arena_loop_v1384:
        caml    0,mm_arena_count
        jrst    mpure_fragmented_v1384
        move    1,mm_arenas(0)
        hrrz    2,1                     ; arena low
        caml    011,2
        jrst    mpure_arena_low_v1384
        aoja    0,mpure_arena_loop_v1384
mpure_arena_low_v1384:
        hlrz    3,1                     ; arena length
        add     2,3                     ; arena high (exclusive)
        move    1,011
        add     1,010
        camg    1,2                     ; skip claim if full end > arena high
        jrst    mpure_claim_v1384
        aoja    0,mpure_arena_loop_v1384

mpure_claim_v1384:
        hrlz    2,010                  ; full size,,original base
        hrr     2,011
        movem   2,mm_extents(013)
        jrst    mpure_success_v1384

mpure_release_or_commit_v1384:
        caie    014,2
        jrst    mpure_release_v1384
        andcm   4,[07000000]           ; remove type without losing owner/pin
        ior     4,[01000000]           ; MM_TYPE_PROCESS
        movem   4,mm_extents+1(013)
        jrst    mpure_success_v1384
mpure_release_v1384:
        hrlz    2,012                  ; retained suffix words
        hrr     2,5                     ; suffix = full base + prefix
        movem   2,mm_extents(013)
mpure_success_v1384:
        setz    1,
        jrst    mpure_return_v1384
mpure_fragmented_v1384:
        movni   1,2
        jrst    mpure_return_v1384
mpure_busy_v1384:
        movni   1,5
        jrst    mpure_return_v1384
mpure_bad_v1384:
        movni   1,4
mpure_return_v1384:
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
