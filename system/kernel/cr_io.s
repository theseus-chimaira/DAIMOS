; cr_io.s -- compact resident PDP-6 card-reader driver.
        .text
        .globl cr_pi_handler
        .globl cr_read_card
        .globl cr_state
        .globl cr_cols
        .globl pdp10_pi_handler_return

; cr_state: bit 0 pending, bit 1 done, bit 2 error, column count in bits 3+.
; cr_cols advances as columns arrive, keeping the interrupt path to AC1 only.
cr_pi_handler:
        move 1,cr_state
        trnn 1,0001
        jrst pdp10_pi_handler_return
        coni 0150,1
        trne 1,0400
        jrst cr_pi_error
        trne 1,0010
        jrst cr_pi_data
        trnn 1,0020
        jrst pdp10_pi_handler_return
        cono 0150,0027
        move 1,cr_state
        andi 1,01770
        iori 1,0002
        movem 1,cr_state
        jrst pdp10_pi_handler_return
cr_pi_data:
        move 1,cr_state
        caige 1,01201
        jrst cr_pi_data_ok
cr_pi_error:
        movei 1,0006
        movem 1,cr_state
        jrst pdp10_pi_handler_return
cr_pi_data_ok:
        datai 0150,1
        andi 1,07777
        movem 1,@cr_cols
        aos cr_cols
        movei 1,0010
        addm 1,cr_state
        jrst pdp10_pi_handler_return

; AC1 = 80-word destination.  Return 80 or a CARD_E_* error.
cr_read_card:
        jumpe 1,cr_read_arg
        move 2,cr_state
        trne 2,0001
        jrst cr_read_busy
        movei 2,0200000
cr_read_ready_wait:
        coni 0150,3
        trne 3,0100
        jrst cr_read_start
        sojg 2,cr_read_ready_wait
        jrst cr_read_timeout
cr_read_start:
        movem 1,cr_cols
        movei 3,1
        movem 3,cr_state
        cono 0150,01237
        movei 2,0200000
cr_read_done_wait:
        move 3,cr_state
        trne 3,0002
        jrst cr_read_done
        sojg 2,cr_read_done_wait
        setzm cr_state
        cono 0150,0007
cr_read_timeout:
        hrroi 1,0777776
        popj 017,
cr_read_done:
        trne 3,0004
        jrst cr_read_io
        move 1,3
        andi 1,01770
        lsh 1,-3
        caie 1,0120
        jrst cr_read_limit
        popj 017,
cr_read_arg:
        seto 1,
        popj 017,
cr_read_io:
        hrroi 1,0777775
        popj 017,
cr_read_busy:
        hrroi 1,0777774
        popj 017,
cr_read_limit:
        hrroi 1,0777773
        popj 017,

        .bss
cr_state:
        .block 1
cr_cols:
        .block 1
