; vfs_pdp10.s -- compact resident VFS primitives for PDP-6/PDP-10.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_one
        .globl  pdp10_ret_neg1
        .globl  pdp10_ret_busy


; Compact namespace operations.  These use the permanent register-provider
; ABI directly and keep only values that must survive a provider call on the
; PDP-6 stack.
        .globl  vfs_mount_target
        .globl  vfs_mount_root
        .globl  vfs_namespace_root
        .globl  devicefs_lookup
        .globl  procfs_lookup

; int vfs_lookup(dir, name, nodep)
        .globl  vfs_lookup
vfs_lookup:
        jumpe   2,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        came    1,vfs_namespace_root
        jrst    vfs_lookup_provider
        move    4,(2)                  ; name chars
        caie    4,6
        jrst    vfs_lookup_proc
        move    4,1(2)
        camn    4,[-0333211263433]     ; DEVICE
        jrst    vfs_lookup_device_root
vfs_lookup_proc:
        move    4,(2)
        caie    4,4
        jrst    vfs_lookup_provider
        move    4,1(2)
        camn    4,[-0171520350000]     ; PROC
        jrst    vfs_lookup_proc_root

vfs_lookup_provider:
        push    17,1                   ; original directory
        push    17,3                   ; output pointer
        ldb     7,[POINT 6,1,5]
        cain    7,2
        jrst    vfs_lookup_device
        cain    7,3
        jrst    vfs_lookup_procfs
        movei   6,1                    ; FS_MRES_OP_LOOKUP
        pushj   17,fs_provider_reg_call
        jrst    vfs_lookup_return
vfs_lookup_device:
        pushj   17,devicefs_lookup
        jrst    vfs_lookup_return
vfs_lookup_procfs:
        pushj   17,procfs_lookup
vfs_lookup_return:
        jumpn   1,vfs_lookup_pop
        move    5,(17)                 ; output pointer
        move    4,(5)                  ; returned provider-local node
        tlz     4,07700                ; clear mount-id bits
        move    6,-1(17)               ; original directory
        and     6,[07700000000]
        ior     4,6                    ; inherit directory mount id
        setz    6,
vfs_lookup_mount_loop:
        move    7,vfs_mount_target(6)
        camn    7,4
        move    4,vfs_mount_root(6)
        addi    6,1
        caige   6,4
        jrst    vfs_lookup_mount_loop
        movem   4,(5)
vfs_lookup_pop:
        sub     17,[2,,2]
        popj    17,

vfs_lookup_device_root:
        movsi   4,020001               ; DEVICEFS root
        movem   4,(3)
        jrst    pdp10_ret_zero
vfs_lookup_proc_root:
        movsi   4,030001               ; PROCFS root
        movem   4,(3)
        jrst    pdp10_ret_zero

; int vfs_readdir(dir, off, ent)
        .globl  vfs_readdir
vfs_readdir:
        jumpe   3,pdp10_ret_neg1
        push    17,1                   ; dir
        push    17,2                   ; requested offset
        push    17,3                   ; dirent
        push    17,[0]                 ; underlying root-entry count
        pushj   17,vfs_readdir_raw
        jumpn   1,vfs_readdir_done
        move    4,-3(17)
        came    4,vfs_namespace_root
        jrst    vfs_readdir_done
vfs_readdir_count:
        move    1,-3(17)
        move    2,(17)
        move    3,-1(17)
        pushj   17,vfs_readdir_raw
        jumple  1,vfs_readdir_extra
        aos     (17)
        jrst    vfs_readdir_count
vfs_readdir_extra:
        move    4,-2(17)               ; requested offset
        came    4,(17)
        jrst    vfs_readdir_try_proc
        movei   4,6
        move    5,[-0333211263433]     ; DEVICE
        jrst    vfs_readdir_emit
vfs_readdir_try_proc:
        move    5,(17)
        addi    5,1
        came    4,5
        jrst    vfs_readdir_zero
        movei   4,4
        move    5,[-0171520350000]     ; PROC
vfs_readdir_emit:
        move    3,-1(17)
        movem   4,(3)
        movem   5,1(3)
        setzm   2(3)
        setzm   3(3)
        setzm   4(3)
        movei   4,1                    ; VFS_TYPE_DIR
        movem   4,5(3)
        movei   1,1
        jrst    vfs_readdir_done
vfs_readdir_zero:
        setz    1,
vfs_readdir_done:
        sub     17,[4,,4]
        popj    17,

; Common parent lookup after mount-root crossing.  AC1=node, AC2=parentp.
vfs_parent_raw_asm:
        ldb     7,[POINT 6,1,5]
        cain    7,2
        jrst    vfs_parent_namespace
        cain    7,3
        jrst    vfs_parent_proc
        push    17,1                   ; original node for mount inheritance
        push    17,2                   ; parent output pointer
        setz    3,
        movei   6,4                    ; FS_MRES_OP_PARENT
        pushj   17,fs_provider_reg_call
        jumpn   1,vfs_parent_pop
        move    5,(17)
        move    4,(5)
        tlz     4,07700
        move    6,-1(17)
        and     6,[07700000000]
        ior     4,6
        movem   4,(5)
vfs_parent_pop:
        sub     17,[2,,2]
        popj    17,
vfs_parent_namespace:
        move    4,vfs_namespace_root
        movem   4,(2)
        jrst    pdp10_ret_zero
vfs_parent_proc:
        ldb     4,[POINT 6,1,17]
        caie    4,1                    ; PROCFS_KIND_ROOT
        jrst    vfs_parent_proc_slot
        move    4,vfs_namespace_root
        movem   4,(2)
        jrst    pdp10_ret_zero
vfs_parent_proc_slot:
        caie    4,2                    ; PROCFS_KIND_PROC
        jrst    pdp10_ret_neg1
        movsi   4,030001               ; PROCFS root
        movem   4,(2)
        jrst    pdp10_ret_zero

; int vfs_parent(node, parentp)
        .globl  vfs_parent
vfs_parent:
        jumpe   2,pdp10_ret_neg1
        came    1,vfs_namespace_root
        jrst    vfs_parent_mount
        movem   1,(2)
        jrst    pdp10_ret_zero
vfs_parent_mount:
        ldb     3,[POINT 6,1,11]
        subi    3,1
        jumpl   3,vfs_parent_raw_asm
        caige   3,4
        jrst    vfs_parent_mount_check
        jrst    vfs_parent_raw_asm
vfs_parent_mount_check:
        move    4,vfs_mount_root(3)
        came    4,1
        jrst    vfs_parent_raw_asm
        move    1,vfs_mount_target(3)
        jrst    vfs_parent_raw_asm

; int vfs_parent_name(node, parentp, namep)
        .globl  vfs_parent_name
vfs_parent_name:
        jumpe   2,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        came    1,vfs_namespace_root
        jrst    vfs_parent_name_mount
        jrst    pdp10_ret_neg1
vfs_parent_name_mount:
        ldb     4,[POINT 6,1,11]
        subi    4,1
        jumpl   4,vfs_parent_name_call
        caige   4,4
        jrst    vfs_parent_name_check
        jrst    vfs_parent_name_call
vfs_parent_name_check:
        move    5,vfs_mount_root(4)
        came    5,1
        jrst    vfs_parent_name_call
        move    1,vfs_mount_target(4)
vfs_parent_name_call:
        push    17,1                   ; node after mount crossing
        push    17,2                   ; parent output pointer
        ldb     7,[POINT 6,1,5]
        movei   6,5                    ; FS_MRES_OP_PARENT_NAME
        pushj   17,fs_provider_reg_call
        jumpn   1,vfs_parent_name_pop
        move    5,(17)
        move    4,(5)
        tlz     4,07700
        move    6,-1(17)
        and     6,[07700000000]
        ior     4,6
        movem   4,(5)
vfs_parent_name_pop:
        sub     17,[2,,2]
        popj    17,


; Direct request-free mutation leaves.  The fifth C argument is at -1(17).
        .globl  fs_provider_reg_call
        .globl  pdp10_ret_neg2
        .globl  vfs_create_op
        .globl  vfs_create
vfs_create:
        movei   5,6                    ; FS_MRES_OP_CREATE
        jrst    vfs_create_common
        .globl  vfs_mkdir
vfs_mkdir:
        movei   5,7                    ; FS_MRES_OP_MKDIR
vfs_create_common:
        push    17,4                   ; nodep as C arg 5
        move    4,3
        move    3,2
        move    2,1
        move    1,5
        pushj   17,vfs_create_op
        sub     17,[1,,1]
        popj    17,

; int vfs_create_op(op, dir, name, mode, nodep)
vfs_create_op:
        skipn   5,-1(17)
        jrst    pdp10_ret_neg1
        ldb     7,[POINT 6,2,5]
        caie    1,7                    ; MKDIR
        jrst    vfs_create_policy
        cain    7,5                    ; DTFS has no directories
        jrst    pdp10_ret_neg2
vfs_create_policy:
        push    17,1
        push    17,2
        push    17,3
        push    17,4
        push    17,5
        move    1,2
        pushj   17,vfs_readonly
        jumpn   1,vfs_create_ro
        pop     17,5
        pop     17,4
        pop     17,3
        pop     17,2
        pop     17,6                   ; operation
        add     17,[3,,3]
        movem   5,(17)                 ; caller nodep
        movem   2,-1(17)               ; dir for mount inheritance
        setzm   -2(17)                 ; provider result node
        ldb     7,[POINT 6,2,5]
        move    1,2                    ; provider a = dir
        move    2,3                    ; provider b = name
        move    3,4                    ; provider c = mode
        movei   4,-2(17)               ; provider d = result nodep
        pushj   17,fs_provider_reg_call
        jumpn   1,vfs_create_done
        move    3,-2(17)
        tlz     3,07700
        move    4,-1(17)
        and     4,[07700000000]
        ior     3,4
        move    4,(17)
        movem   3,(4)
vfs_create_done:
        sub     17,[3,,3]
        popj    17,
vfs_create_ro:
        sub     17,[5,,5]
        jrst    pdp10_ret_neg1

; int vfs_symlink(dir, name, target, target_chars, nodep)
        .globl  vfs_symlink
vfs_symlink:
        skipn   5,-1(17)
        jrst    pdp10_ret_neg1
        push    17,1
        push    17,2
        push    17,3
        push    17,4
        push    17,5
        pushj   17,vfs_readonly
        jumpn   1,vfs_symlink_ro
        pop     17,5
        pop     17,4
        pop     17,3
        pop     17,2
        pop     17,1
        add     17,[3,,3]
        movem   5,(17)
        movem   1,-1(17)
        setzm   -2(17)
        move    5,4                    ; request e normally nodep; use result ptr
        movei   5,-2(17)
        ldb     7,[POINT 6,1,5]
        movei   6,010                  ; FS_MRES_OP_SYMLINK
        pushj   17,fs_provider_reg_call
        jumpn   1,vfs_symlink_done
        move    3,-2(17)
        tlz     3,07700
        move    4,-1(17)
        and     4,[07700000000]
        ior     3,4
        move    4,(17)
        movem   3,(4)
vfs_symlink_done:
        sub     17,[3,,3]
        popj    17,
vfs_symlink_ro:
        sub     17,[5,,5]
        jrst    pdp10_ret_neg1

; int vfs_rename(olddir, oldname, newdir, newname)
        .globl  vfs_rename
vfs_rename:
        push    17,1
        push    17,2
        push    17,3
        push    17,4
        pushj   17,vfs_readonly
        jumpn   1,vfs_rename_ro
        move    1,-1(17)               ; newdir
        pushj   17,vfs_readonly
        jumpn   1,vfs_rename_ro
        pop     17,4
        pop     17,3
        pop     17,2
        pop     17,1
        ldb     7,[POINT 6,1,5]
        ldb     5,[POINT 6,3,5]
        came    7,5
        jrst    pdp10_ret_neg1
        ldb     5,[POINT 6,1,11]
        ldb     0,[POINT 6,3,11]
        came    5,0
        jrst    pdp10_ret_neg1
        movei   6,012                  ; FS_MRES_OP_RENAME
        jrst    fs_provider_reg_call
vfs_rename_ro:
        sub     17,[4,,4]
        jrst    pdp10_ret_neg1

        .globl  vfs_name_valid
; int vfs_name_valid(const struct vfs_name *name)
; Common filesystem namespace rule: non-empty SIXBIT names fit VFS_NAME_WORDS.
vfs_name_valid:
        jumpe   1,pdp10_ret_zero
        move    2,(1)
        jumple  2,pdp10_ret_zero
        caile   2,030                    ; VFS_NAME_MAX_CHARS = 24
        jrst    pdp10_ret_zero
        jrst    pdp10_ret_one

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
        jrst    pdp10_ret_busy          ; VFS_DEVICE_IO = -3

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

; Compact mount policy.  The four-entry namespace table is a bounded PDP-6
; structure, so keeping the policy in fixed assembly avoids the C callee-save
; frames and unsigned comparison glue without changing the VFS ABI.
        .globl  vfs_readonly
vfs_readonly:
        ldb     1,[POINT 6,1,11]        ; mount id
        subi    1,1
        jumpl   1,pdp10_ret_zero
        caige   1,4
        jrst    vfs_readonly_slot
        jrst    pdp10_ret_zero
vfs_readonly_slot:
        skipn   vfs_mount_root(1)
        jrst    pdp10_ret_zero
        move    2,vfs_mount_ro
        movn    3,1
        lsh     2,0(3)
        andi    2,1
        move    1,2
        popj    17,

; int vfs_mount(target, provider, kind, index, flags, rootp)
        .globl  vfs_mount
vfs_mount:
        skipn   -2(17)                  ; rootp
        jrst    pdp10_ret_neg1
        jumpe   2,pdp10_ret_neg1
        cail    2,0
        cail    2,0100                  ; provider <= 077
        jrst    pdp10_ret_neg1
        cail    3,0
        cail    3,0100                  ; local kind <= 077
        jrst    pdp10_ret_neg1
        cail    4,0
        caml    4,[01000000]            ; index <= 0777777
        jrst    pdp10_ret_neg1
        move    5,-1(17)                ; flags
        cail    5,0
        cail    5,2                     ; RW or RDONLY only
        jrst    pdp10_ret_neg1
        jumpn   1,vfs_mount_check_target
        skipe   vfs_namespace_root
        jrst    pdp10_ret_neg1
        jrst    vfs_mount_find

vfs_mount_check_target:
        ; Preserve the four register arguments around vfs_stat().
        push    17,1
        push    17,2
        push    17,3
        push    17,4
        add     17,[4,,4]               ; struct vfs_stat
        movei   2,-3(17)
        move    1,-7(17)
        pushj   17,vfs_stat
        jumpn   1,vfs_mount_stat_fail
        move    6,-3(17)                ; st.type
        move    1,-7(17)
        move    2,-6(17)
        move    3,-5(17)
        move    4,-4(17)
        sub     17,[010,,010]
        caie    6,1                     ; VFS_TYPE_DIR
        jrst    pdp10_ret_neg1
        jrst    vfs_mount_find
vfs_mount_stat_fail:
        sub     17,[010,,010]
        jrst    pdp10_ret_neg1

vfs_mount_find:
        setz    7,
vfs_mount_find_loop:
        skipn   vfs_mount_root(7)
        jrst    vfs_mount_found
        move    6,vfs_mount_target(7)
        camn    6,1
        jrst    pdp10_ret_neg1
        addi    7,1
        caige   7,4
        jrst    vfs_mount_find_loop
        jrst    pdp10_ret_neg1

vfs_mount_found:
        ; root = provider:6 | mount/kind:12 | index:18.
        move    5,2
        lsh     5,036
        move    6,7
        addi    6,1
        lsh     6,6
        ior     6,3
        lsh     6,022
        ior     5,6
        ior     5,4
        movem   1,vfs_mount_target(7)
        movem   5,vfs_mount_root(7)
        move    6,-1(17)
        caie    6,1
        jrst    vfs_mount_clear_ro
        movei   6,1
        lsh     6,0(7)
        iorm    6,vfs_mount_ro
        jrst    vfs_mount_store
vfs_mount_clear_ro:
        hrroi   6,0777776
        rot     6,(7)
        andm    6,vfs_mount_ro
vfs_mount_store:
        move    6,-2(17)
        movem   5,(6)
        jumpn   1,pdp10_ret_zero
        movem   5,vfs_namespace_root
        jrst    pdp10_ret_zero

; int vfs_unmount(root)
        .globl  vfs_unmount
vfs_unmount:
        ldb     2,[POINT 6,1,11]
        subi    2,1
        jumpl   2,pdp10_ret_neg1
        caige   2,4
        jrst    vfs_unmount_slot
        jrst    pdp10_ret_neg1
vfs_unmount_slot:
        move    3,vfs_mount_root(2)
        came    3,1
        jrst    pdp10_ret_neg1
        push    17,1                    ; root
        push    17,2                    ; slot
        pushj   17,vfs_sync
        jumpn   1,vfs_unmount_fail
        move    2,(17)
        move    1,-1(17)
        move    3,1
        lsh     3,-036
        caie    3,6                     ; D6FS_PROVIDER
        jrst    vfs_unmount_after_prepare
        movei   6,020                   ; FS_MRES_OP_PREPARE_UNMOUNT
        movei   7,6
        pushj   17,fs_provider_reg_call
        jumpn   1,vfs_unmount_fail
        move    2,(17)
        move    1,-1(17)
vfs_unmount_after_prepare:
        came    1,vfs_namespace_root
        jrst    vfs_unmount_unlock
        move    3,vfs_mount_target(2)
        movem   3,vfs_namespace_root
vfs_unmount_unlock:
        move    1,2
        addi    1,1
        pushj   17,file_unlock_mount
        move    2,(17)
        setzm   vfs_mount_target(2)
        setzm   vfs_mount_root(2)
        hrroi   3,0777776
        rot     3,(2)
        andm    3,vfs_mount_ro
        sub     17,[2,,2]
        jrst    pdp10_ret_zero
vfs_unmount_fail:
        sub     17,[2,,2]
        jrst    pdp10_ret_neg1

        .bss
        .globl  vfs_mount_target
vfs_mount_target:
        .block  4
        .globl  vfs_mount_root
vfs_mount_root:
        .block  4
        .text
