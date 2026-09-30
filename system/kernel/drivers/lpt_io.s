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
        .globl  mfsdev_io_out
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
        movei   3,0200000
        movei   4,lpt_putchar_send
lpt_putchar_wait:
        coni    0124,2
        trne    2,000400
        jrst    kret_neg4
        trne    2,000100
        jrst    (4)
        sojg    3,lpt_putchar_wait
        jrst    kret_neg2

lpt_putchar_send:
        andi    1,0177
        lsh     1,035                  ; first 7-bit DATAO slot (bit 29)
        datao   0124,1
        movei   3,0200000
        movei   4,lpt_putchar_done
        jrst    lpt_putchar_wait

lpt_putchar_done:
        aos     mfsdev_io_out+022
        jrst    kret_ok
