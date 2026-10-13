; LOGIN requires only these DAIMOS monitor UUO veneers.  Keep the
; calling convention identical to libc/syscall.s (KCC AC1..AC4).
        .text
        .globl dsys_exit
        .globl dsys_open
        .globl dsys_close
        .globl dsys_chdir
        .globl dsys_stat
        .globl dsys_read_words
        .globl dsys_readchar
        .globl dsys_writechar
        .globl dsys_procctl
        .globl dsys_exec
        .globl dsys_gettime
        .globl dsys_logctl
dsys_exit:             uuo 040,0(1)
                       popj 17,
dsys_open:             uuo 041,0(1)
                       popj 17,
dsys_close:            uuo 042,0(1)
                       popj 17,
dsys_chdir:            uuo 045,0(1)
                       popj 17,
dsys_stat:             uuo 047,0(1)
                       popj 17,
dsys_read_words:       uuo 055,0(1)
                       popj 17,
dsys_readchar:         uuo 061,0(1)
                       popj 17,
dsys_writechar:        uuo 062,0(1)
                       popj 17,
dsys_procctl:          uuo 077,0(1)
                       popj 17,
dsys_exec:             move 2,1
                       movei 1,022
                       uuo 077,0(1)
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
