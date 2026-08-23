; tty_io.s -- minimal resident terminal output dispatcher.
;
; Terminal 0 is the CTY.  Terminals 1..16 map to DCS lines 0..15.
; MINIT patches the two backend function-address words after relocation.
; The dispatcher owns no queues and no per-line state.

        .text
        .globl tty_putchar
        .globl tty_cty_putchar_address
        .globl tty_dcs_putchar_address

; AC1 = TTY_PACK(terminal, byte).  Return backend status or TTY_E_INVALID (-1).
tty_putchar:
        move 2,1
        lsh 2,-010
        andi 2,077
        jumpe 2,tty_putchar_cty
        caile 2,020
        jrst tty_putchar_invalid
        move 3,tty_dcs_putchar_address
        jumpe 3,tty_putchar_invalid
        subi 1,0400
        pushj 17,(3)
        popj 17,

tty_putchar_cty:
        move 3,tty_cty_putchar_address
        jumpe 3,tty_putchar_invalid
        pushj 17,(3)
        popj 17,

tty_putchar_invalid:
        seto 1,
        popj 17,

        .bss
tty_cty_putchar_address:
        .block 1
tty_dcs_putchar_address:
        .block 1
