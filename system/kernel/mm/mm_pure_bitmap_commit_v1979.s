; K13: shrink the still-owned pinned full APR extent at its original base.
; AC1 base, AC2 slot, AC3 full words, AC4 PURE skip,,encoded words.
; Caller holds PI and lifecycle exclusion, and has verified candidate
; bytes against original RAW. Validate all geometry/owners before stores.
; Success consumes the original sole pin in the descriptor mutation.
; No new owner, free call, allocation or fallible post-commit operation.
; v1985/v1996 optionally place at a disjoint sector-aligned free interval;
; the original descriptor is reordered, not removed and reinserted.
        .text
        .globl mm_pure_bitmap_commit_v1979
        .globl mm_pure_table_v1922
        .globl mm_extents
        .globl mm_extent_count
        .ifdef DAIMOS_PBM_PLACE_V1985
        .globl mm_pure_bitmap_place_v1985
        .globl mm_pure_range_v1922
mm_pure_bitmap_place_v1985:
        move    5,-1(17)               ; exact proposed destination
        jrst    mbc_enter_v1985
        .endif
mm_pure_bitmap_commit_v1979:
        .ifdef DAIMOS_PBM_PLACE_V1985
        move    5,1                    ; original no-move entry
mbc_enter_v1985:
        add     17,[7,,7]
        movei   0,-6(17)
        hrli    0,010
        blt     0,(17)                 ; AC10..16
        move    15,5
        .else
        add     17,[5,,5]
        movei   0,-4(17)
        hrli    0,010
        blt     0,(17)                 ; AC10..14
        .endif
        move    10,1
        move    11,2
        move    12,3
        hrrz    13,4
        hlrz    4,4
        jumple  10,mbc_bad_v1979
        tlne    10,0777777
        jrst    mbc_bad_v1979
        trne    10,01777
        jrst    mbc_bad_v1979
        jumple  11,mbc_bad_v1979
        caile   11,0377
        jrst    mbc_bad_v1979
        jumple  12,mbc_bad_v1979
        tlne    12,0777777
        jrst    mbc_bad_v1979
        trne    12,01777
        jrst    mbc_bad_v1979
        jumpe   4,mbc_bad_v1979
        trne    4,0177
        jrst    mbc_bad_v1979
        caml    4,12
        jrst    mbc_bad_v1979
        jumpe   13,mbc_bad_v1979
        camle   13,4
        jrst    mbc_bad_v1979
        move    5,12
        sub     5,4                    ; logical RAW length
        caml    13,5
        jrst    mbc_bad_v1979
        addi    5,043
        idivi   5,044
        camge   13,5                   ; room for every directory mask
        jrst    mbc_bad_v1979
        pushj   17,mm_pure_table_v1922
        jumpn   1,mbc_bad_v1979
        setz    1,
        seto    14,                    ; exact descriptor word offset
mbc_scan_v1979:
        caml    1,mm_extent_count
        jrst    mbc_done_v1979
        move    2,1
        lsh     2,1
        move    3,mm_extents+1(2)
        hrrz    4,3
        came    4,11
        jrst    mbc_next_v1979
        hlrz    4,3
        andi    4,7
        caie    4,2                    ; independent namespaces
        cain    4,3
        jrst    mbc_next_v1979
        jumpge  14,mbc_bad_v1979       ; second user owner
        caie    4,1
        jrst    mbc_bad_v1979
        hlrz    4,3
        caie    4,011                  ; exactly one pin, PROCESS
        jrst    mbc_busy_v1979
        move    3,mm_extents(2)
        hrrz    4,3
        came    4,10
        jrst    mbc_bad_v1979
        hlrz    4,3
        came    4,12
        jrst    mbc_bad_v1979
        move    14,2
mbc_next_v1979:
        aoja    1,mbc_scan_v1979
mbc_done_v1979:
        jumpl   14,mbc_bad_v1979
        .ifdef DAIMOS_PBM_PLACE_V1985
        ; Destination must be disjoint from the entire original APR
        ; (or exactly the original base). No additional owner is needed.
        came    15,10
        jrst    mbc_place_check_v1985
        jrst    mbc_place_store_v1985
mbc_place_check_v1985:
        jumple  15,mbc_bad_v1979
        trne    15,0177
        jrst    mbc_bad_v1979
        move    1,15
        move    2,13
        pushj   17,mm_pure_range_v1922
        jumpn   1,mbc_bad_v1979
        move    7,15
        add     7,13                   ; exclusive destination end
        move    16,14                  ; eventual new descriptor offset
        movni   6,2                    ; descriptor shift direction
        camg    7,10
        jrst    mbc_place_scan_v1985
        move    4,10
        add     4,12                   ; original full APR end
        camge   15,4
        jrst    mbc_bad_v1979          ; overlap in either direction
        movei   6,2
        move    1,mm_extent_count
        lsh     1,1                    ; exclusive descriptor end
mbc_place_high_v1996:
        move    2,16
        addi    2,2
        caml    2,1
        jrst    mbc_place_copy_v1985
        move    3,mm_extents(2)
        hrrz    4,3
        caml    4,7                    ; remaining extents above target
        jrst    mbc_place_copy_v1985
        hlrz    5,3
        add     5,4
        camle   5,15                   ; preceding extent ends before target
        jrst    mbc_bad_v1979
        move    16,2
        jrst    mbc_place_high_v1996
mbc_place_scan_v1985:
        jumpe   16,mbc_place_copy_v1985
        move    3,mm_extents-2(16)
        hrrz    4,3
        hlrz    5,3
        add     5,4
        camg    5,15                   ; preceding extent ends before target
        jrst    mbc_place_copy_v1985
        camge   4,7                    ; every intervening extent above target
        jrst    mbc_bad_v1979
        subi    16,2
        jrst    mbc_place_scan_v1985
mbc_place_copy_v1985:
        ; All checks complete. Disjoint BLT and directional table shift
        ; are nonfallible in this PI-excluded window. Count unchanged.
        move    4,15
        hrl     4,10
        subi    7,1
        blt     4,(7)
mbc_place_shift_v1985:
        came    14,16
        jrst    mbc_place_shift_one_v1985
        jrst    mbc_place_store_v1985
mbc_place_shift_one_v1985:
        move    1,14
        add     1,6
        move    2,mm_extents(1)
        move    3,mm_extents+1(1)
        movem   2,mm_extents(14)
        movem   3,mm_extents+1(14)
        move    14,1
        jrst    mbc_place_shift_v1985
mbc_place_store_v1985:
        hrlz    2,13
        hrr     2,15
        .else
        hrlz    2,13
        hrr     2,10
        .endif
        move    3,11
        hrli    3,4                    ; PACKED_PROCESS, zero pins
        movem   2,mm_extents(14)
        movem   3,mm_extents+1(14)
        setz    1,
        jrst    mbc_return_v1979
mbc_busy_v1979:
        movni   1,5
        jrst    mbc_return_v1979
mbc_bad_v1979:
        movni   1,4
mbc_return_v1979:
        .ifdef DAIMOS_PBM_PLACE_V1985
        movei   0,-6(17)
        hrli    0,010
        movs    0,0
        blt     0,016
        sub     17,[7,,7]
        .else
        movei   0,-4(17)
        hrli    0,010
        movs    0,0
        blt     0,014
        sub     17,[5,,5]
        .endif
        popj    17,
