/**
 * @file cp_io.s
 * @brief Resident PDP-6 card-punch MRES for device 0110.
 *
 * KINIT probes the punch and installs this package only when CP is usable, so
 * an absent punch consumes neither driver text nor cp_iowd permanent state.
 * cp_punch_card is synchronous to its caller while PI7 supplies one 12-bit
 * Hollerith column for every DATA REQUEST raised by the hardware.
 *
 * cp_iowd is both the busy flag and AOBJN cursor.  Zero means idle/completed;
 * an active transfer is -remaining,,address-before-next-column; and -1 means
 * the 80th DATAO has been issued and EJECT requested, but END CARD has not yet
 * arrived.  Keeping that -1 sentinel prevents the caller from returning while
 * the device still references the logical card operation.
 */
        .globl mfsdev_io_out
        .text
        .globl cp_pi_handler
        .globl cp_punch_card
        .globl cp_write_words
        .globl pdp10_pi_handler_return
        .globl kret_arg
        .globl kret_busy

/**
 * @brief Service card-punch PI7 events.
 *
 * @return Does not return normally; jumps to pdp10_pi_handler_return.
 *
 * AC1 is clobbered.  AC2, AC3, and AC17 remain untouched for the generic PI
 * dispatcher.  DATA REQUEST advances cp_iowd and emits one masked 12-bit
 * DATAO.  The final column changes cp_iowd to -1 and requests EJECT; END CARD
 * or a hardware error clears cp_iowd and releases the waiting caller.
 */
cp_pi_handler:
        coni 0110,1
        trne 1,05000
        jrst cp_pi_done
        trne 1,0010
        jrst cp_pi_data
        trnn 1,0100
        jrst pdp10_pi_handler_return
cp_pi_done:
        cono 0110,0107
        setzm cp_iowd
        jrst pdp10_pi_handler_return
cp_pi_data:
        move 1,cp_iowd
        aobjn 1,cp_pi_more
        move 1,(1)
        andi 1,07777
        datao 0110,1
        aos mfsdev_io_out+5
        setom cp_iowd
        cono 0110,010207
        jrst pdp10_pi_handler_return
cp_pi_more:
        movem 1,cp_iowd
        move 1,(1)
        andi 1,07777
        datao 0110,1
        aos mfsdev_io_out+5
        jrst pdp10_pi_handler_return

/**
 * @brief Punch one physical 80-column card from a caller buffer.
 *
 * @param AC1 Address of an 80-word input buffer.
 * @return AC1 = 0120 (80 decimal) on success, -1 for a null buffer, -2 on a
 *         bounded completion timeout, -3 on punch error/trouble, or -4 if CP
 *         is already busy.
 *
 * AC2 is a private polling countdown and is clobbered.  AC17 carries the
 * normal return address.  On timeout the routine clears cp_iowd and removes
 * the device PIA, preventing later DATA REQUEST interrupts from reading a
 * caller buffer whose lifetime has ended.
 */
cp_punch_card:
        jumpe 1,kret_arg
        skipe cp_iowd
        jrst kret_neg4
        subi 1,1
        hrli 1,0777660
        movem 1,cp_iowd
        cono 0110,01347
        movei 2,0200000
cp_punch_wait:
        skipn cp_iowd
        jrst cp_punch_done
        sojg 2,cp_punch_wait
        setzm cp_iowd
        cono 0110,0007
        jrst kret_neg2
cp_punch_done:
        consz 0110,05000
        jrst kret_busy
        movei 1,0120
        popj 017,

; int cp_write_words(const kword_t *words, unsigned int nwords)
; Punch exactly one 80-column card from 27 WORDTOKEN12 words.  Polling the
; hardware request directly avoids expanding the packed record to an 80-word
; transient buffer.  The unused third token of the final word must be zero so
; malformed/noncanonical card records are rejected before motion begins.
cp_write_words:
        jumpe 1,kret_arg
        caie 2,033                    ; one complete card only
        jrst kret_arg
        skipe cp_iowd
        jrst kret_neg4
        move 3,032(1)
        andi 3,07777                  ; canonical unused final token
        jumpn 3,kret_arg
        push 17,010
        push 17,011
        push 17,012
        push 17,013
        push 17,014
        move 010,1                    ; input word cursor
        movei 011,0120                ; 80 columns
        setz 012,                     ; slot 0..2
        move 013,(010)
        setom cp_iowd
        cono 0110,01340               ; punch on/status clear, no PIA

cp_words_column:
        movei 014,0200000
cp_words_wait:
        coni 0110,1
        trne 1,05000                  ; ERROR/TROUBLE
        jrst cp_words_io
        trne 1,0010                   ; DATA REQUEST
        jrst cp_words_data
        sojg 014,cp_words_wait
        jrst cp_words_timeout
cp_words_data:
        move 1,013
        lsh 1,-030                    ; high 12-bit token
        andi 1,07777
        datao 0110,1
        aos mfsdev_io_out+5
        lsh 013,014                   ; next 12-bit token to top
        addi 012,1
        caie 012,3
        jrst cp_words_column_done
        setz 012,
        addi 010,1
        move 013,(010)
cp_words_column_done:
        sojg 011,cp_words_column

        cono 0110,010200              ; EJECT + END CARD enable, no PIA
        movei 014,0200000
cp_words_end_wait:
        coni 0110,1
        trne 1,05000
        jrst cp_words_io
        trne 1,0100                   ; END CARD
        jrst cp_words_ok
        sojg 014,cp_words_end_wait
cp_words_timeout:
        movni 011,2
        jrst cp_words_finish
cp_words_io:
        movni 011,3
        jrst cp_words_finish
cp_words_ok:
        movei 011,033
cp_words_finish:
        cono 0110,0
        setzm cp_iowd
        move 1,011
        pop 17,014
        pop 17,013
        pop 17,012
        pop 17,011
        pop 17,010
        popj 17,

        .bss
/**
 * Punch transfer state: 0 idle, -remaining,,pointer while active, -1 waiting
 * for END CARD after the 80th DATAO/EJECT request.
 */
cp_iowd:
        .block 1
