; PID 1 SUPERVISOR: only the monitor UUO veneers it actually calls.
; This is exactly the calling convention of libc/syscall.s, with no
; unrelated syscall entrypoints resident in the supervisor image.
; C arguments arrive in AC1..AC4; return is AC1.

        .text
        .globl dsys_exit
        .globl dsys_getpid
        .globl dsys_ttyctl
        .globl dsys_run
        .globl dsys_wait
        .globl dsys_logctl
        .globl dsys_gettime
        .globl dsys_writechar

dsys_exit:             uuo 040,0(1)
                       popj 17,
dsys_writechar:        uuo 062,0(1)
                       popj 17,
dsys_run:              uuo 074,0(1)
                       popj 17,
dsys_wait:             uuo 075,0(1)
                       popj 17,
dsys_getpid:           uuo 076,0
                       popj 17,
dsys_gettime:          movei 1,023
                       uuo 077,0(1)
                       popj 17,
dsys_logctl:           move 4,3
                       move 3,2
                       move 2,1
                       movei 1,044
                       uuo 077,0(1)
                       popj 17,
dsys_ttyctl:           move 4,3
                       move 3,2
                       move 2,1
                       movei 1,050
                       uuo 077,0(1)
                       popj 17,
