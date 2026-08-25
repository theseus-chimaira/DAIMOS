; ptp_io.s -- compact resident PDP-6 paper-tape punch driver.
        .text
        .globl ptp_pi_handler
        .globl ptp_putchar
        .globl ptp_state
        .globl pdp10_pi_handler_return

ptp_pi_handler:
        coni 0100,1
        trnn 1,0010
        jrst pdp10_pi_handler_return
        setzm ptp_state
        cono 0100,0007
        jrst pdp10_pi_handler_return

; AC1 = byte.  Return 0, PT_E_BUSY (-3), PT_E_IO (-4), or timeout (-2).
ptp_putchar:
        move 2,ptp_state
        jumpn 2,ptp_putchar_busy
        coni 0100,2
        trne 2,0100
        jrst ptp_putchar_io
        trne 2,0020
        jrst ptp_putchar_busy
        movei 2,1
        movem 2,ptp_state
        cono 0100,0007
        andi 1,0377
        datao 0100,1
        movei 2,0200000
ptp_putchar_wait:
        move 3,ptp_state
        jumpe 3,ptp_putchar_ok
        sojg 2,ptp_putchar_wait
        setzm ptp_state
        cono 0100,0007
        hrroi 1,0777776
        popj 017,
ptp_putchar_busy:
        hrroi 1,0777775
        popj 017,
ptp_putchar_io:
        hrroi 1,0777774
        popj 017,
ptp_putchar_ok:
        movei 1,0
        popj 017,

        .bss
ptp_state:
        .block 1
