/**
 * @file ge_io.s
 * @brief Resident PDP-6 General Electric GE/GTY terminal driver.
 *
 * GTYI device 0070 supplies keyboard input for four logical consoles; GTYO
 * device 0750 emits the framed display protocol. GE owns PI4 independently.
 * One hardware input path therefore serves four logical TTYs. Readers request
 * one console, while bytes for another console are deferred in that TTY's
 * existing process-session record instead of allocating four resident queues.
 *
 * ge_rx_word uses LH bit 4 as an internal nonzero marker around the raw GTYI
 * word so line 0/NUL remains representable. ge_tx_state serializes an entire
 * seven-byte GE output frame; higher-level buffering remains in the TTY layer.
 */

        .text
        .globl ge_pi_handler
        .globl ge_getchar
        .globl ge_putchar
        .globl proc_tty_pending_take
        .globl proc_tty_pending_store
        .globl proc_wait_event_intr
        .globl proc_wakeup_event
        .globl pdp10_pi_handler_return
        .globl kret_ok
        .globl kret_arg
        .globl kret_busy

/**
 * @brief Capture one GTYI input word at PI4.
 * @return Does not return normally; jumps to pdp10_pi_handler_return.
 *
 * AC1 is clobbered; AC2, AC3, and AC17 remain valid for the generic PI ABI.
 * If the one-word mailbox is already occupied, GTYI PI is disabled until a
 * reader consumes it. Otherwise DATAI is stored with internal LH bit 4 set,
 * the event is published, and sleepers are awakened before input is disabled.
 */
ge_pi_handler:
        conso 0070,00010
        jrst pdp10_pi_handler_return
        skipe ge_rx_word
        jrst ge_pi_gtyi_disable
        datai 0070,1
        tlo 1,4
        movem 1,ge_rx_word
        setom ge_rx_event
        movei 1,ge_rx_event
        pushj 17,proc_wakeup_event
ge_pi_gtyi_disable:
        cono 0070,0
        jrst pdp10_pi_handler_return

/**
 * @brief Return one byte from exactly one requested GE console.
 * @param AC1 Requested GE console 0..3.
 * @return AC1 = character 0..0177, or a negative event-wait/error status.
 *
 * AC1 is saved on the AC17 stack across process/TTY helper calls; AC2/AC3 are
 * scratch. The event is cleared before input is re-enabled and the mailbox is
 * then rechecked, preventing a lost wakeup. Bytes for other consoles are moved
 * into their logical TTY pending fields and their readers are awakened.
 */
ge_getchar:
        caile 1,3
        jrst kret_arg
        push 17,1
ge_getchar_loop:
        move 1,(17)
        addi 1,021                   ; GE N is logical TTY 17+N
        pushj 17,proc_tty_pending_take
        jumpge 1,ge_getchar_done

        move 2,ge_rx_word
        jumpn 2,ge_getchar_ready
        consz 0070,00010
        jrst ge_getchar_hardware
        setzm ge_rx_event
        cono 0070,000004
        skipe ge_rx_word
        jrst ge_getchar_loop
        movei 1,ge_rx_event
        pushj 17,proc_wait_event_intr
        jumpn 1,ge_getchar_done
        jrst ge_getchar_loop

ge_getchar_hardware:
        datai 0070,2
        tlo 2,4
ge_getchar_ready:
        setzm ge_rx_word
        tlz 2,4
        ldb 3,[POINT 2,2,17]
        camn 3,(17)
        jrst ge_getchar_ready_ours
        move 1,3
        addi 1,021
        andi 2,0177
        pushj 17,proc_tty_pending_store
        setom ge_rx_event
        movei 1,ge_rx_event
        pushj 17,proc_wakeup_event
        jrst ge_getchar_loop

ge_getchar_ready_ours:
        move 1,2
        andi 1,0177
ge_getchar_done:
        sub 17,[1,,1]
        popj 17,

/**
 * @brief Encode and synchronously transmit one decoded seven-bit GE byte.
 * @param AC1 Decoded byte 0..0177.
 * @return After GTYO becomes ready again; AC1/AC3 are clobbered.
 *
 * The hardware representation is the complemented one-bit rotate of the
 * decoded byte. Polling is bounded by hardware readiness rather than a kernel
 * timeout because a complete frame must remain contiguous once owned.
 */
ge_put_decoded:
        conso 0750,00100
        jrst ge_put_decoded
        andi 1,0177
        move 3,1
        lsh 1,-1
        trne 3,1
        iori 1,0100
        xori 1,0177
        datao 0750,1
ge_put_decoded_wait:
        conso 0750,00100
        jrst ge_put_decoded_wait
        popj 017,

/**
 * @brief Emit one complete GE display frame for a logical console byte.
 * @param AC1 GE_PACK(console, byte).
 * @return AC1 = GE_E_OK (0), GE_E_ARG (-1), or GE_E_BUSY (-3).
 *
 * AC4 preserves the packed input and AC5 carries the encoded GE address/check
 * contribution. The frame is SOH, address, NUL, STX, data, ETX, longitudinal
 * parity. ge_tx_state covers the entire frame so two callers cannot interleave
 * protocol bytes.
 */
ge_putchar:
        skipe ge_tx_state
        jrst kret_busy
        cono 0750,0
ge_putchar_idle:
        move 4,1
        ldb 5,[POINT 6,1,27]
        caile 5,3
        jrst kret_arg
        setom ge_tx_state
        movei 1,1
        pushj 017,ge_put_decoded
        lsh 5,3
        addi 5,0140
        move 1,5
        pushj 017,ge_put_decoded
        movei 1,0
        pushj 017,ge_put_decoded
        movei 1,2
        pushj 017,ge_put_decoded
        andi 4,0177
        move 1,4
        pushj 017,ge_put_decoded
        movei 1,3
        pushj 017,ge_put_decoded
        move 1,5
        xor 1,4
        xori 1,1
        pushj 017,ge_put_decoded
        setzm ge_tx_state
        jrst kret_ok

        .bss
/** Raw GTYI receive mailbox plus internal LH-ready marker; zero means empty. */
ge_rx_word:
        .block 1
/** Process event used to sleep/wake readers waiting for GTYI data. */
ge_rx_event:
        .block 1
/** Nonzero while one caller owns the complete seven-byte GTYO frame. */
ge_tx_state:
        .block 1
