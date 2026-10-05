/**
 * @file ptp_io.s
 * @brief Resident PDP-6 paper-tape punch driver for device 0100.
 *
 * KINIT probes PI assignment, then leaves the PIA disabled; native WORDTOKEN8
 * output polls BUSY/DONE synchronously and never sleeps, so executive
 * non-preemption already provides exclusive ownership for the complete call.
 * PTP is installed independently of PTR, so neither optional device forces
 * the other's resident code into memory.
 */
        .globl mfsdev_io_out
        .text
        .globl ptp_write_words
        .globl kret_busy
        .globl kret_ok

; int ptp_write_words(const kword_t *words, unsigned int nwords)
; Decode WORDTOKEN8 payloads directly.  A zero low nibble is the fast/full
; four-byte form; 1..3 marks a partial final group.  4..15 is malformed.
ptp_write_words:
        jumpe 1,kret_arg
        jumpe 2,kret_ok
        push 17,010
        push 17,011
        push 17,012
        push 17,013
        push 17,014
        push 17,015
        move 010,1                    ; input cursor
        hrrz 011,2                    ; requested words
        setz 012,                     ; completed words
ptp_words_next:
        move 013,(010)
        move 014,013
        andi 014,017                  ; partial count, zero means full four
        jumpe 014,ptp_words_full
        caile 014,3
        jrst ptp_words_bad
        jrst ptp_words_byte
ptp_words_full:
        movei 014,4
ptp_words_byte:
        move 015,013
        lsh 015,-034                  ; high eight bits
        andi 015,0377
        coni 0100,1
        trne 1,0100                   ; no output tape attached
        jrst ptp_words_io
        trne 1,0020                   ; previous hardware operation busy
        jrst ptp_words_busy
        datao 0100,015
        aos mfsdev_io_out+3
        movei 1,0200000
ptp_words_wait:
        consz 0100,0010               ; DONE
        jrst ptp_words_done
        sojg 1,ptp_words_wait
        jrst ptp_words_timeout
ptp_words_done:
        cono 0100,0
        lsh 013,010                   ; next byte becomes high byte
        sojg 014,ptp_words_byte
        addi 010,1
        addi 012,1
        sojg 011,ptp_words_next
        jrst ptp_words_return
ptp_words_bad:
ptp_words_io:
        movni 1,4
        jrst ptp_words_error
ptp_words_busy:
        movni 1,3
        jrst ptp_words_error
ptp_words_timeout:
        movni 1,2
ptp_words_error:
        jumpe 012,ptp_words_first_error
        jrst ptp_words_return
ptp_words_first_error:
        move 012,1
ptp_words_return:
        cono 0100,0
        move 1,012
        pop 17,015
        pop 17,014
        pop 17,013
        pop 17,012
        pop 17,011
        pop 17,010
        popj 17,
