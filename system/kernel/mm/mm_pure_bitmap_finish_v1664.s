; v1664 test-only atomic MM compressed->APR finish.
; NOT linked into KCORE or any production MRES.
;
; int mm_pure_bitmap_finish_v1664(slot,source,encoded,target,apr_words)
; KCC args AC1..AC4, fifth at -1(17). Caller owns TRANSITION and
; excludes PI around this leaf and subsequent VM/mode publication.
;
; Exactly TWO process-owned extents permitted: once-pinned
; PACKED_PROCESS encoded source, once-pinned aligned PROCESS APR.
; No fallible operations follow descriptor validation: source is
; deleted and destination's exact pin is consumed atomically.
        .text
        .globl mm_pure_bitmap_finish_v1664
        .globl mm_extents
        .globl mm_extent_count
        .globl mm_extent_capacity

mm_pure_bitmap_finish_v1664:
        move    5,-1(17)
        add     17,[010,,010]
        movei   0,-7(17)
        hrli    0,010
        blt     0,-1(17)               ; save AC10..AC16
        move    010,1                   ; slot
        move    011,2                   ; encoded source
        move    012,3                   ; exact encoded words
        move    013,4                   ; full APR base
        move    014,5                   ; full APR size
        jumpe   010,mbf_bad_v1664
        caile   010,0377
        jrst    mbf_bad_v1664
        jumpe   011,mbf_bad_v1664
        jumpe   012,mbf_bad_v1664
        jumpe   013,mbf_bad_v1664
        jumpe   014,mbf_bad_v1664
        tlne    011,0777777
        jrst    mbf_bad_v1664
        tlne    012,0777777
        jrst    mbf_bad_v1664
        tlne    013,0777777
        jrst    mbf_bad_v1664
        tlne    014,0777777
        jrst    mbf_bad_v1664
        trne    013,01777              ; physical APR relocation
        jrst    mbf_bad_v1664
        trne    014,01777              ; full APR aligned size
        jrst    mbf_bad_v1664
        caml    012,014                ; encoded smaller than full
        jrst    mbf_bad_v1664
        move    1,011
        add     1,012
        camle   1,[01000000]
        jrst    mbf_bad_v1664
        move    2,013
        add     2,014
        camle   2,[01000000]
        jrst    mbf_bad_v1664
        camle   1,013
        camg    2,011
        jrst    mbf_geom_v1664
        jrst    mbf_bad_v1664

mbf_geom_v1664:
        move    1,mm_extent_count
        caige   1,2
        jrst    mbf_bad_v1664
        caile   1,0210                 ; MM_MAX_EXTENTS=136
        jrst    mbf_bad_v1664
        camle   1,mm_extent_capacity
        jrst    mbf_bad_v1664
        seto    015,                    ; APR descriptor index
        seto    016,                    ; source descriptor index
        setz    0,                      ; scan index

mbf_scan_v1664:
        caml    0,mm_extent_count
        jrst    mbf_found_v1664
        move    4,0
        lsh     4,1
        move    5,mm_extents(4)
        move    6,mm_extents+1(4)
        hrrz    7,6
        came    7,010
        jrst    mbf_next_v1664
        hlrz    7,6
        andi    7,7
        caie    7,1                    ; PROCESS or PACKED_PROCESS
        cain    7,4
        jrst    mbf_process_v1664
        caie    7,2                    ; independent MODULE namespace
        cain    7,3                    ; independent KERNEL_DYNAMIC
        jrst    mbf_next_v1664
        jrst    mbf_bad_v1664         ; invalid type with this ID

mbf_process_v1664:
        hrrz    3,5
        caie    7,4
        jrst    mbf_target_v1664
        jumpge  016,mbf_bad_v1664     ; duplicate packed owner
        came    3,011
        jrst    mbf_bad_v1664
        hlrz    3,5
        came    3,012
        jrst    mbf_bad_v1664
        hlrz    3,6
        caie    3,014                 ; type4, exactly one pin
        jrst    mbf_busy_v1664
        move    016,0
        jrst    mbf_next_v1664

mbf_target_v1664:
        jumpge  015,mbf_bad_v1664     ; duplicate PROCESS owner
        came    3,013
        jrst    mbf_bad_v1664
        hlrz    3,5
        came    3,014
        jrst    mbf_bad_v1664
        hlrz    3,6
        caie    3,011                 ; type1, exactly one pin
        jrst    mbf_busy_v1664
        move    015,0
mbf_next_v1664:
        aoja    0,mbf_scan_v1664

mbf_found_v1664:
        jumpl   015,mbf_bad_v1664
        jumpl   016,mbf_bad_v1664
        ; From here onward every MM operation is a nonfallible
        ; metadata store; destination APR is already fully decoded.
        move    2,mm_extent_count
        subi    2,1
        caml    016,2
        jrst    mbf_delete_done_v1664
        move    3,016
mbf_delete_v1664:
        move    4,3
        lsh     4,1
        move    5,mm_extents+2(4)
        move    6,mm_extents+3(4)
        movem   5,mm_extents(4)
        movem   6,mm_extents+1(4)
        aoj     3,
        camge   3,2
        jrst    mbf_delete_v1664
mbf_delete_done_v1664:
        sos     mm_extent_count
        camge   016,015              ; source before target in table
        subi    015,1
        move    4,015
        lsh     4,1
        move    5,mm_extents+1(4)
        sub     5,[010000000]         ; consume exactly one verified pin
        movem   5,mm_extents+1(4)
        setz    1,
        jrst    mbf_return_v1664
mbf_bad_v1664:
        movni   1,4
        jrst    mbf_return_v1664
mbf_busy_v1664:
        movni   1,5
mbf_return_v1664:
        movei   0,010
        hrli    0,-7(17)
        blt     0,016
        sub     17,[010,,010]
        popj    17,
