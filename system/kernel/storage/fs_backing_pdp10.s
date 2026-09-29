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
        .globl  mfsdev_d6set_reads
        .globl  mfsdev_d6set_writes
        .globl  mfsdev_d6set_blocks_read
        .globl  mfsdev_d6set_blocks_written
fs_backing_root_read:
        aos     mfsdev_d6set_reads
        aos     mfsdev_d6set_blocks_read
        move    1,2
        move    2,3
fs_backing_root_read_jump:
        jrst    0

fs_backing_root_write:
        aos     mfsdev_d6set_writes
        aos     mfsdev_d6set_blocks_written
        move    1,2
        move    2,3
fs_backing_root_write_jump:
        jrst    0

; Direct-device bridge.  The opaque word packs selector,,base.  Plain
; selectors 0..3 retain the historical DSK270 ABI.  Selector bit 0400000
; chooses DRM236.  Bit 0200000 changes the low four selector bits from one unit
; number into a four-member mask and applies the same equal-size one-block
; INTERLEAVE policy as BLOCKSET.  Bit 0100000 marks only the boot root so the
; shared direct adapter can account D6SET logical I/O without counting
; unrelated secondary D6FS mounts.  No extra resident descriptor is needed.
;
; The DRM jumps default to failure and MINIT patches them only when the DRM236
; service is present, so an untrusted handoff cannot jump through address zero.
        .globl  fs_backing_direct_read
        .globl  fs_backing_direct_write
        .globl  fs_backing_direct_read_jump
        .globl  fs_backing_direct_write_jump
        .globl  fs_backing_direct_drm_read_jump
        .globl  fs_backing_direct_drm_write_jump
fs_backing_direct_read:
        setz    7,
        jrst    fs_backing_direct_io

fs_backing_direct_write:
        movei   7,1

; AC1=selector,,base, AC2=logical block, AC3=buffer, AC7=write flag.
fs_backing_direct_io:
        move    4,1
        hlrz    6,4                     ; preserve selector/device flags
        hrrz    4,4                     ; common physical base
        trnn    6,0100000               ; singleton boot-root logical I/O?
        jrst    fs_backing_direct_not_root
        jumpe   7,fs_backing_direct_root_read
        aos     mfsdev_d6set_writes
        aos     mfsdev_d6set_blocks_written
        jrst    fs_backing_direct_not_root
fs_backing_direct_root_read:
        aos     mfsdev_d6set_reads
        aos     mfsdev_d6set_blocks_read
fs_backing_direct_not_root:
        trnn    6,0200000               ; compact INTERLEAVE set?
        jrst    fs_backing_direct_single

        ; Count the present units in the four-bit member mask.
        move    0,6
        andi    0,017
        jumpe   0,pdp10_ret_neg1
        setz    5,
fs_backing_direct_count:
        trne    0,1
        aoj     5,
        lsh     0,-1
        jumpn   0,fs_backing_direct_count

        ; Equal-size one-block interleave: quotient is member-relative block,
        ; remainder is the ordinal present member selected for this request.
        move    1,2
        setz    0,
        div     0,5
        move    2,0
        add     2,4
        move    0,6
        andi    0,017
        setz    5,                       ; physical unit index
fs_backing_direct_select:
        trnn    0,1
        jrst    fs_backing_direct_next
        sojl    1,fs_backing_direct_dispatch
fs_backing_direct_next:
        lsh     0,-1
        aoja    5,fs_backing_direct_select

fs_backing_direct_single:
        move    1,6
        andi    1,07
        add     2,4

fs_backing_direct_dispatch:
        ; Set mapping leaves the chosen physical unit in AC5; direct mapping
        ; leaves it in AC1.  The set tag distinguishes the two cases cheaply.
        trnn    6,0200000
        jrst    fs_backing_direct_have_unit
        move    1,5
fs_backing_direct_have_unit:
        trne    6,0400000
        jrst    fs_backing_direct_drm
        jumpe   7,fs_backing_direct_read_jump
fs_backing_direct_write_jump:
        jrst    0
fs_backing_direct_read_jump:
        jrst    0

fs_backing_direct_drm:
        jumpe   7,fs_backing_direct_drm_read_jump
fs_backing_direct_drm_write_jump:
        jrst    pdp10_ret_neg1
fs_backing_direct_drm_read_jump:
        jrst    pdp10_ret_neg1
