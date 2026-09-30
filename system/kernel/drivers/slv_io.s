/**
 * @file slv_io.s
 * @brief Minimal resident PDP-6 slave/interprocessor interrupt service.
 *
 * The slave interface shares main memory with the PDP-6, so no resident buffer,
 * queue, or data-copy path is needed. KINIT probes device 020, installs this
 * two-instruction MRES only when present, and registers it on PI7.
 */

        .text
        .globl slv_pi_handler
        .globl pdp10_pi_handler_return

/**
 * @brief Acknowledge one slave/interprocessor interrupt while retaining PI7.
 * @return Does not return normally; jumps to pdp10_pi_handler_return.
 *
 * No AC is clobbered. CONO 020,017 combines CLEAR_IRQ (010) with PIA 7, so the
 * current interrupt is dismissed without disabling future slave interrupts.
 */
slv_pi_handler:
        cono 0020,0000017
        jrst pdp10_pi_handler_return
