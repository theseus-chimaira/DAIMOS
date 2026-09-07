; vfs_pdp10.s -- compact resident VFS primitives for PDP-6/PDP-10.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1


; Compact wrappers around the shared C create helper.  The fifth helper
; argument (nodep) is passed in one stack word by the PDP-10 C ABI.
        .globl  vfs_create_op
        .globl  vfs_create
vfs_create:
        movei   5,6                    ; FS_MRES_OP_CREATE
        jrst    vfs_create_common

        .globl  vfs_mkdir
vfs_mkdir:
        movei   5,7                    ; FS_MRES_OP_MKDIR
vfs_create_common:
        push    17,4
        move    4,3
        move    3,2
        move    2,1
        move    1,5
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
        jrst    pdp10_ret_zero

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
        jumpe   4,pdp10_ret_neg1
        jumpl   2,pdp10_ret_neg1
        caile   2,6
        jrst    pdp10_ret_neg1
        jumpl   3,pdp10_ret_zero
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
        jrst    pdp10_ret_zero
        movei   5,012                  ; LF
        movem   5,(4)
        movei   1,1
        popj    17,

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
        jrst    pdp10_ret_zero


; MRES-backed VFS leaf operations.  Keeping these here avoids the compiler's
; callee-save frame around a short request block, while preserving the generic
; fs_provider_call ABI used by every optional filesystem.
        .globl  fs_provider_call
        .globl  devicefs_readdir
        .globl  procfs_readdir
        .globl  devicefs_stat
        .globl  procfs_stat

        .globl  vfs_readdir_raw
vfs_readdir_raw:
        move    4,1
        lsh     4,-036
        andi    4,077
        cain    4,2
        jrst    devicefs_readdir
        cain    4,3
        jrst    procfs_readdir
        add     17,[4,,4]
        movei   5,2                    ; FS_MRES_OP_READDIR
        movem   5,-3(17)
        movem   1,-2(17)
        movem   2,-1(17)
        movem   3,(17)
        move    1,4
        movei   2,-3(17)
        pushj   17,fs_provider_call
        sub     17,[4,,4]
        popj    17,

        .globl  vfs_stat
vfs_stat:
        move    3,1
        lsh     3,-036
        andi    3,077
        cain    3,2
        jrst    devicefs_stat
        cain    3,3
        jrst    procfs_stat
        add     17,[3,,3]
        movei   4,3                    ; FS_MRES_OP_STAT
        movem   4,-2(17)
        movem   1,-1(17)
        movem   2,(17)
        move    1,3
        movei   2,-2(17)
        pushj   17,fs_provider_call
        sub     17,[3,,3]
        popj    17,

        .globl  vfs_unlink
vfs_unlink:
        add     17,[3,,3]
        movei   3,011                  ; FS_MRES_OP_UNLINK
        movem   3,-2(17)
        movem   1,-1(17)
        movem   2,(17)
        pushj   17,vfs_readonly
        jumpn   1,vfs_mutate3_ro
        jrst    vfs_mutate3_call

        .globl  vfs_truncate
vfs_truncate:
        add     17,[4,,4]
        movei   4,013                  ; FS_MRES_OP_TRUNCATE
        movem   4,-3(17)
        movem   1,-2(17)
        movem   2,-1(17)
        movem   3,(17)
        pushj   17,vfs_readonly
        jumpn   1,vfs_truncate_ro
        move    1,-2(17)
        lsh     1,-036
        andi    1,077
        movei   2,-3(17)
        pushj   17,fs_provider_call
        sub     17,[4,,4]
        popj    17,
vfs_truncate_ro:
        sub     17,[4,,4]
        jrst    pdp10_ret_neg1

        .globl  vfs_chmod
vfs_chmod:
        add     17,[3,,3]
        movei   3,014                  ; FS_MRES_OP_CHMOD
        movem   3,-2(17)
        movem   1,-1(17)
        movem   2,(17)
        pushj   17,vfs_readonly
        jumpn   1,vfs_mutate3_ro
vfs_mutate3_call:
        move    1,-1(17)
        lsh     1,-036
        andi    1,077
        movei   2,-2(17)
        pushj   17,fs_provider_call
        sub     17,[3,,3]
        popj    17,
vfs_mutate3_ro:
        sub     17,[3,,3]
        jrst    pdp10_ret_neg1

        .globl  vfs_read_words
vfs_read_words:
        move    5,1
        lsh     1,-036
        andi    1,077
        add     17,[5,,5]
        movei   6,015                  ; FS_MRES_OP_READ_WORDS
        movem   6,-4(17)
        movem   5,-3(17)
        movem   2,-2(17)
        movem   3,-1(17)
        movem   4,(17)
        movei   2,-4(17)
        pushj   17,fs_provider_call
        sub     17,[5,,5]
        popj    17,

        .globl  vfs_write_words
vfs_write_words:
        move    5,-1(17)               ; C arg 5: size_chars
        add     17,[6,,6]
        movei   6,016                  ; FS_MRES_OP_WRITE_WORDS
        movem   6,-5(17)
        movem   1,-4(17)
        movem   2,-3(17)
        movem   3,-2(17)
        movem   4,-1(17)
        movem   5,(17)
        pushj   17,vfs_readonly
        jumpn   1,vfs_write_words_ro
        move    1,-4(17)
        lsh     1,-036
        andi    1,077
        movei   2,-5(17)
        pushj   17,fs_provider_call
        sub     17,[6,,6]
        popj    17,
vfs_write_words_ro:
        sub     17,[6,,6]
        jrst    pdp10_ret_neg1

        .globl  vfs_sync
vfs_sync:
        move    4,1
        lsh     1,-036
        andi    1,077
        cail    1,5                    ; DTFS_PROVIDER
        cail    1,7                    ; one past D6FS_PROVIDER
        jrst    pdp10_ret_zero
        add     17,[2,,2]
        movei   3,017                  ; FS_MRES_OP_SYNC
        movem   3,-1(17)
        movem   4,(17)
        movei   2,-1(17)
        pushj   17,fs_provider_call
        sub     17,[2,,2]
        popj    17,

; One 128-word filesystem transfer workspace.  Filesystem providers serialize
; through the resident VFS entry path, so D6FS and DTFS must not each reserve
; a private full-block transfer buffer.
        .bss
        .globl  fs_block_workspace
fs_block_workspace:
        .block  0200
        .text
