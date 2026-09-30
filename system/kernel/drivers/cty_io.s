/**
 * @file cty_io.s
 * @brief Resident PDP-6 console teletype driver for device 0120.
 *
 * KINIT installs this MRES package after probing CTY and publishes its
 * getchar/putchar entry points to the generic TTY layer. The package remains
 * resident for the lifetime of the kernel because PI4 owns input delivery,
 * output completion, and the operator CTRL-\\ real-time escape.
 *
 * Input uses cty_rx_pending as a one-character mailbox. The stored value is
 * character+1 so zero remains the empty marker. cty_rx_event closes the race
 * between checking the mailbox and sleeping: getchar clears the event, then
 * rechecks both the software mailbox and the hardware INPUT READY flag before
 * blocking. Output uses cty_tx_pending as a one-word busy/completion flag.
 * No dynamic storage belongs to the physical CTY driver.
 */

        .text
        .globl mfsdev_io_in
        .globl mfsdev_io_out
        .globl cty_pi_handler
        .globl cty_putchar
        .globl cty_getchar
        .globl cty_tx_pending
        .globl cty_rx_pending
        .globl cty_rx_event
        .globl proc_wait_event_intr
        .globl proc_wakeup_event
        .globl proc_rt_owner
        .globl pdp10_pi_handler_return
        .globl kret_ok
        .globl kret_busy

/**
 * @brief Service CTY PI4 input and output events.
 *
 * @return Does not return normally; jumps to pdp10_pi_handler_return.
 *
 * AC1 is clobbered. AC2, AC3, and AC17 are preserved for the generic PI
 * dispatcher. OUTPUT READY clears cty_tx_pending. INPUT READY consumes one
 * DATAI character, handles CTRL-\\ specially when an RT owner exists, and
 * otherwise publishes character+1 before waking cty_rx_event waiters.
 */
cty_pi_handler:
        conso 0120,0010
        jrst cty_pi_input
        setzm cty_tx_pending
        cono 0120,000204
cty_pi_input:
        conso 0120,0040
        jrst pdp10_pi_handler_return
        datai 0120,1
        aos mfsdev_io_in+0
        andi 1,0177
        caie 1,034                    ; CTRL-\: operator RT escape
        jrst cty_pi_input_normal
        skipn proc_rt_owner
        jrst cty_pi_input_normal
        setzm proc_rt_owner
        jrst pdp10_pi_handler_return
cty_pi_input_normal:
        addi 1,1
        movem 1,cty_rx_pending
        setom cty_rx_event
        movei 1,cty_rx_event
        pushj 17,proc_wakeup_event
        jrst pdp10_pi_handler_return
/**
 * @brief Write one seven-bit character and wait for PI4 completion.
 *
 * @param AC1 Character value; only bits 29..35 (low seven bits) are sent.
 * @return AC1 = 0, CTY_E_BUSY (-3), or CTY_E_TIMEOUT (-2).
 *
 * AC2 is the polling countdown and AC3 is temporary completion state; both are
 * caller-saved. AC17 carries the normal return address. cty_tx_pending is set
 * before DATAO so the interrupt cannot be lost between transmission and the
 * completion wait. Timeout clears the software pending flag.
 */
cty_putchar:
        move 2,cty_tx_pending
        jumpn 2,kret_busy
        movei 2,0200000
cty_putchar_wait_idle:
        conso 0120,0020
        jrst cty_putchar_ready
        sojg 2,cty_putchar_wait_idle
        jrst cty_putchar_timeout
cty_putchar_ready:
        setom cty_tx_pending
        andi 1,0177
        datao 0120,1
        aos mfsdev_io_out+0
        movei 2,0200000
cty_putchar_wait_done:
        move 3,cty_tx_pending
        jumpe 3,kret_ok
        sojg 2,cty_putchar_wait_done
        setzm cty_tx_pending
cty_putchar_timeout:
        jrst    kret_neg2

/**
 * @brief Return one seven-bit input character, sleeping until one is ready.
 *
 * @return AC1 = character 0..0177, or the nonzero status returned by
 *         proc_wait_event_intr when the sleep is interrupted.
 *
 * AC1 is the mailbox/result register; AC17 is the normal kernel stack. The
 * double check after clearing cty_rx_event is required to prevent a lost
 * wakeup if PI4 delivers input between the initial empty test and the sleep.
 * Direct hardware DATAI is also accepted so a character already pending in
 * the device need not wait for another interrupt transition.
 */
cty_getchar:
cty_getchar_loop:
        move 1,cty_rx_pending
        jumpn 1,cty_getchar_pending
        conso 0120,0040
        jrst cty_getchar_sleep
        datai 0120,1
        aos mfsdev_io_in+0
        andi 1,0177
        popj 017,
cty_getchar_sleep:
        setzm cty_rx_event
        move 1,cty_rx_pending
        jumpn 1,cty_getchar_pending
        consz 0120,0040
        jrst cty_getchar_loop
cty_getchar_wait:
        movei 1,cty_rx_event
        pushj 17,proc_wait_event_intr
        jumpn 1,cty_getchar_return
        jrst cty_getchar_loop
cty_getchar_pending:
        setzm cty_rx_pending
        subi 1,1
cty_getchar_return:
        popj 017,

        .bss
/** Nonzero while one synchronous DATAO is awaiting OUTPUT READY PI4. */
cty_tx_pending:
        .block 1
/** Character+1 one-byte mailbox; zero means no software-buffered input. */
cty_rx_pending:
        .block 1
/** Process-event word used to sleep/wake cty_getchar without lost wakeups. */
cty_rx_event:
        .block 1
