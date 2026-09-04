; dtfs_pdp10.s -- resident indirect bridge to block-addressed DTC MRES.
        .text
        .globl dtfs_dtc_call

; int dtfs_dtc_call(address, unit, block, buffer)
; C args arrive in AC1..AC4; DTC service expects unit/block/buffer in AC1..AC3.
dtfs_dtc_call:
        move    5,1
        move    1,2
        move    2,3
        move    3,4
        andi    5,0777777
        jumpe   5,dtfs_dtc_no_service
        pushj   17,(5)
        popj    17,
dtfs_dtc_no_service:
        hrroi   1,1
        popj    17,

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
        .word   022                      ; highest operation: 18 decimal
        .word   dtfs_lookup              ; 1
        .word   dtfs_readdir             ; 2
        .word   dtfs_stat                ; 3
        .word   dtfs_parent              ; 4
        .word   0                        ; 5 PARENT_NAME unsupported
        .word   dtfs_create              ; 6
        .word   0                        ; 7 MKDIR unsupported
        .word   0                        ; 8 SYMLINK unsupported
        .word   dtfs_unlink              ; 9 UNLINK
        .word   dtfs_rename              ; 10 RENAME
        .word   dtfs_truncate            ; 11 TRUNCATE
        .word   dtfs_chmod               ; 12 CHMOD
        .word   dtfs_read_words          ; 13 READ_WORDS
        .word   dtfs_write_words         ; 14 WRITE_WORDS
        .word   dtfs_sync                ; 15 SYNC
        .word   0                        ; 16 PREPARE_UNMOUNT
        .word   dtfs_format_unit         ; 17 FORMAT_UNIT
        .word   dtfs_mount_unit          ; 18 MOUNT_UNIT
        .text
