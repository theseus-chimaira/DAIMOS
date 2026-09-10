; slv_io.s -- PDP-6 slave/interprocessor interrupt service.
;
; The slave interface shares main memory; resident kernel work is therefore
; only interrupt acknowledgement.  Keep the PI assignment while clearing the
; pending interprocessor interrupt.

        .text
        .globl slv_pi_handler
        .globl pdp10_pi_handler_return

slv_pi_handler:
        cono 0020,0000017
        jrst pdp10_pi_handler_return
