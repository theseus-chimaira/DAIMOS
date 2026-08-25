; cp_io.s -- compact PDP-6 card-punch service.
;
; cp_iowd combines the 80-column count and source pointer.  The shared PI7
; handler advances it and requests eject immediately after column 80.

        .text
        .globl cp_punch_card
        .globl cp_iowd
        .globl io7_ret_arg
        .globl io7_ret_timeout
        .globl io7_ret_e3
        .globl io7_ret_e4

cp_punch_card:
        jumpe 1,io7_ret_arg
        skipe cp_iowd
        jrst io7_ret_e4
cp_punch_idle:
        move 2,1
        subi 2,1
        hrli 2,0777660
        movem 2,cp_iowd
        cono 0110,01347
        movei 2,0200000
cp_punch_wait:
        skipn cp_iowd
        jrst cp_punch_done
        sojg 2,cp_punch_wait
        setzm cp_iowd
        cono 0110,0007
        jrst io7_ret_timeout
cp_punch_done:
        consz 0110,05000
        jrst io7_ret_e3
        movei 1,0120
        popj 017,

        .bss
cp_iowd:
        .block 1
