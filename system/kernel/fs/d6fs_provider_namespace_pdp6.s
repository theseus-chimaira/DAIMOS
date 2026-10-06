/**
 * @file d6fs_provider_namespace_pdp6.s
 * @brief Compact PDP-6 D6FS namespace mutation front-end.
 *
 * The portable reference remains d6fs_provider.c.  These cold namespace
 * operations use the same transaction ordering but avoid KCC's large local
 * frames and spill traffic.  The extent/allocation mutation engine remains in
 * d6fs_provider_mutation.s and is appended by the kernel build.
 */

        .text
        .globl  d6fs_provider_scan_slot
        .globl  d6fs_provider_create_object
        .globl  d6fs_provider_unlink
        .globl  d6fs_provider_rename

        .globl  d6fs_provider_dirent
        .globl  d6fs_provider_free_fcb
        .globl  d6fs_provider_write_dirent
        .globl  d6fs_provider_resize_fcb
        .globl  d6fs_provider_free_file_tail
        .globl  d6fs_reader_fcb
        .globl  d6fs_reader_put_fcb
        .globl  d6fs_reader_write_words
        .globl  d6fs_name_hash24
        .globl  d6fs_active_reader
        .globl  vfs_name_valid
        .globl  vfs_name_words_equal
        .globl  vfs_current_owner
        .globl  fs_copy_words
        .globl  fs_zero_words
        .globl  pclk_time36

        .equ    D6FS_TYPE_DIR,2
        .equ    D6FS_TYPE_SYMLINK,3
        .equ    D6FS_FLAG_PROTECTED,022 ; NOUNLINK | IMMUTABLE
        .equ    D6FS_FCB_WORDS,020
        .equ    D6FS_FCB_RESERVED,015
        .equ    VFS_NAME_WORDS,4

; int d6fs_provider_scan_slot(dir, name, slotp, dip)
;
; Eight local words hold one decoded directory entry.  AC16 is -1 until the
; first reusable empty slot is seen; this removes the separate have_empty word
; used by the generated C implementation.
d6fs_provider_scan_slot:
        add     17,[017,,017]
        movei   0,-016(17)
        hrli    0,010
        blt     0,-010(17)
        move    010,1                  ; dir
        move    011,2                  ; name or zero
        move    012,3                  ; slotp
        move    013,4                  ; optional result dirent
        seto    016,                   ; no remembered empty slot
        jumpe   011,d6fs_scan_start
        move    1,011
        pushj   17,vfs_name_valid
        jumpe   1,d6fs_scan_fail
        movei   1,1(011)
        move    2,(011)
        pushj   17,d6fs_name_hash24
        move    014,1                  ; wanted hash

d6fs_scan_start:
        setz    015,                   ; slot
d6fs_scan_loop:
        move    1,010
        move    2,015
        movei   3,-7(17)               ; local struct d6fs_dirent_info
        pushj   17,d6fs_provider_dirent
        jumpe   1,d6fs_scan_end
        jumpl   1,d6fs_scan_fail
        jumpe   011,d6fs_scan_empty_request
        skipn   -0(17)                 ; local child_fcb: -7 + 7 == 0
        jrst    d6fs_scan_empty_entry
        move    1,-3(17)               ; local hash: -7 + 4
        came    1,014
        jrst    d6fs_scan_next
        movei   1,-7(17)
        movei   2,1(011)
        movei   3,VFS_NAME_WORDS
        pushj   17,vfs_name_words_equal
        jumpe   1,d6fs_scan_next
        movem   015,(012)
        jumpe   013,d6fs_scan_found
        movei   1,(013)
        hrli    1,-7(17)
        blt     1,7(013)
d6fs_scan_found:
        setz    1,
        jrst    d6fs_scan_return

d6fs_scan_empty_entry:
        jumpn   013,d6fs_scan_next      ; lookup caller does not need insertion slot
        jumpge  016,d6fs_scan_next
        move    016,015
        jrst    d6fs_scan_next

d6fs_scan_empty_request:
        skipn   (17)                    ; child_fcb == 0
        jrst    d6fs_scan_empty_found
d6fs_scan_next:
        aoja    015,d6fs_scan_loop

d6fs_scan_end:
        jumpe   011,d6fs_scan_end_current
        jumpl   016,d6fs_scan_end_current
        movem   016,(012)
        jrst    d6fs_scan_end_ok
d6fs_scan_end_current:
        movem   015,(012)
d6fs_scan_end_ok:
        movei   1,1                    ; no match; slotp is insertion/end slot
        jrst    d6fs_scan_return
d6fs_scan_empty_found:
        movem   015,(012)
        setz    1,
        jrst    d6fs_scan_return
d6fs_scan_fail:
        seto    1,
d6fs_scan_return:
        movei   0,010
        hrli    0,-016(17)
        blt     0,016
        sub     17,[017,,017]
        popj    17,

; int d6fs_provider_create_object(dir,name,payload,value,type,nodep)
;
; C arguments five and six are immediately below the PUSHJ return word.  Read
; them before opening the local frame.  Local layout after the seven saved ACs:
; 020-word FCB, 012-word decoded FCB info, 010-word dirent, index, slot.
d6fs_provider_create_object:
        move    5,-1(17)               ; type
        move    6,-2(17)               ; nodep
        add     17,[053,,053]
        movei   0,-052(17)
        hrli    0,010
        blt     0,-044(17)
        move    010,1                  ; dir
        move    011,2                  ; name
        move    012,3                  ; payload
        move    013,4                  ; value/mode
        move    014,5                  ; type
        move    015,6                  ; nodep

        move    1,010
        move    2,011
        movei   3,(17)                 ; slot
        setz    4,
        pushj   17,d6fs_provider_scan_slot
        jumpe   1,d6fs_create_fail
        movei   1,-1(17)               ; FCB index
        pushj   17,d6fs_provider_free_fcb
        jumpn   1,d6fs_create_fail

        move    1,013
        caie    014,D6FS_TYPE_SYMLINK
        jrst    d6fs_create_mode_ready
        movei   1,0777
d6fs_create_mode_ready:
        move    016,1                  ; mode
        movei   1,-043(17)             ; FCB
        movei   2,D6FS_FCB_WORDS
        pushj   17,fs_zero_words
        move    1,014
        lsh     1,41                   ; type << 33
        move    2,016
        andi    2,07777
        lsh     2,014
        ior     1,2
        movem   1,-043(17)             ; FCB meta
        pushj   17,pclk_time36
        movem   1,-040(17)             ; FCB mtime, word 3
        pushj   17,vfs_current_owner
        move    2,1
        lsh     2,-011
        andi    2,0777
        lsh     2,022
        andi    1,0777
        ior     2,1
        movem   2,-042(17)             ; FCB owner, word 1
        hrlz    1,010
        movem   1,-037(17)             ; FCB parent, word 4

        move    1,d6fs_active_reader
        move    2,-1(17)
        movei   3,-043(17)
        pushj   17,d6fs_reader_put_fcb
        jumpn   1,d6fs_create_fail
        move    016,-1(17)
        tlo     016,060001              ; provider 6, local kind 1

        caie    014,D6FS_TYPE_SYMLINK
        jrst    d6fs_create_dirent
        movei   1,-023(17)             ; decoded FCB info scratch
        movei   2,012
        pushj   17,fs_zero_words
        movei   1,D6FS_TYPE_SYMLINK
        movem   1,-023(17)             ; fi.type
        move    1,(012)                ; counted SIXBIT target chars
        addi    1,5
        idivi   1,6
        addi    1,1                    ; count word plus packed payload
        move    013,1                  ; symlink words (value no longer needed)
        move    1,016
        movei   2,-043(17)
        movei   3,-023(17)
        move    4,013
        pushj   17,d6fs_provider_resize_fcb
        jumpn   1,d6fs_create_rollback_fcb
        push    17,013                  ; fifth arg: nwords
        move    1,d6fs_active_reader
        movei   2,-043(17)
        setz    3,
        move    4,012
        pushj   17,d6fs_reader_write_words
        sub     17,[1,,1]
        came    1,013
        jrst    d6fs_create_rollback_symlink

d6fs_create_dirent:
        movei   1,1(011)
        movei   2,-011(17)             ; dirent.name
        movei   3,VFS_NAME_WORDS
        pushj   17,fs_copy_words
        movei   1,1(011)
        move    2,(011)
        pushj   17,d6fs_name_hash24
        movem   1,-5(17)               ; dirent.hash
        movem   014,-4(17)             ; type
        setzm   -3(17)                 ; flags
        move    1,-1(17)
        movem   1,-2(17)               ; child_fcb
        move    1,010
        move    2,(17)
        movei   3,-011(17)
        pushj   17,d6fs_provider_write_dirent
        jumpn   1,d6fs_create_dirent_fail
        movem   016,(015)
        setz    1,
        jrst    d6fs_create_return

d6fs_create_dirent_fail:
        caie    014,D6FS_TYPE_SYMLINK
        jrst    d6fs_create_rollback_fcb
d6fs_create_rollback_symlink:
        move    1,016
        movei   2,-043(17)
        movei   3,-023(17)
        setz    4,
        pushj   17,d6fs_provider_resize_fcb
d6fs_create_rollback_fcb:
        movei   1,-043(17)
        movei   2,D6FS_FCB_WORDS
        pushj   17,fs_zero_words
        move    1,d6fs_active_reader
        move    2,-1(17)
        movei   3,-043(17)
        pushj   17,d6fs_reader_put_fcb
d6fs_create_fail:
        seto    1,
d6fs_create_return:
        movei   0,010
        hrli    0,-052(17)
        blt     0,016
        sub     17,[053,,053]
        popj    17,

; int d6fs_provider_unlink(dir,name)
d6fs_provider_unlink:
        add     17,[066,,066]
        movei   0,-065(17)
        hrli    0,010
        blt     0,-057(17)
        move    010,1
        move    011,2
        move    1,010
        move    2,011
        movei   3,(17)                 ; temporary old_fcb[12] is slot
        movei   4,-056(17)             ; dirent
        pushj   17,d6fs_provider_scan_slot
        jumpn   1,d6fs_unlink_fail
        move    012,(17)               ; slot
        move    013,-047(17)           ; dirent.child_fcb
        move    1,d6fs_active_reader
        move    2,013
        movei   3,-046(17)             ; fcb
        movei   4,-026(17)             ; fi
        pushj   17,d6fs_reader_fcb
        jumpn   1,d6fs_unlink_fail
        move    1,-025(17)             ; fi.flags
        trne    1,D6FS_FLAG_PROTECTED
        jrst    d6fs_unlink_fail
        movei   1,-046(17)
        movei   2,-014(17)             ; saved on-disk FCB prefix
        movei   3,D6FS_FCB_RESERVED
        pushj   17,fs_copy_words

        move    014,-026(17)           ; fi.type
        move    016,-017(17)           ; fi.size_words (offset 7)
        caie    014,D6FS_TYPE_DIR
        jrst    d6fs_unlink_remove
        jumpe   016,d6fs_unlink_remove
        setz    015,                   ; child directory slot
d6fs_unlink_dir_loop:
        move    1,013
        tlo     1,060001
        move    2,015
        movei   3,-056(17)
        pushj   17,d6fs_provider_dirent
        jumpe   1,d6fs_unlink_remove
        jumpl   1,d6fs_unlink_fail
        skipe   -047(17)
        jrst    d6fs_unlink_fail
        aoja    015,d6fs_unlink_dir_loop

d6fs_unlink_remove:
        move    1,010
        move    2,012
        setz    3,
        pushj   17,d6fs_provider_write_dirent
        jumpn   1,d6fs_unlink_fail
        movei   1,-046(17)
        movei   2,D6FS_FCB_WORDS
        pushj   17,fs_zero_words
        move    1,d6fs_active_reader
        move    2,013
        movei   3,-046(17)
        pushj   17,d6fs_reader_put_fcb
        jumpn   1,d6fs_unlink_fail
        movei   1,-014(17)
        setz    2,
        pushj   17,d6fs_provider_free_file_tail
        setz    1,                     ; leaked blocks remain fsck-recoverable
        jrst    d6fs_unlink_return
d6fs_unlink_fail:
        seto    1,
d6fs_unlink_return:
        movei   0,010
        hrli    0,-065(17)
        blt     0,016
        sub     17,[066,,066]
        popj    17,

; int d6fs_provider_rename(olddir,oldname,newdir,newname)
d6fs_provider_rename:
        add     17,[061,,061]
        movei   0,-060(17)
        hrli    0,010
        blt     0,-052(17)
        move    010,1
        move    011,2
        move    012,3
        move    013,4
        hlrz    1,010
        lsh     1,-6
        andi    1,077
        hlrz    2,012
        lsh     2,-6
        andi    2,077
        came    1,2
        jrst    d6fs_rename_fail

        move    1,010
        move    2,011
        movei   3,-011(17)             ; temporary fi.type word = oldslot
        movei   4,-051(17)             ; di
        pushj   17,d6fs_provider_scan_slot
        jumpn   1,d6fs_rename_fail
        move    014,-011(17)           ; oldslot
        move    1,012
        move    2,013
        movei   3,-010(17)             ; temporary fi.flags word = newslot
        setz    4,
        pushj   17,d6fs_provider_scan_slot
        jumpe   1,d6fs_rename_fail     ; destination already exists
        move    015,-010(17)           ; insertion/end slot
        move    016,-042(17)           ; child_fcb

        move    1,d6fs_active_reader
        move    2,016
        movei   3,-031(17)             ; fcb
        movei   4,-011(17)             ; fi
        pushj   17,d6fs_reader_fcb
        jumpn   1,d6fs_rename_fail
        move    1,-010(17)             ; fi.flags
        trne    1,D6FS_FLAG_PROTECTED
        jrst    d6fs_rename_fail
        movei   1,-051(17)
        movei   2,-041(17)             ; old_di
        movei   3,010
        pushj   17,fs_copy_words

        move    1,010
        came    1,012
        jrst    d6fs_rename_find_empty
        move    015,014
        jrst    d6fs_rename_name
d6fs_rename_find_empty:
        move    1,012
        setz    2,
        movei   3,-011(17)             ; fi.type scratch for newslot
        setz    4,
        pushj   17,d6fs_provider_scan_slot
        jumpl   1,d6fs_rename_fail
        move    015,-011(17)

d6fs_rename_name:
        movei   1,1(013)
        movei   2,-051(17)
        movei   3,VFS_NAME_WORDS
        pushj   17,fs_copy_words
        movei   1,1(013)
        move    2,(013)
        pushj   17,d6fs_name_hash24
        movem   1,-045(17)             ; di.hash
        move    1,010
        came    1,012
        jrst    d6fs_rename_move
        move    1,012
        move    2,015
        movei   3,-051(17)
        pushj   17,d6fs_provider_write_dirent
        jumpn   1,d6fs_rename_fail
        setz    1,
        jrst    d6fs_rename_return

d6fs_rename_move:
        move    1,010
        move    2,014
        setz    3,
        pushj   17,d6fs_provider_write_dirent
        jumpn   1,d6fs_rename_fail
        hrlz    1,012
        movem   1,-025(17)             ; fcb parent, word 4
        move    1,d6fs_active_reader
        move    2,016
        movei   3,-031(17)
        pushj   17,d6fs_reader_put_fcb
        jumpn   1,d6fs_rename_restore_old
        move    1,012
        move    2,015
        movei   3,-051(17)
        pushj   17,d6fs_provider_write_dirent
        jumpe   1,d6fs_rename_ok

        ; New publication failed: restore parent FCB then old namespace entry.
        move    1,(17)                 ; fi.parent_fcb, offset 9
        hrlzm   1,-025(17)
        move    1,d6fs_active_reader
        move    2,016
        movei   3,-031(17)
        pushj   17,d6fs_reader_put_fcb
d6fs_rename_restore_old:
        move    1,010
        move    2,014
        movei   3,-041(17)
        pushj   17,d6fs_provider_write_dirent
        jrst    d6fs_rename_fail
d6fs_rename_ok:
        setz    1,
        jrst    d6fs_rename_return
d6fs_rename_fail:
        seto    1,
d6fs_rename_return:
        movei   0,010
        hrli    0,-060(17)
        blt     0,016
        sub     17,[061,,061]
        popj    17,

