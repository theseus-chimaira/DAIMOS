; cp_io.s -- resident PDP-6 card-punch driver.
        .globl mfsdev_io_out
        .text
        .globl cp_pi_handler
        .globl cp_punch_card
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_arg
        .globl pdp10_ret_busy

cp_pi_handler:
        coni 0110,1
        trne 1,05000
        jrst cp_pi_done
        trne 1,0010
        jrst cp_pi_data
        trnn 1,0100
        jrst pdp10_pi_handler_return
cp_pi_done:
        cono 0110,0107
        setzm cp_iowd
        jrst pdp10_pi_handler_return
cp_pi_data:
        move 1,cp_iowd
        aobjn 1,cp_pi_more
        move 1,(1)
        andi 1,07777
        datao 0110,1
        aos mfsdev_io_out+5
        setom cp_iowd
        cono 0110,010207
        jrst pdp10_pi_handler_return
cp_pi_more:
        movem 1,cp_iowd
        move 1,(1)
        andi 1,07777
        datao 0110,1
        aos mfsdev_io_out+5
        jrst pdp10_pi_handler_return

cp_punch_card:
        jumpe 1,pdp10_ret_arg
        skipe cp_iowd
        jrst pdp10_ret_neg4
        subi 1,1
        hrli 1,0777660
        movem 1,cp_iowd
        cono 0110,01347
        movei 2,0200000
cp_punch_wait:
        skipn cp_iowd
        jrst cp_punch_done
        sojg 2,cp_punch_wait
        setzm cp_iowd
        cono 0110,0007
        jrst pdp10_ret_neg2
cp_punch_done:
        consz 0110,05000
        jrst pdp10_ret_busy
        movei 1,0120
        popj 017,

        .bss
cp_iowd:
        .block 1

; Device-local accounting state; absent devices consume no fixed KCORE.
        .bss
