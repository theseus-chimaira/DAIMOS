; kinit_io.s -- KINIT-only PDP-6 CTY and PI support.
;
; Stage1 supplies the temporary polling SIXBIT helper at 060.  KINIT uses it
; only for the initial banner.  kinit_cty_hw_init then installs a level-4 CTY
; interrupt vector.  After that transition KCORE may overwrite 060..077.

        .text
        .globl kinit_poll_put6
        .globl kinit_cty_hw_init
        .globl kinit_cty_putchar
        .globl kinit_halt

; void kinit_poll_put6(kword_t word)
; AC1 already contains the packed SIXBIT word.
kinit_poll_put6:
        pushj 017,000060
        popj 017,

; void kinit_cty_hw_init(void)
; Install only the CTY level-4 vector needed during KINIT/MINIT execution.
kinit_cty_hw_init:
        cono 0004,010000
        move 01,[jsr kinit_cty_pi]
        movem 01,000050
        setzm 000051
        setzm kinit_cty_pending
        ; Clear stale CTY ready flags and select PI level 4.
        movei 01,000504
        cono 0120,0(01)
        ; Enable PI system and level 4 (mask 010).
        movei 01,002210
        cono 0004,0(01)
        popj 017,

; int kinit_cty_putchar(int c)
; Output completion is interrupt-driven.  AC1 is the character and return code.
kinit_cty_putchar:
        movem 01,kinit_cty_char
kinit_cty_pending_wait:
        skipn kinit_cty_pending
        jrst kinit_cty_ready_wait
        jrst kinit_cty_pending_wait
kinit_cty_ready_wait:
        coni 0120,kinit_cty_status
        move 02,kinit_cty_status
        trne 02,000020
        jrst kinit_cty_ready_wait
        setom kinit_cty_pending
        datao 0120,kinit_cty_char
kinit_cty_done_wait:
        skipn kinit_cty_pending
        jrst kinit_cty_done
        jrst kinit_cty_done_wait
kinit_cty_done:
        setz 01,
        popj 017,

; PDP-6 priority interrupt handler for CTY level 4.
kinit_cty_pi:
        .word 0
        movem 01,kinit_cty_save1
        movem 02,kinit_cty_save2
        cono 0004,000400
        coni 0120,kinit_cty_status
        move 01,kinit_cty_status
        trnn 01,000040
        jrst kinit_cty_pi_output
        datai 0120,kinit_cty_input
        movei 02,000404
        cono 0120,0(02)
kinit_cty_pi_output:
        trnn 01,000010
        jrst kinit_cty_pi_return
        setzm kinit_cty_pending
        movei 02,000104
        cono 0120,0(02)
kinit_cty_pi_return:
        movei 02,002210
        cono 0004,0(02)
        move 02,kinit_cty_save2
        move 01,kinit_cty_save1
        jrst 10,@kinit_cty_pi

kinit_halt:
        halt .
        jrst kinit_halt

        .data
kinit_cty_pending: .word 0
kinit_cty_char:    .word 0
kinit_cty_status:  .word 0
kinit_cty_input:   .word 0
kinit_cty_save1:   .word 0
kinit_cty_save2:   .word 0
