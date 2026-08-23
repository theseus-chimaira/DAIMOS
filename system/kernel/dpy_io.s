; dpy_io.s -- compact interrupt-driven PDP-6 Type 340 display driver.
;
; DPY shares PI6 with the APR line clock.  MINIT patches
; dpy_clk_pi_service_address with the relocated clock service routine and
; replaces the clock PI-table entry with dpy_pi_handler.  If no clock exists,
; the pointer remains zero and the same handler services DPY alone.

        .text
        .globl dpy_pi_handler
        .globl dpy_putword
        .globl dpy_clk_pi_service_address
        .globl pdp10_pi_handler_return

; One word is in flight at a time.  The Type 340 raises DONE after completing
; the second half of each DATAO word.  The shared PI path clobbers AC1 only.
dpy_pi_handler:
        move 1,dpy_clk_pi_service_address
        jumpe 1,dpy_pi_display
        pushj 17,(1)
dpy_pi_display:
        coni 0130,1
        trnn 1,000200
        jrst pdp10_pi_handler_return
        setzm dpy_pending
        movei 1,6
        cono 0130,0(1)
        jrst pdp10_pi_handler_return

; AC1 = one Type 340 instruction word.  Return 0 or DPY_E_BUSY (-3).
; Completion is interrupt-driven; callers wait for the real DONE interrupt.
dpy_putword:
        skipn dpy_pending
        jrst dpy_put_start
        hrroi 1,0777775
        popj 17,
dpy_put_start:
        setom dpy_pending
        datao 0130,1
dpy_put_wait:
        skipn dpy_pending
        jrst dpy_put_ok
        jrst dpy_put_wait
dpy_put_ok:
        movei 1,0
        popj 17,

        .bss
dpy_pending:
        .block 1
dpy_clk_pi_service_address:
        .block 1
