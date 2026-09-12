; FILE descriptors and cwd live in the current process's stable u-area.
; file_table is a one-word KCORE pointer to its 13-entry descriptor table;
; cwd occupies the word immediately before the table.
        .globl  file_table

        .text
; file_pdp10.s -- compact resident FILE/path primitives for PDP-6/PDP-10.
        .text

        .globl  file_path_char
        .globl  vfs_name_char
; unsigned int file_path_char(path, pos)
; unsigned int vfs_name_char(name, pos) -- identical packed layout on PDP-10
; pos is bounded by FILE_PATH_MAX_CHARS, so signed IDIVI is sufficient and
; avoids constructing a 72-bit unsigned dividend for DIVI.
vfs_name_char:
file_path_char:
        idivi   2,6
        move    4,3
        muli    4,6
        trne    4,1
        tloa    5,0400000
        tlz     5,0400000
        add     1,2
        move    1,1(1)
        move    4,5
        subi    4,036
        lsh     1,0(4)
        andi    1,077
        popj    17,

        .globl  file_path_setchar
        .globl  vfs_name_setchar
; void file_path_setchar(path, pos, ch)
; void vfs_name_setchar(name, pos, ch) -- same packed character field
vfs_name_setchar:
file_path_setchar:
        idivi   2,6
        move    5,3
        muli    5,6
        trne    5,1
        tloa    6,0400000
        tlz     6,0400000
        movei   4,036
        sub     4,6
        movei   5,077
        lsh     5,0(4)
        add     1,2
        andca   5,1(1)
        andi    3,077
        lsh     3,0(4)
        ior     5,3
        movem   5,1(1)
        popj    17,

        .globl  pdp10_ret_zero
        .globl  pdp10_ret_one
        .globl  pdp10_ret_neg1
        .globl  vfs_readchar
        .globl  vfs_writechar

; int file_stdio_enabled(unsigned int fd)
; Native descriptors 0..2 have explicit RUN inheritance bits two words before
; file_table.  Ordinary descriptors remain in the regular file table.
        .globl  file_stdio_enabled
file_stdio_enabled:
        caile   1,2
        jrst    pdp10_ret_zero
        skipn   2,file_table
        jrst    pdp10_ret_zero
        move    2,-2(2)
        jumpe   1,file_stdio_check0
        caie    1,1
        jrst    file_stdio_check2
        trnn    2,2
        jrst    pdp10_ret_zero
        jrst    pdp10_ret_one
file_stdio_check0:
        trnn    2,1
        jrst    pdp10_ret_zero
        jrst    pdp10_ret_one
file_stdio_check2:
        trnn    2,4
        jrst    pdp10_ret_zero
        jrst    pdp10_ret_one

; int file_readchar(int fd)
; Validate the descriptor exactly as the C wrapper did, then advance the
; character offset only after a successful one-character VFS transfer.
        .globl  file_readchar
file_readchar:
        push    17,010
        push    17,0                   ; one-word character result
        pushj   17,file_find
        jumpe   1,file_readchar_fail
        move    3,(1)
        tlne    3,100000               ; directories are not byte streams
        jrst    file_readchar_fail
        tlnn    3,400000               ; FILE_META_READ
        jrst    file_readchar_fail
        move    010,1
        move    1,(010)
        tlz     1,707070               ; strip packed descriptor metadata
        move    2,1(010)
        movei   3,(17)
        pushj   17,vfs_readchar
        camn    1,[-3]                 ; VFS_DEVICE_IO
        jrst    file_readchar_done
        jumpg   1,file_readchar_ok
        jumpe   1,file_readchar_eof
        seto    1,                     ; other VFS errors -> -1
        jrst    file_readchar_done
file_readchar_eof:
        hrroi   1,0777776              ; EOF -> -2
        jrst    file_readchar_done
file_readchar_ok:
        aos     1(010)
        move    1,(17)
file_readchar_done:
        sub     17,[1,,1]
        pop     17,010
        popj    17,
file_readchar_fail:
        seto    1,
        jrst    file_readchar_done

; int file_writechar(int fd, unsigned int ch)
; The input character is held on the stack across file_find; only AC10 needs
; saving across the VFS call.
        .globl  file_writechar
file_writechar:
        push    17,010
        push    17,2
        pushj   17,file_find
        jumpe   1,file_writechar_fail
        move    3,(1)
        tlne    3,100000               ; directories are not byte streams
        jrst    file_writechar_fail
        tlnn    3,200000               ; FILE_META_WRITE
        jrst    file_writechar_fail
        move    010,1
        move    3,(17)
        move    1,(010)
        tlz     1,707070               ; strip packed descriptor metadata
        move    2,1(010)
        pushj   17,vfs_writechar
        jumpn   1,file_writechar_done
        aos     1(010)
file_writechar_done:
        sub     17,[1,,1]
        pop     17,010
        popj    17,
file_writechar_fail:
        seto    1,
        jrst    file_writechar_done

; int file_close(int fd)
; Mask the packed descriptor bits only at the VFS boundary.
        .globl  file_close
file_close:
        push    17,010
        pushj   17,file_find
        jumpe   1,file_close_fail
        move    010,1
        move    1,(1)
        tlz     1,707070
        pushj   17,vfs_sync
        jumpn   1,file_close_done
        setzm   (010)
file_close_done:
        pop     17,010
        popj    17,
file_close_fail:
        seto    1,
        jrst    file_close_done

; int file_dup(int fd)
; A duplicate has the same vnode, access state, lock-family token, lock state and
; offset.  Copying both words directly is smaller than unpacking and rebuilding
; a descriptor through file_new_fd, and preserves lock-family semantics exactly.
        .globl  file_dup
file_dup:
        pushj   17,file_find
        jumpe   1,pdp10_ret_neg1
        move    2,file_table
        movei   3,3                    ; returned descriptor
        movei   4,015                  ; FILE_NFILE = 13
file_dup_scan:
        skipn   (2)
        jrst    file_dup_store
        addi    2,2
        addi    3,1
        sojg    4,file_dup_scan
        jrst    pdp10_ret_neg1
file_dup_store:
        move    4,(1)
        movem   4,(2)
        move    4,1(1)
        movem   4,1(2)
        move    1,3
        popj    17,

; int file_lock(int fd, unsigned int op)
; The packed node word carries both the canonical vnode and descriptor state.
; FLOCK compares canonical vnode bits and the four-bit dup-family token
; directly in packed form, so neither field needs unpacking.
        .globl  file_lock
file_lock:
        push    17,2                    ; file_find may clobber operation
        pushj   17,file_find
        pop     17,2
        jumpe   1,pdp10_ret_neg1
        move    4,(1)                   ; selected packed node/state
        tlnn    4,000030                ; regular state is nonzero
        jrst    pdp10_ret_neg1
        caige   2,1                    ; SHARED..UNLOCK are 1..3
        jrst    pdp10_ret_neg1
        caile   2,3
        jrst    pdp10_ret_neg1
        move    6,file_lock_modes-1(2)
        jumpe   6,file_lock_update      ; unlock cannot conflict

        move    2,file_table
        movei   3,015                  ; FILE_NFILE = 13
file_lock_check:
        move    4,(2)
        move    7,4
        xor     7,(1)
        trne    7,0777777               ; different vnode index
        jrst    file_lock_check_next
        tlne    7,070707                ; provider/mount/local-kind differ
        jrst    file_lock_check_next
        tlnn    7,007040                ; same owner never conflicts
        jrst    file_lock_check_next
        tlnn    4,000020                ; candidate is not locked
        jrst    file_lock_check_next
; An exclusive request conflicts with either lock.  A shared request conflicts
; only with exclusive (state 11); shared itself is state 10.
        tlne    6,000010                ; requested exclusive?
        jrst    pdp10_ret_neg1
        tlne    4,000010                ; candidate exclusive?
        jrst    pdp10_ret_neg1
file_lock_check_next:
        addi    2,2
        sojg    3,file_lock_check

file_lock_update:
        move    2,file_table
        movei   3,015
file_lock_update_loop:
        move    4,(2)
        move    7,4
        xor     7,(1)
        trne    7,0777777
        jrst    file_lock_update_next
        tlne    7,070707
        jrst    file_lock_update_next
        tlne    7,007040
        jrst    file_lock_update_next
        tlz     4,000030                ; replace regular/lock state
        jumpn   6,file_lock_update_set
        tlo     4,000010                ; unlock -> regular, unlocked
        jrst    file_lock_update_store
file_lock_update_set:
        ior     4,6
file_lock_update_store:
        movem   4,(2)
file_lock_update_next:
        addi    2,2
        sojg    3,file_lock_update_loop
        jrst    pdp10_ret_zero

file_lock_modes:
        .long   000020000000            ; VFS_LOCK_SHARED
        .long   000030000000            ; VFS_LOCK_EXCLUSIVE
        .long   0                       ; VFS_LOCK_UNLOCK

; void file_unlock_mount(unsigned int mount_id)
; Clear lock/regular state for descriptors of an unmounted filesystem.  Only
; the low three canonical mount-id bits are extracted; the high three bits of
; that six-bit VFS field carry packed FILE metadata.
        .globl  file_unlock_mount
file_unlock_mount:
        move    2,file_table
        movei   3,015                  ; FILE_NFILE = 13
file_unlock_mount_loop:
        skipn   4,(2)
        jrst    file_unlock_mount_next
        ldb     4,[POINT 3,4,11]
        came    4,1
        jrst    file_unlock_mount_next
        move    4,(2)
        tlz     4,000030
        movem   4,(2)
file_unlock_mount_next:
        addi    2,2
        sojg    3,file_unlock_mount_loop
        popj    17,

; void file_close_all(void)
        .globl  file_close_all
file_close_all:
        push    17,010
        push    17,011
        movei   010,0
        move    011,file_table
file_close_all_loop:
        skipn   (011)
        jrst    file_close_all_next
        movei   1,3(010)               ; FILE_FD_FIRST + slot
        pushj   17,file_close
file_close_all_next:
        addi    011,2
        addi    010,1
        caige   010,015                 ; FILE_NFILE
        jrst    file_close_all_loop
        pop     17,011
        pop     17,010
        popj    17,

; int file_read_words(int fd, kword_t *buf, unsigned int nwords)
        .globl  file_read_words
file_read_words:
        push    17,010
        push    17,2                    ; buf
        push    17,3                    ; nwords
        pushj   17,file_find
        jumpe   1,file_read_words_fail
        skipn   -1(17)                  ; buf
        jrst    file_read_words_fail
        move    4,(1)
        tlne    4,100000                ; FILE_META_DIR
        jrst    file_read_words_fail
        tlnn    4,400000                ; FILE_META_READ
        jrst    file_read_words_fail
        move    010,1
        move    2,1(010)
        lsh     2,-2                    ; character offset -> word offset
        move    1,(010)
        tlz     1,707070                ; canonical vnode
        move    3,-1(17)
        move    4,(17)
        pushj   17,vfs_read_words
        jumple  1,file_read_words_done
        move    4,1
        lsh     4,2
        addm    4,1(010)
file_read_words_done:
        sub     17,[2,,2]
        pop     17,010
        popj    17,
file_read_words_fail:
        seto    1,
        jrst    file_read_words_done

; int file_write_words(int fd, const kword_t *buf, unsigned int nwords,
;     kword_t size_chars)
; The caller's size_chars word remains at the top of our local argument stack,
; exactly where vfs_write_words sees its fifth C argument after PUSHJ.
        .globl  file_write_words
file_write_words:
        push    17,010
        push    17,2                    ; buf
        push    17,3                    ; nwords
        push    17,4                    ; size_chars / VFS arg 5
        pushj   17,file_find
        jumpe   1,file_write_words_fail
        skipn   -2(17)                  ; buf
        jrst    file_write_words_fail
        move    4,(1)
        tlne    4,100000                ; FILE_META_DIR
        jrst    file_write_words_fail
        tlnn    4,200000                ; FILE_META_WRITE
        jrst    file_write_words_fail
        move    010,1
        move    2,1(010)
        lsh     2,-2
        move    1,(010)
        tlz     1,707070                ; canonical vnode
        move    3,-2(17)
        move    4,-1(17)
        pushj   17,vfs_write_words
        jumple  1,file_write_words_done
        move    4,1
        lsh     4,2
        addm    4,1(010)
file_write_words_done:
        sub     17,[3,,3]
        pop     17,010
        popj    17,
file_write_words_fail:
        seto    1,
        jrst    file_write_words_done

; int file_readdir(int fd, struct vfs_dirent *ent)
        .globl  file_readdir
file_readdir:
        push    17,010
        push    17,2                    ; ent
        pushj   17,file_find
        jumpe   1,file_readdir_fail
        skipn   (17)                    ; ent
        jrst    file_readdir_fail
        move    4,(1)
        tlnn    4,100000                ; FILE_META_DIR
        jrst    file_readdir_fail
        move    010,1
        move    1,(010)
        tlz     1,707070                ; canonical vnode
        move    2,1(010)
        move    3,(17)
        pushj   17,vfs_readdir
        jumple  1,file_readdir_done
        aos     1(010)
file_readdir_done:
        sub     17,[1,,1]
        pop     17,010
        popj    17,
file_readdir_fail:
        seto    1,
        jrst    file_readdir_done

; int file_component(const kword_t *path, unsigned int *posp,
;     struct vfs_name *name)
;
; Packed paths and VFS names both use six 6-bit SIXBIT characters per word.
; Build byte pointers once and copy the component directly instead of
; repeatedly dividing, shifting, decoding to ASCII, and re-encoding.
        .globl  file_component
file_component:
        jumpe   1,pdp10_ret_neg1
        jumpe   2,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        move    4,(1)           ; total path characters
        move    5,(2)           ; current character position
        caml    5,4             ; pos >= n
        jrst    file_component_empty

; Build a SIXBIT byte pointer to path[pos].  DIVI leaves quotient in AC6
; and remainder in AC7.  At most five bytes precede the requested position.
        move    7,5
        setz    6,
        divi    6,6
        add     6,1
        addi    6,1             ; path data starts at path[1]
        move    0,[POINT 6,0]
        hrr     0,6
        move    6,0
        move    0,7
file_component_seek:
        jumpe   0,file_component_skip
        ildb    7,6             ; discard bytes before path[pos]
        sojg    0,file_component_seek

; Skip leading separators.  SIXBIT '/' is 017.
file_component_skip:
        ildb    7,6
        caie    7,017
        jrst    file_component_start
        addi    5,1
        caml    5,4
        jrst    file_component_empty
        jrst    file_component_skip

file_component_start:
; Clear the five-word vfs_name and make an output SIXBIT byte pointer.
        setzm   (3)
        movei   0,1(3)
        hrli    0,(3)
        blt     0,4(3)
        move    1,[POINT 6,0]
        movei   0,1(3)
        hrr     1,0
        movei   0,0             ; component character count

file_component_copy:
        cain    0,030           ; maximum is 24 characters
        jrst    pdp10_ret_neg1
        idpb    7,1
        addi    0,1
        addi    5,1
        caml    5,4
        jrst    file_component_done
        ildb    7,6
        caie    7,017
        jrst    file_component_copy

; Consume the separator run before returning.  This leaves *posp at the
; next component (or n), so the C walker never needs a separate path-char
; decoder merely to skip '/'.
file_component_trailing:
        addi    5,1
        caml    5,4
        jrst    file_component_done
        ildb    7,6
        caie    7,017
        jrst    file_component_done
        jrst    file_component_trailing

file_component_done:
        movem   0,(3)
        movem   5,(2)
        jrst    pdp10_ret_one
file_component_empty:
        movem   5,(2)
        jrst    pdp10_ret_zero

; int file_getcwd(kword_t *buf, unsigned int nwords)
;
; Construct cwd paths directly as packed SIXBIT.
        .globl  file_getcwd
file_getcwd:
        jumpe   1,pdp10_ret_neg1
        jumpl   2,file_getcwd_nwords_ok ; unsigned value with bit 35 set
        cail    2,2
        jrst    file_getcwd_nwords_ok
        jrst    pdp10_ret_neg1
file_getcwd_nwords_ok:
        move    5,file_table
        move    4,-1(5)
        jumpn   4,file_getcwd_have_node
        move    4,vfs_namespace_root
file_getcwd_have_node:
        ldb     5,[POINT 6,4,5]
        caige   5,4
        jrst    file_getcwd_pseudo_tail
        caile   5,6
        jrst    file_getcwd_pseudo_tail

; The 0121-word local area is one parent vnode followed by sixteen five-word
; vfs_name records.  AC15 is reused as output count after the upward walk.
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        add     17,[0121,,0121]
        move    010,4                    ; current node
        move    011,1                    ; output buffer
        move    012,2                    ; output words

        move    015,vfs_namespace_root

; Clear the complete supplied output record, preserving existing semantics.
        move    4,012
        move    5,011
file_getcwd_clear:
        setzm   (5)
        addi    5,1
        sojg    4,file_getcwd_clear

; Walk to the selected root, saving component names leaf-first.
        movei   013,0                    ; depth
file_getcwd_up:
        camn    010,015
        jrst    file_getcwd_up_done
        cail    013,020                  ; depth >= 16
        jrst    file_getcwd_local_fail
        move    4,013
        lsh     4,2
        add     4,013                    ; depth * 5
        movei   5,-0120(17)             ; parts[0]
        add     4,5
        move    1,010                   ; current vnode
        movei   2,(17)                   ; parent vnode
        move    3,4                      ; saved component name
        pushj   17,vfs_parent_name
        jumpn   1,file_getcwd_local_fail
        move    010,(17)
        addi    013,1
        jrst    file_getcwd_up

file_getcwd_up_done:
; Capacity is the number of SIXBIT characters in buf[1..nwords-1].
        move    5,012
        subi    5,1
        imuli   5,6
        jumpe   5,file_getcwd_local_fail
        move    6,[POINT 6,0]
        movei   4,1(011)
        hrr     6,4
        movei   015,0                    ; output character count

; Every cwd starts with '/'.
        movei   4,017
        idpb    4,6
        addi    015,1

file_getcwd_components:
        jumpe   013,file_getcwd_store_len
        subi    013,1
        move    4,013
        lsh     4,2
        add     4,013
        movei   7,-0120(17)
        add     7,4                     ; AC7 -> component name
        move    3,(7)                   ; component chars

; Check room for the entire component and its separator before writing either.
        move    4,015
        cain    015,1
        jrst    file_getcwd_no_sep_need
        addi    4,1
file_getcwd_no_sep_need:
        add     4,3
        camle   4,5
        jrst    file_getcwd_local_fail
        cain    015,1
        jrst    file_getcwd_copy_name
        movei   4,017
        idpb    4,6
        addi    015,1

file_getcwd_copy_name:
        move    1,[POINT 6,0]
        movei   4,1(7)
        hrr     1,4
        jumpe   3,file_getcwd_components
file_getcwd_copy_loop:
        ildb    4,1
        idpb    4,6
        addi    015,1
        sojg    3,file_getcwd_copy_loop
        jrst    file_getcwd_components

file_getcwd_store_len:
        movem   015,(011)
        movei   1,0
        jrst    file_getcwd_return
file_getcwd_local_fail:
        seto    1,
file_getcwd_return:
        sub     17,[0121,,0121]
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; Non-MEMFS cwd formatting stays in the existing shared C helper.
file_getcwd_pseudo_tail:
        move    3,2
        move    2,1
        move    1,4
        jrst    file_getcwd_pseudo


; int file_symlink(const kword_t *target, const kword_t *linkpath)
; Save target and character count below one parent-vnode/name record.
        .globl  vfs_symlink
        .globl  file_symlink
file_symlink:
        jumpe   1,pdp10_ret_neg1
        jumpe   2,pdp10_ret_neg1
        move    3,(1)
        cail    3,1
        cail    3,0147                 ; FILE_PATH_MAX_CHARS + 1
        jrst    pdp10_ret_neg1
        push    17,1                   ; target
        push    17,3                   ; chars
        add     17,[6,,6]
        move    1,2                    ; linkpath
        movei   2,-5(17)              ; dir
        movei   3,-4(17)              ; leaf
        pushj   17,file_parent_path
        jumpn   1,file_symlink_fail
        move    1,-5(17)              ; dir
        movei   2,-4(17)              ; leaf
        move    3,-7(17)              ; target
        addi    3,1
        move    4,-6(17)              ; chars
        movei   5,-5(17)              ; reuse dir slot for output vnode
        push    17,5
        pushj   17,vfs_symlink
        sub     17,[1,,1]
file_symlink_done:
        sub     17,[6,,6]
        pop     17,3
        pop     17,2
        popj    17,
file_symlink_fail:
        seto    1,
        jrst    file_symlink_done

; int file_rename(const kword_t *oldpath, const kword_t *newpath)
; Save newpath below two parent-vnode/name records.
        .globl  vfs_rename
        .globl  file_rename
file_rename:
        push    17,2
        add     17,[014,,014]
        movei   2,-013(17)             ; olddir
        movei   3,-012(17)             ; oldname
        pushj   17,file_parent_path
        jumpn   1,file_rename_fail
        move    1,-014(17)             ; saved newpath
        movei   2,-5(17)               ; newdir
        movei   3,-4(17)               ; newname
        pushj   17,file_parent_path
        jumpn   1,file_rename_fail
        move    1,-013(17)
        movei   2,-012(17)
        move    3,-5(17)
        movei   4,-4(17)
        pushj   17,vfs_rename
file_rename_done:
        sub     17,[014,,014]
        pop     17,2
        popj    17,
file_rename_fail:
        seto    1,
        jrst    file_rename_done

; int file_chdir(const kword_t *path)
; Five locals hold one vnode followed by a four-word vfs_stat.
        .globl  file_chdir
file_chdir:
        add     17,[5,,5]
        movei   2,-4(17)
        pushj   17,file_lookup_path
        jumpn   1,file_chdir_fail
        move    1,-4(17)
        movei   2,-3(17)
        pushj   17,vfs_stat
        jumpn   1,file_chdir_fail
        move    3,-3(17)
        caie    3,1                    ; VFS_TYPE_DIR
        jrst    file_chdir_fail
        move    1,-4(17)
        move    2,file_table
        movem   1,-1(2)
        setz    1,
file_chdir_done:
        sub     17,[5,,5]
        popj    17,
file_chdir_fail:
        seto    1,
        jrst    file_chdir_done

; int file_mkdir(const kword_t *path, unsigned int mode)
; Mode is saved below one parent vnode and one five-word name.  The parent
; vnode slot becomes the ignored output vnode once the parent is loaded.
        .globl  vfs_mkdir
        .globl  file_mkdir
file_mkdir:
        push    17,2
        add     17,[6,,6]
        movei   2,-5(17)
        movei   3,-4(17)
        pushj   17,file_parent_path
        jumpn   1,file_mkdir_fail
        move    3,-6(17)
        move    1,-5(17)
        movei   2,-4(17)
        movei   4,-5(17)
        pushj   17,vfs_mkdir
file_mkdir_done:
        sub     17,[6,,6]
        pop     17,2
        popj    17,
file_mkdir_fail:
        seto    1,
        jrst    file_mkdir_done

; int file_unlink(const kword_t *path)
; Six stack words hold one parent vnode and one five-word name.
        .globl  vfs_unlink
        .globl  file_unlink
file_unlink:
        add     17,[6,,6]
        movei   2,-5(17)
        movei   3,-4(17)
        pushj   17,file_parent_path
        jumpn   1,file_unlink_fail
        move    1,-5(17)
        movei   2,-4(17)
        pushj   17,vfs_unlink
file_unlink_done:
        sub     17,[6,,6]
        popj    17,
file_unlink_fail:
        seto    1,
        jrst    file_unlink_done

; int file_truncate(const kword_t *path, kword_t size_chars)
; One saved size argument plus one vnode local.
        .globl  vfs_truncate
        .globl  file_truncate
file_truncate:
        push    17,2
        add     17,[1,,1]
        movei   2,(17)
        pushj   17,file_lookup_path
        jumpn   1,file_path_onearg_fail
        move    3,-1(17)
        move    2,3
        addi    2,3
        lsh     2,-2
        move    1,(17)
        pushj   17,vfs_truncate
file_truncate_done:
        jrst    file_path_onearg_done

; int file_stat_path(const kword_t *path, struct vfs_stat *st)
; One saved argument plus one vnode local.
        .globl  file_lookup_path
        .globl  vfs_stat
        .globl  file_stat_path
file_stat_path:
        push    17,2
        add     17,[1,,1]
        movei   2,(17)
        pushj   17,file_lookup_path
        jumpn   1,file_stat_path_fail
        move    1,(17)
        move    2,-1(17)
        pushj   17,vfs_stat
file_stat_path_done:
        jrst    file_path_onearg_done
file_stat_path_fail:
file_path_onearg_fail:
        seto    1,
file_path_onearg_done:
        sub     17,[1,,1]
        pop     17,2
        popj    17,

; struct file *file_find(int fd)
; File descriptors 3..15 map directly onto the 13 FILE records.
        .globl  file_find
file_find:
        caige   1,3
        jrst    pdp10_ret_zero
        caile   1,017
        jrst    pdp10_ret_zero
        subi    1,3
        lsh     1,1
        add     1,file_table
        skipn   (1)
        jrst    pdp10_ret_zero
        popj    17,

; int file_new_fd(vnode_t node, unsigned int flags, int isdir)
; Pack the persistent READ/WRITE/DIR bits and lock-family token into the nine
; vnode bits which are zero for every current DAIMOS provider/mount/kind.
; OPEN-only APPEND/CREATE/TRUNC bits are deliberately not retained.
        .globl  file_new_fd
file_new_fd:
        tlne    1,707070               ; packed metadata bits must be free
        jrst    file_new_fd_fail
        trne    2,1
        tlo     1,400000               ; FILE_META_READ
        trne    2,2
        tlo     1,200000               ; FILE_META_WRITE
        jumpe   3,file_new_fd_nodir
        tlo     1,100000               ; FILE_META_DIR
file_new_fd_nodir:
        move    4,file_table
        movei   5,3                    ; descriptor / lock-family token
        movei   6,015                  ; FILE_NFILE = 13
file_new_fd_scan:
        skipn   (4)
        jrst    file_new_fd_store
        addi    4,2
        addi    5,1
        sojg    6,file_new_fd_scan
file_new_fd_fail:
        jrst    pdp10_ret_neg1
file_new_fd_store:
; Scatter fd bits 0..2 into LH 007000 and bit 3 into LH 000040.
        move    0,5
        andi    0,7
        lsh     0,033
        ior     1,0
        trne    5,010
        tlo     1,000040
        movem   1,(4)
        setzm   1(4)
        move    1,5
        popj    17,

; int file_getcwd_pseudo(vnode_t node, kword_t *buf,
;     unsigned int nwords)
; Only fixed DAIMOS synthetic directories can be current directories.
        .globl  file_getcwd_pseudo
        .globl  procfs_getcwd_slot
file_getcwd_pseudo:
        move    4,2
        move    5,3
        jumpe   5,file_pseudo_zero_done
file_pseudo_zero:
        setzm   (4)
        addi    4,1
        sojg    5,file_pseudo_zero
file_pseudo_zero_done:
        camn    1,[020001000000]       ; /DEVICE
        jrst    file_pseudo_device
        camn    1,[020003000000]       ; /DEVICE/CTY0
        jrst    file_pseudo_cty
        camn    1,[030001000000]       ; /PROC
        jrst    file_pseudo_proc
        hlrz    4,1
        caie    4,030002               ; /PROC/<runtime slot>
        jrst    pdp10_ret_neg1
        hrrz    1,1                    ; slot
        ; AC2 already points at buf; AC3 is nwords.
        jrst    procfs_getcwd_slot
file_pseudo_device:
        caige   3,3
        jrst    pdp10_ret_neg1
        movei   4,7
        movem   4,(2)
        move    4,[0174445665143]      ; SIXBIT //DEVIC/
        movem   4,1(2)
        movsi   4,0450000              ; SIXBIT /E     /
        movem   4,2(2)
        jrst    pdp10_ret_zero
file_pseudo_cty:
        caige   3,4
        jrst    pdp10_ret_neg1
        movei   4,014
        movem   4,(2)
        move    4,[0174445665143]      ; SIXBIT //DEVIC/
        movem   4,1(2)
        move    4,[-0326034130660]     ; SIXBIT /E/CTY0/
        movem   4,2(2)
        jrst    pdp10_ret_zero
file_pseudo_proc:
        caige   3,2
        jrst    pdp10_ret_neg1
        movei   4,5
        movem   4,(2)
        move    4,[0176062574300]      ; SIXBIT //PROC /
        movem   4,1(2)
        jrst    pdp10_ret_zero
