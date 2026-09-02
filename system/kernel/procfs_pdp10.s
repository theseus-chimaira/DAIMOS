; procfs_pdp10.s -- fixed two-process PROCFS implementation.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1
        .globl  vfs_sixbit_readchar
        .globl  kfmt_u36_decimal_readchar
        .globl  proc_table
        .globl  proc_comm_words

; int procfs_stat(vnode_t node, struct vfs_stat *st)
        .globl procfs_stat
procfs_stat:
        jumpe   2,procfs_stat_fail
        hlrz    3,1
        caie    3,030001               ; root directory
        jrst    procfs_stat_nonroot
        movei   4,1
        movei   5,0555
        jrst    procfs_stat_store
procfs_stat_nonroot:
        hrrz    4,1
        cail    4,2                    ; process/file nodes only slots 0..1
        jrst    procfs_stat_fail
        caie    3,030002
        jrst    procfs_stat_file
        movei   4,1
        movei   5,0555
        jrst    procfs_stat_store
procfs_stat_file:
        caige   3,030003
        jrst    procfs_stat_fail
        caile   3,030006
        jrst    procfs_stat_fail
        movei   4,2
        movei   5,0444
procfs_stat_store:
        movem   4,(2)
        movem   5,1(2)
        setzm   2(2)
        setzm   3(2)
        movei   1,0
        popj    17,
procfs_stat_fail:
        jrst    pdp10_ret_neg1

; int procfs_readchar(vnode_t node, kword_t off, unsigned int *chp)
        .globl  procfs_readchar
procfs_readchar:
        jumpe   3,procfs_readchar_fail
        hlrz    4,1
        caige   4,030003               ; PPID..COMM kinds 3..6
        jrst    procfs_readchar_fail
        caile   4,030006
        jrst    procfs_readchar_fail
        hrrz    5,1
        cail    5,2                    ; fixed slots 0 and 1
        jrst    procfs_readchar_fail
        caie    4,030006               ; COMM
        jrst    procfs_readchar_not_comm
        move    4,3                    ; vfs readchar chp
        move    3,2                    ; vfs readchar off
        movei   2,6
        move    1,proc_comm_words(5)
        jrst    vfs_sixbit_readchar
procfs_readchar_not_comm:
        caie    4,030004               ; STATE
        jrst    procfs_readchar_numeric
        move    4,3
        move    3,2
        movei   2,3
        movsi   1,0626556              ; SIXBIT /RUN   /
        jrst    vfs_sixbit_readchar
procfs_readchar_numeric:
        caie    4,030003               ; PPID
        jrst    procfs_readchar_words
        movei   1,0
        jrst    kfmt_u36_decimal_readchar
procfs_readchar_words:
        caie    4,030005               ; WORDS
        jrst    procfs_readchar_fail
        lsh     5,1
        hlrz    1,proc_table+1(5)
        jrst    kfmt_u36_decimal_readchar
procfs_readchar_fail:
        jrst    pdp10_ret_neg1

; Fixed two-process PROCFS directory operations.
; int procfs_lookup(vnode_t dir, const struct vfs_name *name,
;     vnode_t *nodep)
        .globl  procfs_lookup
procfs_lookup:
        jumpe   2,procfs_lookup_fail
        jumpe   3,procfs_lookup_fail
        hlrz    4,1
        caie    4,030001               ; root
        jrst    procfs_lookup_proc
        move    4,(2)
        caie    4,1
        jrst    procfs_lookup_fail
        move    4,1(2)
        camn    4,[0200000000000]      ; SIXBIT /0     /
        jrst    procfs_lookup_slot0
        came    4,[0210000000000]      ; SIXBIT /1     /
        jrst    procfs_lookup_fail
        movsi   4,030002
        hrri    4,1
        movem   4,(3)
        jrst    pdp10_ret_zero
procfs_lookup_slot0:
        movsi   4,030002
        movem   4,(3)
        jrst    pdp10_ret_zero
procfs_lookup_proc:
        caie    4,030002
        jrst    procfs_lookup_fail
        hrrz    4,1
        cail    4,2
        jrst    procfs_lookup_fail
        move    4,(2)                  ; name chars
        move    5,1(2)                 ; packed name
        movei   6,0
        caie    4,4
        jrst    procfs_lookup_len5
        camn    5,procfs_readdir_names+0 ; PPID
        movei   6,3
        camn    5,procfs_readdir_names+3 ; COMM
        movei   6,6
        jumpn   6,procfs_lookup_file
        jrst    procfs_lookup_fail
procfs_lookup_len5:
        caie    4,5
        jrst    procfs_lookup_fail
        camn    5,procfs_readdir_names+1 ; STATE
        movei   6,4
        camn    5,procfs_readdir_names+2 ; WORDS
        movei   6,5
        jumpe   6,procfs_lookup_fail
procfs_lookup_file:
        lsh     6,022                  ; kind -> LH (18 bits)
        tlo     6,030000               ; provider 3
        hrrz    4,1                    ; process slot
        ior     6,4
        movem   6,(3)
        jrst    pdp10_ret_zero
procfs_lookup_fail:
        jrst    pdp10_ret_neg1

; int procfs_readdir(vnode_t dir, unsigned int off,
;     struct vfs_dirent *ent)
        .globl  procfs_readdir
procfs_readdir:
        jumpe   3,procfs_readdir_fail
        hlrz    4,1
        caie    4,030001
        jrst    procfs_readdir_proc
        cail    2,2
        jrst    pdp10_ret_zero
        movei   4,1
        move    5,2
        lsh     5,036
        add     5,[0200000000000]
        movei   6,1
        jrst    procfs_readdir_store
procfs_readdir_proc:
        caie    4,030002
        jrst    procfs_readdir_fail
        hrrz    4,1
        cail    4,2
        jrst    procfs_readdir_fail
        cail    2,4
        jrst    pdp10_ret_zero
        movei   4,5
        caie    2,0
        cain    2,3
        movei   4,4
        move    5,procfs_readdir_names(2)
        movei   6,2
procfs_readdir_store:
        movem   4,(3)
        movem   5,1(3)
        setzm   2(3)
        setzm   3(3)
        setzm   4(3)
        movem   6,5(3)
        movei   1,1
        popj    17,
procfs_readdir_fail:
        jrst    pdp10_ret_neg1

procfs_readdir_names:
        .long   0606051440000          ; PPID
        .long   0636441644500          ; STATE
        .long   0675762446300          ; WORDS
        .long   0435755550000          ; COMM
