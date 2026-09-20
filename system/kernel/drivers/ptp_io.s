; ptp_io.s -- resident PDP-6 paper-tape punch driver.
        .globl mfsdev_io_out
        .text
        .globl ptp_pi_handler
        .globl ptp_putchar
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_busy
        .globl pdp10_ret_ok

ptp_pi_handler:
        conso 0100,0010
        jrst pdp10_pi_handler_return
        setzm ptp_state
        cono 0100,0007
        jrst pdp10_pi_handler_return

; AC1 = byte.  Return 0, PT_E_BUSY (-3), PT_E_IO (-4), or timeout (-2).
ptp_putchar:
        skipe ptp_state
        jrst pdp10_ret_busy
        coni 0100,2
        trne 2,0100
        jrst pdp10_ret_neg4
        trne 2,0020
        jrst pdp10_ret_busy
        setom ptp_state
        cono 0100,0007
        andi 1,0377
        datao 0100,1
        aos mfsdev_io_out+3
        movei 2,0200000
ptp_putchar_wait:
        skipn ptp_state
        jrst pdp10_ret_ok
        sojg 2,ptp_putchar_wait
        setzm ptp_state
        cono 0100,0007
ptp_ret_timeout:
        jrst    pdp10_ret_neg2
        .bss
ptp_state:
        .block 1

; Device-local accounting state; absent devices consume no fixed KCORE.
        .bss
