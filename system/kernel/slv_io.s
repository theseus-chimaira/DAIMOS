; slv_io.s -- PDP-6 slave/interprocessor interrupt service.
;
; The slave interface shares main memory; resident kernel work is therefore
; only interrupt acknowledgement.  Keep the PI assignment while clearing the
; pending interprocessor interrupt.

        .text
        .globl slv_pi_handler
        .globl pdp10_pi_handler_return

slv_pi_handler:
        movei 1,0000017
        cono 0020,0(1)
        jrst pdp10_pi_handler_return
