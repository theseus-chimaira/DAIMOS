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
        cail    2,026                  ; DTFS_FILE_SLOTS = 22 decimal
        jrst    dtfs_is_false
        movei   1,1
        popj    17,

dtfs_is_false:
        setz    1,
        popj    17,

; DTC call veneers.  Shuffle arguments high-to-low so no temporary ACs are
; needed: (unit, block, buf) -> (service, unit, block, buf).
        .globl  dtfs_dtc_read
dtfs_dtc_read:
        move    4,3
        move    3,2
        move    2,1
        move    1,dtfs_dtc_read_addr
        jrst    dtfs_dtc_call

        .globl  dtfs_dtc_write
dtfs_dtc_write:
        move    4,3
        move    3,2
        move    2,1
        move    1,dtfs_dtc_write_addr
        jrst    dtfs_dtc_call
