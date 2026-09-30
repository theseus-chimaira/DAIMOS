; lpt_io.s -- resident PDP-6 line-printer output driver.
        .text
        .globl  lpt_putchar
        .globl  mfsdev_io_out
        .globl  kret_ok
        .globl  kret_neg2
        .globl  kret_neg4

; AC1 = 7-bit character.  Return 0, -4 on printer error, -2 on timeout.
;
; The Type-124/SIMH interface accepts five packed 7-bit characters per DATAO.
; Zero characters are ignored, so put one character in the first slot and
; leave the remaining four slots zero.  DAIMOS deliberately uses polling:
; line-printer output is synchronous today, and avoiding PI state keeps the
; resident driver very small.
lpt_putchar:
        movei   3,0200000
lpt_putchar_ready:
        coni    0124,2
        trne    2,000400
        jrst    kret_neg4
        trne    2,000100
        jrst    lpt_putchar_send
        sojg    3,lpt_putchar_ready
        jrst    kret_neg2

lpt_putchar_send:
        andi    1,0177
        lsh     1,035                  ; first 7-bit DATAO slot (bit 29)
        datao   0124,1
        movei   3,0200000
lpt_putchar_wait:
        coni    0124,2
        trne    2,000400
        jrst    kret_neg4
        trne    2,000100
        jrst    lpt_putchar_done
        sojg    3,lpt_putchar_wait
        jrst    kret_neg2

lpt_putchar_done:
        aos     mfsdev_io_out+022
        jrst    kret_ok
