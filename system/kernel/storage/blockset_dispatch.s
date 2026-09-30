; blockset_dispatch.s -- compact runtime BLOCKSET MRES.
;
; The resident root package contains only the homogeneous DSK INTERLEAVE path.
; The generic reentrant INTERLEAVE/CONCAT mapper lives in blockset_map.s and is
; linked only by consumers that need arbitrary descriptors.

        .text
        .globl blockset_map_block
        .globl blockset_read_block
        .globl blockset_write_block
        .globl blockset_boot
        .globl blockset_backend_read_jump
        .globl blockset_backend_write_jump
        .globl kret_neg1
        .globl kret_zero
        .globl kret_one

; Compact root descriptor layout, in words.  The boot root has at most
; four DSK270 members; the separate generic mapper retains the seven-member
; descriptor used by arbitrary BLOCKSET consumers.
;   0: LH members | (policy << 3), RH equal tail blocks
;   1: total logical blocks
;   2..5: physical unit numbers
;   6..9: member base,,tail-base

; Root-set homogeneous INTERLEAVE mapper.  The boot root uses equal-sized
; DSK270 members, so it does not carry the generic CONCAT descriptor mapper.
; AC1=logical; returns AC1=unit, AC2=physical block, or AC1=-1.
blockset_map_block:
        jumpl 1,kret_neg1
        caml 1,blockset_boot+1
        jrst kret_neg1
        hlrz 7,blockset_boot
        andi 7,07
        setz 0,
        div 0,7                          ; relative block AC0, member AC1
        move 2,0
        move 6,blockset_boot+6(1)
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
        move 4,2                         ; internal callers supply a buffer
        pushj 17,blockset_map_block
        move 3,4
        jumpl 1,kret_neg1
        jumpe 5,blockset_block_read_account
        pushj 17,blockset_backend_write
        jrst blockset_block_done
blockset_block_read_account:
blockset_block_read:
        pushj 17,blockset_backend_read
blockset_block_done:
        jumpe 1,blockset_block_return
blockset_block_return:
        popj 17,

; Homogeneous root backend.
blockset_backend_read:
blockset_backend_read_jump:
        jrst 0
blockset_backend_write:
blockset_backend_write_jump:
        jrst 0

; The package service export points directly at blockset_map_block.
; Fixed KCORE offsets raw-tail requests past the filesystem-visible span.

        .bss
; Packed DSK root descriptor: four units plus four ranges.
blockset_boot:         .block 012
