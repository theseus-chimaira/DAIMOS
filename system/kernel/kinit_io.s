; kinit_io.s -- KINIT-only bootstrap helpers.
;
; Every Stage1 installs the exact 16-word polling SIXBIT helper at the fixed
; top of the minimum supported 32K memory: 077760..077777.  That area remains
; valid for the entire disposable KINIT lifetime, so KCORE may occupy 060 and
; MRES may follow it gaplessly before any device driver is initialized.

        .text
        .globl kinit_poll_put6
        .globl kinit_halt

; void kinit_poll_put6(kword_t word)
; AC1 already contains the packed SIXBIT word.
kinit_poll_put6:
        pushj 017,077760
        popj 017,

kinit_halt:
        halt .
        jrst kinit_halt
