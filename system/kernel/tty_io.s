; tty_io.s -- minimal resident terminal output dispatcher.
;
; Terminal 0 is CTY, 1..16 are DCS lines 0..15, and 17..20 are GE consoles.
; MINIT patches the right halves of the three JRST words below.  Missing
; backends leave the default jump to tty_putchar_invalid.  Tail jumps reuse
; the caller's return PC and require no resident backend-address words.

        .text
        .globl tty_putchar
        .globl tty_cty_putchar_address
        .globl tty_dcs_putchar_address
        .globl tty_ge_putchar_address
        .globl pdp10_ret_arg_v34

tty_putchar:
        move 2,1
        lsh 2,-010
        andi 2,077
        jumpe 2,tty_putchar_cty
        caile 2,020
        jrst tty_putchar_ge
        subi 1,0400
tty_dcs_putchar_address:
        jrst tty_putchar_invalid

tty_putchar_ge:
        caile 2,024
        jrst tty_putchar_invalid
        subi 1,010400
tty_ge_putchar_address:
        jrst tty_putchar_invalid

tty_putchar_cty:
tty_cty_putchar_address:
        jrst tty_putchar_invalid

tty_putchar_invalid:
        jrst pdp10_ret_arg_v34
