; dcs_io.s -- compact resident PDP-6 Type 630 DCS driver.
;
; Receive service uses the hardware scanner and PI4.  Input is armed only while
; dcs_getchar is waiting, so no resident queue is required.  The interrupt
; handler obeys the KCORE PI ABI and clobbers AC1 only.
;
; Output defaults to current upstream SIMH semantics: CONO DCSB selects the
; line and DATAO DCSB transmits through that send buffer.  With
; DCS_SIMH_COMPAT=0, DATAO DCSA is emitted instead, matching the PDP-6
; Handbook description.  See ERRATA/README.md.

        .text
        .globl dcs_pi_handler
        .globl dcs_pi_service
        .globl dcs_getchar
        .globl dcs_putchar
        .globl pdp10_pi_handler_return

; dcs_rx_word is zero when idle, -1 while a receive is pending, and the packed
; nonnegative line/byte result once the PI handler has serviced the scanner.
dcs_pi_handler:
        pushj 017,dcs_pi_service
        jrst pdp10_pi_handler_return

; Callable PI service used by the GE shared PI4 handler.  Clobbers AC1 only.
dcs_pi_service:
        coni 0300,1
        trnn 1,000010
        popj 017,
        skipge dcs_rx_word
        jrst dcs_pi_receive
        popj 017,

dcs_pi_receive:
        coni 0304,1
        subi 1,2
        andi 1,077
        lsh 1,010
        movem 1,dcs_rx_word
        datai 0304,1
        andi 1,0377
        iorm 1,dcs_rx_word
        cono 0300,0
        popj 017,

; Return DCS_PACK(line, byte), or DCS_E_BUSY (-3) if another receive is active.
dcs_getchar:
        move 1,dcs_rx_word
        jumpn 1,dcs_getchar_busy
        setom dcs_rx_word
        cono 0300,000014

dcs_getchar_wait:
        move 1,dcs_rx_word
        jumpl 1,dcs_getchar_wait
        setzm dcs_rx_word
        popj 017,

dcs_getchar_busy:
        hrroi 1,0777775
        popj 017,

; AC1 = DCS_PACK(line, byte).  Return 0 or DCS_E_ARG (-1).
dcs_putchar:
        move 2,1
        lsh 2,-010
        andi 2,077
        caile 2,017
        jrst dcs_putchar_arg
        addi 2,2
        cono 0304,0(2)
        andi 1,0377
.ifdef DCS_SIMH_COMPAT
        datao 0304,1
.else
        datao 0300,1
.endif
        movei 1,0
        popj 017,

dcs_putchar_arg:
        seto 1,
        popj 017,

        .bss
dcs_rx_word:
        .block 1
