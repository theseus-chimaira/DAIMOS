; dpy_io.s -- compact interrupt-driven PDP-6 Type 340 display driver.
;
; DPY shares PI6 with the APR line clock.  MINIT patches the right half of
; dpy_clk_pi_service_call with the relocated clock service routine and replaces
; the clock PI-table entry with dpy_pi_handler.  If no clock exists, the call
; initially targets the KCORE zero-return stub; MINIT patches it to the clock service when present.

        .globl devicefs_io_out
        .text
        .globl dpy_pi_handler
        .globl dpy_putword
        .globl dpy_clk_pi_service_call
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_ok
        .globl pdp10_ret_busy

; One word is in flight at a time.  The Type 340 raises DONE after completing
; the second half of each DATAO word.  The shared PI path clobbers AC1 only.
dpy_pi_handler:
dpy_clk_pi_service_call:
        pushj 017,pdp10_ret_ok
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
        jrst pdp10_ret_busy
dpy_put_start:
        setom dpy_pending
        datao 0130,1
        aos devicefs_io_out+010
dpy_put_wait:
        skipe dpy_pending
        jrst dpy_put_wait
dpy_put_ok:
        jrst pdp10_ret_ok

        .bss
dpy_pending:
        .block 1

; Device-local accounting state; absent devices consume no fixed KCORE.
        .bss
