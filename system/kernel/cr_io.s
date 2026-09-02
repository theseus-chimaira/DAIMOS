; cr_io.s -- compact PDP-6 card-reader service.
;
; cr_iowd combines the 80-column count and destination pointer.  The shared
; PI7 handler advances it with AOBJN semantics and sets it to -1 after the
; final column while waiting for END CARD; zero means idle/done.

        .text
        .globl cr_read_card
        .globl cr_iowd
        .globl pdp10_ret_arg
        .globl io7_ret_timeout
        .globl pdp10_ret_busy
        .globl io7_ret_card_count
        .globl io7_ret_e4

cr_read_card:
        jumpe 1,pdp10_ret_arg
        skipe cr_iowd
        jrst io7_ret_e4
cr_read_idle:
        movei 2,0200000
cr_read_ready_wait:
        consz 0150,0100
        jrst cr_read_start
        sojg 2,cr_read_ready_wait
        jrst io7_ret_timeout
cr_read_start:
        subi 1,1
        hrli 1,0777660
        movem 1,cr_iowd
        cono 0150,01237
        movei 2,0200000
cr_read_done_wait:
        skipn cr_iowd
        jrst cr_read_done
        sojg 2,cr_read_done_wait
        setzm cr_iowd
        cono 0150,0007
        jrst io7_ret_timeout
cr_read_done:
        consz 0150,0400
        jrst pdp10_ret_busy
io7_ret_card_count:
        movei 1,0120
        popj 017,

        .bss
cr_iowd:
        .block 1
