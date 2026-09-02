; dcs_io.s -- compact resident PDP-6 Type 630 DCS driver.
;
; Receive service uses the hardware scanner and PI2.  Input is armed only while
; dcs_getchar is waiting, so no resident queue is required.  The interrupt
; handler obeys the KCORE PI ABI and clobbers AC1 only.
;
; Type 630 IOT semantics follow the PDP-6 Handbook directly.

        .text
        .globl devicefs_io_in
        .globl devicefs_io_out
        .globl dcs_pi_handler
        .globl dcs_getchar
        .globl dcs_putchar
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_ok
        .globl pdp10_ret_arg
        .globl pdp10_ret_busy

; dcs_rx_word is zero when idle, -1 while a receive is pending, and the packed
; nonnegative line/byte result once the PI handler has serviced the scanner.
dcs_pi_handler:
        conso 0300,000010
        jrst pdp10_pi_handler_return
        skipl dcs_rx_word
        jrst pdp10_pi_handler_return
dcs_pi_receive:
        coni 0304,1
        andi 1,077
        lsh 1,010
        movem 1,dcs_rx_word
        datai 0304,1
        aos devicefs_io_in+3
        andi 1,0377
        iorm 1,dcs_rx_word
        cono 0300,0
        jrst pdp10_pi_handler_return

; Return DCS_PACK(line, byte), or DCS_E_BUSY (-3) if another receive is active.
dcs_getchar:
        move 1,dcs_rx_word
        jumpn 1,pdp10_ret_busy
        setom dcs_rx_word
        cono 0300,000012

dcs_getchar_wait:
        move 1,dcs_rx_word
        jumpl 1,dcs_getchar_wait
        setzm dcs_rx_word
        popj 017,

; AC1 = DCS_PACK(line, byte).  Return 0 or DCS_E_ARG (-1).
dcs_putchar:
        move 2,1
        lsh 2,-010
        andi 2,077
        caile 2,017
        jrst pdp10_ret_arg
        cono 0304,0(2)
        andi 1,0377
        datao 0300,1
        aos devicefs_io_out+3
        jrst pdp10_ret_ok

        .bss
dcs_rx_word:
        .block 1
