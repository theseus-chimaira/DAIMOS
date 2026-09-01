; memfs_pdp10.s -- compact resident MEMFS primitives for PDP-6/PDP-10.
        .text
        .globl  pdp10_ret_zero_v1
        .globl  pdp10_ret_neg1_v1

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
; fs layout: nodes,node_count,pool,pool_words,used_words,writable,image_data.
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
        jrst    pdp10_ret_neg1_v1

; int memfs_read_words(const struct memfs *fs, vnode_t node,
;     unsigned int off, kword_t *buf, unsigned int nwords)
        .globl  memfs_read_words
memfs_read_words:
        jumpe   4,memfs_read_fail
        hlrz    5,2
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
        add     2,6(1)          ; image_data
        jrst    memfs_read_source
memfs_read_pool:
        add     2,2(1)          ; pool
memfs_read_source:
        add     2,3
        move    6,5             ; preserve return count
        jumpe   5,memfs_read_done
memfs_read_copy:
        move    0,(2)
        movem   0,(4)
        addi    2,1
        addi    4,1
        sojg    5,memfs_read_copy
memfs_read_done:
        move    1,6
        popj    17,
memfs_read_eof:
        jrst    pdp10_ret_zero_v1
memfs_read_fail:
        jrst    pdp10_ret_neg1_v1

; int memfs_write_words(struct memfs *fs, vnode_t node,
;     unsigned int off, const kword_t *buf, unsigned int nwords,
;     kword_t size_chars)
        .globl  memfs_write_words
memfs_write_words:
        jumpe   4,memfs_write_fail
        hlrz    5,2
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
        jumpe   7,memfs_write_size
memfs_write_copy:
        move    0,(4)
        movem   0,(6)
        addi    4,1
        addi    6,1
        sojg    7,memfs_write_copy

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
        jrst    pdp10_ret_neg1_v1

; int memfs_name_valid(const struct vfs_name *name)
        .globl  memfs_name_valid
memfs_name_valid:
        jumpe   1,memfs_name_valid_fail
        move    2,(1)
        jumpge  2,memfs_name_valid_small
        jrst    memfs_name_valid_fail
memfs_name_valid_small:
        caige   2,1
        jrst    memfs_name_valid_fail
        caile   2,030                  ; VFS_V1_NAME_MAX_CHARS = 24
        jrst    memfs_name_valid_fail
        movei   1,1
        popj    17,
memfs_name_valid_fail:
        jrst    pdp10_ret_zero_v1

; int memfs_slot(const struct memfs *fs, vnode_t node,
;     unsigned int *slotp)
        .globl  memfs_slot
memfs_slot:
        jumpe   1,memfs_slot_fail
        hlrz    4,2
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
        jumpe   3,memfs_slot_ok
        movem   4,(3)
memfs_slot_ok:
        movei   1,0
        popj    17,
memfs_slot_fail:
        jrst    pdp10_ret_neg1_v1

; int memfs_find_child(const struct memfs *fs, unsigned int parent,
;     const struct vfs_name *name, unsigned int *slotp)
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
        move    0,(5)
        came    0,(3)
        jrst    memfs_find_child_next
        move    0,1(5)
        came    0,1(3)
        jrst    memfs_find_child_next
        move    0,2(5)
        came    0,2(3)
        jrst    memfs_find_child_next
        move    0,3(5)
        came    0,3(3)
        jrst    memfs_find_child_next
        move    0,4(5)
        came    0,4(3)
        jrst    memfs_find_child_next
        jumpe   4,memfs_find_child_ok
        movem   6,(4)
memfs_find_child_ok:
        movei   1,0
        popj    17,
memfs_find_child_next:
        addi    5,010
        addi    6,1
        jrst    memfs_find_child_loop
memfs_find_child_fail:
        jrst    pdp10_ret_neg1_v1

; int memfs_free_slot(const struct memfs *fs, unsigned int *slotp)
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
        movem   4,(2)
        movei   1,0
        popj    17,
memfs_free_slot_fail:
        jrst    pdp10_ret_neg1_v1

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
        jrst    pdp10_ret_zero_v1

; void memfs_clear_node(struct memfs_node *np)
        .globl  memfs_clear_node
memfs_clear_node:
        setzm   (1)
        movei   2,1(1)
        hrli    2,(1)
        blt     2,7(1)
        popj    17,

        .globl  memfs_node_handle
memfs_node_handle:
        hrrz    1,1
        tlo     1,040001
        popj    17,

; int memfs_readdir(const struct memfs *fs, vnode_t dir,
;     unsigned int off, struct vfs_dirent *ent)
        .globl  memfs_readdir
memfs_readdir:
        jumpe   4,memfs_readdir_fail
        jumpe   1,memfs_readdir_fail
        hlrz    5,2
        caie    5,040001
        jrst    memfs_readdir_fail
        hrrz    2,2                     ; parent slot
        caml    2,1(1)
        jrst    memfs_readdir_fail
        move    5,2
        lsh     5,3
        add     5,(1)
        move    7,5(5)
        trnn    7,1
        jrst    memfs_readdir_fail
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
        move    6,0                     ; preserve scratch ordinal no longer needed
        move    0,5
        hrl     0,5
        hrr     0,4
        blt     0,4(4)                  ; copy five-word name
        ldb     0,[POINT 3,5(5),20]
        movem   0,5(4)
        movei   1,1
        popj    17,
memfs_readdir_eof:
        jrst    pdp10_ret_zero_v1
memfs_readdir_fail:
        jrst    pdp10_ret_neg1_v1

; int memfs_stat(const struct memfs *fs, vnode_t node,
;     struct vfs_stat *st)
        .globl  memfs_stat
memfs_stat:
        jumpe   3,memfs_stat_fail
        jumpe   1,memfs_stat_fail
        hlrz    4,2
        caie    4,040001
        jrst    memfs_stat_fail
        hrrz    4,2
        caml    4,1(1)
        jrst    memfs_stat_fail
        lsh     4,3
        add     4,(1)
        move    5,5(4)
        trnn    5,1
        jrst    memfs_stat_fail
        ldb     6,[POINT 3,5(4),20]
        movem   6,(3)
        move    6,5
        lsh     6,-3
        andi    6,07777
        movem   6,1(3)
        move    6,6(4)
        movem   6,2(3)
        hrrz    6,7(4)
        movem   6,3(3)
        movei   1,0
        popj    17,
memfs_stat_fail:
        jrst    pdp10_ret_neg1_v1

; int memfs_parent(const struct memfs *fs, vnode_t node,
;     vnode_t *parentp, struct vfs_name *namep)
        .globl  memfs_parent
memfs_parent:
        jumpe   3,memfs_parent_fail
        jumpe   1,memfs_parent_fail
        hlrz    5,2
        caie    5,040001
        jrst    memfs_parent_fail
        hrrz    5,2                     ; child slot
        caml    5,1(1)
        jrst    memfs_parent_fail
        move    6,5
        lsh     6,3
        add     6,(1)                   ; child np
        move    7,5(6)
        trnn    7,1
        jrst    memfs_parent_fail
        hlrz    5,7                     ; parent slot
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
        move    0,6
        hrl     0,6
        hrr     0,4
        blt     0,4(4)                  ; copy child name
memfs_parent_ok:
        movei   1,0
        popj    17,
memfs_parent_fail:
        jrst    pdp10_ret_neg1_v1
