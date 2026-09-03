; diskset_dispatch.s -- compact runtime DISKSET MRES request dispatcher.
; Boot validation/state construction lives in KINIT diskset_boot.c.
        .text
        .globl diskset_mres_dispatch
        .globl diskset_blocks
        .globl diskset_read_block
        .globl diskset_write_block
        .globl diskset_writable
        .globl diskset_swap_blocks
        .globl diskset_swap_read
        .globl diskset_swap_write
        .globl diskset_log_blocks
        .globl diskset_log_read
        .globl diskset_log_write
        .globl pdp10_ret_neg1

diskset_mres_dispatch:
        jumpe 1,pdp10_ret_neg1
        move 2,(1)
        subi 2,2
        jumpl 2,pdp10_ret_neg1
        cail 2,012
        jrst pdp10_ret_neg1
        jrst @diskset_dispatch_table(2)

diskset_dispatch_blocks:
        jrst diskset_blocks
diskset_dispatch_read:
        move 3,1
        move 1,1(3)
        move 2,2(3)
        jrst diskset_read_block
diskset_dispatch_write:
        move 3,1
        move 1,1(3)
        move 2,2(3)
        jrst diskset_write_block
diskset_dispatch_writable:
        jrst diskset_writable
diskset_dispatch_swap_blocks:
        jrst diskset_swap_blocks
diskset_dispatch_swap_read:
        move 4,1
        move 1,1(4)
        move 2,2(4)
        move 3,3(4)
        jrst diskset_swap_read
diskset_dispatch_swap_write:
        move 4,1
        move 1,1(4)
        move 2,2(4)
        move 3,3(4)
        jrst diskset_swap_write
diskset_dispatch_log_blocks:
        jrst diskset_log_blocks
diskset_dispatch_log_read:
        move 3,1
        move 1,1(3)
        move 2,2(3)
        jrst diskset_log_read
diskset_dispatch_log_write:
        move 3,1
        move 1,1(3)
        move 2,2(3)
        jrst diskset_log_write

diskset_dispatch_table:
        .word diskset_dispatch_blocks
        .word diskset_dispatch_read
        .word diskset_dispatch_write
        .word diskset_dispatch_writable
        .word diskset_dispatch_swap_blocks
        .word diskset_dispatch_swap_read
        .word diskset_dispatch_swap_write
        .word diskset_dispatch_log_blocks
        .word diskset_dispatch_log_read
        .word diskset_dispatch_log_write
