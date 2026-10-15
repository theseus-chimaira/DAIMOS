/**
 * @file proc_pure_backing_pdp6.s
 * @brief PURE executable backing and text reconstruction shared by VM paths.
 *
 * This is source isolation only.  The current swap caller still owns the
 * backing record and the restored writable image, and the existing reload
 * contract is unchanged.  A future resident packer can share these leaves.
 */
        .text
        .globl proc_swap_unpack_backing
        .globl proc_swap_reload_pure
        .globl vfs_read_words
        .globl d6lz36_decode_vfs
        .globl proc_table
        .globl proc_high_slot
        .globl vfs_mount_root
        /* Constants are supplied by the parent proc_swap assembly unit.
         * When this leaf becomes an independently linked package, move the
         * constants into a common assembler include. */

/*
 * Reconstruct the canonical executable vnode encoded by the one-word swap
 * backing record. Provider identity is supplied by the mount root: a mounted
 * slot has one provider, and swapped PURE users prevent its unmount.
 *
 * Input:  AC1 = packed resident backing state.
 * Output: AC1 = vnode, AC2 = PURE text words.
 * Clobbers AC3..AC5.
 */
proc_swap_unpack_backing:
        move    5,1
        move    4,1
        lsh     4,-PROC_SWAP_TEXT_SHIFT
        andi    4,PROC_SWAP_TEXT_MASK
        hrrz    3,5                     ; vnode index
        move    1,5
        lsh     1,-PROC_SWAP_MOUNT_SHIFT
        andi    1,017                   ; 16 zero-based mount slots
        move    2,vfs_mount_root(1)     ; provider of the still-mounted volume
        jumpe   2,proc_swap_unpack_backing_bad
        lsh     2,-036                  ; vnode provider:6
        caige   2,4                     ; reject anything outside MEMFS..TSFS
        jrst    proc_swap_unpack_backing_bad
        caile   2,7
        jrst    proc_swap_unpack_backing_bad
        addi    1,1                     ; public mount id
        move    5,2
        andi    5,1
        addi    5,1                     ; provider-local executable kind
        lsh     1,6
        add     1,5
        lsh     1,022                   ; VFS kind/mount field
        ior     3,1
        lsh     2,036                   ; VFS provider field
        ior     3,2
        move    1,3
        move    2,4
        popj    17,

proc_swap_unpack_backing_bad:
        setz    1,                      ; invalid/unmounted backing
        setz    2,                      ; PURE reload must fail closed
        popj    17,

/*
 * Recreate PURE executable text in a newly allocated process extent.
 *
 * Plain DXR2 needs only the text prefix.  Compressed DXR2 is decoded as a
 * complete initialized image and the subsequently restored swap suffix
 * overwrites all writable state, avoiding a second compressed-text decoder.
 *
 * Input: AC1 = packed backing state, AC2 = physical process base.
 * Return AC1 = 0 on success, -1 on backing/read/decode failure.
 */
proc_swap_reload_pure:
        add     17,[010,,010]
        movei   0,-7(17)
        hrli    0,010
        blt     0,-1(17)                ; preserve AC10..AC16
        move    10,1                    ; packed backing
        move    11,2                    ; physical base
        pushj   17,proc_swap_unpack_backing
        move    12,1                    ; executable vnode
        move    13,2                    ; text words
        jumpe   13,proc_swap_reload_fail

        move    1,12
        movei   2,1                     ; DXR flags/image word
        movei   3,(17)
        movei   4,1
        pushj   17,vfs_read_words
        caie    1,1
        jrst    proc_swap_reload_fail
        move    14,(17)
        trne    14,EXEC_DXR_F_COMPRESSED
        jrst    proc_swap_reload_compressed

        move    1,12
        movei   2,3                     ; DXR2 payload offset
        move    3,11
        addi    3,EXEC_USER_ORIGIN
        move    4,13
        pushj   17,vfs_read_words
        came    1,13
        jrst    proc_swap_reload_fail
        jrst    proc_swap_reload_ok

proc_swap_reload_compressed:
        hlrz    15,14                   ; uncompressed image words
        jumpe   15,proc_swap_reload_fail
        hrlz    4,15
        move    6,11
        addi    6,EXEC_USER_ORIGIN
        hrr     4,6
        move    1,12
        movei   2,3
        setz    3,                      ; trusted immutable backing
        pushj   17,d6lz36_decode_vfs
        jumpn   1,proc_swap_reload_fail
proc_swap_reload_ok:
        setz    1,
        jrst    proc_swap_reload_return
proc_swap_reload_fail:
        seto    1,
proc_swap_reload_return:
        movei   0,010
        hrli    0,-7(17)
        blt     0,016
        sub     17,[010,,010]
        popj    17,


/* Provider-independent backing immutability guards. */
/*
 * Return nonzero while a nonresident PURE process depends on NODE as its
 * executable backing.  Resident processes keep their text in core and do not
 * constrain later vnode mutation.
 */
        .globl  proc_swap_backing_busy
proc_swap_backing_busy:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        hrrz    10,1                    ; packed key: index + 4-bit mount slot
        move    11,1
        lsh     11,-030                 ; public mount id
        andi    11,077
        subi    11,1
        andi    11,017
        lsh     11,PROC_SWAP_MOUNT_SHIFT
        ior     10,11

        movei   11,1
        move    12,proc_high_slot
proc_swap_backing_busy_loop:
        caml    11,12
        jrst    proc_swap_backing_not_busy
        move    13,11
        lsh     13,1
        add     13,11
        add     13,proc_table
        hlrz    1,(13)
        jumpe   1,proc_swap_backing_busy_next
        move    1,PROC_SWAP_BACKING_OFFSET(1)
        move    2,1
        lsh     2,-PROC_SWAP_TEXT_SHIFT
        andi    2,PROC_SWAP_TEXT_MASK
        jumpe   2,proc_swap_backing_busy_next
        and     1,[017,,0777777]
        camn    1,10
        jrst    proc_swap_backing_is_busy
proc_swap_backing_busy_next:
        aoja    11,proc_swap_backing_busy_loop
proc_swap_backing_is_busy:
        movei   1,1
        jrst    proc_swap_backing_busy_return
proc_swap_backing_not_busy:
        setz    1,
proc_swap_backing_busy_return:
        pop     17,13
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,

/* Return nonzero while any swapped PURE process depends on public MOUNT id. */
        .globl  proc_swap_mount_busy
proc_swap_mount_busy:
        push    17,10
        push    17,11
        push    17,12
        move    10,1
        movei   11,1
        move    12,proc_high_slot
proc_swap_mount_busy_loop:
        caml    11,12
        jrst    proc_swap_mount_not_busy
        move    2,11
        lsh     2,1
        add     2,11
        add     2,proc_table
        hlrz    1,(2)
        jumpe   1,proc_swap_mount_busy_next
        move    1,PROC_SWAP_BACKING_OFFSET(1)
        move    3,1
        lsh     3,-PROC_SWAP_TEXT_SHIFT
        andi    3,PROC_SWAP_TEXT_MASK
        jumpe   3,proc_swap_mount_busy_next
        lsh     1,-PROC_SWAP_MOUNT_SHIFT
        andi    1,017
        addi    1,1
        camn    1,10
        jrst    proc_swap_mount_is_busy
proc_swap_mount_busy_next:
        aoja    11,proc_swap_mount_busy_loop
proc_swap_mount_is_busy:
        movei   1,1
        jrst    proc_swap_mount_busy_return
proc_swap_mount_not_busy:
        setz    1,
proc_swap_mount_busy_return:
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,

