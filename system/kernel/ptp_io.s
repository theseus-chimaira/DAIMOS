; ptp_io.s -- compact resident PDP-6 paper-tape punch service.
;
; PI7 completion is handled directly by io7_io.s; this file contains only
; the synchronous public service and its one-word ownership state.
        .text
        .globl ptp_putchar
        .globl ptp_state
        .globl io7_ret_timeout
        .globl io7_ret_e3
        .globl io7_ret_e4
        .globl pdp10_ret_ok_v34

; AC1 = byte.  Return 0, PT_E_BUSY (-3), PT_E_IO (-4), or timeout (-2).
ptp_putchar:
        skipe ptp_state
        jrst io7_ret_e3
ptp_putchar_idle:
        coni 0100,2
        trne 2,0100
        jrst ptp_putchar_io
        trne 2,0020
        jrst io7_ret_e3
        setom ptp_state
        cono 0100,0007
        andi 1,0377
        datao 0100,1
        movei 2,0200000
ptp_putchar_wait:
        skipn ptp_state
        jrst ptp_putchar_ok
        sojg 2,ptp_putchar_wait
        setzm ptp_state
        cono 0100,0007
        jrst io7_ret_timeout
ptp_putchar_io:
        jrst io7_ret_e4
ptp_putchar_ok:
        jrst pdp10_ret_ok_v34

        .bss
ptp_state:
        .block 1
