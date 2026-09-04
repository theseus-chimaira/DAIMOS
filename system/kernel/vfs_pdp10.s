; vfs_pdp10.s -- compact resident VFS primitives for PDP-6/PDP-10.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1

; int vfs_name_set6(struct vfs_name *name, kword_t word,
;     unsigned int chars)
        .globl  vfs_name_set6
vfs_name_set6:
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
        jrst    pdp10_ret_neg1

        .globl  vfs_name_valid
; int vfs_name_valid(const struct vfs_name *name)
; Common filesystem namespace rule: non-empty SIXBIT names fit VFS_NAME_WORDS.
vfs_name_valid:
        jumpe   1,vfs_name_valid_fail
        move    2,(1)
        jumpge  2,vfs_name_valid_small
        jrst    vfs_name_valid_fail
vfs_name_valid_small:
        caige   2,1
        jrst    vfs_name_valid_fail
        caile   2,030                    ; VFS_NAME_MAX_CHARS = 24
        jrst    vfs_name_valid_fail
        movei   1,1
        popj    17,
vfs_name_valid_fail:
        setz    1,
        popj    17,

; int vfs_name_is6(const struct vfs_name *name, kword_t word,
;     unsigned int chars)
        .globl  vfs_name_is6
vfs_name_is6:
        jumpe   1,vfs_name_is6_fail
        camn    3,(1)
        came    2,1(1)
        jrst    vfs_name_is6_fail
        movei   1,1
        popj    17,
vfs_name_is6_fail:
        movei   1,0
        popj    17,

; int vfs_sixbit_readchar(kword_t word, unsigned int nchars, kword_t off,
;     unsigned int *chp)
        .globl  vfs_sixbit_readchar
vfs_sixbit_readchar:
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
        jrst    pdp10_ret_zero
vfs_sixchar_fail:
        jrst    pdp10_ret_neg1

; unsigned int vfs_sixbit_name_chars(words, maxchars)
; Return the last nonzero character position in a packed SIXBIT name.
        .globl  vfs_sixbit_name_chars
vfs_sixbit_name_chars:
        jumpe   1,vfs_name_chars_zero
        jumpe   2,vfs_name_chars_zero
        caile   2,030                    ; VFS names are at most 24 chars
        jrst    vfs_name_chars_zero
        move    3,[POINT 6,0]
        hrr     3,1
        setz    4,                       ; last nonzero position
        setz    5,                       ; current position
vfs_name_chars_loop:
        ildb    6,3
        addi    5,1
        jumpe   6,vfs_name_chars_next
        move    4,5
vfs_name_chars_next:
        came    5,2
        jrst    vfs_name_chars_loop
        move    1,4
        popj    17,
vfs_name_chars_zero:
        setz    1,
        popj    17,

; One 128-word filesystem transfer workspace.  Filesystem providers serialize
; through the resident VFS entry path, so D6FS and DTFS must not each reserve
; a private full-block transfer buffer.
        .bss
        .globl  fs_block_workspace
fs_block_workspace:
        .block  0200
        .text
