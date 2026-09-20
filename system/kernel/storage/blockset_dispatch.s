; blockset_dispatch.s -- compact runtime BLOCKSET MRES.
;
; The resident root package contains only the homogeneous DSK INTERLEAVE path.
; The generic reentrant INTERLEAVE/CONCAT mapper lives in blockset_map.s and is
; linked only by consumers that need arbitrary descriptors.

        .text
        .globl blockset_mres_dispatch
        .globl blockset_blocks
        .globl blockset_map_block
        .globl blockset_read_block
        .globl blockset_write_block
        .globl blockset_writable
        .globl blockset_tail_map
        .globl blockset_boot
        .globl blockset_backend_read_jump
        .globl blockset_backend_write_jump
        .globl pdp10_ret_neg1
        .globl pdp10_ret_zero
        .globl pdp10_ret_one
        .globl mfsdev_storage_errors
        .globl mfsdev_d6set_reads
        .globl mfsdev_d6set_writes
        .globl mfsdev_d6set_blocks_read
        .globl mfsdev_d6set_blocks_written

; Packed descriptor layout, in words.
;   0: LH members | (policy << 3), RH equal tail blocks
;   1: total logical blocks
;   2..8: physical unit numbers
;   9..15: member base,,block-count

; Return root-set total usable blocks.
blockset_blocks:
        move 1,blockset_boot+1
        popj 17,

; Root-set homogeneous INTERLEAVE mapper.  The boot root uses equal-sized
; DSK270 members, so it does not carry the generic CONCAT descriptor mapper.
; AC1=logical; returns AC1=unit, AC2=physical block, or AC1=-1.
blockset_map_block:
        jumpl 1,pdp10_ret_neg1
        caml 1,blockset_boot+1
        jrst pdp10_ret_neg1
        hlrz 5,blockset_boot
        andi 5,07
        jumpe 5,pdp10_ret_neg1
        setz 0,
        div 0,5                          ; relative block AC0, member AC1
        move 2,0
        move 6,blockset_boot+011(1)
        hlrz 3,6
        add 2,3
        move 1,blockset_boot+2(1)
        popj 17,

; Root-set block I/O.  The compact root mapper returns the physical address;
; only the current root backend remains patched globally.
blockset_read_block:
        setz 5,
        jrst blockset_block_io

blockset_write_block:
        movei 5,1

blockset_block_io:
        jumpe 2,pdp10_ret_neg1
        move 4,2                         ; caller buffer
        push 17,4
        push 17,5
        pushj 17,blockset_map_block
        pop 17,5
        pop 17,3
        jumpl 1,pdp10_ret_neg1
        aos mfsdev_d6set_reads(5)
        aos mfsdev_d6set_blocks_read(5)
        jumpe 5,blockset_block_read
        pushj 17,blockset_backend_write
        jrst blockset_block_done
blockset_block_read:
        pushj 17,blockset_backend_read
blockset_block_done:
        jumpe 1,blockset_block_return
        aos mfsdev_storage_errors+4    ; D6SET0
blockset_block_return:
        popj 17,

blockset_writable:
        jrst pdp10_ret_one


; Map one logical root-tail block.  Validation, transfer looping, and
; accounting are shared in fixed KCORE so singleton and multi-member roots do
; not carry two copies of the same swap-tail machinery.
; AC1=logical tail block; returns AC1=unit, AC2=physical block, or AC1=-1.
blockset_tail_map:
        hrrz 5,blockset_boot             ; validated equal tail size
        setz 0,
        div 0,5                          ; member AC0, local block AC1
        move 6,blockset_boot+011(0)
        move 2,1
        hlrz 3,6
        add 2,3
        hrrz 3,6
        add 2,3
        move 1,blockset_boot+2(0)
        popj 17,

; Homogeneous root backend.
blockset_backend_read:
blockset_backend_read_jump:
        jrst 0
blockset_backend_write:
blockset_backend_write_jump:
        jrst 0

; Permanent export is the register ABI.  KINIT owns request unpacking.
blockset_mres_dispatch:
blockset_mres_reg_dispatch:
        caie 5,012                       ; BLOCKSET_MRES_OP_TAIL_MAP
        jrst blockset_mres_regular
        jrst blockset_tail_map
blockset_mres_regular:
        subi 5,2
        jumpl 5,pdp10_ret_neg1
        caile 5,3
        jrst pdp10_ret_neg1
        jrst @blockset_dispatch_table(5)

blockset_dispatch_table:
        .word blockset_blocks
        .word blockset_read_block
        .word blockset_write_block
        .word blockset_writable

        .bss
; Packed DSK root descriptor: four units plus four ranges.
blockset_boot:         .block 015
