; memfs_pdp10.s -- compact resident MEMFS primitives for PDP-6/PDP-10.
        .text

; void memfs_v1_shift_after(struct memfs_v1 *fs, unsigned int start,
;     int delta, unsigned int exclude)
        .globl  memfs_v1_shift_after
memfs_v1_shift_after:
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

; int memfs_v1_resize(struct memfs_v1 *fs, unsigned int slot,
;     unsigned int words)
; fs layout: nodes,node_count,pool,pool_words,used_words,writable,image_data.
; node layout is 8 words; meta at +5, packed data at +7.
        .globl  memfs_v1_resize
memfs_v1_resize:
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
        camg    3,5             ; new > old => grow
        jrst    memfs_resize_shrink

; Grow the node by delta = new-old. Move following pool words upward,
; clear the inserted gap, then relocate later mutable-node offsets.
        move    7,3
        sub     7,5             ; delta
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
        pushj   17,memfs_v1_shift_after
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
        pushj   17,memfs_v1_shift_after
memfs_resize_ok:
        movei   1,0
        popj    17,
memfs_resize_fail:
        seto    1,
        popj    17,
