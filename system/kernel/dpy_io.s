; dpy_io.s -- compact interrupt-driven PDP-6 Type 340 display driver.
;
; DPY shares PI6 with the APR line clock.  MINIT patches the right half of
; dpy_clk_pi_service_call with the relocated clock service routine and replaces
; the clock PI-table entry with dpy_pi_handler.  If no clock exists, the call
; initially targets the KCORE zero-return stub; MINIT patches it to the clock service when present.

        .text
        .globl devicefs_v1_io_in
        .globl devicefs_v1_io_out
        .globl dpy_pi_handler
        .globl dpy_putword
        .globl dpy_clk_pi_service_call
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_ok_v34
        .globl pdp10_ret_busy_v34

; One word is in flight at a time.  The Type 340 raises DONE after completing
; the second half of each DATAO word.  The shared PI path clobbers AC1 only.
dpy_pi_handler:
dpy_clk_pi_service_call:
        pushj 017,pdp10_ret_ok_v34
dpy_pi_display:
        conso 0130,000200
        jrst pdp10_pi_handler_return
        setzm dpy_pending
        cono 0130,000006
        jrst pdp10_pi_handler_return

; AC1 = one Type 340 instruction word.  Return 0 or DPY_E_BUSY (-3).
; Completion is interrupt-driven; callers wait for the real DONE interrupt.
dpy_putword:
        skipe dpy_pending
        jrst pdp10_ret_busy_v34
dpy_put_start:
        setom dpy_pending
        datao 0130,1
        aos devicefs_v1_io_out+8
dpy_put_wait:
        skipe dpy_pending
        jrst dpy_put_wait
dpy_put_ok:
        jrst pdp10_ret_ok_v34

        .bss
dpy_pending:
        .block 1
