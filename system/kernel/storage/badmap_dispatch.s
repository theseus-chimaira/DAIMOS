; badmap_dispatch.s -- optional source->spare physical exception mapper.
;
; Installed only when the boot storage set contains at least one remap.
; The table itself lives in managed core: one 36-bit source,,replacement word
; per exception.  Clean media therefore pays no permanent BADMAP MRES cost.

        .text
        .globl badmap_map_block
        .globl badmap_read_block
        .globl badmap_write_block
        .globl badmap_state
        .globl badmap_backend_read_jump
        .globl badmap_backend_write_jump
        .globl badmap_base_map_jump
        .globl pdp10_ret_neg1

; state layout:
;   0 count
;   1 table address
;   2 member count
;   3..6 physical unit numbers by member index
;   7 direct singleton base (used by the built-in base mapper)
;   8 direct singleton total blocks including raw tail

; AC1=logical block; return AC1=physical unit, AC2=member-local block.
badmap_map_block:
        pushj   17,badmap_base_map
        jumpl   1,pdp10_ret_neg1
        move    6,1                     ; physical unit
        move    7,2                     ; physical block
        setz    4,                      ; member index
badmap_find_member:
        came    6,badmap_unit0(4)
        aoja    4,badmap_find_member_next
        jrst    badmap_have_member
badmap_find_member_next:
        caml    4,badmap_members
        jrst    pdp10_ret_neg1
        jrst    badmap_find_member
badmap_have_member:
        move    5,4
        lsh     5,020                   ; member in locator bits 16..17
        ior     5,7                     ; AC5 source locator
        move    3,badmap_table        ; table pointer
        move    4,badmap_count          ; count
        jumpe   4,badmap_no_match
badmap_scan:
        hlrz    0,(3)
        camn    0,5
        jrst    badmap_match
        camg    0,5                     ; sorted source table: stop once source > key
        jrst    badmap_scan_next
        jrst    badmap_no_match
badmap_scan_next:
        aoj     3,
        sojg    4,badmap_scan
badmap_no_match:
        move    1,6
        move    2,7
        popj    17,
badmap_match:
        hrrz    0,(3)
        move    4,0
        lsh     4,-020
        andi    4,03
        move    1,badmap_unit0(4)
        andi    0,0177777
        move    2,0
        popj    17,

; Base mapper.  MINIT patches the jump to BLOCKSET for a multi-member set.
; The singleton default uses the compact direct geometry in state words 7/8.
badmap_base_map:
badmap_base_map_jump:
        jrst    badmap_direct_map
badmap_direct_map:
        jumpl   1,pdp10_ret_neg1
        caml    1,badmap_direct_blocks
        jrst    pdp10_ret_neg1
        move    2,badmap_direct_base
        add     2,1
        move    1,badmap_unit0
        popj    17,

badmap_read_block:
        setz    5,
        jrst    badmap_block_io
badmap_write_block:
        movei   5,1
badmap_block_io:
        push    17,2                    ; mapper clobbers AC2..AC7
        push    17,5                    ; preserve read/write selector too
        pushj   17,badmap_map_block
        pop     17,5
        pop     17,3                    ; DSK backend buffer argument
        jumpl   1,pdp10_ret_neg1
        jumpe   5,badmap_block_read
badmap_backend_write:
badmap_backend_write_jump:
        jrst    0
badmap_block_read:
badmap_backend_read:
badmap_backend_read_jump:
        jrst    0

        .bss
badmap_state:
badmap_count:            .block 1
badmap_table:            .block 1
badmap_members:          .block 1
badmap_unit0:            .block 1
badmap_unit1:            .block 1
badmap_unit2:            .block 1
badmap_unit3:            .block 1
badmap_direct_base:      .block 1
badmap_direct_blocks:    .block 1
