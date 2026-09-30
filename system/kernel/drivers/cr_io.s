; cr_io.s -- resident PDP-6 card-reader driver.
        .globl mfsdev_io_in
        .text
        .globl cr_pi_handler
        .globl cr_read_card
        .globl pdp10_pi_handler_return
        .globl kret_arg
        .globl kret_busy

cr_pi_handler:
        coni 0150,1
        trne 1,0400
        jrst cr_pi_done
        trne 1,0010
        jrst cr_pi_data
        trnn 1,0020
        jrst pdp10_pi_handler_return
cr_pi_done:
        cono 0150,0027
        setzm cr_iowd
        jrst pdp10_pi_handler_return
cr_pi_data:
        move 1,cr_iowd
        aobjn 1,cr_pi_more
        setom cr_iowd
        jrst cr_pi_xfer
cr_pi_more:
        movem 1,cr_iowd
cr_pi_xfer:
        datai 0150,(1)
        aos mfsdev_io_in+4
        jrst pdp10_pi_handler_return

cr_read_card:
        jumpe 1,kret_arg
        skipe cr_iowd
        jrst kret_neg4
        movei 2,0200000
cr_read_ready_wait:
        consz 0150,0100
        jrst cr_read_start
        sojg 2,cr_read_ready_wait
        jrst kret_neg2
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
        jrst kret_neg2
cr_read_done:
        consz 0150,0400
        jrst kret_busy
        movei 1,0120
        popj 017,

        .bss
cr_iowd:
        .block 1

; Device-local accounting state; absent devices consume no fixed KCORE.
        .bss
