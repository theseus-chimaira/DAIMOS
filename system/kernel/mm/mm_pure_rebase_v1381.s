; Staged, nonresident MM primitive. NOT linked to production KCORE.
; Convert a pinned APR process extent to the owner-identical raw suffix.
; No data is copied: preserving the original high-address suffix already
; preserves every mutable word. Only the now-reconstructible PURE prefix
; is released. This eliminates all scratch RAM during the drop.
;
; int mm_pure_rebase_pinned_v1381(base, owner, prefix, original_words)
; AC1..AC4 arguments, AC1 return; AC5..AC7 scratch; AC10 preserved.
; REQUIRES: PI disabled by caller and PROCESS lifecycle TRANSITION held.
; No fallible step follows the descriptor publication within this routine.
; On failure neither the descriptor nor any physical process word changes.
;
; The existing PROCESS alignment is 02000 octal words. The omitted PURE
; prefix is a nonempty multiple of 0200 octal sector words and is strictly
; smaller than the process. Suffix extents need no APR alignment because
; they cannot be scheduled as-is.
        .text
        .globl mm_pure_rebase_pinned_v1381
        .globl mm_extents
        .globl mm_extent_count

        .equ MM_PROCESS_V1381,1
        .equ MM_PACKED_V1381,4
        .equ MM_INVALID_V1381,-4
        .equ MM_BUSY_V1381,-5

mm_pure_rebase_pinned_v1381:
        push    17,010
        move    5,1                     ; current APR base
        move    6,2                     ; slot owner
        move    7,3                     ; omitted PURE words
        move    010,4                   ; expected full APR size
        jumpe   5,mm_pure_rebase_bad_v1381
        jumpe   6,mm_pure_rebase_bad_v1381
        caile   6,0377
        jrst    mm_pure_rebase_bad_v1381
        jumpe   7,mm_pure_rebase_bad_v1381
        trne    7,0177                 ; require exact 0200 sector multiple
        jrst    mm_pure_rebase_bad_v1381
        trne    5,01777                ; original APR relocation alignment
        jrst    mm_pure_rebase_bad_v1381
        jumpe   010,mm_pure_rebase_bad_v1381
        tlne    010,0777777            ; expected size fits 18 bits
        jrst    mm_pure_rebase_bad_v1381
        trne    010,01777
        jrst    mm_pure_rebase_bad_v1381
        caml    7,010                  ; prefix must be smaller than full
        jrst    mm_pure_rebase_bad_v1381
        move    4,5
        add     4,010
        camle   4,[01000000]           ; exclusive 18-bit physical limit
        jrst    mm_pure_rebase_bad_v1381

        setz    1,
mm_pure_rebase_scan_v1381:
        caml    1,mm_extent_count
        jrst    mm_pure_rebase_bad_v1381
        move    3,1
        lsh     3,1                     ; two MM words per extent
        hrrz    4,mm_extents(3)
        camn    4,5
        jrst    mm_pure_rebase_found_v1381
        aoja    1,mm_pure_rebase_scan_v1381

mm_pure_rebase_found_v1381:
        hlrz    4,mm_extents(3)
        came    4,010                  ; exact original allocation
        jrst    mm_pure_rebase_bad_v1381
        move    4,mm_extents+1(3)
        hlrz    2,4
        andi    2,7
        caie    2,MM_PROCESS_V1381
        jrst    mm_pure_rebase_bad_v1381
        hrrz    2,4
        came    2,6                     ; exact slot owner
        jrst    mm_pure_rebase_bad_v1381
        move    2,4
        and     2,[03770000000]        ; pin count field, no type bits
        came    2,[010000000]          ; require exactly ONE pin
        jrst    mm_pure_rebase_busy_v1381

        ; All checks are complete. These two stores are the only mutation.
        ; The caller keeps PI disabled until backing/VM/mode publication.
        move    2,010
        sub     2,7
        hrlz    2,2                     ; new raw suffix length
        hrr     2,5
        add     2,7                     ; suffix begins at base + prefix
        movem   2,mm_extents(3)
        andcm   4,[07000000]           ; remove old type bits only
        ior     4,[04000000]           ; type MM_PACKED_V1381
        movem   4,mm_extents+1(3)      ; owner and pin count unchanged
        setz    1,
        jrst    mm_pure_rebase_return_v1381

mm_pure_rebase_busy_v1381:
        movni   1,5
        jrst    mm_pure_rebase_return_v1381
mm_pure_rebase_bad_v1381:
        movni   1,4
mm_pure_rebase_return_v1381:
        pop     17,010
        popj    17,
