        .text
        .globl __syscall
__syscall:
        move 5,-1(17)
        ; PDP-6 user UUO.  AC1 carries the syscall number; AC2..AC5 args.
        .word 0
        popj 17,
