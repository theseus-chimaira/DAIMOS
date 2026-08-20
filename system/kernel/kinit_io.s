; kinit_io.s -- KINIT-only polling bootstrap helpers.
;
; Stage1 supplies the temporary packed-SIXBIT helper at 060.  KINIT uses it
; only for the initial banner.  kinit_relocate() then overwrites 060 with
; KCORE, whose resident PI/CTY implementation owns all subsequent console I/O.

        .text
        .globl kinit_poll_put6
        .globl kinit_halt

; void kinit_poll_put6(kword_t word)
; AC1 already contains the packed SIXBIT word.
kinit_poll_put6:
        pushj 017,000060
        popj 017,

kinit_halt:
        halt .
        jrst kinit_halt
