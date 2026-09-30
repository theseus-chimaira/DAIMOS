/**
 * @file dcs_io.s
 * @brief Resident PDP-6 Type 630 DCS terminal-multiplexer driver.
 *
 * The hardware has one receiver scanner for all sixteen lines. Readers name
 * the line they want; a scanner result for another line is deferred in that
 * logical TTY's existing process-session record by proc_tty_pending_store().
 * One MRES receive word and one event word are therefore sufficient for all
 * DCS lines, avoiding fixed per-line resident RAM.
 *
 * On the PDP-6 interface DCSA 0300 owns scanner control and character DATAO,
 * while DCSB 0304 reports the stopped receiver line through CONI and selects
 * the send-buffer line through CONO. The scanner is armed only while a reader
 * is waiting and is disabled after PI service until the consuming path rearms
 * it, so dcs_rx_word cannot be overwritten by a second line.
 */

        .text
        .globl dcs_pi_handler
        .globl dcs_getchar
        .globl dcs_putchar
        .globl proc_tty_pending_take
        .globl proc_tty_pending_store
        .globl proc_wait_event_intr
        .globl proc_wakeup_event
        .globl pdp10_pi_handler_return
        .globl kret_ok
        .globl kret_arg

/**
 * @brief Capture one stopped-scanner receive byte at PI2.
 * @return Does not return normally; jumps to pdp10_pi_handler_return.
 *
 * AC1 is clobbered; AC2, AC3, and AC17 are preserved for the generic PI ABI.
 * dcs_rx_word is zero when empty and packed(line,byte)+1 when ready, keeping
 * line 0/NUL distinct from the empty marker. If a word is already pending the
 * scanner is simply disabled, preventing overwrite until a reader consumes it.
 */
dcs_pi_handler:
        conso 0300,000010
        jrst pdp10_pi_handler_return
        skipe dcs_rx_word
        jrst dcs_pi_disable
        coni 0304,1
        andi 1,077
        lsh 1,010
        movem 1,dcs_rx_word
        datai 0304,1
        andi 1,0377
        iorm 1,dcs_rx_word
        aos dcs_rx_word
        setom dcs_rx_event
        movei 1,dcs_rx_event
        pushj 17,proc_wakeup_event
dcs_pi_disable:
        cono 0300,0
        jrst pdp10_pi_handler_return

/**
 * @brief Return one byte from exactly one requested DCS line.
 * @param AC1 Requested physical DCS line 0..15.
 * @return AC1 = byte 0..0377, or a negative status from the event wait path;
 *         invalid line numbers return DCS_E_ARG (-1).
 *
 * The requested line is saved on the AC17 stack across calls into process/TTY
 * helpers. Before arming the scanner the routine clears dcs_rx_event and then
 * rechecks dcs_rx_word, closing the interrupt-versus-sleep lost-wakeup race.
 * A byte for another scanner line is moved into that logical TTY's packed
 * pending field and its readers are awakened before scanning resumes.
 */
dcs_getchar:
        caile 1,017
        jrst kret_arg
        push 17,1
dcs_getchar_loop:
        move 1,(17)
        addi 1,1                     ; DCS line N is logical TTY N+1
        pushj 17,proc_tty_pending_take
        jumpge 1,dcs_getchar_done

        move 2,dcs_rx_word
        jumpn 2,dcs_getchar_ready
        setzm dcs_rx_event
        ; Clear-before-arm plus the second ready check prevents lost wakeups.
        cono 0300,000012
        skipe dcs_rx_word
        jrst dcs_getchar_loop
        movei 1,dcs_rx_event
        pushj 17,proc_wait_event_intr
        jumpn 1,dcs_getchar_done
        jrst dcs_getchar_loop

dcs_getchar_ready:
        setzm dcs_rx_word
        subi 2,1
        ldb 3,[POINT 6,2,27]
        camn 3,(17)
        jrst dcs_getchar_ready_ours
        move 1,3
        addi 1,1
        andi 2,0377
        pushj 17,proc_tty_pending_store
        ; Wake a reader of the line for which this byte was deferred.
        setom dcs_rx_event
        movei 1,dcs_rx_event
        pushj 17,proc_wakeup_event
        jrst dcs_getchar_loop

dcs_getchar_ready_ours:
        move 1,2
        andi 1,0377
dcs_getchar_done:
        sub 17,[1,,1]
        popj 17,

/**
 * @brief Send one byte through the Type 630 send buffer.
 * @param AC1 DCS_PACK(line, byte), with physical line 0..15.
 * @return AC1 = DCS_E_OK (0) or DCS_E_ARG (-1).
 *
 * AC2 is clobbered while extracting the line; AC17 is the normal return stack.
 * DCSB CONO selects the send-buffer line, then DCSA DATAO transmits the masked
 * eight-bit character. Higher-level output serialization belongs to TTY.
 */
dcs_putchar:
        ldb 2,[POINT 6,1,27]
        caile 2,017
        jrst kret_arg
        cono 0304,0(2)
        andi 1,0377
        datao 0300,1
        jrst kret_ok

        .bss
/** packed(line,byte)+1 receive mailbox; zero means no scanner result pending. */
dcs_rx_word:
        .block 1
/** Process-event word for readers sleeping on receiver-scanner progress. */
dcs_rx_event:
        .block 1
