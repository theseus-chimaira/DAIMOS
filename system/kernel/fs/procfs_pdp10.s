; procfs_pdp10.s -- compact dynamic PROCFS implementation.
;
; The process table has three words per slot.  Keep synthetic namespace and
; fixed-format file reads in PDP-6 code so descriptor operations do not pay
; GCC save-frame costs.

        .equ    PROC_WORDS,3
        .equ    PROC_STATE_LH_MASK,0700000

        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_one
        .globl  pdp10_ret_neg1
        .globl  proc_table
        .globl  proc_slots
        .globl  proc_high_slot

; AC1 = slot. Return AC1 = active struct proc address, or zero.
; AC2..AC4 are caller-scratch.
procfs_proc_ptr:
        skipn   2,proc_table
        jrst    pdp10_ret_zero
        caml    1,proc_slots
        jrst    pdp10_ret_zero
        move    3,1
        lsh     3,1
        add     3,1
        add     3,2
        hlrz    4,2(3)
        andi    4,PROC_STATE_LH_MASK
        jumpe   4,pdp10_ret_zero
        move    1,3
        popj    17,

; AC1 = slot (0..255). Return AC1 = chars, AC2 = SIXBIT decimal name.
; AC3 is preserved; AC4..AC7 are caller-scratch.
procfs_format_slot:
        setz    4,
        move    5,1
        divi    4,0144
        setz    6,
        move    7,5
        divi    6,012
        jumpn   4,procfs_format_three
        jumpn   6,procfs_format_two
        move    2,7
        addi    2,020
        lsh     2,036
        movei   1,1
        popj    17,
procfs_format_two:
        move    2,6
        addi    2,020
        lsh     2,036
        move    4,7
        addi    4,020
        lsh     4,030
        ior     2,4
        movei   1,2
        popj    17,
procfs_format_three:
        move    2,4
        addi    2,020
        lsh     2,036
        move    4,6
        addi    4,020
        lsh     4,030
        ior     2,4
        move    4,7
        addi    4,020
        lsh     4,022
        ior     2,4
        movei   1,3
        popj    17,

; int procfs_lookup(vnode_t dir, const struct vfs_name *name, vnode_t *nodep)
        .globl  procfs_lookup
procfs_lookup:
        jumpe   2,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        hlrz    4,1
        caie    4,030001
        jrst    procfs_lookup_proc
        move    4,(2)
        jumpe   4,pdp10_ret_neg1
        cail    4,4
        jrst    pdp10_ret_neg1
        move    5,1(2)
        move    6,5
        lsh     6,-036
        andi    6,077
        cail    6,020
        cail    6,032
        jrst    pdp10_ret_neg1
        subi    6,020
        caie    4,1
        jrst    procfs_lookup_digit2
        jrst    procfs_lookup_slot
procfs_lookup_digit2:
        move    7,5
        lsh     7,-030
        andi    7,077
        cail    7,020
        cail    7,032
        jrst    pdp10_ret_neg1
        subi    7,020
        muli    6,012
        add     6,7
        caie    4,2
        jrst    procfs_lookup_digit3
        jrst    procfs_lookup_slot
procfs_lookup_digit3:
        move    7,5
        lsh     7,-022
        andi    7,077
        cail    7,020
        cail    7,032
        jrst    pdp10_ret_neg1
        subi    7,020
        muli    6,012
        add     6,7
        cail    6,0400
        jrst    pdp10_ret_neg1
procfs_lookup_slot:
        move    7,3
        move    1,6
        pushj   17,procfs_proc_ptr
        jumpe   1,pdp10_ret_neg1
        move    1,6
        tlo     1,030002
        movem   1,(7)
        jrst    pdp10_ret_zero
procfs_lookup_proc:
        caie    4,030002
        jrst    pdp10_ret_neg1
        move    5,2
        move    7,3
        hrrz    6,1
        move    1,6
        pushj   17,procfs_proc_ptr
        jumpe   1,pdp10_ret_neg1
        move    2,(5)
        move    3,1(5)
        movei   4,0
        caie    2,4
        jrst    procfs_lookup_len5
        camn    3,procfs_names+0
        movei   4,3
        camn    3,procfs_names+3
        movei   4,6
        jrst    procfs_lookup_have_kind
procfs_lookup_len5:
        caie    2,5
        jrst    procfs_lookup_len6
        camn    3,procfs_names+1
        movei   4,4
        camn    3,procfs_names+2
        movei   4,5
        jrst    procfs_lookup_have_kind
procfs_lookup_len6:
        caie    2,6
        jrst    pdp10_ret_neg1
        camn    3,procfs_names+4
        movei   4,7
procfs_lookup_have_kind:
        jumpe   4,pdp10_ret_neg1
        move    1,6
        hrl     1,4
        tlo     1,030000
        movem   1,(7)
        jrst    pdp10_ret_zero

; int procfs_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent)
        .globl  procfs_readdir
procfs_readdir:
        jumpe   3,pdp10_ret_neg1
        hlrz    4,1
        caie    4,030001
        jrst    procfs_readdir_proc
        skipn   5,proc_table
        jrst    pdp10_ret_zero
        movei   6,0
        movei   7,0
procfs_readdir_root_loop:
        caml    6,proc_high_slot
        jrst    pdp10_ret_zero
        hlrz    4,2(5)
        andi    4,PROC_STATE_LH_MASK
        jumpe   4,procfs_readdir_root_next
        camn    7,2
        jrst    procfs_readdir_root_found
        addi    7,1
procfs_readdir_root_next:
        addi    5,PROC_WORDS
        addi    6,1
        jrst    procfs_readdir_root_loop
procfs_readdir_root_found:
        move    1,6
        pushj   17,procfs_format_slot
        movem   1,(3)
        movem   2,1(3)
        setzm   2(3)
        setzm   3(3)
        setzm   4(3)
        movei   4,1
        movem   4,5(3)
        jrst    pdp10_ret_one
procfs_readdir_proc:
        caie    4,030002
        jrst    pdp10_ret_neg1
        move    7,3
        move    6,2
        hrrz    1,1
        pushj   17,procfs_proc_ptr
        jumpe   1,pdp10_ret_neg1
        cail    6,5
        jrst    pdp10_ret_zero
        move    5,procfs_names(6)
        movei   4,5
        caie    6,0
        cain    6,3
        movei   4,4
        caie    6,4
        jrst    procfs_readdir_proc_store
        movei   4,6
procfs_readdir_proc_store:
        movem   4,(7)
        movem   5,1(7)
        setzm   2(7)
        setzm   3(7)
        setzm   4(7)
        movei   4,2
        movem   4,5(7)
        jrst    pdp10_ret_one

; int procfs_stat(vnode_t node, struct vfs_stat *st)
        .globl  procfs_stat
procfs_stat:
        jumpe   2,pdp10_ret_neg1
        move    7,2
        hlrz    5,1
        caie    5,030001
        jrst    procfs_stat_nonroot
        movei   3,1
        movei   4,0555
        jrst    procfs_stat_store
procfs_stat_nonroot:
        hrrz    6,1
        move    1,6
        pushj   17,procfs_proc_ptr
        jumpe   1,pdp10_ret_neg1
        caie    5,030002
        jrst    procfs_stat_file
        movei   3,1
        movei   4,0555
        jrst    procfs_stat_store
procfs_stat_file:
        caige   5,030003
        jrst    pdp10_ret_neg1
        caile   5,030007
        jrst    pdp10_ret_neg1
        movei   3,2
        movei   4,0444
procfs_stat_store:
        movem   3,(7)
        movem   4,1(7)
        setzm   2(7)
        setzm   3(7)
        jrst    pdp10_ret_zero

; int procfs_getcwd_slot(unsigned int slot, kword_t *buf, unsigned int nwords)
        .globl  procfs_getcwd_slot
procfs_getcwd_slot:
        jumpe   2,pdp10_ret_neg1
        cail    3,3
        jrst    procfs_getcwd_size_ok
        jrst    pdp10_ret_neg1
procfs_getcwd_size_ok:
        move    6,1
        move    7,2
        pushj   17,procfs_proc_ptr
        jumpe   1,pdp10_ret_neg1
        move    1,6
        pushj   17,procfs_format_slot
        addi    1,6
        movem   1,(7)
        move    4,[0176062574317]
        movem   4,1(7)
        movem   2,2(7)
        jrst    pdp10_ret_zero

; Return printable process-state SIXBIT word and length.
; AC1 = struct proc *, AC2 = slot. Return AC1 = word, AC2 = chars.
procfs_state_word:
        hlrz    3,2(1)
        andi    3,PROC_STATE_LH_MASK
        lsh     3,-017
        cain    3,4                    ; ZOMB outranks nonresident/SWAP
        jrst    procfs_state_not_swapped
        jumpe   2,procfs_state_not_swapped
        hrrz    4,1(1)
        jumpe   4,procfs_state_swapped
procfs_state_not_swapped:
        move    1,procfs_state_names(3)
        movei   2,4
        cain    3,1
        jrst    procfs_state_len3
        cain    3,2
        jrst    procfs_state_len3
        cain    3,3
        movei   2,5
        popj    17,
procfs_state_len3:
        movei   2,3
        popj    17,
procfs_state_swapped:
        move    1,[0636741600000]
        movei   2,4
        popj    17,

; STATUS reader. AC1 = proc *, AC2 = slot, AC5 = off, AC7 = chp.
procfs_status_readchar:
        cail    5,024
        jrst    procfs_status_tail
        move    3,5
        lsh     3,-2
        move    4,5
        andi    4,3
        cain    4,3
        jrst    procfs_status_space
        jumpe   3,procfs_status_pid
        cain    3,1
        jrst    procfs_status_ppid
        cain    3,2
        jrst    procfs_status_pgrp
        pushj   17,proc_scope_id
        move    6,1
        caie    3,3
        lsh     6,-010
        andi    6,0377
        jrst    procfs_status_digit
procfs_status_pid:
        move    6,2
        jrst    procfs_status_digit
procfs_status_ppid:
        ldb     6,[POINT 8,(1),27]
        jrst    procfs_status_digit
procfs_status_pgrp:
        hrrz    6,(1)
        andi    6,0377
procfs_status_digit:
        jumpe   4,procfs_status_digit0
        caie    4,1
        jrst    procfs_status_digit2
        lsh     6,-3
        jrst    procfs_status_digit_store
procfs_status_digit0:
        lsh     6,-6
procfs_status_digit2:
procfs_status_digit_store:
        andi    6,7
        addi    6,060
        movem   6,(7)
        jrst    pdp10_ret_one
procfs_status_space:
        movei   6,040
        movem   6,(7)
        jrst    pdp10_ret_one
procfs_status_tail:
        caie    5,024
        jrst    procfs_status_cr
        pushj   17,procfs_state_word
        move    6,1
        lsh     6,-036
        andi    6,077
        addi    6,040
        movem   6,(7)
        jrst    pdp10_ret_one
procfs_status_cr:
        caie    5,025
        jrst    procfs_status_lf
        movei   6,015
        movem   6,(7)
        jrst    pdp10_ret_one
procfs_status_lf:
        caie    5,026
        jrst    pdp10_ret_zero
        movei   6,012
        movem   6,(7)
        jrst    pdp10_ret_one

; int procfs_readchar(vnode_t node, kword_t off, unsigned int *chp)
        .globl  procfs_readchar
        .globl  vfs_sixbit_readchar
        .globl  kfmt_u18_decimal_readchar
        .globl  proc_scope_id
        .globl  proc_comm_words
procfs_readchar:
        jumpe   3,pdp10_ret_neg1
        move    7,3
        move    5,2
        move    6,1
        hlrz    4,1
        caige   4,030003
        jrst    pdp10_ret_neg1
        caile   4,030007
        jrst    pdp10_ret_neg1
        hrrz    1,6
        pushj   17,procfs_proc_ptr
        jumpe   1,pdp10_ret_neg1
        hlrz    4,6
        hrrz    2,6
        caie    4,030006
        jrst    procfs_readchar_not_comm
        movei   4,2
        jumpe   2,procfs_readchar_comm0
        cain    2,1
        movei   4,1
        jrst    procfs_readchar_comm_load
procfs_readchar_comm0:
        movei   4,0
procfs_readchar_comm_load:
        move    1,proc_comm_words(4)
        movei   2,6
        move    3,5
        move    4,7
        jrst    vfs_sixbit_readchar
procfs_readchar_not_comm:
        caie    4,030007
        jrst    procfs_readchar_not_status
        jrst    procfs_status_readchar
procfs_readchar_not_status:
        caie    4,030004
        jrst    procfs_readchar_numeric
        pushj   17,procfs_state_word
        move    3,5
        move    4,7
        jrst    vfs_sixbit_readchar
procfs_readchar_numeric:
        caie    4,030003
        jrst    procfs_readchar_words
        ldb     1,[POINT 8,(1),27]
        jrst    procfs_readchar_number
procfs_readchar_words:
        caie    4,030005
        jrst    pdp10_ret_neg1
        hlrz    1,1(1)
procfs_readchar_number:
        move    2,5
        move    3,7
        jrst    kfmt_u18_decimal_readchar

        .data
procfs_names:
        .word   0606051440000
        .word   0636441644500
        .word   0675762446300
        .word   0435755550000
        .word   0636441646563
procfs_state_names:
        .word   0466245450000
        .word   0514454000000
        .word   0626556000000
        .word   0635445456000
        .word   0725755420000
        .word   0466245450000
        .word   0636457600000
