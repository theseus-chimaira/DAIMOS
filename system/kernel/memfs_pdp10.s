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
