; cty_io.s -- compact resident PDP-6 console driver.
;
; The interrupt path uses only AC1, as required by the KCORE PI ABI.
; Synchronous output services use caller-saved ACs only and perform their
; wait loops directly so no generic resident delay helper is required.

        .text
        .globl mfsdev_io_in
        .globl mfsdev_io_out
        .globl cty_pi_handler
        .globl cty_putchar
        .globl cty_getchar
        .globl cty_tx_pending
        .globl cty_rx_pending
        .globl cty_rx_event
        .globl proc_wait_event_intr
        .globl proc_wakeup_event
        .globl proc_rt_owner
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_ok
        .globl pdp10_ret_busy

cty_pi_handler:
        conso 0120,0010
        jrst cty_pi_input
        setzm cty_tx_pending
        cono 0120,000204
cty_pi_input:
        conso 0120,0040
        jrst pdp10_pi_handler_return
        datai 0120,1
        aos mfsdev_io_in+0
        andi 1,0177
        caie 1,034                    ; CTRL-\: operator RT escape
        jrst cty_pi_input_normal
        skipn proc_rt_owner
        jrst cty_pi_input_normal
        setzm proc_rt_owner
        jrst pdp10_pi_handler_return
cty_pi_input_normal:
        addi 1,1
        movem 1,cty_rx_pending
        setom cty_rx_event
        movei 1,cty_rx_event
        pushj 17,proc_wakeup_event
        jrst pdp10_pi_handler_return
; AC1 = 7-bit character.  Return 0, CTY_E_BUSY (-3), or CTY_E_TIMEOUT (-2).
cty_putchar:
        move 2,cty_tx_pending
        jumpn 2,pdp10_ret_busy
        movei 2,0200000
cty_putchar_wait_idle:
        conso 0120,0020
        jrst cty_putchar_ready
        sojg 2,cty_putchar_wait_idle
        jrst cty_putchar_timeout
cty_putchar_ready:
        setom cty_tx_pending
        andi 1,0177
        datao 0120,1
        aos mfsdev_io_out+0
        movei 2,0200000
cty_putchar_wait_done:
        move 3,cty_tx_pending
        jumpe 3,pdp10_ret_ok
        sojg 2,cty_putchar_wait_done
        setzm cty_tx_pending
cty_putchar_timeout:
        jrst    pdp10_ret_neg2

; Return one 7-bit character in AC1.  Input blocks until available.
; The PI handler stores character+1 so zero remains the empty marker.
cty_getchar:
cty_getchar_loop:
        move 1,cty_rx_pending
        jumpn 1,cty_getchar_pending
        conso 0120,0040
        jrst cty_getchar_sleep
        datai 0120,1
        aos mfsdev_io_in+0
        andi 1,0177
        popj 017,
cty_getchar_sleep:
        setzm cty_rx_event
        move 1,cty_rx_pending
        jumpn 1,cty_getchar_pending
        conso 0120,0040
        jrst cty_getchar_wait
        jrst cty_getchar_loop
cty_getchar_wait:
        movei 1,cty_rx_event
        pushj 17,proc_wait_event_intr
        jumpn 1,cty_getchar_return
        jrst cty_getchar_loop
cty_getchar_pending:
        setzm cty_rx_pending
        subi 1,1
cty_getchar_return:
        popj 017,

        .bss
cty_tx_pending:
        .block 1
cty_rx_pending:
        .block 1
cty_rx_event:
        .block 1
