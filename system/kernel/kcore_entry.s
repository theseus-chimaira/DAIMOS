; kcore_entry.s -- fixed KCORE V0.1 call gate at relocated base 060.
;
; KINIT may call the early service vectors and return to KINIT.  The final
; entry remains separate and is not used until the bootstrap is complete.

        .text
        .globl __kcore_image_start
        .globl kcore_entry
        .globl kcore_early_init
        .globl kcore_putchar
        .globl kcore_entry_impl
        .globl kcore_early_init_impl
        .globl kcore_putchar_impl

__kcore_image_start:
kcore_entry:      jrst kcore_entry_impl
kcore_early_init: jrst kcore_early_init_impl
kcore_putchar:    jrst kcore_putchar_impl
