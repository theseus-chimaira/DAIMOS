/**
 * @file lpt_io.s
 * @brief Permanent PDP-6 LP10-compatible synchronous line-printer driver.
 *
 * Device 0124 accepts five packed seven-bit characters in one DATAO word.
 * DAIMOS sends a single character in the first slot and zero-fills the other
 * four, relying on the controller's zero-character suppression. Output is
 * intentionally polled: keeping no PI handler, queue, BSS, or completion event
 * minimizes permanent KCORE cost for this low-rate peripheral.
 */
        .text
        .globl  lpt_putchar
        .globl  lpt_write_s6rec
        .globl  kret_ok
        .globl  kret_neg2
        .globl  kret_neg4

/**
 * @brief Write one character and synchronously wait for LP10 completion.
 * @param AC1 Character value; low seven bits are used.
 * @return AC1 = 0 on success, -2 on bounded timeout, or -4 on printer error.
 *
 * AC2 holds CONI status; AC3 is the poll countdown; AC4 is the success
 * continuation for the shared wait loop. AC17 is untouched. Both the initial
 * ready wait and the post-DATAO completion wait use the same status loop so the
 * driver carries only one copy of the error/timeout policy.
 */
lpt_putchar:
        andi    1,0177
        lsh     1,035                  ; first 7-bit DATAO slot (bit 29)
lpt_putword:
        movei   3,0200000
        movei   4,lpt_putword_send
lpt_putchar_wait:
        coni    0124,2
        trne    2,000400
        jrst    kret_neg4
        trne    2,000100
        jrst    (4)
        sojg    3,lpt_putchar_wait
        jrst    kret_neg2

lpt_putword_send:
        datao   0124,1
        movei   3,0200000
        movei   4,lpt_putchar_done
        jrst    lpt_putchar_wait

lpt_putchar_done:
        jrst    kret_ok

; int lpt_write_s6rec(const kword_t *words, unsigned int nwords)
; Render exactly one complete S6REC TEXT record.  Five converted SIXBIT
; characters are packed into each native LP10 DATAO, reducing hardware waits
; by up to 5x relative to WRITECHAR.  CR/LF is emitted as one final packed
; DATAO word.  The record is validated completely before the first transfer.
lpt_write_s6rec:
        jumpe   1,kret_neg4
        jumpe   2,kret_ok
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        move    010,1                  ; record base
        hrrz    011,2                  ; supplied/return word count
        move    3,(010)
        ldb     4,[POINT 6,3,5]
        caie    4,1                    ; S6REC TEXT
        jrst    lpt_s6_bad
        and     3,[077777777]
        move    012,3                  ; character count
        move    4,3
        addi    4,5
        idivi   4,6
        addi    4,1
        came    4,011                  ; exactly one complete record
        jrst    lpt_s6_bad

        move    015,[POINT 6,0]
        movei   5,1(010)
        hrr     015,5
        jumpe   012,lpt_s6_eol
lpt_s6_batch:
        setz    013,                   ; packed LP10 DATAO word
        movei   014,5                  ; hardware character slots
        movei   7,035                  ; first slot shift = 29 decimal
lpt_s6_char:
        ildb    2,015
        addi    2,040                  ; SIXBIT -> ASCII
        lsh     2,0(7)
        ior     013,2
        subi    7,7
        subi    012,1
        jumpe   012,lpt_s6_flush
        sojg    014,lpt_s6_char
lpt_s6_flush:
        move    1,013
        pushj   17,lpt_putword
        jumpn   1,lpt_s6_bad
        jumpn   012,lpt_s6_batch

lpt_s6_eol:
        movei   1,015
        lsh     1,035                  ; CR in slot 0
        movei   2,012
        lsh     2,026                  ; LF in slot 1 (22 decimal)
        ior     1,2
        pushj   17,lpt_putword
        jumpn   1,lpt_s6_bad
        move    1,011
        jrst    lpt_s6_done
lpt_s6_bad:
        seto    1,
lpt_s6_done:
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
