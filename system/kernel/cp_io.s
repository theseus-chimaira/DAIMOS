; cp_io.s -- compact resident PDP-6 card-punch driver.
        .text
        .globl cp_pi_handler
        .globl cp_punch_card
        .globl cp_state
        .globl cp_cols
        .globl pdp10_pi_handler_return

; cp_cols advances as the punch requests columns, keeping PI to AC1 only.
cp_pi_handler:
        move 1,cp_state
        trnn 1,0001
        jrst pdp10_pi_handler_return
        coni 0110,1
        trne 1,05000
        jrst cp_pi_error
        trne 1,0010
        jrst cp_pi_data
        trnn 1,0100
        jrst pdp10_pi_handler_return
        cono 0110,0107
        move 1,cp_state
        andi 1,01770
        iori 1,0002
        movem 1,cp_state
        jrst pdp10_pi_handler_return
cp_pi_data:
        move 1,cp_state
        caige 1,01201
        jrst cp_pi_data_ok
cp_pi_error:
        movei 1,0006
        movem 1,cp_state
        jrst pdp10_pi_handler_return
cp_pi_data_ok:
        move 1,@cp_cols
        andi 1,07777
        datao 0110,1
        aos cp_cols
        movei 1,0010
        addm 1,cp_state
        move 1,cp_state
        caige 1,01201
        jrst pdp10_pi_handler_return
        cono 0110,010207
        jrst pdp10_pi_handler_return

; AC1 = 80-word source.  Return 80 or a CARD_E_* error.
cp_punch_card:
        jumpe 1,cp_punch_arg
        move 2,cp_state
        trne 2,0001
        jrst cp_punch_busy
        movem 1,cp_cols
        movei 3,1
        movem 3,cp_state
        cono 0110,01347
        movei 2,0200000
cp_punch_wait:
        move 3,cp_state
        trne 3,0002
        jrst cp_punch_done
        sojg 2,cp_punch_wait
        setzm cp_state
        cono 0110,0007
        hrroi 1,0777776
        popj 17,
cp_punch_done:
        trne 3,0004
        jrst cp_punch_io
        move 1,3
        andi 1,01770
        lsh 1,-3
        caie 1,0120
        jrst cp_punch_limit
        popj 17,
cp_punch_arg:
        seto 1,
        popj 17,
cp_punch_io:
        hrroi 1,0777775
        popj 17,
cp_punch_busy:
        hrroi 1,0777774
        popj 17,
cp_punch_limit:
        hrroi 1,0777773
        popj 17,

        .bss
cp_state:
        .block 1
cp_cols:
        .block 1
