        .text
        .globl _start
        .globl main
        .globl dsys_exit
; Native startup ABI: AC1=argc, AC2=argv, AC3=envp.  KCC passes these
; registers straight through to main().  A normal C return is process exit;
; preserve main's return value in AC1 and use the native exit veneer.  The
; HALT is only a defensive fallback if SYS_EXIT ever returns unexpectedly.
_start:
        pushj 17,main
        pushj 17,dsys_exit
        halt .
