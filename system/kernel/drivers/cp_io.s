/**
 * @file cp_io.s
 * @brief Resident PDP-6 card-punch MRES for device 0110.
 *
 * KINIT probes the punch and installs this package only when CP is usable, so
 * an absent punch consumes neither driver text nor its one-word busy state.
 * The resident interface accepts one native 27-word CARD12 image and polls
 * DATA REQUEST/END CARD directly with the PIA disabled.
 */
        .globl mfsdev_io_out
        .text
        .globl cp_write_words
        .globl kret_arg
        .globl kret_busy

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
 * Zero when idle, nonzero while one CARD12 write owns the physical punch.
 */
cp_iowd:
        .block 1
