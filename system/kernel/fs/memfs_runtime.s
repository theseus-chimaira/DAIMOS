; memfs_runtime.s -- compact resident MEMFS primitives for PDP-6/PDP-10.
        .text
        .globl  vfs_name_valid
        .globl  fs_copy_words
        .globl  memfs_data_ensure
        .globl  memfs_data_dirty
        .globl  kret_zero
        .globl  kret_neg1

; int memfs_read_words(const struct memfs *fs, vnode_t node,
;     unsigned int off, kword_t *buf, unsigned int nwords)
        .globl  memfs_read_words
memfs_read_words:
        jumpe   4,kret_neg1
        move    0,4             ; preserve destination; slot clobbers AC4
        move    7,1             ; preserve fs across slot validation
        pushj   17,memfs_slot   ; AC5=np, AC6=meta
        jumpl   1,kret_neg1
        move    2,1             ; preserve slot for possible fault-in
        ldb     1,[POINT 3,5(5),20]
        caie    1,2             ; regular file
        jrst    kret_neg1
        move    4,6
        andi    4,2             ; IMAGE flag for source selection
        hrrz    6,6(5)          ; stored words
        jumpl   3,kret_zero ; unsigned off exceeds 18-bit length
        caml    3,6             ; off < stored words
        jrst    kret_zero
        sub     6,3             ; available words
        move    1,7             ; restore fs before reusing AC7
        move    7,-1(17)        ; nwords, fifth C argument
        jumpl   7,memfs_read_count
        camle   6,7
        move    6,7
memfs_read_count:
        jumpn   4,memfs_read_source_ready
        push    17,0            ; destination
        push    17,3            ; off
        push    17,6            ; count
        push    17,2            ; slot
        push    17,1            ; fs
        pushj   17,memfs_data_ensure
        jumpn   1,memfs_read_ensure_fail
        pop     17,7            ; fs
        pop     17,1
        pop     17,6
        pop     17,3
        pop     17,0
        move    5,1
        imuli   5,7
        add     5,(7)
        setz    4,
        jrst    memfs_read_source_ready
memfs_read_ensure_fail:
        sub     17,[5,,5]
        jrst    kret_neg1
memfs_read_source_ready:
        hlrz    2,6(5)
        jumpe   4,memfs_read_pool
        add     2,5(1)          ; image_data
        jrst    memfs_read_source
memfs_read_pool:
        ; Mutable data words are direct physical addresses.
memfs_read_source:
        add     2,3
        move    1,2             ; source
        move    2,0             ; destination
        move    3,6             ; count
        pushj   17,fs_copy_words
        move    1,6
        popj    17,

; int memfs_write_words(struct memfs *fs, vnode_t node,
;     unsigned int off, const kword_t *buf, unsigned int nwords)
        .globl  memfs_write_words
memfs_write_words:
        jumpe   4,kret_neg1
        move    0,4             ; preserve source; slot clobbers AC4
        move    7,1             ; preserve fs
        pushj   17,memfs_slot   ; AC5=np, AC6=meta
        jumpl   1,kret_neg1
        move    2,1             ; preserve slot
        ldb     4,[POINT 3,5(5),20]
        caie    4,2             ; regular file
        jrst    kret_neg1
        trnn    6,4             ; writable
        jrst    kret_neg1

; Compute need = off+nwords and reject 36-bit unsigned wrap.
        move    4,-1(17)        ; nwords
        move    6,4
        add     6,3             ; need
        move    1,6
        tlc     1,0400000
        move    4,3
        tlc     4,0400000
        camge   1,4             ; need >= off (unsigned) => no overflow
        jrst    kret_neg1
memfs_write_need_ok:
        hrrz    4,6(5)          ; current word count
        jumpl   6,memfs_write_grow
        camg    6,4
        jrst    memfs_write_ready
memfs_write_grow:
; Preserve live arguments across the internal resize call.
        push    17,7             ; fs
        push    17,2             ; slot
        push    17,3             ; off
        push    17,0             ; source
        move    3,6             ; new word count
        move    2,-2(17)        ; slot
        move    1,-3(17)        ; fs
        pushj   17,memfs_resize
        move    6,1             ; preserve resize status
        pop     17,0
        pop     17,3
        pop     17,2
        pop     17,7
        jumpn   6,kret_neg1

memfs_write_ready:
; Recompute np after resize and copy nwords into its resident extent.
        push    17,7
        push    17,2
        push    17,3
        push    17,0
        move    1,7
        pushj   17,memfs_data_ensure
        jumpn   1,memfs_write_ensure_fail
        move    1,-2(17)
        pushj   17,memfs_data_dirty
        pop     17,0
        pop     17,3
        pop     17,2
        pop     17,7
        move    5,2
        imuli   5,7
        add     5,(7)           ; np
        hlrz    6,6(5)
        ; Mutable data words are direct physical addresses.
        add     6,3             ; + off
        move    1,0             ; source
        move    2,6             ; destination
        move    3,-1(17)        ; count
        pushj   17,fs_copy_words

        move    1,-1(17)
        popj    17,
memfs_write_ensure_fail:
        sub     17,[4,,4]
        jrst    kret_neg1

; int memfs_slot(const struct memfs *fs, vnode_t node)
; Return the validated slot directly, or -1.  On success AC5=np, AC6=meta.
        .globl  memfs_slot
memfs_slot:
        jumpe   1,kret_neg1
        hlrz    4,2
        andi    4,0770077
        caie    4,040001               ; provider 4, node kind 1
        jrst    kret_neg1
        hrrz    4,2
        caml    4,1(1)
        jrst    kret_neg1
        move    5,4
        imuli   5,7
        add     5,(1)
        move    6,5(5)
        trnn    6,1
        jrst    kret_neg1
        move    1,4
        popj    17,

; int memfs_find_child(const struct memfs *fs, unsigned int parent,
;     const struct vfs_name *name)
; Return the child slot directly, or -1.
        .globl  memfs_find_child
memfs_find_child:
        push    17,010                  ; name survives name comparison
        move    010,3
        move    7,1                     ; fs; helper leaves AC7 alone
        move    0,2                     ; parent
        move    5,(7)
        addi    5,7                     ; slot 1
        movei   6,1
memfs_find_child_loop:
        caml    6,1(7)
        jrst    memfs_find_child_fail
        move    4,5(5)
        trnn    4,1
        jrst    memfs_find_child_next
        hlrz    4,4
        came    4,0
        jrst    memfs_find_child_next
        move    1,5
        move    2,010
        movei   3,5                    ; chars + four packed SIXBIT words
        pushj   17,vfs_name_words_equal
        jumpn   1,memfs_find_child_found
memfs_find_child_next:
        addi    5,7
        addi    6,1
        jrst    memfs_find_child_loop
memfs_find_child_found:
        move    1,6
        jrst    memfs_find_child_done
memfs_find_child_fail:
        seto    1,
memfs_find_child_done:
        jrst    memfs_restore1

; void memfs_clear_node(struct memfs_node *np)
        .globl  memfs_clear_node
memfs_clear_node:
        movei   2,7
        jrst    fs_zero_words

; Compact namespace/mutation operations.  The slot helpers above return
; their result directly, avoiding the stack temporaries emitted by C.
        .globl  memfs_lookup
memfs_lookup:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                   ; fs
        move    011,2                   ; dir
        move    012,3                   ; name
        move    013,4                   ; nodep
        jumpe   013,memfs_restore4_fail
        move    1,012
        pushj   17,vfs_name_valid
        jumpe   1,memfs_restore4_fail
        move    1,010
        move    2,011
        pushj   17,memfs_slot
        jumpl   1,memfs_restore4_fail
        move    2,1                     ; parent slot
        ldb     4,[POINT 3,5(5),20]
        caie    4,1
        jrst    memfs_restore4_fail
        move    1,010
        move    3,012
        pushj   17,memfs_find_child
        jumpl   1,memfs_restore4_fail
        hrrz    1,1
        tlo     1,040001
        movem   1,(013)
memfs_restore4_zero:
        setz    1,
        jrst    memfs_restore4
memfs_restore4_fail:
        seto    1,
        jrst    memfs_restore4

        .globl  memfs_create
memfs_create:
        movei   5,2                     ; VFS_TYPE_REG
        jrst    memfs_new_node
        .globl  memfs_mkfifo
memfs_mkfifo:
        movei   5,7                     ; VFS_TYPE_FIFO
        jrst    memfs_new_node
        .globl  memfs_mkdir
memfs_mkdir:
        movei   5,1                     ; VFS_TYPE_DIR
memfs_new_node:
        move    7,-1(17)                ; C arg 5: nodep
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        move    010,1                   ; fs
        move    011,2                   ; dir, then parent slot
        move    012,3                   ; name
        move    013,4                   ; mode
        move    014,7                   ; nodep
        move    015,5                   ; type
        jumpe   010,memfs_new_fail
        jumpe   014,memfs_new_fail
        move    1,012
        pushj   17,vfs_name_valid
        jumpe   1,memfs_new_fail
        move    1,010
        move    2,011
        pushj   17,memfs_slot
        jumpl   1,memfs_new_fail
        move    011,1                   ; parent slot
        ldb     4,[POINT 3,5(5),20]
        caie    4,1
        jrst    memfs_new_fail
        trnn    6,4                     ; parent writable
        jrst    memfs_new_fail
        move    1,010
        move    2,011
        move    3,012
        pushj   17,memfs_find_child
        jumpge  1,memfs_new_fail        ; duplicate name
        move    3,(010)
        addi    3,7                     ; slot 1
        movei   4,1
memfs_new_free_loop:
        caml    4,1(010)
        jrst    memfs_new_fail
        move    5,5(3)
        trnn    5,1
        jrst    memfs_new_free_found
        addi    3,7
        addi    4,1
        jrst    memfs_new_free_loop
memfs_new_free_found:
        move    7,4                     ; new slot
        move    6,7
        imuli   6,7
        add     6,(010)                 ; np
        move    1,6
        pushj   17,memfs_clear_node
        move    1,6
        hrl     1,012
        blt     1,4(6)                  ; five-word name
        hrlz    3,011                   ; parent in high half
        iori    3,5                     ; USED|WRITABLE
        movem   3,5(6)
        move    4,015
        dpb     4,[POINT 3,5(6),20]     ; type
        move    4,013
        dpb     4,[POINT 12,5(6),32]    ; mode
        setzm   6(6)                     ; no data allocation yet
        hrrz    1,7
        tlo     1,040001
        movem   1,(014)
        setz    1,
        jrst    memfs_new_done
memfs_new_fail:
        seto    1,
memfs_new_done:
        jrst    memfs_restore6

        .globl  memfs_unlink
memfs_unlink:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                   ; fs
        move    011,2                   ; dir, then parent
        move    012,3                   ; name
        jumpe   010,memfs_restore4_fail
        move    1,012
        pushj   17,vfs_name_valid
        jumpe   1,memfs_restore4_fail
        move    1,010
        move    2,011
        pushj   17,memfs_slot
        jumpl   1,memfs_restore4_fail
        move    011,1
        ldb     4,[POINT 3,5(5),20]
        caie    4,1
        jrst    memfs_restore4_fail
        trnn    6,4
        jrst    memfs_restore4_fail
        move    1,010
        move    2,011
        move    3,012
        pushj   17,memfs_find_child
        jumpl   1,memfs_restore4_fail
        move    013,1                   ; victim slot
        move    3,(010)
        addi    3,7                     ; slot 1
        movei   4,1
memfs_unlink_child_loop:
        caml    4,1(010)
        jrst    memfs_unlink_no_children
        move    5,5(3)
        trnn    5,1
        jrst    memfs_unlink_child_next
        hlrz    5,5
        camn    5,013
        jrst    memfs_restore4_fail
memfs_unlink_child_next:
        addi    3,7
        addi    4,1
        jrst    memfs_unlink_child_loop
memfs_unlink_no_children:
        move    4,013
        imuli   4,7
        add     4,(010)
        ldb     5,[POINT 3,5(4),20]
        caie    5,2
        jrst    memfs_unlink_clear
        move    1,010
        move    2,013
        movei   3,0
        pushj   17,memfs_resize
        jumpn   1,memfs_restore4_fail
memfs_unlink_clear:
        move    1,013
        imuli   1,7
        add     1,(010)
        pushj   17,memfs_clear_node
        jrst    memfs_restore4_zero

        .globl  memfs_rename
memfs_rename:
        move    7,-1(17)                ; C arg 5: newname
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        push    17,016
        move    010,1                   ; fs
        move    011,2                   ; olddir, later victim slot
        move    012,3                   ; oldname
        move    013,4                   ; newdir
        move    014,7                   ; newname
        jumpe   010,memfs_rename_fail
        move    1,012
        pushj   17,vfs_name_valid
        jumpe   1,memfs_rename_fail
        move    1,014
        pushj   17,vfs_name_valid
        jumpe   1,memfs_rename_fail
        move    1,010
        move    2,011
        pushj   17,memfs_slot
        jumpl   1,memfs_rename_fail
        move    015,1                   ; old parent
        ldb     4,[POINT 3,5(5),20]
        caie    4,1
        jrst    memfs_rename_fail
        trnn    6,4
        jrst    memfs_rename_fail
        move    1,010
        move    2,013
        pushj   17,memfs_slot
        jumpl   1,memfs_rename_fail
        move    016,1                   ; new parent
        ldb     4,[POINT 3,5(5),20]
        caie    4,1
        jrst    memfs_rename_fail
        trnn    6,4
        jrst    memfs_rename_fail
        move    1,010
        move    2,015
        move    3,012
        pushj   17,memfs_find_child
        jumpl   1,memfs_rename_fail
        move    011,1                   ; victim slot
        move    1,010
        move    2,016
        move    3,014
        pushj   17,memfs_find_child
        jumpge  1,memfs_rename_fail     ; destination exists
        move    4,011
        imuli   4,7
        add     4,(010)                 ; victim np
        ldb     5,[POINT 3,5(4),20]
        caie    5,1
        jrst    memfs_rename_apply
        ; Prevent moving a directory below itself.
        move    7,016
memfs_rename_up:
        camn    7,011
        jrst    memfs_rename_fail
        jumpe   7,memfs_rename_apply
        caml    7,1(010)
        jrst    memfs_rename_fail
        move    5,7
        imuli   5,7
        add     5,(010)
        move    6,5(5)
        trnn    6,1
        jrst    memfs_rename_fail
        hlrz    7,6
        jrst    memfs_rename_up
memfs_rename_apply:
        move    4,011
        imuli   4,7
        add     4,(010)
        hrlm    016,5(4)
        move    1,4
        hrl     1,014
        blt     1,4(4)
        setz    1,
        jrst    memfs_rename_done
memfs_rename_fail:
        seto    1,
memfs_rename_done:
memfs_restore7:
        pop     17,016
memfs_restore6:
        pop     17,015
memfs_restore5:
        pop     17,014
memfs_restore4:
        pop     17,013
memfs_restore3:
        pop     17,012
memfs_restore2:
        pop     17,011
memfs_restore1:
        pop     17,010
        popj    17,

        .globl  memfs_chmod
memfs_chmod:
        jumpe   1,kret_neg1
        pushj   17,memfs_slot           ; mode remains in AC3
        jumpl   1,kret_neg1
        trnn    6,4
        jrst    kret_neg1
        move    4,3
        andi    4,07777
        dpb     4,[POINT 12,5(5),32]
        jrst    kret_zero

        .globl  memfs_truncate_words
memfs_truncate_words:
        push    17,1                     ; fs
        push    17,0                     ; slot placeholder
        pushj   17,memfs_slot           ; words remain in AC3
        jumpl   1,memfs_truncate_fail
        movem   1,(17)                   ; slot
        ldb     4,[POINT 3,5(5),20]
        caie    4,2
        jrst    memfs_truncate_fail
        move    2,(17)
        move    1,-1(17)
        pushj   17,memfs_resize
        jumpn   1,memfs_truncate_fail
        setz    1,
        jrst    memfs_truncate_done
memfs_truncate_fail:
        seto    1,
memfs_truncate_done:
        sub     17,[2,,2]
        popj    17,

; int memfs_readdir(const struct memfs *fs, vnode_t dir,
;     unsigned int off, struct vfs_dirent *ent)
        .globl  memfs_readdir
memfs_readdir:
        jumpe   4,kret_neg1
        move    0,4                     ; memfs_slot clobbers AC4
        move    7,1
        pushj   17,memfs_slot
        jumpl   1,kret_neg1
        move    2,1                     ; parent slot
        move    1,7                     ; restore fs
        move    4,0                     ; restore ent
        ldb     0,[POINT 3,5(5),20]
        caie    0,1                     ; directory
        jrst    kret_neg1

        move    5,(1)
        addi    5,7                     ; slot 1
        movei   6,1
        movei   0,0                     ; matching-entry ordinal
memfs_readdir_loop:
        caml    6,1(1)
        jrst    kret_zero
        move    7,5(5)
        trnn    7,1
        jrst    memfs_readdir_next
        hlrz    7,7
        came    7,2
        jrst    memfs_readdir_next
        camn    0,3
        jrst    memfs_readdir_found
        addi    0,1
memfs_readdir_next:
        addi    5,7
        addi    6,1
        jrst    memfs_readdir_loop
memfs_readdir_found:
        move    0,5
        hrl     0,5
        hrr     0,4
        blt     0,4(4)                  ; copy five-word name
        ldb     0,[POINT 3,5(5),20]
        movem   0,5(4)
        jrst    kret_one

; int memfs_stat(const struct memfs *fs, vnode_t node,
;     struct vfs_stat *st)
        .globl  memfs_stat
memfs_stat:
        jumpe   3,kret_neg1
        pushj   17,memfs_slot
        jumpl   1,kret_neg1
        ldb     4,[POINT 3,5(5),20]
        movem   4,(3)
        ldb     4,[POINT 12,5(5),32]
        movem   4,1(3)
        setzm   2(3)                    ; reserved
        hrrz    4,6(5)
        movem   4,3(3)
        jrst    kret_zero

; int memfs_parent(const struct memfs *fs, vnode_t node,
;     vnode_t *parentp, struct vfs_name *namep)
        .globl  memfs_parent
memfs_parent:
        jumpe   3,kret_neg1
        move    0,4                     ; memfs_slot clobbers AC4
        move    7,1
        pushj   17,memfs_slot
        jumpl   1,kret_neg1
        move    2,5                     ; preserve child np
        move    1,7                     ; restore fs
        move    4,0                     ; restore optional namep
        hlrz    5,6                     ; parent slot
        caml    5,1(1)
        jrst    kret_neg1
        move    7,5
        imuli   7,7
        add     7,(1)
        move    0,5(7)
        trnn    0,1
        jrst    kret_neg1
        hrrz    0,5
        tlo     0,040001
        movem   0,(3)
        jumpe   4,memfs_parent_ok
        move    0,2
        hrl     0,2
        hrr     0,4
        blt     0,4(4)                  ; copy child name
memfs_parent_ok:
        jrst    kret_zero

; Singleton resident MEMFS state.  A MEMFS MRES represents exactly one mounted
; in-memory filesystem; carrying a context pointer through a C switch was dead
; generality.
        .bss
        .globl  memfs_mres_fs
memfs_mres_fs:
        .block  6
memfs_mount_flags:
        .block  1
        .text

        .globl  vfs_name_words_equal
        .globl  fs_zero_words
        .globl  fs_mres_context_vector_dispatch
        .globl  mm_alloc
        .globl  mm_free
        .globl  memfs_data_init
        .globl  memfs_data_destroy
        .globl  memfs_snapshot_mount
        .globl  vfs_mount
        .globl  memfs_mres_dispatch
        .globl  memfs_lookup
        .globl  memfs_create
        .globl  memfs_mkdir
        .globl  memfs_unlink
        .globl  memfs_rename
        .globl  memfs_chmod
        .globl  memfs_truncate_words
        .globl  memfs_read_words
        .globl  memfs_write_words

; Create the singleton MEMFS instance on demand.
;
; Namespace nodes remain one compact allocation.  Mutable file data is
; demand-allocated by memfs_data in a bounded number of MM chunks; its free
; lists live inside free chunk storage.  Growth may relocate the affected file
; but never compacts unrelated files, and a completely free chunk returns to MM.
;
; AC1 = already-resolved mount-point vnode
; AC2 = total words to allocate
; AC3 = MEMFS mount-policy flags; bit 0002 requests persistent backing.
;
; 64 seven-word nodes consume the first 0700 words.  Require at least
; 01100 words for file data so the existing 02000-word minimum is unchanged.  Dynamic owner 011 is reserved for the singleton MEMFS allocation.
memfs_mres_mount:
        skipe   memfs_mres_fs
        jrst    kret_neg1          ; singleton already instantiated
        trne    3,07775
        jrst    kret_neg1          ; reject unknown policy bits
        movem   3,memfs_mount_flags
        cail    2,02000
        jrst    memfs_mres_mount_size_ok
        jrst    kret_neg1
memfs_mres_mount_size_ok:
        push    17,1                    ; target vnode
        push    17,2                    ; total words
        push    17,[0]                  ; allocation base
        movei   5,(17)
        push    17,5                    ; fifth mm_alloc arg: basep
        movei   1,01000                 ; nodes + 64 backing descriptors
        movei   2,3                     ; MM_TYPE_KERNEL_DYNAMIC
        movei   3,011                   ; MEMFS_MM_OWNER
        setz    4,                      ; MM_ALLOC_LOW
        pushj   17,mm_alloc
        sub     17,[1,,1]
        jumpn   1,memfs_mres_mount_bad

        move    1,(17)
        movei   2,01000
        pushj   17,fs_zero_words
        move    5,(17)
        move    6,[0107775]             ; DIR, mode 0777, USED|WRITABLE
        movem   6,5(5)                  ; root-node meta

        movem   5,memfs_mres_fs
        movei   6,0100                  ; 64 node slots
        movem   6,memfs_mres_fs+1
        move    7,5
        addi    7,0700
        movem   7,memfs_mres_fs+2       ; per-node swap backing descriptors
        move    6,-1(17)                ; requested total-word ceiling
        subi    6,0700                  ; preserve old data-capacity semantics
        movem   6,memfs_mres_fs+3
        setzm   memfs_mres_fs+4         ; logical file words in use
        setzm   memfs_mres_fs+5         ; no immutable image backing
        movei   1,memfs_mres_fs
        move    2,6
        pushj   17,memfs_data_init

        movei   1,memfs_mres_fs
        move    2,memfs_mount_flags
        pushj   17,memfs_snapshot_mount
        jumpn   1,memfs_mres_mount_bad

        push    17,[0]                  ; mounted-root scratch
        movei   6,(17)
        push    17,6                    ; sixth arg: rootp
        push    17,[0]                  ; fifth arg: VFS_MOUNT_RW
        move    1,-5(17)                ; target vnode
        movei   2,4                     ; MEMFS_PROVIDER
        movei   3,1                     ; MEMFS_KIND_NODE
        setz    4,                      ; root node slot
        pushj   17,vfs_mount
        sub     17,[3,,3]
        jumpe   1,memfs_mres_mount_done

        movei   1,memfs_mres_fs
        movei   2,6
        pushj   17,fs_zero_words
        move    1,(17)
        movei   2,3
        movei   3,011
        pushj   17,mm_free
        seto    1,
        jrst    memfs_mres_mount_done
memfs_mres_mount_bad:
        seto    1,
memfs_mres_mount_done:
        sub     17,[3,,3]
        popj    17,

; Release all demand data and the namespace allocation when VFS unmounts MEMFS.  VFS has already
; rejected active FIFO users and synchronized the provider before this call,
; so no live vnode may retain storage after the allocations are returned.
memfs_mres_prepare_unmount:
        push    17,1                    ; struct memfs * context
        pushj   17,memfs_data_destroy   ; release demand-allocated data chunks
        move    1,(17)
        move    1,(1)                   ; namespace allocation base
        jumpe   1,memfs_mres_unmount_bad
        movei   2,3                     ; MM_TYPE_KERNEL_DYNAMIC
        movei   3,011                   ; MEMFS_MM_OWNER
        pushj   17,mm_free
        jumpn   1,memfs_mres_unmount_bad
        move    1,(17)
        movei   2,6
        pushj   17,fs_zero_words
        sub     17,[1,,1]
        jrst    kret_zero
memfs_mres_unmount_bad:
        sub     17,[1,,1]
        jrst    kret_neg1

; MEMINFO calls this exported entry directly; overwrite request a/b.
        .globl  memfs_mres_usage
memfs_mres_usage:
        move    2,memfs_mres_fs+4       ; used_words
        movem   2,1(1)
        move    2,memfs_mres_fs+3       ; pool_words
        movem   2,2(1)
        jrst    kret_zero

; Shutdown hook.  Snapshot publication is added behind this entry; until a
; persistent mount is active this is intentionally a zero-cost provider no-op.
        .globl  memfs_mres_shutdown
memfs_mres_shutdown:
        jrst    memfs_snapshot_shutdown

; CREATE op multiplexes regular files and FIFO nodes so FIFO support costs
; no additional permanent provider-vector slot.  Bit 010000 is outside the
; twelve-bit mode and is private to vfs_mkfifo().
memfs_mres_create:
        trnn    4,010000
        jrst    memfs_create
        andi    4,07777
        jrst    memfs_mkfifo

memfs_mres_dispatch:
memfs_mres_reg_dispatch:
        caie    6,023                   ; 19 decimal: MEMFS_MOUNT
        jrst    memfs_mres_not_mount
        jrst    memfs_mres_mount
memfs_mres_not_mount:
        move    7,[memfs_mres_vector]
        movei   0,memfs_mres_fs
        jrst    fs_mres_context_vector_dispatch

        .data
memfs_mres_vector:
        .word   memfs_lookup,,memfs_readdir
        .word   memfs_stat,,memfs_parent
        .word   memfs_parent,,memfs_mres_create
        .word   memfs_mkdir,,0
        .word   memfs_unlink,,memfs_rename
        .word   memfs_truncate_words,,memfs_chmod
        .word   memfs_read_words,,memfs_write_words
        .word   kret_zero,,memfs_mres_prepare_unmount
        .text
