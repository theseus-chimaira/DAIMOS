; fs_backing_pdp10.s -- compact filesystem-independent backing dispatch.
;
; A backing descriptor contains packed read/write callback addresses, one
; adapter-private word, and the exported logical block count.  No filesystem
; or device policy lives here.

        .text
        .globl  fs_backing_read
        .globl  fs_backing_write
        .globl  pdp10_ret_neg1

; int fs_backing_read(backing, logical, block)
; Callback ABI: AC1=opaque, AC2=logical, AC3=block.
fs_backing_read:
        jumpe   1,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        jumpl   2,pdp10_ret_neg1
        caml    2,2(1)
        jrst    pdp10_ret_neg1
        move    4,(1)
        hlrz    4,4
        jumpe   4,pdp10_ret_neg1
        move    1,1(1)
        jrst    (4)

; int fs_backing_write(backing, logical, block)
fs_backing_write:
        jumpe   1,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        jumpl   2,pdp10_ret_neg1
        caml    2,2(1)
        jrst    pdp10_ret_neg1
        move    4,(1)
        hrrz    4,4
        jumpe   4,pdp10_ret_neg1
        move    1,1(1)
        jrst    (4)


; Root-storage bridge used while the boot root still exposes a singleton
; BLOCKSET service.  AC1 (backing opaque) is deliberately ignored; a future
; descriptor-backed adapter can consume it without changing filesystem code.
        .globl  fs_backing_root_read
        .globl  fs_backing_root_write
        .globl  fs_backing_root_read_jump
        .globl  fs_backing_root_write_jump
fs_backing_root_read:
        move    1,2
        move    2,3
fs_backing_root_read_jump:
        jrst    0

fs_backing_root_write:
        move    1,2
        move    2,3
fs_backing_root_write_jump:
        jrst    0

; Direct-device bridge.  The opaque word packs unit,,base.  The bridge is
; storage-layer code: individual filesystem providers neither know nor care
; which direct device callback MINIT patches below these entry points.
        .globl  fs_backing_direct_read
        .globl  fs_backing_direct_write
        .globl  fs_backing_direct_read_jump
        .globl  fs_backing_direct_write_jump
fs_backing_direct_read:
        move    4,1
        hlrz    1,4
        hrrz    4,4
        add     2,4
fs_backing_direct_read_jump:
        jrst    0

fs_backing_direct_write:
        move    4,1
        hlrz    1,4
        hrrz    4,4
        add     2,4
fs_backing_direct_write_jump:
        jrst    0
