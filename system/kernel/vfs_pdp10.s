; vfs_pdp10.s -- compact resident VFS primitives for PDP-6/PDP-10.
        .text
        .globl  pdp10_ret_zero_v1
        .globl  pdp10_ret_neg1_v1

; int vfs_v1_name_set6(struct vfs_v1_name *name, kword_t word,
;     unsigned int chars)
        .globl  vfs_v1_name_set6
vfs_v1_name_set6:
        jumpe   1,vfs_name_set6_fail
        jumpge  3,vfs_name_set6_small
        jrst    vfs_name_set6_fail
vfs_name_set6_small:
        caile   3,6
        jrst    vfs_name_set6_fail
        movem   3,(1)
        movem   2,1(1)
        setzm   2(1)
        setzm   3(1)
        setzm   4(1)
        movei   1,0
        popj    17,
vfs_name_set6_fail:
        jrst    pdp10_ret_neg1_v1

; int vfs_v1_name_is6(const struct vfs_v1_name *name, kword_t word,
;     unsigned int chars)
        .globl  vfs_v1_name_is6
vfs_v1_name_is6:
        jumpe   1,vfs_name_is6_fail
        camn    3,(1)
        came    2,1(1)
        jrst    vfs_name_is6_fail
        movei   1,1
        popj    17,
vfs_name_is6_fail:
        movei   1,0
        popj    17,

; int vfs_v1_sixbit_readchar(kword_t word, unsigned int nchars, kword_t off,
;     unsigned int *chp)
        .globl  vfs_v1_sixbit_readchar
vfs_v1_sixbit_readchar:
        jumpe   4,vfs_sixchar_fail
        jumpge  2,vfs_sixchar_count_small
        jrst    vfs_sixchar_fail
vfs_sixchar_count_small:
        caile   2,6
        jrst    vfs_sixchar_fail
        jumpge  3,vfs_sixchar_off_small
        jrst    vfs_sixchar_eof
vfs_sixchar_off_small:
        caml    3,2
        jrst    vfs_sixchar_tail
        move    6,3
        imuli   6,6
        subi    6,036
        move    5,1
        lsh     5,0(6)
        andi    5,077
        addi    5,040
        movem   5,(4)
        movei   1,1
        popj    17,
vfs_sixchar_tail:
        came    3,2
        jrst    vfs_sixchar_lf
        movei   5,015                  ; CR
        movem   5,(4)
        movei   1,1
        popj    17,
vfs_sixchar_lf:
        addi    2,1
        came    3,2
        jrst    vfs_sixchar_eof
        movei   5,012                  ; LF
        movem   5,(4)
        movei   1,1
        popj    17,
vfs_sixchar_eof:
        jrst    pdp10_ret_zero_v1
vfs_sixchar_fail:
        jrst    pdp10_ret_neg1_v1
