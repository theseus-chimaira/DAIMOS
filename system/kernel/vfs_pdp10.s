; vfs_pdp10.s -- compact resident VFS primitives for PDP-6/PDP-10.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1


; Compact wrappers around the shared C create helper.  The fifth helper
; argument (nodep) is passed in one stack word by the PDP-10 C ABI.
        .globl  vfs_create_op
        .globl  vfs_create
vfs_create:
        push    17,4
        move    4,3
        move    3,2
        move    2,1
        movei   1,6                    ; FS_MRES_OP_CREATE
        pushj   17,vfs_create_op
        sub     17,[1,,1]
        popj    17,

        .globl  vfs_mkdir
vfs_mkdir:
        push    17,4
        move    4,3
        move    3,2
        move    2,1
        movei   1,7                    ; FS_MRES_OP_MKDIR
        pushj   17,vfs_create_op
        sub     17,[1,,1]
        popj    17,

        .globl  vfs_name_valid
; int vfs_name_valid(const struct vfs_name *name)
; Common filesystem namespace rule: non-empty SIXBIT names fit VFS_NAME_WORDS.
vfs_name_valid:
        jumpe   1,vfs_name_valid_fail
        move    2,(1)
        jumple  2,vfs_name_valid_fail
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
        ; Current callers always pass a valid vfs_name pointer.
        camn    3,(1)
        came    2,1(1)
        tdza    1,1
        movei   1,1
        popj    17,

; int vfs_sixbit_readchar(kword_t word, unsigned int nchars, kword_t off,
;     unsigned int *chp)
        .globl  vfs_sixbit_readchar
vfs_sixbit_readchar:
        jumpe   4,vfs_sixchar_fail
        jumpl   2,vfs_sixchar_fail
        caile   2,6
        jrst    vfs_sixchar_fail
        jumpl   3,vfs_sixchar_eof
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
