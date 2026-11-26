; K11 mask-directory traversal. Staged, not linked to installed MRES.
; AC1 RAW base, AC2 packed base, AC3 RAW count,,encoded count,
; AC4 mode: bit0 decode, bit1 write (else read-only), bit2 reverse.
; v1979 bit4 compare: decode read-only and compare each RAW word.
; All lengths and physical spans are validated by the C frontend.
; AC1 returns 0, 2 (overlap), or -4 (invalid canonical stream).
; No array or permanent scratch; AC10..16 saved. The same schedule
; is used in both passes, and callers must hold exclusive stable spans.
;
; Working ACs: 1 RAW cursor, 2 mask cursor, 3 literal cursor,
; 4 RAW remaining, 5 mask, 6 bit, 7 value; 10 flags (bit3 hazard),
; 11 bit shift (-1 forward/+1 reverse), 12 group remaining,
; 13 RAW base, 14 RAW exclusive end, 15 packed base, 16 split.
; Locals -1(17)=packed exclusive end, (17)=literal stop cursor.
        .text
        .globl pbmd_walk_v1977
pbmd_walk_v1977:
        add     17,[011,,011]
        movei   0,-010(17)
        hrli    0,010
        blt     0,-2(17)
        move    10,4
        move    13,1
        move    15,2
        hlrz    4,3
        hrrz    3,3
        add     3,15
        movem   3,-1(17)               ; packed exclusive end
        move    14,13
        add     14,4
        move    5,4
        addi    5,043
        idivi   5,044                  ; ceil(RAW/36), (RAW+35)%36
        move    16,15
        add     16,5
        move    12,4
        movni   11,1                   ; forward: subtract -1 from cursors
        trne    10,4
        jrst    pbmd_reverse_v1977
        movem   3,(17)                 ; forward literals end here
        move    3,16
        jrst    pbmd_group_bound_v1977
pbmd_reverse_v1977:
        movei   11,1
        movei   1,-1(14)
        movei   2,-1(16)
        subi    3,1
        movem   2,(17)                 ; reverse literals stop before split
        movei   12,1(6)                ; first reverse group may be partial
        jrst    pbmd_group_v1977
pbmd_group_bound_v1977:
        movei   12,044
        caige   4,044
        move    12,4
pbmd_group_v1977:
        setz    5,
        trnn    10,1
        jrst    pbmd_bit_v1977
        move    5,(2)                  ; consume mask before output stores
        sub     2,11
        move    0,5
        lsh     0,(12)                 ; any unused low mask bits?
        jumpn   0,pbmd_invalid_v1977
pbmd_bit_v1977:
        movsi   6,0400000
        jumpl   11,pbmd_word_v1977
        movei   6,1
        movn    7,12
        lsh     6,044(7)               ; last used bit of partial/full group
pbmd_word_v1977:
        trne    10,1
        jrst    pbmd_decode_word_v1977
        move    7,(1)                  ; consume RAW before checking store
        sub     1,11
        jumpe   7,pbmd_advance_v1977
        ior     5,6
        trnn    10,2
        jrst    pbmd_encode_check_v1977
        movem   7,(3)
        jrst    pbmd_literal_next_v1977
pbmd_encode_check_v1977:
        move    0,3
        pushj   17,pbmd_raw_guard_v1977
pbmd_literal_next_v1977:
        sub     3,11
        jrst    pbmd_advance_v1977

pbmd_decode_word_v1977:
        setz    7,
        tdnn    5,6
        jrst    pbmd_decode_store_v1977
        camn    3,(17)                 ; missing literal, no out-of-range read
        jrst    pbmd_invalid_v1977
        move    7,(3)                  ; consume literal BEFORE output store
        sub     3,11
        jumpe   7,pbmd_invalid_v1977    ; canonical literals are nonzero
pbmd_decode_store_v1977:
        trnn    10,2
        jrst    pbmd_decode_check_v1977
        movem   7,(1)
        jrst    pbmd_decode_next_v1977
pbmd_decode_check_v1977:
        trnn    10,020
        jrst    pbmd_decode_overlap_v1979
        came    7,(1)                  ; exact RAW equality, including zero
        jrst    pbmd_invalid_v1977
        jrst    pbmd_decode_next_v1977
pbmd_decode_overlap_v1979:
        ; Unread masks and literals are independent intervals. In reverse
        ; they are [packed base,mask cursor] and [split,literal cursor].
        jumpl   11,pbmd_decode_fwd_v1977
        camge   1,15
        jrst    pbmd_decode_rev_lit_v1977
        camg    1,2
        jrst    pbmd_decode_hazard_v1977
pbmd_decode_rev_lit_v1977:
        camge   1,16
        jrst    pbmd_decode_next_v1977
        camg    1,3
        jrst    pbmd_decode_hazard_v1977
        jrst    pbmd_decode_next_v1977
pbmd_decode_fwd_v1977:
        ; Forward unread intervals are [mask cursor,split) and
        ; [literal cursor,packed end).
        camge   1,2
        jrst    pbmd_decode_fwd_lit_v1977
        camge   1,16
        jrst    pbmd_decode_hazard_v1977
pbmd_decode_fwd_lit_v1977:
        camge   1,3
        jrst    pbmd_decode_next_v1977
        camge   1,-1(17)
        jrst    pbmd_decode_hazard_v1977
        jrst    pbmd_decode_next_v1977
pbmd_decode_hazard_v1977:
        iori    10,010                 ; still check complete canonical shape
pbmd_decode_next_v1977:
        sub     1,11
pbmd_advance_v1977:
        soj     4,
        lsh     6,(11)
        sojg    12,pbmd_word_v1977
        trne    10,1
        jrst    pbmd_next_group_v1977
        ; Encoder writes the mask only AFTER reading the complete group.
        trnn    10,2
        jrst    pbmd_mask_check_v1977
        movem   5,(2)
        jrst    pbmd_mask_next_v1977
pbmd_mask_check_v1977:
        move    0,2
        pushj   17,pbmd_raw_guard_v1977
pbmd_mask_next_v1977:
        sub     2,11
pbmd_next_group_v1977:
        jumpn   4,pbmd_group_bound_v1977
        came    3,(17)                 ; exact literal consumption/production
        jrst    pbmd_invalid_v1977
        setz    1,
        trne    10,010
        movei   1,2
        jrst    pbmd_return_v1977
pbmd_invalid_v1977:
        movni   1,4
pbmd_return_v1977:
        ; Restore AC10..16 from their contiguous saved words.
        movei   0,-010(17)
        hrli    0,010
        movs    0,0
        blt     0,016
        sub     17,[011,,011]
        popj    17,

; AC0 prospective packed store. RAW cursor already advanced, so the
; just-read word is safe. Set hazard only; no store is performed here.
pbmd_raw_guard_v1977:
        jumpl   11,pbmd_raw_fwd_v1977
        camge   0,13
        popj    17,
        camg    0,1
        iori    10,010
        popj    17,
pbmd_raw_fwd_v1977:
        camge   0,1
        popj    17,
        camge   0,14
        iori    10,010
        popj    17,
