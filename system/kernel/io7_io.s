; io7_io.s -- shared PI7 dispatcher for paper/card peripherals.
;
; PTR, PTP, CR, and CP all use PI7.  One handler slot and one MRES package
; serves all four, following the ITS practice of sharing interrupt glue.

        .text
        .globl io7_pi_handler
        .globl ptr_pi_service
        .globl ptp_pi_service
        .globl cr_pi_service
        .globl cp_pi_service
        .globl pdp10_pi_handler_return

io7_pi_handler:
        pushj 017,ptr_pi_service
        pushj 017,ptp_pi_service
        pushj 017,cr_pi_service
        pushj 017,cp_pi_service
        jrst pdp10_pi_handler_return
