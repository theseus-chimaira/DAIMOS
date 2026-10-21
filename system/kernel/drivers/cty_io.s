/**
 * @file cty_io.s
 * @brief Resident PDP-6 console teletype driver for device 0120.
 *
 * KINIT installs this MRES package after probing CTY and publishes its
 * getchar/putchar entry points to the generic TTY layer. The package remains
 * resident for the lifetime of the kernel because PI4 owns input delivery,
 * output completion, and the operator CTRL-\\ real-time escape.
 *
 * A ten-character input FIFO occupies two packed 36-bit words (five
 * seven-bit characters per word) plus one count.  When full, PI4 is masked
 * without consuming Type-626 DATAI; the device retains the unacknowledged
 * byte and stops its PTY feeder.  getchar unmasks PI4 after freeing a slot.
 * No input is silently overwritten during slow disk/tape I/O. This costs
 * two more resident words than the former one-character mailbox.
 */

        .text
        .globl cty_pi_handler
        .globl cty_pi_tail
        .globl cty_putchar
        .globl cty_getchar
        .globl cty_tx_pending
        .globl cty_rx_words
        .globl cty_rx_count
        .globl cty_rx_event
        .globl cty_rx_put
        .globl cty_rx_get
        .globl proc_wait_event_intr
        .globl proc_wakeup_event
        .globl proc_rt_owner
        .globl pdp10_pi_handler_return
        .globl kret_ok
        .globl kret_busy
        .globl mfsdev_io_in
        .globl mfsdev_io_out

/**
 * @brief Service CTY PI4 input and output events.
 *
 * @return Does not return normally; jumps to pdp10_pi_handler_return.
 *
 * AC1 is clobbered. AC2, AC3, and AC17 are preserved for the generic PI
 * dispatcher. OUTPUT READY clears cty_tx_pending. INPUT READY consumes one
 * DATAI character, handles CTRL-\\ specially when an RT owner exists, and
 * otherwise queues the byte before waking cty_rx_event waiters.
 */
cty_pi_handler:
        conso 0120,0010
        jrst cty_pi_input
        setzm cty_tx_pending
        cono 0120,000204
cty_pi_input:
        conso 0120,0040
        jrst cty_pi_tail
        move 1,cty_rx_count
        caige 1,012                     ; ten characters, octal 12
        jrst cty_pi_input_ready
        cono 0120,0                     ; hold byte in Type-626 hardware
        jrst cty_pi_tail
cty_pi_input_ready:
        datai 0120,1
        andi 1,0177
        caie 1,034                    ; CTRL-\: operator RT escape
        jrst cty_pi_input_normal
        skipn proc_rt_owner
        jrst cty_pi_input_normal
        setzm proc_rt_owner
        jrst cty_pi_tail
cty_pi_input_normal:
        pushj 17,cty_rx_put
        setom cty_rx_event
        movei 1,cty_rx_event
        pushj 17,proc_wakeup_event
        jrst cty_pi_tail
cty_pi_tail:
        jrst pdp10_pi_handler_return

/** Append a seven-bit character (AC1). Zero succeeds, -1 means full.
 * Caller serializes against dequeue by PI4 priority. Preserves AC2/AC17. */
cty_rx_put:
        push 17,2
        move 2,cty_rx_count
        caige 2,012
        jrst cty_rx_put_ready
        seto 1,
        pop 17,2
        popj 17,
cty_rx_put_ready:
        andi 1,0177
        caige 2,5
        jrst cty_rx_put_first
        subi 2,5
        imuli 2,7
        lsh 1,0(2)
        iorm 1,cty_rx_words+1
        jrst cty_rx_put_done
cty_rx_put_first:
        imuli 2,7
        lsh 1,0(2)
        iorm 1,cty_rx_words
cty_rx_put_done:
        aos cty_rx_count
        setz 1,
        pop 17,2
        popj 17,

/** Remove oldest character, or -1 when empty. Serialized by masking PI4.
 * The fixed two-word compaction is bounded, preserving AC2/AC3/AC17. */
cty_rx_get:
        push 17,2
        push 17,3
        skipg cty_rx_count
        jrst cty_rx_get_empty
        move 1,cty_rx_words
        andi 1,0177
        move 2,cty_rx_words
        lsh 2,-7
        move 3,cty_rx_words+1
        andi 3,0177
        lsh 3,034                     ; 28 decimal, fifth field
        ior 2,3
        movem 2,cty_rx_words
        move 2,cty_rx_words+1
        lsh 2,-7
        movem 2,cty_rx_words+1
        sos cty_rx_count
        jrst cty_rx_get_done
cty_rx_get_empty:
        seto 1,
cty_rx_get_done:
        pop 17,3
        pop 17,2
        popj 17,
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
        movei 2,0200000
cty_putchar_wait_done:
        move 3,cty_tx_pending
        jumpe 3,cty_putchar_complete
        sojg 2,cty_putchar_wait_done
        setzm cty_tx_pending
cty_putchar_timeout:
        jrst    kret_neg2
cty_putchar_complete:
        ; Account a character only after PI4 confirms transmission.
        aos mfsdev_io_out
        jrst kret_ok

/**
 * @brief Return one seven-bit input character, sleeping until one is ready.
 *
 * @return AC1 = character 0..0177, or the nonzero status returned by
 *         proc_wait_event_intr when the sleep is interrupted.
 *
 * PI4 is disabled while the packed FIFO is changed. When the reader removes
 * one entry and restores the interrupt priority, an unacknowledged Type-626
 * input byte can resume delivery. Check the queue and hardware twice before
 * sleeping, so a concurrent interrupt cannot strand a pending character.
 */
cty_getchar:
cty_getchar_loop:
        cono 0120,0                    ; serialize with PI4 producer
        skipg cty_rx_count
        jrst cty_getchar_empty
        pushj 17,cty_rx_get
        ; Count delivered input, not bytes merely queued by PI4.
        aos mfsdev_io_in
        cono 0120,4                    ; re-arm a held hardware character
        popj 17,
cty_getchar_empty:
        conso 0120,0040
        jrst cty_getchar_sleep
        datai 0120,1
        andi 1,0177
        aos mfsdev_io_in
        cono 0120,4
        popj 017,
cty_getchar_sleep:
        setzm cty_rx_event
        skipg cty_rx_count
        jrst cty_getchar_check_hw
        jrst cty_getchar_loop
cty_getchar_check_hw:
        consz 0120,0040
        jrst cty_getchar_loop
cty_getchar_wait:
        cono 0120,4                    ; always reenable before blocking
        movei 1,cty_rx_event
        pushj 17,proc_wait_event_intr
        jumpn 1,cty_getchar_return
        jrst cty_getchar_loop
cty_getchar_return:
        popj 017,

        .bss
/** Nonzero while one synchronous DATAO is awaiting OUTPUT READY PI4. */
cty_tx_pending:
        .block 1
/** Ten seven-bit characters packed oldest-first in two words. */
cty_rx_words:
        .block 2
/** Occupancy 0..10; NUL is a valid queued character. */
cty_rx_count:
        .block 1
/** Process-event word used to sleep/wake cty_getchar without lost wakeups. */
cty_rx_event:
        .block 1
