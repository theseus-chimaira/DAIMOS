; tty_io.s -- compact logical-terminal I/O dispatcher.
;
; Terminal 0 is CTY, 1..16 are DCS lines 0..15, and 17..20 are GE consoles.
; MINIT patches the six backend tail jumps below.  Missing backends retain the
; shared argument-error target.  Tail jumps reuse the caller return PC.

        .text
        .globl tty_putchar
        .globl tty_getchar
        .globl tty_cty_putchar_address
        .globl tty_dcs_putchar_address
        .globl tty_ge_putchar_address
        .globl tty_cty_getchar_address
        .globl tty_dcs_getchar_address
        .globl tty_ge_getchar_address
        .globl pdp10_ret_arg
        .globl devicefs_io_out

tty_putchar:
        ldb 2,[POINT 6,1,27]
        jumpe 2,tty_putchar_cty
        caile 2,020
        jrst tty_putchar_ge
        subi 1,0400
        aos devicefs_io_out+011
tty_dcs_putchar_address:
        jrst pdp10_ret_arg

tty_putchar_ge:
        caile 2,024
        jrst pdp10_ret_arg
        subi 1,010400
        aos devicefs_io_out+011
tty_ge_putchar_address:
        jrst pdp10_ret_arg

tty_putchar_cty:
        aos devicefs_io_out+011
tty_cty_putchar_address:
        jrst pdp10_ret_arg

; AC1 = logical TTY id.  DCS/GE backends receive their zero-based line id and
; return a character from that exact logical line.
tty_getchar:
        jumpe 1,tty_getchar_cty
        caile 1,020
        jrst tty_getchar_ge
        subi 1,1
tty_dcs_getchar_address:
        jrst pdp10_ret_arg

tty_getchar_ge:
        caile 1,024
        jrst pdp10_ret_arg
        subi 1,021
tty_ge_getchar_address:
        jrst pdp10_ret_arg

tty_getchar_cty:
tty_cty_getchar_address:
        jrst pdp10_ret_arg
