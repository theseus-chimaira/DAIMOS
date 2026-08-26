        .text
        .globl __syscall
        .globl mach_syscall
__syscall:
        pushj 17,mach_syscall
        popj 17,
