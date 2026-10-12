; dsh_var_pdp6.s -- compact packed, heap-backed DSH variables.
; Each descriptor contains metadata and one payload pointer (two words).
; Payload stores the used SIXBIT name words followed by value words.
; The same heap also owns transient wide argument arrays.
; AC1..AC7 scratch, AC10..AC15 callee-preserved.
        .text
        .globl dsh_var_set
        .globl dsh_var_get
        .globl dsh_var_unset
        .globl dsh_var_at
        .globl dsh_var_scratch
        .globl dsh_heap_alloc
        .globl dsh_heap_free

; Slot AC5 = state AC10 + 2 * index AC14.
dv_slot:
        move    5,14
        imuli   5,2
        add     5,10
        popj    17,

; Quotient AC6 = ceiling(AC6/6), remainder AC7 overwritten.
dv_words:
        addi    6,5
        idivi   6,6
        popj    17,

; Compare slot AC5 to name AC11, returning AC1 boolean.
dv_nameeq:
        move    2,0(5)
        andi    2,077
        came    2,0(11)
         jrst   dv_ne_no
        move    6,2
        pushj   17,dv_words
        move    4,1(5)
        movei   7,1
 dv_ne_loop:
        move    2,4
        add     2,7
        move    3,11
        add     3,7
        move    2,-1(2)
        came    2,0(3)
         jrst   dv_ne_no
        caige   7,0(6)
         aoja   7,dv_ne_loop
        movei   1,1
        popj    17,
 dv_ne_no:
        setz    1,
        popj    17,

; Copy as many packed payload words as the source length requires.
; AC5 base pointer, AC2 destination word offset, AC3 source s6.
dv_copy_in:
        move    6,0(3)
        pushj   17,dv_words
        movei   7,0
 dv_ci_loop:
        caml    7,6
         popj   17,
        move    4,3
        add     4,7
        move    1,1(4)
        move    4,5
        add     4,2
        add     4,7
        movem   1,0(4)
        aoja    7,dv_ci_loop

; AC5 base pointer, AC2 source offset, AC3 destination s6.
; Clear all unused destination words, preserving canonical s6 form.
dv_copy_out:
        movei   7,1
 dv_co_clear:
        move    4,3
        add     4,7
        setzm   0(4)
        caige   7,6
         aoja   7,dv_co_clear
        move    6,0(3)
        pushj   17,dv_words
        movei   7,0
 dv_co_loop:
        caml    7,6
         popj   17,
        move    4,5
        add     4,2
        add     4,7
        move    1,0(4)
        move    4,3
        add     4,7
        movem   1,1(4)
        aoja    7,dv_co_loop

; int dsh_var_set(state, name, value, exported)
dsh_var_set:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        push    17,14
        push    17,15
        move    10,1
        move    11,2
        move    12,3
        move    13,4
        skipn   10
         jrst   dvs_fail
        skipn   11
         jrst   dvs_fail
        skipn   12
         jrst   dvs_fail
        skipn   0(11)
         jrst   dvs_fail
        setz    14,
        seto    15,                      ; first free slot
 dvs_scan:
        pushj   17,dv_slot
        skipn   0(5)
         jrst   dvs_free
        pushj   17,dv_nameeq
        jumpn   1,dvs_store
 dvs_next:
        addi    14,1
        caige   14,0100
         jrst   dvs_scan
        jrst    dvs_use_free
 dvs_free:
        jumpge  15,dvs_next
        move    15,14
        jrst    dvs_next
 dvs_use_free:
        jumpl   15,dvs_fail
        move    14,15
 dvs_store:
; Allocate replacement before freeing old storage.  This also supports
; setting a variable to a value expanded from its current payload.
        move    6,0(11)
        pushj   17,dv_words
        push    17,6
        move    6,0(12)
        pushj   17,dv_words
        pop     17,1
        add     1,6
        pushj   17,dsh_heap_alloc
        skipn   1
         jrst   dvs_fail
        move    15,1
        pushj   17,dv_slot
        push    17,1(5)                 ; old payload, if any
        move    1,0(5)
        tlz     1,0577777               ; retain previous export flag
        hrri    1,0
        jumpl   13,dvs_meta
        setz    1,
        skipg   13
         jrst   dvs_meta
        movsi   1,0200000
 dvs_meta:
        move    2,0(12)
        lsh     2,6
        ior     1,2
        ior     1,0(11)
        tlo     1,0400000
        movem   1,0(5)
        movem   15,1(5)
        move    5,15
        setz    2,
        move    3,11
        pushj   17,dv_copy_in
        move    6,0(11)
        pushj   17,dv_words
        move    2,6
        move    3,12
        pushj   17,dv_copy_in
        pop     17,1
        pushj   17,dsh_heap_free
        setz    1,
        jrst    dvs_done
 dvs_fail:
        seto    1,
 dvs_done:
        pop     17,15
        pop     17,14
        pop     17,13
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,

; const struct dsh_s6 *dsh_var_get(state, name)
dsh_var_get:
        push    17,10
        push    17,11
        push    17,14
        move    10,1
        move    11,2
        setz    14,
 dvg_scan:
        pushj   17,dv_slot
        skipn   0(5)
         jrst   dvg_next
        pushj   17,dv_nameeq
        jumpn   1,dvg_found
 dvg_next:
        addi    14,1
        caige   14,0100
         jrst   dvg_scan
        setz    1,
        jrst    dvg_done
 dvg_found:
        move    1,0(5)
        lsh     1,-6
        andi    1,077
        movem   1,dsh_var_scratch
        move    6,0(5)
        andi    6,077
        pushj   17,dv_words
        move    2,6
        move    5,1(5)
        movei   3,dsh_var_scratch
        pushj   17,dv_copy_out
        movei   1,dsh_var_scratch
 dvg_done:
        pop     17,14
        pop     17,11
        pop     17,10
        popj    17,

; int dsh_var_unset(state, name)
dsh_var_unset:
        push    17,10
        push    17,11
        push    17,14
        move    10,1
        move    11,2
        setz    14,
 dvu_scan:
        pushj   17,dv_slot
        skipn   0(5)
         jrst   dvu_next
        pushj   17,dv_nameeq
        jumpn   1,dvu_found
 dvu_next:
        addi    14,1
        caige   14,0100
         jrst   dvu_scan
        jrst    dvu_done
 dvu_found:
        move    1,1(5)
        setzm   0(5)
        ; Metadata alone controls occupancy; stale payload is never read.
        pushj   17,dsh_heap_free
 dvu_done:
        setz    1,
        pop     17,14
        pop     17,11
        pop     17,10
        popj    17,

; int dsh_var_at(state, slot, name, value)
dsh_var_at:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        push    17,14
        move    10,1
        move    11,2
        move    12,3
        move    13,4
        move    14,11
        caige   14,0100
         jrst   dva_slot
        setz    1,
        jrst    dva_done
 dva_slot:
        pushj   17,dv_slot
        skipn   0(5)
         jrst   dva_none
        move    1,0(5)
        andi    1,077
        movem   1,0(12)
        move    6,1
        pushj   17,dv_words
        move    14,6                   ; save name-word offset
        move    5,1(5)
        setz    2,
        move    3,12
        pushj   17,dv_copy_out
        move    2,14
        move    3,13
        ; Metadata reload is through the original slot, not payload.
        move    4,11
        imuli   4,2
        add     4,10
        move    1,0(4)
        lsh     1,-6
        andi    1,077
        movem   1,0(13)
        pushj   17,dv_copy_out
        movei   1,1
        move    4,11
        imuli   4,2
        add     4,10
        hlrz    2,0(4)
        andi    2,0200000
        jumpe   2,dva_done
        movei   1,2
        jrst    dva_done
 dva_none:
        setz    1,
 dva_done:
        pop     17,14
        pop     17,13
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,
