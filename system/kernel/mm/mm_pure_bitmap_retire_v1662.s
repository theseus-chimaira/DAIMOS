; v1662 opt-in MM commit: retire one exactly pinned PURE RAW owner.
; This object is NOT linked into fixed KCORE, KINIT or production MRES.
;
; int mm_pure_bitmap_retire_v1662(slot,raw_base,raw_words,
;                                 packed_base,packed_words)
; AC1..AC4 = first four args; fifth is -1(17) on KCC entry.
; The caller holds process TRANSITION and disables PI around this leaf
; and the nonfallible storage-mode update. AC10..AC16 preserved.
;
; Both source and destination have MM_TYPE_PACKED_PROCESS and the same
; slot. Source MUST carry exactly one pin; destination MUST carry zero.
; Validate both descriptors, geometry and uniqueness BEFORE deleting
; source. The original pinned RAW contents remain untouched on failure.
; Success deletes RAW in one MM metadata transaction, retaining only
; the unpinned compressed owner. There is NO fallible post-delete call.
        .text
        .globl  mm_pure_bitmap_retire_v1662
        .globl  mm_extents
        .globl  mm_extent_count
        .globl  mm_extent_capacity

mm_pure_bitmap_retire_v1662:
        move    5,-1(17)                ; fifth KCC argument
        add     17,[010,,010]
        movei   0,-7(17)
        hrli    0,010
        blt     0,-1(17)               ; save AC10..AC16
        move    010,1                   ; process slot
        move    011,2                   ; raw base
        move    012,3                   ; raw words
        move    013,4                   ; encoded base
        move    014,5                   ; encoded words
        jumpe   010,mm_bitmap_bad_v1662
        caile   010,0377
        jrst    mm_bitmap_bad_v1662
        jumpe   011,mm_bitmap_bad_v1662
        jumpe   012,mm_bitmap_bad_v1662
        jumpe   013,mm_bitmap_bad_v1662
        jumpe   014,mm_bitmap_bad_v1662
        tlne    011,0777777            ; 18-bit physical bases
        jrst    mm_bitmap_bad_v1662
        tlne    013,0777777
        jrst    mm_bitmap_bad_v1662
        tlne    012,0777777            ; 18-bit positive lengths
        jrst    mm_bitmap_bad_v1662
        tlne    014,0777777
        jrst    mm_bitmap_bad_v1662
        caml    014,012                 ; encoded must save RAM
        jrst    mm_bitmap_bad_v1662
        move    1,011
        add     1,012
        camle   1,[01000000]           ; last physical word allowed
        jrst    mm_bitmap_bad_v1662
        move    2,013
        add     2,014
        camle   2,[01000000]
        jrst    mm_bitmap_bad_v1662
        camle   1,013                  ; disjoint [raw,raw+words)
        camg    2,011                  ; or [packed,packed+words)
        jrst    mm_bitmap_geom_v1662
        jrst    mm_bitmap_bad_v1662
mm_bitmap_geom_v1662:
        move    1,mm_extent_count
        jumple  1,mm_bitmap_bad_v1662
        caile   1,0210                 ; permanent MM_MAX_EXTENTS=136
        jrst    mm_bitmap_bad_v1662
        camle   1,mm_extent_capacity
        jrst    mm_bitmap_bad_v1662
        setz    015,                    ; descriptor index
        seto    016,                    ; original RAW index (-1 absent)
        setzm   (17)                    ; encoded owner seen

mm_bitmap_scan_v1662:
        caml    015,mm_extent_count
        jrst    mm_bitmap_found_v1662
        move    4,015
        lsh     4,1
        move    5,mm_extents(4)         ; LH words, RH physical base
        move    6,mm_extents+1(4)       ; LH pins/type, RH owner
        hrrz    7,6
        came    7,010
        jrst    mm_bitmap_next_v1662
        hlrz    7,6
        andi    7,7
        caie    7,1                     ; PROCESS and PACKED_PROCESS
        cain    7,4
        jrst    mm_bitmap_owner_v1662
        caie    7,2                     ; MODULE/KERNEL_DYNAMIC other
        cain    7,3                     ; independent ID namespaces
        jrst    mm_bitmap_next_v1662
        jrst    mm_bitmap_bad_v1662     ; reserved/free type using slot

mm_bitmap_owner_v1662:
        caie    7,4
        jrst    mm_bitmap_bad_v1662     ; foreign PROCESS owner
        hrrz    7,5
        camn    7,011
        jrst    mm_bitmap_raw_v1662
        camn    7,013
        jrst    mm_bitmap_packed_v1662
        jrst    mm_bitmap_bad_v1662     ; third PACKED owner
mm_bitmap_raw_v1662:
        jumpge  016,mm_bitmap_bad_v1662
        hlrz    7,5
        came    7,012
        jrst    mm_bitmap_bad_v1662
        hlrz    7,6
        caie    7,014                   ; exactly ONE pin and type4
        jrst    mm_bitmap_busy_v1662
        move    016,015                 ; record index for deletion
        jrst    mm_bitmap_next_v1662
mm_bitmap_packed_v1662:
        skipn   (17)                    ; reject second packed owner
        jrst    mm_bitmap_packed_first_v1662
        jrst    mm_bitmap_bad_v1662
mm_bitmap_packed_first_v1662:
        hlrz    7,5
        came    7,014
        jrst    mm_bitmap_bad_v1662
        hlrz    7,6
        caie    7,04                    ; type4, ZERO pins
        jrst    mm_bitmap_busy_v1662
        movei   7,1
        movem   7,(17)
mm_bitmap_next_v1662:
        aoja    015,mm_bitmap_scan_v1662

mm_bitmap_found_v1662:
        jumpl   016,mm_bitmap_bad_v1662
        skipn   (17)
        jrst    mm_bitmap_bad_v1662
        ; All checks passed. Delete only the source descriptor using
        ; the original mm_delete shift loop; do not unpin separately.
        move    2,mm_extent_count
        subi    2,1
        caml    016,2
        jrst    mm_bitmap_delete_done_v1662
        move    3,016
mm_bitmap_delete_v1662:
        move    4,3
        lsh     4,1
        move    5,mm_extents+2(4)
        move    6,mm_extents+3(4)
        movem   5,mm_extents(4)
        movem   6,mm_extents+1(4)
        aoj     3,
        camge   3,2
        jrst    mm_bitmap_delete_v1662
mm_bitmap_delete_done_v1662:
        sos     mm_extent_count
        setz    1,
        jrst    mm_bitmap_return_v1662
mm_bitmap_bad_v1662:
        movni   1,4                     ; invalid descriptor, no writes
        jrst    mm_bitmap_return_v1662
mm_bitmap_busy_v1662:
        movni   1,5                     ; wrong pins, no writes
mm_bitmap_return_v1662:
        movei   0,010
        hrli    0,-7(17)
        blt     0,016
        sub     17,[010,,010]
        popj    17,
