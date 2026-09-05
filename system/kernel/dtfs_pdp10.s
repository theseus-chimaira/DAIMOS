; dtfs_pdp10.s -- DTFS runtime and boot-patched DTC veneers.
        .text

        .globl  fs_mres_vector_dispatch
        .globl  dtfs_mres_dispatch
        .globl  dtfs_lookup
        .globl  dtfs_readdir
        .globl  dtfs_stat
        .globl  dtfs_parent
        .globl  dtfs_create
        .globl  dtfs_unlink
        .globl  dtfs_rename
        .globl  dtfs_truncate
        .globl  dtfs_chmod
        .globl  dtfs_read_words
        .globl  dtfs_write_words
        .globl  dtfs_sync
        .globl  dtfs_format_unit
        .globl  dtfs_mount_unit

dtfs_mres_dispatch:
        move    2,[dtfs_mres_vector]
        jrst    fs_mres_vector_dispatch

        .data
dtfs_mres_vector:
        .word   017                      ; highest runtime VFS operation: 15
        .word   dtfs_lookup              ; 1
        .word   dtfs_readdir             ; 2
        .word   dtfs_stat                ; 3
        .word   dtfs_parent              ; 4
        .word   0                        ; 5 PARENT_NAME unsupported
        .word   dtfs_create              ; 6
        .word   dtfs_mkdir_unsupported   ; 7 MKDIR unsupported
        .word   0                        ; 8 SYMLINK unsupported
        .word   dtfs_unlink              ; 9 UNLINK
        .word   dtfs_rename              ; 10 RENAME
        .word   dtfs_truncate            ; 11 TRUNCATE
        .word   dtfs_chmod               ; 12 CHMOD
        .word   dtfs_read_words          ; 13 READ_WORDS
        .word   dtfs_write_words         ; 14 WRITE_WORDS
        .word   dtfs_sync                ; 15 SYNC
        .text

; DTFS is deliberately flat.  Preserve a distinct error through the syscall
; boundary so userland can report that MKDIR is unsupported rather than an
; undifferentiated filesystem failure.
dtfs_mkdir_unsupported:
        hrroi   1,0777776              ; SYS_ERR_UNSUPPORTED (-2)
        popj    17,

; Compact vnode predicates.  The vnode encoding is provider:6, kind/mount:12,
; index:18.  Mask provider plus local kind in one operation; mount-id and file
; index remain independent tests.
        .globl  dtfs_is_root
dtfs_is_root:
        move    2,1
        and     2,[770077000000]
        came    2,[050001000000]       ; DTFS provider, root local kind
        jrst    dtfs_is_false
        move    2,1
        and     2,[007700000000]       ; non-zero mount id required
        jumpe   2,dtfs_is_false
        movei   1,1
        popj    17,

        .globl  dtfs_is_file
dtfs_is_file:
        move    2,1
        and     2,[770077000000]
        came    2,[050002000000]       ; DTFS provider, file local kind
        jrst    dtfs_is_false
        move    2,1
        and     2,[007700000000]
        jumpe   2,dtfs_is_false
        move    2,1
        andi    2,0777777
        cail    2,027                  ; ITS has 23 file slots
        jrst    dtfs_is_false
        movei   1,1
        popj    17,

dtfs_is_false:
        setz    1,
        popj    17,

; DTC veneers.  MINIT patches the RH of each JRST with the installed DTC
; service entry.  The DTFS and DTC ABIs are identical: AC1=unit, AC2=block,
; AC3=buffer, so the tail jump needs no argument shuffling or resident pointer.
        .globl  dtfs_dtc_read
        .globl  dtfs_dtc_read_jump
dtfs_dtc_read:
dtfs_dtc_read_jump:
        jrst    0

        .globl  dtfs_dtc_write
        .globl  dtfs_dtc_write_jump
dtfs_dtc_write:
dtfs_dtc_write_jump:
        jrst    0
