; memfs_pdp10.s -- compact resident MEMFS primitives for PDP-6/PDP-10.
        .text
        .globl  vfs_name_valid
        .globl  fs_copy_words
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1

; void memfs_shift_after(struct memfs *fs, unsigned int start,
;     int delta, unsigned int exclude)
        .globl  memfs_shift_after
memfs_shift_after:
        move    5,(1)           ; nodes
        addi    5,010           ; slot 1, nodes are exactly 8 words
        movei   6,1
memfs_shift_check:
        camge   6,1(1)          ; i >= node_count
        jrst    memfs_shift_body
        popj    17,
memfs_shift_body:
        camn    6,4
        jrst    memfs_shift_next
        move    7,5(5)          ; meta
        andi    7,3             ; USED/IMAGE
        caie    7,1             ; used and not image-backed
        jrst    memfs_shift_next
        hlrz    7,7(5)          ; data word offset
        camge   7,2
        jrst    memfs_shift_next
        add     7,3             ; signed delta
        hrlm    7,7(5)          ; preserve low-half data length
memfs_shift_next:
        addi    6,1
        addi    5,010
        jrst    memfs_shift_check

; int memfs_resize(struct memfs *fs, unsigned int slot,
;     unsigned int words)
; fs layout: nodes,node_count,pool,pool_words,used_words,image_data.
; node layout is 8 words; meta at +5, packed data at +7.
        .globl  memfs_resize
memfs_resize:
        move    4,2
        lsh     4,3
        add     4,(1)           ; node = fs->nodes + slot
        move    0,5(4)
        andi    0,6             ; require WRITABLE and reject IMAGE
        caie    0,4
        jrst    memfs_resize_fail
        hrrz    5,7(4)          ; old word count
        camn    3,5
        jrst    memfs_resize_ok
        hlrz    6,7(4)          ; data start
        jumpl   3,memfs_resize_grow ; unsigned high half is always > old
        camg    3,5             ; new > old => grow
        jrst    memfs_resize_shrink
memfs_resize_grow:

; Grow the node by delta = new-old. Move following pool words upward,
; clear the inserted gap, then relocate later mutable-node offsets.
        move    7,3
        sub     7,5             ; delta
        jumpl   7,memfs_resize_fail ; cannot fit in the small resident pool
        move    0,3(1)
        sub     0,4(1)          ; available pool words
        camle   7,0
        jrst    memfs_resize_fail
        add     5,6             ; old end / shift threshold
        hrrm    3,7(4)          ; install new length; preserve data start
        move    4,2             ; exclude slot for shift_after
        move    0,2(1)          ; pool base
        move    2,0
        add     2,4(1)          ; src = pool + used_words
        move    3,2
        add     3,7             ; dst = src + delta
        move    6,0
        add     6,5             ; stop = pool + old end
memfs_resize_grow_move:
        camg    2,6
        jrst    memfs_resize_grow_zero_setup
        subi    2,1
        subi    3,1
        move    0,(2)
        movem   0,(3)
        jrst    memfs_resize_grow_move
memfs_resize_grow_zero_setup:
        move    2,6
        move    3,6
        add     3,7             ; end of inserted gap
memfs_resize_grow_zero:
        caml    2,3
        jrst    memfs_resize_grow_done
        setzm   (2)
        addi    2,1
        jrst    memfs_resize_grow_zero
memfs_resize_grow_done:
        addm    7,4(1)
        move    2,5             ; start = old end
        move    3,7             ; positive delta
        pushj   17,memfs_shift_after
        jrst    memfs_resize_ok

; Shrink by delta = old-new. Move following pool words downward and then
; relocate later mutable-node offsets by -delta.
memfs_resize_shrink:
        move    7,5
        sub     7,3             ; delta = old-new
        add     5,6             ; old end / shift threshold
        hrrm    3,7(4)          ; install new length; preserve data start
        move    4,2             ; exclude slot
        move    0,2(1)          ; pool base
        add     6,3             ; new end
        add     6,0             ; dst = pool + new end
        move    3,6
        add     3,7             ; src = dst + delta = old end
        add     0,4(1)          ; end = pool + used_words
memfs_resize_shrink_move:
        caml    3,0
        jrst    memfs_resize_shrink_done
        move    2,(3)
        movem   2,(6)
        addi    3,1
        addi    6,1
        jrst    memfs_resize_shrink_move
memfs_resize_shrink_done:
        move    2,4(1)
        sub     2,7
        movem   2,4(1)
        move    2,5             ; start = old end
        movn    3,7             ; negative delta
        pushj   17,memfs_shift_after
memfs_resize_ok:
        movei   1,0
        popj    17,
memfs_resize_fail:
        jrst    pdp10_ret_neg1

; int memfs_read_words(const struct memfs *fs, vnode_t node,
;     unsigned int off, kword_t *buf, unsigned int nwords)
        .globl  memfs_read_words
memfs_read_words:
        jumpe   4,memfs_read_fail
        hlrz    5,2
        andi    5,0770077
        caie    5,040001        ; MEMFS provider 4, node kind 1
        jrst    memfs_read_fail
        hrrz    5,2             ; slot
        caml    5,1(1)          ; slot < node_count
        jrst    memfs_read_fail
        move    6,5
        lsh     6,3
        add     6,(1)           ; np
        move    7,5(6)
        andi    7,1
        jumpe   7,memfs_read_fail
        ldb     7,[POINT 3,5(6),20]
        caie    7,2             ; regular file
        jrst    memfs_read_fail
        hrrz    5,7(6)          ; stored words
        jumpl   3,memfs_read_eof ; unsigned off exceeds 18-bit length
        caml    3,5             ; off < stored words
        jrst    memfs_read_eof
        sub     5,3             ; available words
        move    7,-1(17)        ; nwords, fifth C argument
        jumpl   7,memfs_read_count
        camle   5,7
        move    5,7
memfs_read_count:
        move    0,5(6)
        andi    0,2
        hlrz    2,7(6)
        jumpe   0,memfs_read_pool
        add     2,5(1)          ; image_data
        jrst    memfs_read_source
memfs_read_pool:
        add     2,2(1)          ; pool
memfs_read_source:
        add     2,3
        move    6,5             ; preserve return count
        move    1,2             ; source
        move    2,4             ; destination
        move    3,5             ; count
        pushj   17,fs_copy_words
        move    1,6
        popj    17,
memfs_read_eof:
        jrst    pdp10_ret_zero
memfs_read_fail:
        jrst    pdp10_ret_neg1

; int memfs_write_words(struct memfs *fs, vnode_t node,
;     unsigned int off, const kword_t *buf, unsigned int nwords,
;     kword_t size_chars)
        .globl  memfs_write_words
memfs_write_words:
        jumpe   4,memfs_write_fail
        hlrz    5,2
        andi    5,0770077
        caie    5,040001        ; MEMFS provider 4, node kind 1
        jrst    memfs_write_fail
        hrrz    5,2             ; slot
        caml    5,1(1)
        jrst    memfs_write_fail
        move    6,5
        lsh     6,3
        add     6,(1)           ; np
        move    7,5(6)
        andi    7,1
        jumpe   7,memfs_write_fail
        ldb     7,[POINT 3,5(6),20]
        caie    7,2
        jrst    memfs_write_fail
        move    7,5(6)
        andi    7,4
        jumpe   7,memfs_write_fail

; Compute need = off+nwords and reject 36-bit unsigned wrap.
        move    5,-1(17)        ; nwords
        move    7,5
        add     7,3             ; need
        move    0,7
        tlc     0,0400000
        move    5,3
        tlc     5,0400000
        caml    0,5             ; need < off (unsigned) => overflow
        jrst    memfs_write_need_ok
        jrst    memfs_write_fail
memfs_write_need_ok:
        hrrz    5,7(6)          ; current word count
        jumpl   7,memfs_write_grow
        camg    7,5
        jrst    memfs_write_ready
memfs_write_grow:
; Preserve the four register arguments across the internal resize call.
        push    17,1
        push    17,2
        push    17,3
        push    17,4
        move    3,7
        move    2,-2(17)        ; saved vnode => slot in low half
        hrrz    2,2
        move    1,-3(17)        ; saved fs
        pushj   17,memfs_resize
        move    0,1
        pop     17,4
        pop     17,3
        pop     17,2
        pop     17,1
        jumpn   0,memfs_write_fail

memfs_write_ready:
; Recompute np after resize and copy nwords into the mutable pool.
        hrrz    5,2
        lsh     5,3
        add     5,(1)           ; np
        hlrz    6,7(5)
        add     6,2(1)          ; pool + data word
        add     6,3             ; + off
        move    7,-1(17)        ; count
        move    1,4             ; source
        move    2,6             ; destination
        move    3,7             ; count
        pushj   17,fs_copy_words

memfs_write_size:
; size_chars is unsigned 36-bit state, so compare after toggling sign bits.
        move    6,-2(17)
        move    0,6
        tlc     0,0400000
        move    7,6(5)
        tlc     7,0400000
        camle   0,7
        movem   6,6(5)
        move    1,-1(17)
        popj    17,
memfs_write_fail:
        jrst    pdp10_ret_neg1

; int memfs_slot(const struct memfs *fs, vnode_t node)
; Return the validated slot directly, or -1.  On success AC5=np, AC6=meta.
        .globl  memfs_slot
memfs_slot:
        jumpe   1,memfs_slot_fail
        hlrz    4,2
        andi    4,0770077
        caie    4,040001               ; provider 4, node kind 1
        jrst    memfs_slot_fail
        hrrz    4,2
        caml    4,1(1)
        jrst    memfs_slot_fail
        move    5,4
        lsh     5,3
        add     5,(1)
        move    6,5(5)
        trnn    6,1
        jrst    memfs_slot_fail
        move    1,4
        popj    17,
memfs_slot_fail:
        jrst    pdp10_ret_neg1

; int memfs_find_child(const struct memfs *fs, unsigned int parent,
;     const struct vfs_name *name)
; Return the child slot directly, or -1.
        .globl  memfs_find_child
memfs_find_child:
        move    5,(1)
        addi    5,010                  ; slot 1
        movei   6,1
memfs_find_child_loop:
        caml    6,1(1)
        jrst    memfs_find_child_fail
        move    7,5(5)
        trnn    7,1
        jrst    memfs_find_child_next
        hlrz    7,7
        came    7,2
        jrst    memfs_find_child_next
        push    17,1
        push    17,2
        push    17,3
        move    1,5
        move    2,3
        movei   3,5                    ; chars + four SIXBIT words
        pushj   17,fs_words_equal
        move    0,1
        pop     17,3
        pop     17,2
        pop     17,1
        jumpe   0,memfs_find_child_next
        move    1,6
        popj    17,
memfs_find_child_next:
        addi    5,010
        addi    6,1
        jrst    memfs_find_child_loop
memfs_find_child_fail:
        jrst    pdp10_ret_neg1

; int memfs_free_slot(const struct memfs *fs)
; Return the free slot directly, or -1.
        .globl  memfs_free_slot
memfs_free_slot:
        move    3,(1)
        addi    3,010
        movei   4,1
memfs_free_slot_loop:
        caml    4,1(1)
        jrst    memfs_free_slot_fail
        move    5,5(3)
        trnn    5,1
        jrst    memfs_free_slot_found
        addi    3,010
        addi    4,1
        jrst    memfs_free_slot_loop
memfs_free_slot_found:
        move    1,4
        popj    17,
memfs_free_slot_fail:
        jrst    pdp10_ret_neg1

; int memfs_has_children(const struct memfs *fs, unsigned int slot)
        .globl  memfs_has_children
memfs_has_children:
        move    3,(1)
        addi    3,010
        movei   4,1
memfs_has_children_loop:
        caml    4,1(1)
        jrst    memfs_has_children_none
        move    5,5(3)
        trnn    5,1
        jrst    memfs_has_children_next
        hlrz    5,5
        camn    5,2
        jrst    memfs_has_children_yes
memfs_has_children_next:
        addi    3,010
        addi    4,1
        jrst    memfs_has_children_loop
memfs_has_children_yes:
        movei   1,1
        popj    17,
memfs_has_children_none:
        jrst    pdp10_ret_zero

; void memfs_clear_node(struct memfs_node *np)
        .globl  memfs_clear_node
memfs_clear_node:
        movei   2,010
        jrst    fs_zero_words

        .globl  memfs_node_handle
memfs_node_handle:
        hrrz    1,1
        tlo     1,040001
        popj    17,

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
        jumpe   013,memfs_lookup_fail
        move    1,012
        pushj   17,vfs_name_valid
        jumpe   1,memfs_lookup_fail
        move    1,010
        move    2,011
        pushj   17,memfs_slot
        jumpl   1,memfs_lookup_fail
        move    2,1                     ; parent slot
        ldb     4,[POINT 3,5(5),20]
        caie    4,1
        jrst    memfs_lookup_fail
        move    1,010
        move    3,012
        pushj   17,memfs_find_child
        jumpl   1,memfs_lookup_fail
        hrrz    1,1
        tlo     1,040001
        movem   1,(013)
        setz    1,
        jrst    memfs_lookup_done
memfs_lookup_fail:
        seto    1,
memfs_lookup_done:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

        .globl  memfs_create
memfs_create:
        movei   5,2                     ; VFS_TYPE_REG
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
        move    1,010
        pushj   17,memfs_free_slot
        jumpl   1,memfs_new_fail
        move    7,1                     ; new slot
        move    6,1
        lsh     6,3
        add     6,(010)                 ; np
        move    1,6
        pushj   17,memfs_clear_node
        move    1,6
        hrl     1,012
        blt     1,4(6)                  ; five-word name
        hrlz    3,011                   ; parent in high half
        move    4,015
        andi    4,7
        lsh     4,017                   ; type << 15
        ior     3,4
        move    4,013
        andi    4,07777
        lsh     4,3
        ior     3,4
        iori    3,5                     ; USED|WRITABLE
        movem   3,5(6)
        move    4,4(010)                ; used_words
        hrlzm   4,7(6)
        move    1,7
        pushj   17,memfs_node_handle
        movem   1,(014)
        setz    1,
        jrst    memfs_new_done
memfs_new_fail:
        seto    1,
memfs_new_done:
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

        .globl  memfs_unlink
memfs_unlink:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                   ; fs
        move    011,2                   ; dir, then parent
        move    012,3                   ; name
        jumpe   010,memfs_unlink_fail
        skipn   5(010)
        jrst    memfs_unlink_fail
        move    1,012
        pushj   17,vfs_name_valid
        jumpe   1,memfs_unlink_fail
        move    1,010
        move    2,011
        pushj   17,memfs_slot
        jumpl   1,memfs_unlink_fail
        move    011,1
        ldb     4,[POINT 3,5(5),20]
        caie    4,1
        jrst    memfs_unlink_fail
        trnn    6,4
        jrst    memfs_unlink_fail
        move    1,010
        move    2,011
        move    3,012
        pushj   17,memfs_find_child
        jumpl   1,memfs_unlink_fail
        move    013,1                   ; victim slot
        move    2,1
        move    1,010
        pushj   17,memfs_has_children
        jumpn   1,memfs_unlink_fail
        move    4,013
        lsh     4,3
        add     4,(010)
        ldb     5,[POINT 3,5(4),20]
        caie    5,2
        jrst    memfs_unlink_clear
        move    1,010
        move    2,013
        movei   3,0
        pushj   17,memfs_resize
        jumpn   1,memfs_unlink_fail
memfs_unlink_clear:
        move    1,013
        lsh     1,3
        add     1,(010)
        pushj   17,memfs_clear_node
        setz    1,
        jrst    memfs_unlink_done
memfs_unlink_fail:
        seto    1,
memfs_unlink_done:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

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
        skipn   5(010)
        jrst    memfs_rename_fail
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
        lsh     4,3
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
        lsh     5,3
        add     5,(010)
        move    6,5(5)
        trnn    6,1
        jrst    memfs_rename_fail
        hlrz    7,6
        jrst    memfs_rename_up
memfs_rename_apply:
        move    4,011
        lsh     4,3
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
        pop     17,016
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

        .globl  memfs_chmod
memfs_chmod:
        jumpe   1,memfs_chmod_fail
        skipn   5(1)
        jrst    memfs_chmod_fail
        pushj   17,memfs_slot           ; mode remains in AC3
        jumpl   1,memfs_chmod_fail
        trnn    6,4
        jrst    memfs_chmod_fail
        move    4,3
        andi    4,07777
        dpb     4,[POINT 12,5(5),32]
        setz    1,
        popj    17,
memfs_chmod_fail:
        seto    1,
        popj    17,

        .globl  memfs_truncate_words
memfs_truncate_words:
        push    17,1                     ; fs
        push    17,4                     ; size chars
        push    17,0                     ; slot placeholder
        pushj   17,memfs_slot           ; words remain in AC3
        jumpl   1,memfs_truncate_fail
        movem   1,(17)                  ; slot
        ldb     4,[POINT 3,5(5),20]
        caie    4,2
        jrst    memfs_truncate_fail
        move    2,(17)                  ; slot
        move    1,-2(17)                ; fs
        pushj   17,memfs_resize
        jumpn   1,memfs_truncate_fail
        move    4,(17)
        lsh     4,3
        move    5,-2(17)
        add     4,(5)
        move    5,-1(17)                ; size chars
        movem   5,6(4)
        setz    1,
        jrst    memfs_truncate_done
memfs_truncate_fail:
        seto    1,
memfs_truncate_done:
        adjsp   17,-3
        popj    17,

; int memfs_readdir(const struct memfs *fs, vnode_t dir,
;     unsigned int off, struct vfs_dirent *ent)
        .globl  memfs_readdir
memfs_readdir:
        jumpe   4,memfs_readdir_fail
        move    0,4                     ; memfs_slot clobbers AC4
        move    7,1
        pushj   17,memfs_slot
        jumpl   1,memfs_readdir_fail
        move    2,1                     ; parent slot
        move    1,7                     ; restore fs
        move    4,0                     ; restore ent
        ldb     0,[POINT 3,5(5),20]
        caie    0,1                     ; directory
        jrst    memfs_readdir_fail

        move    5,(1)
        addi    5,010                   ; slot 1
        movei   6,1
        movei   0,0                     ; matching-entry ordinal
memfs_readdir_loop:
        caml    6,1(1)
        jrst    memfs_readdir_eof
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
        addi    5,010
        addi    6,1
        jrst    memfs_readdir_loop
memfs_readdir_found:
        move    0,5
        hrl     0,5
        hrr     0,4
        blt     0,4(4)                  ; copy five-word name
        ldb     0,[POINT 3,5(5),20]
        movem   0,5(4)
        movei   1,1
        popj    17,
memfs_readdir_eof:
        jrst    pdp10_ret_zero
memfs_readdir_fail:
        jrst    pdp10_ret_neg1

; int memfs_stat(const struct memfs *fs, vnode_t node,
;     struct vfs_stat *st)
        .globl  memfs_stat
memfs_stat:
        jumpe   3,memfs_stat_fail
        pushj   17,memfs_slot
        jumpl   1,memfs_stat_fail
        ldb     4,[POINT 3,5(5),20]
        movem   4,(3)
        move    4,6
        lsh     4,-3
        andi    4,07777
        movem   4,1(3)
        move    4,6(5)
        movem   4,2(3)
        hrrz    4,7(5)
        movem   4,3(3)
        movei   1,0
        popj    17,
memfs_stat_fail:
        jrst    pdp10_ret_neg1

; int memfs_parent(const struct memfs *fs, vnode_t node,
;     vnode_t *parentp, struct vfs_name *namep)
        .globl  memfs_parent
memfs_parent:
        jumpe   3,memfs_parent_fail
        move    0,4                     ; memfs_slot clobbers AC4
        move    7,1
        pushj   17,memfs_slot
        jumpl   1,memfs_parent_fail
        move    2,5                     ; preserve child np
        move    1,7                     ; restore fs
        move    4,0                     ; restore optional namep
        hlrz    5,6                     ; parent slot
        caml    5,1(1)
        jrst    memfs_parent_fail
        move    7,5
        lsh     7,3
        add     7,(1)
        move    0,5(7)
        trnn    0,1
        jrst    memfs_parent_fail
        hrrz    0,5
        tlo     0,040001
        movem   0,(3)
        jumpe   4,memfs_parent_ok
        move    0,2
        hrl     0,2
        hrr     0,4
        blt     0,4(4)                  ; copy child name
memfs_parent_ok:
        movei   1,0
        popj    17,
memfs_parent_fail:
        jrst    pdp10_ret_neg1

; Singleton resident MEMFS state.  A MEMFS MRES represents exactly one mounted
; in-memory filesystem; carrying a context pointer through a C switch was dead
; generality.
        .bss
        .globl  memfs_mres_fs
memfs_mres_fs:
        .block  6
        .text

        .globl  fs_words_equal
        .globl  fs_zero_words
        .globl  fs_mres_context_vector_dispatch
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

; Initialize singleton state from the KINIT-provided six-word struct.
memfs_mres_init:
        jumpe   2,memfs_mres_bad
        hrl     2,2
        hrri    2,memfs_mres_fs
        blt     2,memfs_mres_fs+5
        setz    1,
        popj    17,

; MEMINFO calls this exported entry directly; overwrite request a/b.
        .globl  memfs_mres_usage
memfs_mres_usage:
        move    2,memfs_mres_fs+4       ; used_words
        movem   2,1(1)
        move    2,memfs_mres_fs+3       ; pool_words
        movem   2,2(1)
        setz    1,
        popj    17,

memfs_mres_bad:
        seto    1,
        popj    17,

memfs_mres_dispatch:
        jumpe   1,memfs_mres_bad
        move    2,(1)
        caie    2,023                   ; 19 decimal: MEMFS_INIT
        jrst    memfs_mres_not_init
        move    2,1(1)
        jrst    memfs_mres_init
memfs_mres_not_init:
        move    2,[memfs_mres_vector]
        movei   3,memfs_mres_fs
        jrst    fs_mres_context_vector_dispatch

        .data
memfs_mres_vector:
        .word   017                      ; operations 1..15
        .word   memfs_lookup             ; 1 LOOKUP
        .word   memfs_readdir            ; 2 READDIR
        .word   memfs_stat               ; 3 STAT
        .word   memfs_parent             ; 4 PARENT
        .word   memfs_parent             ; 5 PARENT_NAME
        .word   memfs_create             ; 6 CREATE
        .word   memfs_mkdir              ; 7 MKDIR
        .word   0                        ; 8 SYMLINK
        .word   memfs_unlink             ; 9 UNLINK
        .word   memfs_rename             ; 10 RENAME
        .word   memfs_truncate_words     ; 11 TRUNCATE
        .word   memfs_chmod              ; 12 CHMOD
        .word   memfs_read_words         ; 13 READ_WORDS
        .word   memfs_write_words        ; 14 WRITE_WORDS
        .word   pdp10_ret_zero           ; 15 SYNC
        .text
