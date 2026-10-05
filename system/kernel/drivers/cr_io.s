/**
 * @file cr_io.s
 * @brief Resident PDP-6 card-reader MRES for device 0150.
 *
 * KINIT probes the reader and installs this package only when the device is
 * usable.  The resident interface is the native 27-word CARD12 transfer; it
 * polls DATA READY directly with the PIA disabled and packs columns while the
 * card is moving.  The path never sleeps, so executive non-preemption is the
 * ownership lock.
 */
        .globl mfsdev_io_in
        .text
        .globl cr_read_words
        .globl kret_arg
        .globl kret_busy

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
        push 17,010
        push 17,011
        push 17,012
        push 17,013
        push 17,014
        move 010,1                    ; output cursor

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
        move 1,012
        pop 17,014
        pop 17,013
        pop 17,012
        pop 17,011
        pop 17,010
        popj 17,
