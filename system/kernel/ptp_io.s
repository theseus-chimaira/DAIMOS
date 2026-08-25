; ptp_io.s -- compact resident PDP-6 paper-tape punch service.
;
; PI7 completion is handled directly by io7_io.s; this file contains only
; the synchronous public service and its one-word ownership state.
        .text
        .globl ptp_putchar
        .globl ptp_state
        .globl io7_ret_timeout
        .globl io7_ret_e3

; AC1 = byte.  Return 0, PT_E_BUSY (-3), PT_E_IO (-4), or timeout (-2).
ptp_putchar:
        skipn ptp_state
        jrst ptp_putchar_idle
        hrroi 1,0777775
        popj 017,
ptp_putchar_idle:
        coni 0100,2
        trne 2,0100
        jrst ptp_putchar_io
        trne 2,0020
        jrst io7_ret_e3
        movei 2,1
        movem 2,ptp_state
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
        hrroi 1,0777774
        popj 017,
ptp_putchar_ok:
        movei 1,0
        popj 017,

        .bss
ptp_state:
        .block 1
