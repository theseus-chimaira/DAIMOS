; Source-linked by the experimental v1611 PURE MRES only.
; The default boot does not install it; no permanent KCORE words.
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

        push    17,5
        push    17,6
        push    17,7
        pushj   17,mm_pure_table_v1922
        pop     17,7
        pop     17,6
        pop     17,5
        jumpn   1,mm_pure_rebase_bad_v1381
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

; Shared optional PURE MRES physical-range proof. AC1=base, AC2=words.
; Return 0 if contained in one managed arena, -2 for an unmanaged
; range in an otherwise valid arena table, -4 for malformed geometry.
; Validate ALL arenas, even those following the matching range.
; On success AC2 also returns the containing arena end for the ASM
; table scanner. PI exclusion is held by caller. AC0..AC7 scratch.
        .globl mm_pure_range_v1922
        .globl mm_arenas
        .globl mm_arena_count
        .globl mm_core_words
mm_pure_range_v1922:
        jumpe   1,mpure_range_bad_v1922
        jumpe   2,mpure_range_bad_v1922
        tlne    1,0777777
        jrst    mpure_range_bad_v1922
        tlne    2,0777777
        jrst    mpure_range_bad_v1922
        move    3,1
        add     3,2
mpure_range_core_v1922:
        move    4,mm_core_words
        jumple  4,mpure_range_bad_v1922
        camle   4,[01000000]
        jrst    mpure_range_bad_v1922
        camle   3,4
        jrst    mpure_range_bad_v1922
        move    4,mm_arena_count
        jumpl   4,mpure_range_bad_v1922
        caile   4,3
        jrst    mpure_range_bad_v1922
        setz    4,                     ; arena index
        setz    5,                     ; previous arena end
        movni   0,2                    ; no containing arena yet
mpure_range_loop_v1922:
        caml    4,mm_arena_count
        jrst    mpure_range_done_v1922
        move    6,mm_arenas(4)
        hrrz    7,6
        hlrz    6,6
        jumpe   6,mpure_range_bad_v1922
        camge   7,5
        jrst    mpure_range_bad_v1922
        add     6,7
        camle   6,mm_core_words
        jrst    mpure_range_bad_v1922
        move    5,6
        camge   1,7
        jrst    mpure_range_next_v1922
        camle   3,6
        jrst    mpure_range_next_v1922
        setz    0,
        move    2,6                    ; containing arena's exclusive end
mpure_range_next_v1922:
        aoja    4,mpure_range_loop_v1922
mpure_range_done_v1922:
        move    1,0
        popj    17,
mpure_range_bad_v1922:
        movni   1,4
        popj    17,

; Check every live descriptor's sorted nonoverlap and containment.
; Reuse the containing arena end while advancing through sorted extents.
; The first range proof validates the entire arena table; subsequent
; calls are needed only when crossing an arena boundary. No DATA/BSS.
        .globl mm_pure_table_v1922
        .globl mm_extent_capacity
mm_pure_table_v1922:
        move    1,mm_extent_count
        jumpl   1,mpure_range_bad_v1922
        caile   1,0210
        jrst    mpure_range_bad_v1922
        camle   1,mm_extent_capacity
        jrst    mpure_range_bad_v1922
        jumpe   1,mpure_table_empty_v1922
        push    17,010
        push    17,011
        push    17,012
        setz    010,                   ; descriptor word index
        setz    011,                   ; preceding extent end
        setz    012,                   ; containing arena end
mpure_table_loop_v1922:
        move    3,010
        lsh     3,-1
        caml    3,mm_extent_count
        jrst    mpure_table_done_v1922
        hrrz    1,mm_extents(010)
        hlrz    2,mm_extents(010)
        jumpe   1,mpure_table_bad_v1922
        jumpe   2,mpure_table_bad_v1922
        camge   1,011                  ; sorted, nonoverlapping
        jrst    mpure_table_bad_v1922
        move    011,1
        add     011,2
        ; A sorted range starting after the previous extent and ending
        ; inside its arena is already proven. A hole needs a fresh proof.
        camg    011,012
        jrst    mpure_table_next_v1922
        pushj   17,mm_pure_range_v1922
        jumpn   1,mpure_table_bad_v1922
        move    012,2
mpure_table_next_v1922:
        addi    010,2
        jrst    mpure_table_loop_v1922
mpure_table_bad_v1922:
        movni   1,4
        jrst    mpure_table_return_v1922
mpure_table_done_v1922:
        setz    1,
mpure_table_return_v1922:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; Empty extent table still requires valid core and arena geometry.
; Enter the private arena-only scan with no physical ownership range;
; a valid table may contain no managed arenas yet.
mpure_table_empty_v1922:
        setz    1,
        setz    3,
        pushj   17,mpure_range_core_v1922
        came    1,[-4]
        setz    1,
        popj    17,
