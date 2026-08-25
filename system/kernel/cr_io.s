; cr_io.s -- compact PDP-6 card-reader service.
;
; cr_iowd combines the 80-column count and destination pointer.  The shared
; PI7 handler advances it with AOBJN semantics and sets it to -1 after the
; final column while waiting for END CARD; zero means idle/done.

        .text
        .globl cr_read_card
        .globl cr_iowd
        .globl io7_ret_arg
        .globl io7_ret_timeout
        .globl io7_ret_e3

cr_read_card:
        jumpe 1,io7_ret_arg
        skipn cr_iowd
        jrst cr_read_idle
        hrroi 1,0777774
        popj 017,
cr_read_idle:
        movei 2,0200000
cr_read_ready_wait:
        coni 0150,3
        trne 3,0100
        jrst cr_read_start
        sojg 2,cr_read_ready_wait
        jrst io7_ret_timeout
cr_read_start:
        move 2,1
        subi 2,1
        movei 3,0120
        movn 3,3
        hrl 2,3
        movem 2,cr_iowd
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
        coni 0150,3
        trne 3,0400
        jrst io7_ret_e3
        movei 1,0120
        popj 017,

        .bss
cr_iowd:
        .block 1
