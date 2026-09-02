        .text
        .globl __syscall
        .globl mach_syscall
__syscall:
        move 5,-1(17)
        pushj 17,mach_syscall
        popj 17,
