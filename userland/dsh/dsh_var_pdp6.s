; dsh_var_pdp6.s -- compact fixed-slot DSH variables for PDP-6/10.
;
; Each slot is 13 words:
;   0      metadata
;   1..6   packed SIXBIT name payload
;   7..12  packed SIXBIT value payload
;
; Metadata:
;   bit 35       used
;   bit 34       exported
;   bits 11..6   value length (0..36)
;   bits 5..0    name length  (1..36)
;
; This preserves the full DSH_S6_MAX_CHARS limits while reducing a variable
; slot from 16 words to 13.  AC1..AC7 are scratch; AC10..AC15 are preserved.

        .text

        .globl dsh_var_set
        .globl dsh_var_get
        .globl dsh_var_unset
        .globl dsh_var_at
        .globl dsh_var_scratch

; Build slot address in AC5 from state AC10 and index AC14.
; 13 decimal is 015 octal.
dv_slot:
        move    5,14
        imuli   5,015
        add     5,10
        popj    17,

; Compare slot AC5 name with struct dsh_s6 * in AC11.
; Return AC1 = 1 equal, 0 otherwise.  Clobbers AC2..AC4, AC6..AC7.
dv_nameeq:
        move    2,0(5)
        andi    2,077
        came    2,0(11)
         jrst   dv_ne_no
        movei   6,1
dv_ne_loop:
        move    3,5
        add     3,6
        move    4,11
        add     4,6
        move    7,0(3)
        came    7,0(4)
         jrst   dv_ne_no
        caige   6,6
         aoja   6,dv_ne_loop
        movei   1,1
        popj    17,
dv_ne_no:
        setz    1,
        popj    17,

; Copy six payload words from struct dsh_s6 * AC3 into slot AC5+AC2.
; AC2 is destination offset (1 for name, 7 for value).
dv_copy_in:
        movei   6,1
dv_ci_loop:
        move    4,3
        add     4,6
        move    7,0(4)
        move    4,5
        add     4,2
        subi    4,1
        add     4,6
        movem   7,0(4)
        caige   6,6
         aoja   6,dv_ci_loop
        popj    17,

; Copy six slot payload words AC5+AC2 into struct dsh_s6 * AC3.
; Caller stores the length word first.
dv_copy_out:
        movei   6,1
dv_co_loop:
        move    4,5
        add     4,2
        subi    4,1
        add     4,6
        move    7,0(4)
        move    4,3
        add     4,6
        movem   7,0(4)
        caige   6,6
         aoja   6,dv_co_loop
        popj    17,

; int dsh_var_set(st, name, value, exported)
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
        seto    15,                       ; first free slot
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
        pushj   17,dv_slot
        setz    1,                        ; no old export bit
        skipg   13
         jrst   dvs_meta
        movsi   1,0200000
        jrst    dvs_meta
dvs_store:
        move    1,0(5)
        tlz     1,0577777                 ; retain export bit only
        hrri    1,0
        jumpge  13,dvs_meta_explicit
        jrst    dvs_meta
dvs_meta_explicit:
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
        movei   2,1
        move    3,11
        pushj   17,dv_copy_in
        movei   2,7
        move    3,12
        pushj   17,dv_copy_in
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

; const struct dsh_s6 *dsh_var_get(st, name)
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
        movei   2,7
        movei   3,dsh_var_scratch
        pushj   17,dv_copy_out
        movei   1,dsh_var_scratch
dvg_done:
        pop     17,14
        pop     17,11
        pop     17,10
        popj    17,

; int dsh_var_unset(st, name)
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
        setzm   0(5)
dvu_done:
        setz    1,
        pop     17,14
        pop     17,11
        pop     17,10
        popj    17,

; int dsh_var_at(st, slot, name, value)
; Return 0 if absent, 1 if present, 2 if present and exported.
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
        movei   2,1
        move    3,12
        pushj   17,dv_copy_out
        move    1,0(5)
        lsh     1,-6
        andi    1,077
        movem   1,0(13)
        movei   2,7
        move    3,13
        pushj   17,dv_copy_out
        movei   1,1
        hlrz    2,0(5)
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
