; vfs_pdp10.s -- compact resident VFS primitives for PDP-6/PDP-10.
        .text
        .globl  file_table
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
        .globl  mfsdev_lookup
        .globl  mfsproc_lookup
        .globl  pipe_fifo_mount_busy

; int vfs_lookup(dir, name, nodep)
        .globl  vfs_lookup
vfs_lookup:
        jumpe   2,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        camn    1,vfs_namespace_root
        jrst    vfs_lookup_root_names
        move    4,monitorfs_names+7    ; /MONITOR vnode
        came    1,4
        jrst    vfs_lookup_provider
        movei   4,monitorfs_names+014  ; PROCESSES, DOMAINS, DEVICES
        movei   0,3
        jrst    vfs_lookup_builtin_start
vfs_lookup_root_names:
        movei   4,monitorfs_names      ; DEV, MONITOR, PROC alias
        movei   0,3
vfs_lookup_builtin_start:
        move    5,(2)                  ; name chars
vfs_lookup_builtin_loop:
        came    5,(4)
        jrst    vfs_lookup_builtin_next
        move    6,1(2)
        came    6,1(4)
        jrst    vfs_lookup_builtin_next
        move    6,2(2)
        came    6,2(4)
        jrst    vfs_lookup_builtin_next
        move    4,3(4)                 ; vnode from shared name table
        jrst    vfs_lookup_root_store
vfs_lookup_builtin_next:
        addi    4,4
        sojg    0,vfs_lookup_builtin_loop
        jrst    vfs_lookup_provider

vfs_lookup_provider:
        push    17,1                   ; original directory
        push    17,3                   ; output pointer
        ldb     7,[POINT 6,1,5]
        cain    7,2
        jrst    vfs_lookup_device
        cain    7,3
        jrst    vfs_lookup_monitor_process
        movei   6,1                    ; FS_MRES_OP_LOOKUP
        pushj   17,fs_provider_reg_call
        jrst    vfs_lookup_return
vfs_lookup_device:
        pushj   17,mfsdev_lookup
        jrst    vfs_lookup_return
vfs_lookup_monitor_process:
        pushj   17,mfsproc_lookup
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

vfs_lookup_root_store:
        movem   4,(3)
        jrst    pdp10_ret_zero

; int vfs_readdir(dir, off, ent)
        .globl  vfs_readdir
vfs_readdir:
        jumpe   3,pdp10_ret_neg1
        move    4,monitorfs_names+7    ; /MONITOR
        came    1,4
        jrst    vfs_readdir_general
        cail    2,3
        jrst    pdp10_ret_zero
        move    4,2
        lsh     4,2
        addi    4,monitorfs_names+014
        jrst    vfs_readdir_table
vfs_readdir_general:
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
        sub     4,(17)                  ; built-in index after mounted entries
        jumpl   4,vfs_readdir_zero
        caige   4,2
        jrst    vfs_readdir_builtin
        jrst    vfs_readdir_zero
vfs_readdir_builtin:
        lsh     4,2                    ; four-word name-table record
        addi    4,monitorfs_names
        move    3,-1(17)
        pushj   17,vfs_readdir_table
        jrst    vfs_readdir_done
vfs_readdir_zero:
        setz    1,
vfs_readdir_done:
        sub     17,[4,,4]
        popj    17,

; AC4 -> static four-word MonitorFS name record, AC3 -> dirent.
vfs_readdir_table:
        move    5,(4)
        movem   5,(3)
        move    5,1(4)
        movem   5,1(3)
        move    5,2(4)
        movem   5,2(3)
        setzm   3(3)
        setzm   4(3)
        movei   5,1                    ; VFS_TYPE_DIR
        movem   5,5(3)
        jrst    pdp10_ret_one

; Common parent lookup after mount-root crossing.  AC1=node, AC2=parentp.
vfs_parent_raw_asm:
        ldb     7,[POINT 6,1,5]
        cain    7,2
        jrst    vfs_parent_device
        cain    7,3
        jrst    vfs_parent_proc
        push    17,1                   ; original node for mount inheritance
        push    17,2                   ; parent output pointer
        setz    3,
        movei   6,4                    ; FS_MRES_OP_PARENT
        pushj   17,fs_provider_reg_call
vfs_parent_return:
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
        jrst    vfs_parent_store
vfs_parent_device:
        ldb     4,[POINT 6,1,17]
        jumpe   4,vfs_parent_namespace ; /DEV
        cain    4,1                    ; /MONITOR/DEVICES
        jrst    vfs_parent_monitor
        caie    4,3                    ; state-view device directory
        jrst    pdp10_ret_neg1
        move    4,monitorfs_names+027  ; /MONITOR/DEVICES
        jrst    vfs_parent_store
vfs_parent_monitor:
        move    4,monitorfs_names+7    ; /MONITOR
        jrst    vfs_parent_store
vfs_parent_proc:
        ldb     4,[POINT 6,1,17]
        jumpe   4,vfs_parent_namespace ; /MONITOR
        cain    4,1                    ; PROC/DOMAIN root
        jrst    vfs_parent_monitor
        caie    4,2                    ; process/domain ID directory
        jrst    pdp10_ret_neg1
        movsi   4,030001               ; /MONITOR/PROCESSES
        trne    1,0400000
        tro     4,0400000              ; /MONITOR/DOMAIN
vfs_parent_store:
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
        cail   3,4
        jrst    vfs_parent_raw_asm
vfs_parent_mount_check:
        move    4,vfs_mount_root(3)
        came    4,1
        jrst    vfs_parent_raw_asm
        move    1,vfs_mount_target(3)
        jrst    vfs_parent_raw_asm

; int vfs_parent_name(node, parentp, namep)
; MonitorFS directories use this same component-name path as mounted providers,
; so getcwd needs no synthetic absolute-path formatter.
        .globl  vfs_parent_name
        .globl  mfsdev_names
vfs_parent_name:
        jumpe   2,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        camn    1,vfs_namespace_root
        jrst    pdp10_ret_neg1
        ldb     4,[POINT 6,1,11]
        subi    4,1
        jumpl   4,vfs_parent_name_dispatch
        cail    4,4
        jrst    vfs_parent_name_dispatch
        move    5,vfs_mount_root(4)
        camn    5,1
        move    1,vfs_mount_target(4)
vfs_parent_name_dispatch:
        ldb     7,[POINT 6,1,5]
        cain    7,2
        jrst    monitorfs_parent_name
        cain    7,3
        jrst    monitorfs_parent_name
        push    17,1
        push    17,2
        movei   6,5                    ; FS_MRES_OP_PARENT_NAME
        pushj   17,fs_provider_reg_call
        jrst    vfs_parent_return

; AC1=node, AC2=parentp, AC3=namep.  Only directories need a component name.
monitorfs_parent_name:
        push    17,1
        push    17,3
        pushj   17,vfs_parent_raw_asm
        jumpn   1,monitorfs_parent_name_fail
        move    6,-1(17)               ; original node
        move    7,(17)                  ; namep
        ldb     5,[POINT 6,6,5]
        cain    5,2
        jrst    monitorfs_parent_name_device

        ; Provider 3: MONITOR, PROC/DOMAIN, or an ID directory.
        ldb     4,[POINT 6,6,17]
        jumpe   4,monitorfs_name_monitor
        cain    4,1
        jrst    monitorfs_name_proc_domain
        caie    4,2
        jrst    monitorfs_parent_name_fail
        hrrz    1,6
        andi    1,0377
        pushj   17,mfsproc_format_slot
        move    7,(17)                 ; formatter uses AC7
        movem   1,(7)
        movem   2,1(7)
        jrst    monitorfs_parent_name_done
monitorfs_name_proc_domain:
        movei   4,monitorfs_names+014   ; PROCESSES record
        trne    6,0400000
        addi    4,4                    ; DOMAINS record
        jrst    monitorfs_name_record
monitorfs_name_monitor:
        movei   4,monitorfs_names+4    ; MONITOR record
        jrst    monitorfs_name_record

monitorfs_parent_name_device:
        ldb     4,[POINT 6,6,17]
        jumpe   4,monitorfs_name_dev
        cain    4,1
        jrst    monitorfs_name_devices
        caie    4,3
        jrst    monitorfs_parent_name_fail
        hrrz    4,6
        cail    4,023
        jrst    monitorfs_parent_name_fail
        skipn   5,mfsdev_names(4)
        jrst    monitorfs_parent_name_fail
        movem   5,1(7)
        movei   1,mfsdev_names(4)
        movei   2,6
        pushj   17,vfs_sixbit_name_chars
        movem   1,(7)
        jrst    monitorfs_parent_name_done
monitorfs_name_dev:
        movei   4,monitorfs_names
        jrst    monitorfs_name_record
monitorfs_name_devices:
        movei   4,monitorfs_names+024
monitorfs_name_record:
        move    5,(4)
        movem   5,(7)
        move    5,1(4)
        movem   5,1(7)
        move    5,2(4)
        movem   5,2(7)
monitorfs_parent_name_done:
        sub     17,[2,,2]
        jrst    pdp10_ret_zero
monitorfs_parent_name_fail:
        sub     17,[2,,2]
        jrst    pdp10_ret_neg1


; Direct request-free mutation leaves.  The fifth C argument is at -1(17).
        .globl  fs_provider_reg_call
        .globl  pdp10_ret_neg2
        .globl  vfs_create_op
        .globl  vfs_create
vfs_create:
        movei   5,6                    ; FS_MRES_OP_CREATE
        jrst    vfs_create_common
        .globl  vfs_mkfifo
vfs_mkfifo:
        ldb     5,[POINT 6,1,5]
        andi    5,075                  ; providers 4 and 6 both become 4
        caie    5,4
        jrst    pdp10_ret_neg2
        ori     3,010000               ; private CREATE-as-FIFO marker
        movei   5,6                    ; reuse FS_MRES_OP_CREATE
        jrst    vfs_create_common
        .globl  vfs_mkdir
vfs_mkdir:
        movei   5,7                    ; FS_MRES_OP_MKDIR
vfs_create_common:
        move    6,file_table
        move    6,041(6)               ; u-area 0110: process umask
        andca   6,3                    ; mode &= ~umask
        move    3,6
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
vfs_create_store_result:
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
        jrst    vfs_create_store_result
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
        .globl  mfsdev_readdir
        .globl  mfsproc_readdir
        .globl  mfsdev_stat
        .globl  mfsproc_stat

        .globl  vfs_readdir_raw
vfs_readdir_raw:
        ldb     7,[POINT 6,1,5]
        cain    7,2
        jrst    mfsdev_readdir
        cain    7,3
        jrst    mfsproc_readdir
        movei   6,2                    ; FS_MRES_OP_READDIR
        jrst    fs_provider_reg_call

        .globl  vfs_stat
vfs_stat:
        ; Providers without persistent ownership are root-owned by default.
        setzm   4(2)
        setzm   5(2)
        setzm   6(2)                    ; mtime unknown unless provider supplies it
        ldb     7,[POINT 6,1,5]
        cain    7,2
        jrst    mfsdev_stat
        cain    7,3
        jrst    mfsproc_stat
        movei   6,3                    ; FS_MRES_OP_STAT
        jrst    fs_provider_reg_call

        .globl  vfs_unlink
vfs_unlink:
        movei   6,11                   ; FS_MRES_OP_UNLINK
        jrst    vfs_mutate2

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
        movei   6,14                   ; FS_MRES_OP_CHMOD
vfs_mutate2:
        push    17,1
        push    17,2
        pushj   17,vfs_readonly
        jumpn   1,vfs_mutate2_ro
        pop     17,2
        pop     17,1
        ldb     7,[POINT 6,1,5]
        jrst    fs_provider_reg_call
vfs_mutate3_ro:
        sub     17,[1,,1]
vfs_mutate2_ro:
        sub     17,[2,,2]
        jrst    pdp10_ret_neg1

; D6FS reuses its CHMOD provider operation as a compact private setattr
; channel.  Other providers never see these command values.
; AC2 command 0100000 = CHOWN, AC3 = uid,,gid.
; AC2 command 0100001 = UTIME, AC3 = TIME36.
        .globl  vfs_chown
        .globl  vfs_utime
vfs_chown:
        move    4,2
        lsh     4,022                  ; uid to LH (18 bits)
        andi    3,0777777
        ior     3,4
        movei   2,0100000
        jrst    vfs_d6fs_setattr
vfs_utime:
        move    3,2
        movei   2,0100001
vfs_d6fs_setattr:
        ldb     7,[POINT 6,1,5]
        caie    7,6                    ; D6FS provider only
        jrst    pdp10_ret_neg1
        push    17,1
        push    17,2
        push    17,3
        pushj   17,vfs_readonly
        jumpn   1,vfs_d6fs_setattr_ro
        pop     17,3
        pop     17,2
        pop     17,1
        movei   6,14                   ; D6FS private setattr via CHMOD slot
        ldb     7,[POINT 6,1,5]
        jrst    fs_provider_reg_call
vfs_d6fs_setattr_ro:
        sub     17,[3,,3]
        jrst    pdp10_ret_neg1

        .globl  mfsdom_read_words
        .globl  vfs_read_words
vfs_read_words:
        ldb     7,[POINT 6,1,5]
        caie    7,3
        jrst    vfs_read_words_provider
        trne    1,0400000
        jrst    mfsdom_read_words
        jrst    pdp10_ret_neg1
vfs_read_words_provider:
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
        .globl  mfsproc_readchar
        .globl  mfsdev_readchar

; int vfs_readchar(vnode_t node, kword_t off, unsigned int *chp)
        .globl  vfs_readchar
vfs_readchar:
        jumpe   3,pdp10_ret_neg1
        ldb     4,[POINT 6,1,5]
        cain    4,3
        jrst    mfsproc_readchar
        cain    4,2
        jrst    mfsdev_readchar

        add     17,[012,,012]
        movem   1,-011(17)             ; node
        movem   2,-010(17)             ; character offset
        movem   3,-7(17)               ; result pointer
        movei   2,-6(17)               ; seven-word struct vfs_stat
        pushj   17,vfs_stat
        jumpn   1,vfs_readchar_fail
        move    1,-6(17)               ; st.type
        caie    1,2                    ; VFS_TYPE_REG
        jrst    vfs_readchar_fail

        ; Compare unsigned character offset with st.size_chars.
        move    2,-010(17)
        tlc     2,0400000
        move    3,-4(17)
        tlc     3,0400000
        caml    2,3
        jrst    vfs_readchar_eof

        move    2,-010(17)
        move    4,2
        andi    4,3                    ; quarter-word number
        lsh     2,-2                   ; word offset
        move    1,-011(17)
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
        move    3,-7(17)
        movem   6,(3)
        movei   1,1
        jrst    vfs_readchar_done
vfs_readchar_eof:
        setz    1,
        jrst    vfs_readchar_done
vfs_readchar_fail:
        seto    1,
vfs_readchar_done:
        sub     17,[012,,012]
        popj    17,

; int vfs_writechar(vnode_t node, kword_t off, unsigned int ch)
        .globl  vfs_writechar
vfs_writechar:
        ldb     4,[POINT 6,1,5]
        caie    4,2                    ; MonitorFS device view_PROVIDER
        jrst    vfs_writechar_regular
        ldb     4,[POINT 6,1,17]       ; VFS local kind
        cain    4,2                    ; MonitorFS device view_KIND_DEVICE
        jrst    pdp10_ret_busy          ; VFS_DEVICE_IO = -3

vfs_writechar_regular:
        add     17,[012,,012]
        movem   1,-011(17)             ; node
        movem   2,-010(17)             ; character offset
        movem   3,-7(17)               ; character
        movei   2,-6(17)               ; seven-word struct vfs_stat
        pushj   17,vfs_stat
        jumpn   1,vfs_writechar_fail
        move    1,-6(17)
        caie    1,2                    ; VFS_TYPE_REG
        jrst    vfs_writechar_fail

        move    2,-010(17)
        addi    2,1
        movem   2,(17)                 ; end_chars; later fifth argument
        addi    2,3
        lsh     2,-2                   ; ceil(end_chars / 4)
        move    3,-3(17)               ; st.size_words
        camle   2,3
        jrst    vfs_writechar_grow
vfs_writechar_after_grow:
        move    2,-010(17)
        lsh     2,-2                   ; word offset
        movem   2,-1(17)
        setzm   -4(17)                 ; read beyond EOF as zero word
        move    1,-011(17)
        movei   3,-4(17)
        movei   4,1
        pushj   17,vfs_read_words

        move    3,-010(17)
        andi    3,3                    ; quarter-word number
        move    4,3
        lsh     4,3
        add     4,3                    ; 9 * bi
        movei   5,033
        sub     5,4                    ; shift = 27 - 9*bi
        movei   4,0777
        lsh     4,0(5)
        andca   4,-4(17)
        move    3,-7(17)
        andi    3,0777
        lsh     3,0(5)
        ior     4,3
        movem   4,-4(17)

        move    1,-011(17)
        move    2,-1(17)
        movei   3,-4(17)
        movei   4,1
        pushj   17,vfs_write_words
        caie    1,1
        jrst    vfs_writechar_fail
        setz    1,
        jrst    vfs_writechar_done

vfs_writechar_grow:
        move    1,-011(17)
        move    3,(17)                  ; end_chars
        pushj   17,vfs_truncate
        jumpn   1,vfs_writechar_fail
        jrst    vfs_writechar_after_grow

vfs_writechar_fail:
        seto    1,
vfs_writechar_done:
        sub     17,[012,,012]
        popj    17,

; Compact mount policy.  The four-entry namespace table is a bounded PDP-6
; structure, so keeping the policy in fixed assembly avoids the C callee-save
; frames and unsigned comparison glue without changing the VFS ABI.
        .globl  vfs_readonly
vfs_readonly:
        ldb     1,[POINT 6,1,11]        ; mount id
        subi    1,1
        jumpl   1,pdp10_ret_zero
        cail   1,4
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
        add     17,[7,,7]               ; seven-word struct vfs_stat
        movei   2,-6(17)
        move    1,-012(17)
        pushj   17,vfs_stat
        jumpn   1,vfs_mount_stat_fail
        move    6,-6(17)                ; st.type
        move    1,-012(17)
        move    2,-011(17)
        move    3,-010(17)
        move    4,-7(17)
        sub     17,[013,,013]
        caie    6,1                     ; VFS_TYPE_DIR
        jrst    pdp10_ret_neg1
        jrst    vfs_mount_find
vfs_mount_stat_fail:
        sub     17,[013,,013]
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
        cail   2,4
        jrst    pdp10_ret_neg1
vfs_unmount_slot:
        move    3,vfs_mount_root(2)
        came    3,1
        jrst    pdp10_ret_neg1
        push    17,1                    ; root
        push    17,2                    ; slot
        movei   1,1(2)                  ; public mount id is slot + 1
        pushj   17,pipe_fifo_mount_busy
        jumpn   1,vfs_unmount_fail
        move    1,-1(17)
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

        .data
        .globl  monitorfs_names
; Four words per static MonitorFS namespace component: length, two SIXBIT
; words, and canonical vnode.  Lookup, readdir, and getcwd share this table.
monitorfs_names:
        ; root namespace: DEV, MONITOR, PROC(alias)
        .word   3
        .word   0444566000000
        .word   0
        .word   020000000000           ; DEV
        .word   7
        .word   0555756516457
        .word   0620000000000
        .word   030000000000           ; MONITOR
        .word   4
        .word   0606257430000
        .word   0
        .word   030001000000           ; PROC alias == PROCESSES
        ; /MONITOR children
        .word   9
        .word   0606257434563
        .word   0634563000000
        .word   030001000000           ; PROCESSES
        .word   7
        .word   0445755415156
        .word   0630000000000
        .word   030001400000           ; DOMAINS
        .word   7
        .word   0444566514345
        .word   0630000000000
        .word   020001000000           ; DEVICES

        .bss
        .globl  vfs_mount_target
vfs_mount_target:
        .block  4
        .globl  vfs_mount_root
vfs_mount_root:
        .block  4
        .text
