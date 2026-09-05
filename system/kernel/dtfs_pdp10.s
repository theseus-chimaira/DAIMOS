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


; Compact five-bit allocation-map accessors.  All DTFS map indices are small
; non-negative values, so IDIVI avoids GCC's 72-bit unsigned DIV setup.
        .globl  dtfs_owner
dtfs_owner:
        idivi   2,7                     ; AC2 word index, AC3 remainder
        add     1,2                     ; AC1 map base + word index
        move    2,3
        lsh     2,2
        add     2,3                     ; remainder * 5
        move    1,dtfs_dir(1)
        lsh     1,-037(2)               ; right by 31 - remainder * 5
        andi    1,037
        popj    17,

        .globl  dtfs_set_owner
dtfs_set_owner:
        move    4,3                     ; preserve new owner
        idivi   2,7                     ; AC2 word index, AC3 remainder
        add     1,2
        move    2,3
        lsh     2,2
        add     2,3                     ; remainder * 5
        movei   5,037
        sub     5,2                     ; left shift = 31 - remainder * 5
        movei   3,037
        lsh     3,0(5)
        andca   3,dtfs_dir(1)
        lsh     4,0(5)
        ior     3,4
        movem   3,dtfs_dir(1)
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

; int dtfs_foreign_set_name(slot, name, its)
; Pack a VFS SIXBIT NAME[.EXT] directly into the ITS/TENEX directory words.
; ITS permits six extension characters, TENEX three.  Names are at most 13
; characters, so validating the format here also satisfies vfs_name_valid().
        .globl  dtfs_foreign_set_name
dtfs_foreign_set_name:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        push    17,016
        move    010,1                   ; slot
        move    011,2                   ; struct vfs_name *
        move    012,3                   ; ITS flag
        jumpe   011,dtfs_foreign_set_name_fail
        move    013,(011)               ; total characters
        jumple  013,dtfs_foreign_set_name_fail
        skipn   012
        jrst    dtfs_foreign_set_name_tenex_limit
        caile   013,015                 ; ITS: 13 decimal
        jrst    dtfs_foreign_set_name_fail
        movei   3,6                     ; maximum extension characters
        jrst    dtfs_foreign_set_name_start
dtfs_foreign_set_name_tenex_limit:
        caile   013,012                 ; TENEX: 10 decimal
        jrst    dtfs_foreign_set_name_fail
        movei   3,3

dtfs_foreign_set_name_start:
        move    014,[POINT 6,0]
        movei   2,1(011)                ; packed name begins at name->words[0]
        hrr     014,2
        setz    015,                    ; packed NAME
        setz    016,                    ; packed EXT
        setz    4,                      ; NAME count
        setz    5,                      ; EXT count
        setz    6,                      ; dot seen

dtfs_foreign_set_name_loop:
        ildb    7,014
        cain    7,016                   ; SIXBIT '.'
        jrst    dtfs_foreign_set_name_dot
        jumpn   6,dtfs_foreign_set_name_ext
        addi    4,1
        caile   4,6
        jrst    dtfs_foreign_set_name_fail
        lsh     015,6
        ior     015,7
        jrst    dtfs_foreign_set_name_next
dtfs_foreign_set_name_ext:
        addi    5,1
        camle   5,3
        jrst    dtfs_foreign_set_name_fail
        lsh     016,6
        ior     016,7
        jrst    dtfs_foreign_set_name_next
dtfs_foreign_set_name_dot:
        jumpn   6,dtfs_foreign_set_name_fail
        jumpe   4,dtfs_foreign_set_name_fail
        movei   6,1
dtfs_foreign_set_name_next:
        sojg    013,dtfs_foreign_set_name_loop
        jumpn   6,dtfs_foreign_set_name_need_ext
        jrst    dtfs_foreign_set_name_align
dtfs_foreign_set_name_need_ext:
        jumpe   5,dtfs_foreign_set_name_fail

dtfs_foreign_set_name_align:
        movei   7,6
        sub     7,4
        imuli   7,6
        lsh     015,0(7)
        movei   7,6
        sub     7,5
        imuli   7,6
        lsh     016,0(7)
        skipn   012
        jrst    dtfs_foreign_set_name_tenex_store
        move    4,010
        lsh     4,1
        movem   015,dtfs_dir(4)
        movem   016,dtfs_dir+1(4)
        jrst    dtfs_foreign_set_name_ok
dtfs_foreign_set_name_tenex_store:
        movei   4,dtfs_dir(010)
        movem   015,0123(4)
        movem   016,0151(4)
dtfs_foreign_set_name_ok:
        setz    1,
        jrst    dtfs_foreign_set_name_return
dtfs_foreign_set_name_fail:
        seto    1,
dtfs_foreign_set_name_return:
        pop     17,016
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
