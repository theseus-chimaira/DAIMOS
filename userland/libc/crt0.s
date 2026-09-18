        .text
        .globl _start
        .globl main
; Native startup ABI: AC1=argc, AC2=argv, AC3=envp.  KCC passes these
; registers straight through to main().
_start:
        pushj 17,main
        halt .
