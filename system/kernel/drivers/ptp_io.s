/**
 * @file ptp_io.s
 * @brief Resident PDP-6 paper-tape punch driver for device 0100.
 *
 * The punch uses one MRES state word and PI7 completion. PTP is installed
 * independently of PTR; no shared resident paper-tape package is required.
 * ptp_state is nonzero only while a caller-owned DATAO awaits DONE.
 */
        .globl mfsdev_io_out
        .text
        .globl ptp_pi_handler
        .globl ptp_putchar
        .globl ptp_write_words
        .globl pdp10_pi_handler_return
        .globl kret_busy
        .globl kret_ok

/**
 * @brief Complete one PTP DATAO at PI7.
 * @return Does not return normally; jumps to pdp10_pi_handler_return.
 *
 * No scratch AC is required. DONE clears ptp_state and CONO retains PI7 while
 * acknowledging the completion condition.
 */
ptp_pi_handler:
        conso 0100,0010
        jrst pdp10_pi_handler_return
        setzm ptp_state
        cono 0100,0007
        jrst pdp10_pi_handler_return

/**
 * @brief Punch one eight-bit byte and synchronously await PI7 completion.
 * @param AC1 Byte value; low eight bits are transmitted.
 * @return AC1 = 0, PT_E_TIMEOUT (-2), PT_E_BUSY (-3), or PT_E_IO (-4).
 *
 * AC2 holds status/countdown state; AC17 is untouched. The routine rejects an
 * unattached punch before setting software ownership, marks ptp_state before
 * DATAO to close the completion race, and bounds the wait. A timeout releases
 * software ownership but leaves PI7 enabled so delayed hardware completion is
 * safely acknowledged; hardware BUSY prevents a new DATAO from overlapping it.
 */
ptp_putchar:
        skipe ptp_state
        jrst kret_busy
        coni 0100,2
        trne 2,0100
        jrst kret_neg4
        trne 2,0020
        jrst kret_busy
        setom ptp_state
        cono 0100,0007
        andi 1,0377
        datao 0100,1
        aos mfsdev_io_out+3
        movei 2,0200000
ptp_putchar_wait:
        skipn ptp_state
        jrst kret_ok
        sojg 2,ptp_putchar_wait
        setzm ptp_state
        cono 0100,0007
ptp_ret_timeout:
        jrst    kret_neg2

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
        move 1,015
        pushj 17,ptp_putchar
        jumpn 1,ptp_words_error
        lsh 013,010                   ; next byte becomes high byte
        sojg 014,ptp_words_byte
        addi 010,1
        addi 012,1
        sojg 011,ptp_words_next
        jrst ptp_words_return
ptp_words_bad:
ptp_words_error:
        jumpe 012,ptp_words_first_error
        jrst ptp_words_return
ptp_words_first_error:
        seto 012,
ptp_words_return:
        move 1,012
        pop 17,015
        pop 17,014
        pop 17,013
        pop 17,012
        pop 17,011
        pop 17,010
        popj 17,
        .bss
/** Nonzero while one caller-owned DATAO awaits punch DONE. */
ptp_state:
        .block 1
