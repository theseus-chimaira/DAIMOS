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
        jumpe   1,pdp10_ret_zero
        move    2,(1)
        jumple  2,pdp10_ret_zero
        caile   2,030                    ; VFS_NAME_MAX_CHARS = 24
        jrst    pdp10_ret_zero
        movei   1,1
        popj    17,

; int vfs_name_is6(const struct vfs_name *name, kword_t word,
;     unsigned int chars)
        .globl  vfs_name_is6
vfs_name_is6:
        ; Current callers always pass a valid vfs_name pointer.
        camn    3,(1)
        came    2,1(1)
        jrst    pdp10_ret_zero
        jrst    pdp10_ret_one

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
vfs_sixchar_store:
        movem   5,(4)
        jrst    pdp10_ret_one
vfs_sixchar_tail:
        came    3,2
        jrst    vfs_sixchar_lf
        movei   5,015                  ; CR
        jrst    vfs_sixchar_store
vfs_sixchar_lf:
        addi    2,1
        came    3,2
        jrst    pdp10_ret_zero
        movei   5,012                  ; LF
        jrst    vfs_sixchar_store

; unsigned int vfs_sixbit_name_chars(words, maxchars)
; Return the last nonzero character position in a packed SIXBIT name.
        .globl  vfs_sixbit_name_chars
vfs_sixbit_name_chars:
        jumpe   1,pdp10_ret_zero
        jumpe   2,pdp10_ret_zero
        caile   2,030                    ; VFS names are at most 24 chars
        jrst    pdp10_ret_zero
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


; MRES-backed VFS leaf operations.  AC6 carries the FS_MRES operation and AC7
; the dynamic provider.  Tail-calling the resident register bridge avoids an
; executive programmed-operator trap while retaining the compact request-free
; ABI and the movable-provider indirection.
        .globl  devicefs_readdir
        .globl  procfs_readdir
        .globl  devicefs_stat
        .globl  procfs_stat

        .globl  vfs_readdir_raw
vfs_readdir_raw:
        ldb     7,[POINT 6,1,5]
        cain    7,2
        jrst    devicefs_readdir
        cain    7,3
        jrst    procfs_readdir
        movei   6,2                    ; FS_MRES_OP_READDIR
        jrst    fs_provider_reg_call

        .globl  vfs_stat
vfs_stat:
        ldb     7,[POINT 6,1,5]
        cain    7,2
        jrst    devicefs_stat
        cain    7,3
        jrst    procfs_stat
        movei   6,3                    ; FS_MRES_OP_STAT
        jrst    fs_provider_reg_call

        .globl  vfs_unlink
vfs_unlink:
        push    17,1
        push    17,2
        pushj   17,vfs_readonly
        jumpn   1,vfs_mutate2_ro
        pop     17,2
        pop     17,1
        ldb     7,[POINT 6,1,5]
        movei   6,11                   ; FS_MRES_OP_UNLINK
        jrst    fs_provider_reg_call

        .globl  vfs_truncate
vfs_truncate:
        push    17,1
        push    17,2
        push    17,3
        pushj   17,vfs_readonly
        jumpn   1,vfs_mutate3_ro
        pop     17,3
        pop     17,2
        pop     17,1
        ldb     7,[POINT 6,1,5]
        movei   6,13                   ; FS_MRES_OP_TRUNCATE
        jrst    fs_provider_reg_call

        .globl  vfs_chmod
vfs_chmod:
        push    17,1
        push    17,2
        pushj   17,vfs_readonly
        jumpn   1,vfs_mutate2_ro
        pop     17,2
        pop     17,1
        ldb     7,[POINT 6,1,5]
        movei   6,14                   ; FS_MRES_OP_CHMOD
        jrst    fs_provider_reg_call
vfs_mutate3_ro:
        sub     17,[1,,1]
vfs_mutate2_ro:
        sub     17,[2,,2]
        jrst    pdp10_ret_neg1

        .globl  vfs_read_words
vfs_read_words:
        ldb     7,[POINT 6,1,5]
        movei   6,15                   ; FS_MRES_OP_READ_WORDS
        jrst    fs_provider_reg_call

        .globl  vfs_write_words
vfs_write_words:
        push    17,1
        push    17,2
        push    17,3
        push    17,4
        pushj   17,vfs_readonly
        jumpn   1,vfs_write_words_ro
        pop     17,4
        pop     17,3
        pop     17,2
        pop     17,1
        move    5,-1(17)               ; request e / C arg 5: size_chars
        ldb     7,[POINT 6,1,5]
        movei   6,16                   ; FS_MRES_OP_WRITE_WORDS
        jrst    fs_provider_reg_call
vfs_write_words_ro:
        sub     17,[4,,4]
        jrst    pdp10_ret_neg1

        .globl  vfs_sync
vfs_sync:
        ldb     7,[POINT 6,1,5]
        cail    7,5                    ; DTFS_PROVIDER
        cail    7,7                    ; one past D6FS_PROVIDER
        jrst    pdp10_ret_zero
        movei   6,17                   ; FS_MRES_OP_SYNC
        jrst    fs_provider_reg_call

; One 128-word filesystem transfer workspace.  Filesystem providers serialize
; through the resident VFS entry path, so D6FS and DTFS must not each reserve
; a private full-block transfer buffer.
        .bss
        .globl  fs_block_workspace
fs_block_workspace:
        .block  0200
        .text

; Character I/O is deliberately handwritten.  The C versions need large
; callee-save frames around the short stat/read/write sequence.  These leaf
; wrappers use only caller-scratch ACs and ordinary PDP-6 stack operations.
        .globl  procfs_readchar
        .globl  devicefs_readchar

; int vfs_readchar(vnode_t node, kword_t off, unsigned int *chp)
        .globl  vfs_readchar
vfs_readchar:
        jumpe   3,pdp10_ret_neg1
        ldb     4,[POINT 6,1,5]
        cain    4,3
        jrst    procfs_readchar
        cain    4,2
        jrst    devicefs_readchar

        add     17,[010,,010]
        movem   1,-7(17)               ; node
        movem   2,-6(17)               ; character offset
        movem   3,-5(17)               ; result pointer
        movei   2,-4(17)               ; struct vfs_stat
        pushj   17,vfs_stat
        jumpn   1,vfs_readchar_fail
        move    1,-4(17)               ; st.type
        caie    1,2                    ; VFS_TYPE_REG
        jrst    vfs_readchar_fail

        ; Compare unsigned character offset with st.size_chars.
        move    2,-6(17)
        tlc     2,0400000
        move    3,-2(17)
        tlc     3,0400000
        caml    2,3
        jrst    vfs_readchar_eof

        move    2,-6(17)
        move    4,2
        andi    4,3                    ; quarter-word number
        lsh     2,-2                   ; word offset
        move    1,-7(17)
        movei   3,(17)                 ; one-word buffer
        movei   5,4                    ; preserve bi across call in stack
        movem   4,-1(17)
        movei   4,1
        pushj   17,vfs_read_words
        caie    1,1
        jrst    vfs_readchar_fail

        move    4,-1(17)
        move    5,4
        lsh     5,3
        add     5,4                    ; 9 * bi
        move    6,(17)
        lsh     6,-033(5)              ; right by 27 - 9*bi
        andi    6,0777
        move    3,-5(17)
        movem   6,(3)
        movei   1,1
        jrst    vfs_readchar_done
vfs_readchar_eof:
        setz    1,
        jrst    vfs_readchar_done
vfs_readchar_fail:
        seto    1,
vfs_readchar_done:
        sub     17,[010,,010]
        popj    17,

; int vfs_writechar(vnode_t node, kword_t off, unsigned int ch)
        .globl  vfs_writechar
vfs_writechar:
        ldb     4,[POINT 6,1,5]
        caie    4,2                    ; DEVICEFS_PROVIDER
        jrst    vfs_writechar_regular
        ldb     4,[POINT 6,1,17]       ; VFS local kind
        caie    4,2                    ; DEVICEFS_KIND_DEVICE
        jrst    vfs_writechar_regular
        hrroi   1,0777775              ; VFS_DEVICE_IO = -3
        popj    17,

vfs_writechar_regular:
        add     17,[010,,010]
        movem   1,-7(17)               ; node
        movem   2,-6(17)               ; character offset
        movem   3,-5(17)               ; character
        movei   2,-4(17)               ; struct vfs_stat
        pushj   17,vfs_stat
        jumpn   1,vfs_writechar_fail
        move    1,-4(17)
        caie    1,2                    ; VFS_TYPE_REG
        jrst    vfs_writechar_fail

        move    2,-6(17)
        addi    2,1
        movem   2,(17)                 ; end_chars; later fifth argument
        addi    2,3
        lsh     2,-2                   ; ceil(end_chars / 4)
        move    3,-1(17)               ; st.size_words
        camle   2,3
        jrst    vfs_writechar_grow
vfs_writechar_after_grow:
        move    2,-6(17)
        lsh     2,-2                   ; word offset
        movem   2,-1(17)
        setzm   -4(17)                 ; read beyond EOF as zero word
        move    1,-7(17)
        movei   3,-4(17)
        movei   4,1
        pushj   17,vfs_read_words

        move    3,-6(17)
        andi    3,3                    ; quarter-word number
        move    4,3
        lsh     4,3
        add     4,3                    ; 9 * bi
        movei   5,033
        sub     5,4                    ; shift = 27 - 9*bi
        movei   4,0777
        lsh     4,0(5)
        andca   4,-4(17)
        move    3,-5(17)
        andi    3,0777
        lsh     3,0(5)
        ior     4,3
        movem   4,-4(17)

        move    1,-7(17)
        move    2,-1(17)
        movei   3,-4(17)
        movei   4,1
        pushj   17,vfs_write_words
        caie    1,1
        jrst    vfs_writechar_fail
        setz    1,
        jrst    vfs_writechar_done

vfs_writechar_grow:
        move    1,-7(17)
        move    3,(17)                  ; end_chars
        pushj   17,vfs_truncate
        jumpn   1,vfs_writechar_fail
        jrst    vfs_writechar_after_grow

vfs_writechar_fail:
        seto    1,
vfs_writechar_done:
        sub     17,[010,,010]
        popj    17,
