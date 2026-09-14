; domainfs_pdp10.s -- compact PDP-10 DOMAINFS leaf implementation.
;
; The portable domainfs.c remains the host/reference implementation.  This
; target leaf mirrors the compact procfs_pdp10.s implementation so synthetic
; process-domain directory operations do not pay GCC frame and unsigned-
; arithmetic costs in permanent KCORE.
;
; Domain IDs are 0..255.  Live process scope is stored in the stable u-area
; control word; zombie scope is retained in the scheduler word.  proc_scope_id
; already normalizes those representations, so scans use it rather than
; duplicating the scope lifetime rules here.

        .equ    PROC_WORDS,3
        .equ    PROC_STATE_LH_MASK,0700000
        .equ    PROC_STATE_ZOMB_LH,0400000
        .equ    PROC_STATE_STOP_LH,0600000
        .equ    PROC_MAX_SLOTS,0400
        .equ    DOMAIN_TAG,0400000
        .equ    DOMAIN_MASK,0377
        .equ    DOMAIN_SHIFT,010
        .equ    DOMAIN_STATUS_WORDS,6

        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_one
        .globl  pdp10_ret_neg1
        .globl  proc_table
        .globl  proc_high_slot
        .globl  proc_swap_records
        .globl  proc_scope_id
        .globl  procfs_parse_slot
        .globl  procfs_format_slot

; AC1 = domain ID.  Return AC1 = 1 if at least one active process belongs to
; the domain, otherwise zero.  AC3 and AC7 are preserved; AC6 returns the ID.
        .globl  domainfs_exists
domainfs_exists:
        skipn   4,proc_table
        jrst    pdp10_ret_zero
        move    6,1
        movei   5,0
domainfs_exists_loop:
        caml    5,proc_high_slot
        jrst    pdp10_ret_zero
        hlrz    2,2(4)
        andi    2,PROC_STATE_LH_MASK
        jumpe   2,domainfs_exists_next
        move    1,4
        pushj   17,proc_scope_id
        lsh     1,-DOMAIN_SHIFT
        andi    1,DOMAIN_MASK
        camn    1,6
        jrst    pdp10_ret_one
domainfs_exists_next:
        addi    4,PROC_WORDS
        aoja    5,domainfs_exists_loop

; Fill six DOMAIN/ID/STATUS words at AC2 for domain AC1.  Return the process
; count in AC1.  The layout is:
;   DID, processes, resident user words, swapped processes,
;   swap sectors, stopped processes.
domainfs_status:
        move    7,2
        move    5,1
        movem   1,(7)
        setzm   1(7)
        setzm   2(7)
        setzm   3(7)
        setzm   4(7)
        setzm   5(7)
        skipn   6,proc_table
        jrst    domainfs_status_done
        movei   4,0
domainfs_status_loop:
        caml    4,proc_high_slot
        jrst    domainfs_status_done
        hlrz    3,2(6)
        andi    3,PROC_STATE_LH_MASK
        jumpe   3,domainfs_status_next
        move    1,6
        pushj   17,proc_scope_id
        lsh     1,-DOMAIN_SHIFT
        andi    1,DOMAIN_MASK
        came    1,5
        jrst    domainfs_status_next
        aos     1(7)
        cain    3,PROC_STATE_ZOMB_LH
        jrst    domainfs_status_next
domainfs_status_live:
        hrrz    1,1(6)
        jumpe   1,domainfs_status_nonresident
        hlrz    1,1(6)
        addm    1,2(7)
        jrst    domainfs_status_stop
domainfs_status_nonresident:
        jumpe   4,domainfs_status_stop
        skipn   1,proc_swap_records
        jrst    domainfs_status_stop
        add     1,4
        skipn   2,(1)
        jrst    domainfs_status_stop
        aos     3(7)
        hrrz    2,2
        addm    2,4(7)
domainfs_status_stop:
        cain    3,PROC_STATE_STOP_LH
        aos     5(7)
domainfs_status_next:
        addi    6,PROC_WORDS
        aoja    4,domainfs_status_loop
domainfs_status_done:
        move    1,1(7)
        popj    17,

; int domainfs_read_words(vnode_t node, unsigned int off, kword_t *buf,
;     unsigned int nwords)
        .globl  domainfs_read_words
domainfs_read_words:
        jumpe   3,pdp10_ret_neg1
        hlrz    5,1
        caie    5,030007
        jrst    pdp10_ret_neg1
        move    5,1
        andi    5,DOMAIN_MASK
        cail    2,DOMAIN_STATUS_WORDS
        jrst    pdp10_ret_zero
        ; Six status words plus saved off/buf/nwords.  This is transient stack
        ; storage only; no resident process or KCORE data is added.
        add     17,[011,,011]
        movem   2,-2(17)
        movem   3,-1(17)
        movem   4,(17)
        move    1,5
        movei   2,-010(17)
        pushj   17,domainfs_status
        jumpe   1,domainfs_read_missing
        move    5,-2(17)
        move    6,-1(17)
        move    7,(17)
        movei   1,0
domainfs_read_copy:
        cail    5,DOMAIN_STATUS_WORDS
        jrst    domainfs_read_done
        jumpe   7,domainfs_read_done
        movei   2,-010(17)
        add     2,5
        move    3,(2)
        movem   3,(6)
        addi    6,1
        addi    5,1
        subi    7,1
        aoja    1,domainfs_read_copy
domainfs_read_done:
        sub     17,[011,,011]
        popj    17,
domainfs_read_missing:
        sub     17,[011,,011]
        jrst    pdp10_ret_neg1

; int domainfs_getcwd_did(unsigned int did, kword_t *buf,
;     unsigned int nwords)
        .globl  domainfs_getcwd_did
domainfs_getcwd_did:
        jumpe   2,pdp10_ret_neg1
        caige   3,3
        jrst    pdp10_ret_neg1
        move    3,1                    ; domain ID; exists preserves AC3
        move    7,2                    ; output; exists preserves AC7
        pushj   17,domainfs_exists
        jumpe   1,pdp10_ret_neg1
        move    1,3
        move    3,7                    ; format_slot preserves AC3
        pushj   17,procfs_format_slot
        addi    1,010
        movem   1,(3)
        move    1,[0174457554151]      ; /DOMAI
        movem   1,1(3)
        lsh     2,-014
        ior     2,[0561700000000]      ; N/ + left-justified decimal ID
        movem   2,2(3)
        jrst    pdp10_ret_zero
