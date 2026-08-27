; vfs_pdp10.s -- compact resident VFS primitives for PDP-6/PDP-10.
        .text

; int vfs_v1_name_set_pid(struct vfs_v1_name *name, unsigned int value)
;
; PID names are decimal SIXBIT and are limited to 0..0377 (255 decimal).
; Build the packed name directly instead of using repeated subtraction and
; a position-dependent helper.
        .globl  vfs_v1_name_set_pid
vfs_v1_name_set_pid:
        jumpe   1,vfs_pid_name_fail
        jumpge  2,vfs_pid_name_small
        jrst    vfs_pid_name_fail       ; unsigned value has bit 35 set
vfs_pid_name_small:
        caile   2,0377
        jrst    vfs_pid_name_fail
        move    7,1                     ; preserve name pointer
        caige   2,0144                  ; 100 decimal
        jrst    vfs_pid_name_lt100

; Three decimal digits.
        move    4,2
        idivi   4,0144                  ; AC4=hundreds, AC5=remainder
        addi    4,020
        lsh     4,036
        move    6,4
        move    4,5
        idivi   4,012                   ; AC4=tens, AC5=units
        addi    4,020
        lsh     4,030
        ior     6,4
        addi    5,020
        lsh     5,022
        ior     6,5
        movei   3,3
        jrst    vfs_pid_name_store

vfs_pid_name_lt100:
        caige   2,012                   ; 10 decimal
        jrst    vfs_pid_name_lt10
        move    4,2
        idivi   4,012                   ; AC4=tens, AC5=units
        addi    4,020
        lsh     4,036
        move    6,4
        addi    5,020
        lsh     5,030
        ior     6,5
        movei   3,2
        jrst    vfs_pid_name_store

vfs_pid_name_lt10:
        move    6,2
        addi    6,020
        lsh     6,036
        movei   3,1

vfs_pid_name_store:
        movem   3,(7)
        movem   6,1(7)
        setzm   2(7)
        setzm   3(7)
        setzm   4(7)
        movei   1,0
        popj    17,

vfs_pid_name_fail:
        seto    1,
        popj    17,

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
        seto    1,
        popj    17,

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

; int vfs_v1_name_get_pid(const struct vfs_v1_name *name,
;     unsigned int *valuep)
        .globl  vfs_v1_name_get_pid
vfs_v1_name_get_pid:
        jumpe   1,vfs_name_pid_fail
        jumpe   2,vfs_name_pid_fail
        move    3,(1)                   ; character count
        caige   3,1
        jrst    vfs_name_pid_fail
        caile   3,3
        jrst    vfs_name_pid_fail
        move    6,[POINT 6,0]
        movei   4,1(1)
        hrr     6,4
        movei   5,0                     ; accumulated value
vfs_name_pid_loop:
        ildb    7,6
        caige   7,020
        jrst    vfs_name_pid_fail
        caile   7,031
        jrst    vfs_name_pid_fail
        imuli   5,012
        subi    7,020
        add     5,7
        sojg    3,vfs_name_pid_loop
        caile   5,0377
        jrst    vfs_name_pid_fail
        movem   5,(2)
        movei   1,0
        popj    17,
vfs_name_pid_fail:
        seto    1,
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
        movei   1,0
        popj    17,
vfs_sixchar_fail:
        seto    1,
        popj    17,
