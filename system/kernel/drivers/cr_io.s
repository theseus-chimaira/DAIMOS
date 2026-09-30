/**
 * @file cr_io.s
 * @brief Resident PDP-6 card-reader MRES for device 0150.
 *
 * KINIT probes the reader and installs this package only when the device is
 * usable, so both its code and cr_iowd state disappear entirely when CR is
 * absent.  The public operation is synchronous, but transfer is interrupt
 * driven: PI7 accepts one 12-bit card column at a time into the caller's
 * 80-word buffer and wakes the polling caller when END CARD clears cr_iowd.
 *
 * cr_iowd is both the busy flag and AOBJN cursor.  Zero means idle/completed;
 * an active transfer is -remaining,,address-before-next-column; and -1 means
 * all 80 columns have been transferred but the final END CARD interrupt is
 * still outstanding.  The caller's buffer must therefore remain valid until
 * cr_read_card returns.
 */
        .globl mfsdev_io_in
        .text
        .globl cr_pi_handler
        .globl cr_read_card
        .globl cr_read_words
        .globl pdp10_pi_handler_return
        .globl kret_arg
        .globl kret_busy

/**
 * @brief Service card-reader PI7 events.
 *
 * @return Does not return normally; jumps to pdp10_pi_handler_return.
 *
 * AC1 is clobbered.  AC2, AC3, and AC17 are untouched, preserving the generic
 * PI dispatcher ABI.  TROUBLE or END CARD completes the operation; DATA READY
 * advances the AOBJN cursor and performs one DATAI directly into the buffer.
 */
cr_pi_handler:
        coni 0150,1
        trne 1,0400
        jrst cr_pi_done
        trne 1,0010
        jrst cr_pi_data
        trnn 1,0020
        jrst pdp10_pi_handler_return
cr_pi_done:
        cono 0150,0027
        setzm cr_iowd
        jrst pdp10_pi_handler_return
cr_pi_data:
        move 1,cr_iowd
        aobjn 1,cr_pi_more
        setom cr_iowd
        jrst cr_pi_xfer
cr_pi_more:
        movem 1,cr_iowd
cr_pi_xfer:
        datai 0150,(1)
        aos mfsdev_io_in+4
        jrst pdp10_pi_handler_return

/**
 * @brief Read one physical 80-column card into a caller buffer.
 *
 * @param AC1 Address of an 80-word output buffer.
 * @return AC1 = 0120 (80 decimal) on success, -1 for a null buffer, -2 on a
 *         bounded wait timeout, -3 on reader trouble, or -4 if CR is busy.
 *
 * AC2 is a private polling countdown and is clobbered.  AC17 is used only for
 * the normal return ABI.  The initial readiness wait prevents starting motion
 * before RDY READ; after CONO READ CARD the routine waits until PI7 changes
 * cr_iowd to zero.  A timeout clears both software state and the hardware PIA
 * so a stale transfer cannot continue into a returned caller buffer.
 */
cr_read_card:
        jumpe 1,kret_arg
        skipe cr_iowd
        jrst kret_neg4
        movei 2,0200000
cr_read_ready_wait:
        consz 0150,0100
        jrst cr_read_start
        sojg 2,cr_read_ready_wait
        jrst kret_neg2
cr_read_start:
        subi 1,1
        hrli 1,0777660
        movem 1,cr_iowd
        cono 0150,01237
        movei 2,0200000
cr_read_done_wait:
        skipn cr_iowd
        jrst cr_read_done
        sojg 2,cr_read_done_wait
        setzm cr_iowd
        cono 0150,0007
        jrst kret_neg2
cr_read_done:
        consz 0150,0400
        jrst kret_busy
        movei 1,0120
        popj 017,

; int cr_read_words(kword_t *words, unsigned int nwords)
; Read one fixed 80-column card directly into 27 WORDTOKEN12 words.  The bulk
; path owns the reader for the whole mechanical operation and polls DATA READY
; itself, avoiding both an 80-word temporary buffer and per-column PI traffic.
; The last packed word contains columns 78 and 79 in its first two slots; its
; low 12-bit third slot is zero.  Capacity below 27 words is rejected before
; card motion begins.
cr_read_words:
        jumpe 1,kret_arg
        caige 2,033                    ; 27 decimal
        jrst kret_arg
        skipe cr_iowd
        jrst kret_neg4
        push 17,010
        push 17,011
        push 17,012
        push 17,013
        push 17,014
        move 010,1                    ; output cursor
        setom cr_iowd                 ; exclusive bulk ownership

        movei 011,0200000
cr_words_ready_wait:
        coni 0150,1
        trne 1,0400                   ; TROUBLE
        jrst cr_words_io
        trne 1,0100                   ; RDY READ
        jrst cr_words_start
        sojg 011,cr_words_ready_wait
        jrst cr_words_timeout
cr_words_start:
        cono 0150,01230               ; READ CARD, clear status, no PIA
        movei 012,0120                ; 80 columns
        setz 013,                     ; packed accumulator
        setz 014,                     ; tokens in accumulator

cr_words_column:
        movei 011,0200000
cr_words_data_wait:
        coni 0150,1
        trne 1,0400
        jrst cr_words_io
        trne 1,0010                   ; DATA READY
        jrst cr_words_data
        sojg 011,cr_words_data_wait
        jrst cr_words_timeout
cr_words_data:
        datai 0150,1
        aos mfsdev_io_in+4
        andi 1,07777
        lsh 013,014                   ; 12 decimal == octal 014
        ior 013,1
        addi 014,1
        caie 014,3
        jrst cr_words_column_done
        movem 013,(010)
        addi 010,1
        setz 013,
        setz 014,
cr_words_column_done:
        sojg 012,cr_words_column

        jumpe 014,cr_words_end_wait
        lsh 013,014                   ; two final tokens -> top 24 bits
        movem 013,(010)
cr_words_end_wait:
        movei 011,0200000
cr_words_end_loop:
        coni 0150,1
        trne 1,0400
        jrst cr_words_io
        trne 1,0020                   ; END CARD
        jrst cr_words_ok
        sojg 011,cr_words_end_loop
cr_words_timeout:
        movni 012,2
        jrst cr_words_finish
cr_words_io:
        movni 012,3
        jrst cr_words_finish
cr_words_ok:
        movei 012,033
cr_words_finish:
        cono 0150,0
        setzm cr_iowd
        move 1,012
        pop 17,014
        pop 17,013
        pop 17,012
        pop 17,011
        pop 17,010
        popj 17,

        .bss
/**
 * Reader transfer state: 0 idle, -remaining,,pointer while active, -1 waiting
 * for END CARD after the 80th DATAI.
 */
cr_iowd:
        .block 1
