; diskset_dispatch.s -- compact runtime DISKSET MRES.
;
; Boot validation/state construction lives in KINIT diskset_boot.c.  This file
; intentionally implements the current PDP-6 four-member runtime directly in
; PDP-10 assembly: mapping, raw D6FS blocks, SWAP tails, LOGSTORE blocks, and
; the MRES request dispatcher.  The mapping algorithm and on-disk layout are
; unchanged.  Revisit the representation when the KA10 profile grows beyond
; four members; do not generalize this runtime prematurely.

        .text
        .globl diskset_mres_dispatch
        .globl diskset_blocks
        .globl diskset_map_block
        .globl diskset_read_block
        .globl diskset_write_block
        .globl diskset_writable
        .globl diskset_swap_blocks
        .globl diskset_swap_read
        .globl diskset_swap_write
        .globl diskset_log_blocks
        .globl diskset_log_read
        .globl diskset_log_write
        .globl diskset_boot
        .globl diskset_total_blocks
        .globl dsk270_read_sector
        .globl dsk270_write_sector
        .globl pdp10_ret_neg1
        .globl pdp10_ret_zero
        .globl devicefs_io_in
        .globl devicefs_io_out
        .globl devicefs_storage_errors
        .globl devicefs_d6set_reads
        .globl devicefs_d6set_writes
        .globl devicefs_d6set_blocks_read
        .globl devicefs_d6set_blocks_written
        .globl devicefs_log_reads
        .globl devicefs_log_writes
        .globl devicefs_log_blocks_read
        .globl devicefs_log_blocks_written
        .globl devicefs_log_errors

; Return total usable blocks.
diskset_blocks:
        move 1,diskset_total_blocks
        popj 17,

; Internal mapper.  AC1=logical.  Returns AC1=member, AC2=member-relative
; block, or AC1=-1.  AC4/AC5 are preserved for the block-I/O caller.
;
; KINIT validates all member counts/extents against the Type-270 geometry, so
; valid runtime values fit below the 36-bit sign bit.  Rejecting a high-half
; logical value preserves the C implementation's unsigned bounds semantics
; without GCC's repeated sign-bit transforms.
diskset_map_block:
        jumpl 1,pdp10_ret_neg1
        caml 1,diskset_total_blocks
        jrst pdp10_ret_neg1
        setz 2,                        ; floor

diskset_map_zone:
        setzb 3,6                     ; next, width
        setz 7,                       ; member index

diskset_map_scan:
        caml 7,diskset_boot
        jrst diskset_map_scan_done
        move 0,diskset_boot+011(7)
        camg 0,2                      ; active only above floor
        jrst diskset_map_scan_next
        aoj 6,
        jumpe 3,diskset_map_set_next
        camge 0,3
        move 3,0
diskset_map_scan_next:
        aoja 7,diskset_map_scan

diskset_map_set_next:
        move 3,0
        aoja 7,diskset_map_scan

diskset_map_scan_done:
        jumpe 3,pdp10_ret_neg1
        move 0,3
        sub 0,2
        imul 0,6                       ; zone blocks fit one validated word
        camge 1,0
        jrst diskset_map_found_zone
        sub 1,0
        move 2,3
        jrst diskset_map_zone

diskset_map_found_zone:
        ; DIV AC0,width consumes AC0/AC1 as a positive 72-bit dividend.
        ; Quotient -> AC0 (relative block), remainder -> AC1 (active slot).
        setz 0,
        div 0,6
        setz 7,

diskset_map_pick:
        caml 7,diskset_boot
        jrst pdp10_ret_neg1
        move 3,diskset_boot+011(7)
        camg 3,2
        jrst diskset_map_pick_next
        jumpe 1,diskset_map_emit
        soj 1,
diskset_map_pick_next:
        aoja 7,diskset_map_pick

diskset_map_emit:
        move 1,7
        add 2,0
        popj 17,


; Raw region block I/O.  AC5 is an internal write flag and AC4 preserves the
; caller buffer while the mapper uses the volatile argument registers.
diskset_read_block:
        setz 5,
        jrst diskset_block_io

diskset_write_block:
        movei 5,1

diskset_block_io:
        jumpe 2,pdp10_ret_neg1
        move 4,2
        pushj 17,diskset_map_block
        jumpl 1,pdp10_ret_neg1
        move 3,4
        add 2,diskset_boot+5(1)
        move 1,diskset_boot+1(1)
        aos devicefs_d6set_reads(5)
        aos devicefs_d6set_blocks_read(5)
        jumpe 5,diskset_block_read
        pushj 17,dsk270_write_sector
        jrst diskset_block_done
diskset_block_read:
        pushj 17,dsk270_read_sector
diskset_block_done:
        jumpe 1,diskset_block_return
        aos devicefs_storage_errors+4 ; D6SET0
diskset_block_return:
        popj 17,

; Writable iff at least one validated member is configured.  DISKSET is only
; installed after DSK MINIT has bound both read and write services.
diskset_writable:
        skipn   diskset_boot
        jrst    pdp10_ret_zero
        jrst    pdp10_ret_one

; Current profile uses <=4 members and per-member tails below one Type-270
; unit, so the product is safely within a single 36-bit IMUL result.
diskset_swap_blocks:
        move 1,diskset_boot+015
        imul 1,diskset_boot
        popj 17,

diskset_swap_read:
        setz 4,
        jrst diskset_swap_io

diskset_swap_write:
        movei 4,1

diskset_swap_io:
        jumpe 2,pdp10_ret_zero         ; zero count ignores buffer
        jumpe 3,pdp10_ret_neg1
        jumpl 1,pdp10_ret_neg1
        jumpl 2,pdp10_ret_neg1
        move 5,diskset_boot+015
        mul 5,diskset_boot             ; AC6 = total SWAP blocks
        caml 1,6
        jrst pdp10_ret_neg1
        sub 6,1                        ; blocks remaining from logical
        camle 2,6
        jrst pdp10_ret_neg1

diskset_swap_valid:
        ; SWAP has no lifetime I/O counters.  D6SET retains aggregate device
        ; accounting; SWAP itself exposes only current allocation state.
        aos devicefs_d6set_reads(4)
        addm 2,devicefs_d6set_blocks_read(4)
        ; Preserve only the state live across DSK service calls.
        add 17,[6,,6]
        movei 0,-5(17)
        hrli 0,010
        blt 0,(17)
        move 015,4                     ; write flag
        move 012,2                     ; count
        move 013,3                     ; buffer
        move 014,diskset_boot+015      ; per-member tail size
        setz 0,
        div 0,014                      ; member=AC0, local=AC1
        move 010,0
        move 011,1

diskset_swap_loop:
        move 2,diskset_boot+5(010)
        add 2,diskset_boot+011(010)
        add 2,011
        move 1,diskset_boot+1(010)
        move 3,013
        jumpe 015,diskset_swap_read_one
        pushj 17,dsk270_write_sector
        jrst diskset_swap_after_one

diskset_swap_read_one:
        pushj 17,dsk270_read_sector

diskset_swap_after_one:
        jumpe 1,diskset_swap_after_ok
        aos devicefs_storage_errors+4 ; D6SET0
        jrst diskset_swap_done
diskset_swap_after_ok:
        addi 013,0200
        aoj 011,
        came 011,014
        jrst diskset_swap_same_member
        setz 011,
        aoj 010,
diskset_swap_same_member:
        sojn 012,diskset_swap_loop
        setz 1,

diskset_swap_done:
        movei 0,010
        hrli 0,-5(17)
        blt 0,015
        sub 17,[6,,6]
        popj 17,

; LOGSTORE is a bounded logical subregion of the ordinary DISKSET mapping.
diskset_log_blocks:
        move 1,diskset_boot+017
        popj 17,

diskset_log_read:
        setz 5,
        jrst diskset_log_io

diskset_log_write:
        movei 5,1

diskset_log_io:
        jumpe 2,pdp10_ret_neg1
        jumpl 1,pdp10_ret_neg1
        caml 1,diskset_boot+017
        jrst pdp10_ret_neg1
        add 1,diskset_boot+016
        jumpe 5,diskset_log_account_read
        aos devicefs_log_writes
        aos devicefs_log_blocks_written
        jrst diskset_log_call
diskset_log_account_read:
        aos devicefs_log_reads
        aos devicefs_log_blocks_read
diskset_log_call:
        pushj 17,diskset_block_io
        jumpe 1,diskset_log_return
        aos devicefs_log_errors
diskset_log_return:
        popj 17,

; Legacy request ABI entry.  Keep this export exactly two words so KCORE
; can derive the register entry from the single republished service jump.
diskset_mres_dispatch:
        jumpe 1,pdp10_ret_neg1
        jrst diskset_mres_request_call

; Register ABI: AC5=operation (2..11), AC1..AC3=a..c.
; Operations needing fewer arguments simply ignore the extra AC values.
diskset_mres_reg_dispatch:
        subi 5,2
        jumpl 5,pdp10_ret_neg1
        cail 5,012
        jrst pdp10_ret_neg1
        move 5,diskset_dispatch_table(5)
        jrst (5)

diskset_mres_request_call:
        move 4,1
        move 5,(4)
        move 1,1(4)
        move 2,2(4)
        move 3,3(4)
        jrst diskset_mres_reg_dispatch

diskset_dispatch_table:
        .word diskset_blocks
        .word diskset_read_block
        .word diskset_write_block
        .word diskset_writable
        .word diskset_swap_blocks
        .word diskset_swap_read
        .word diskset_swap_write
        .word diskset_log_blocks
        .word diskset_log_read
        .word diskset_log_write

        .bss
; struct diskset: members, unit[4], base[4], blocks[4], swap, log start/count.
diskset_boot:         .block 020
diskset_total_blocks: .block 1
