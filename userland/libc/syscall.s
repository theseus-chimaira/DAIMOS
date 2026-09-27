; syscall.s -- shared DAIMOS PDP-6 monitor-UUO veneers.
;
; C arguments arrive in AC1..AC4.  The UUO effective address carries arg0,
; leaving AC2..AC4 as the remaining real arguments.  The kernel returns the
; result in AC1.  UUO 043 is the bulk character-stream write path;
; opcodes 074..077 remain reserved for future ABI extension.

        .text

        .globl dsys_exit
        .globl dsys_open
        .globl dsys_close
        .globl dsys_chdir
        .globl dsys_getcwd
        .globl dsys_stat
        .globl dsys_dirread
        .globl dsys_unlink
        .globl dsys_rename
        .globl dsys_truncate
        .globl dsys_read_words
        .globl dsys_write_words
        .globl dsys_procinfo
        .globl dsys_meminfo
        .globl dsys_readchar
        .globl dsys_writechar
        .globl dsys_write_chars
        .globl dsys_halt
        .globl dsys_chmod
        .globl dsys_dtfs_format
        .globl dsys_dtfs_mount
        .globl dsys_unmount
        .globl dsys_flock
        .globl dsys_dup
        .globl dsys_dup2
        .globl dsys_seek
        .globl dsys_symlink
        .globl dsys_nice
        .globl dsys_run
        .globl dsys_wait
        .globl dsys_getpid
        .globl dsys_procctl
        .globl dsys_pipe
        .globl dsys_exec
        .globl dsys_gettime
        .globl dsys_rtctl
        .globl dsys_logctl
        .globl dsys_utime
        .globl dsys_rmdir
        .globl dsys_chown
        .globl dsys_dtc_read_block
        .globl dsys_tsfs_mount
        .globl dsys_d6fs_mount

dsys_exit:             uuo 040,0(1)
                       popj 17,
dsys_open:             uuo 041,0(1)
                       popj 17,
dsys_close:            uuo 042,0(1)
                       popj 17,
dsys_write_chars:      uuo 043,0(1)
                       popj 17,
dsys_chdir:            uuo 045,0(1)
                       popj 17,
dsys_getcwd:           uuo 046,0(1)
                       popj 17,
dsys_stat:             uuo 047,0(1)
                       popj 17,
dsys_dirread:          uuo 050,0(1)
                       popj 17,
dsys_unlink:           uuo 052,0(1)
                       popj 17,
dsys_rename:           uuo 053,0(1)
                       popj 17,
dsys_truncate:         uuo 054,0(1)
                       popj 17,
dsys_read_words:       uuo 055,0(1)
                       popj 17,
dsys_write_words:      uuo 056,0(1)
                       popj 17,
dsys_procinfo:         uuo 057,0(1)
                       popj 17,
dsys_meminfo:          uuo 060,0(1)
                       popj 17,
dsys_readchar:         uuo 061,0(1)
                       popj 17,
dsys_writechar:        uuo 062,0(1)
                       popj 17,
dsys_halt:             uuo 063,0
                       popj 17,
dsys_chmod:            uuo 064,0(1)
                       popj 17,
dsys_dtfs_format:      uuo 065,0(1)
                       popj 17,
dsys_dtfs_mount:       uuo 066,0(1)
                       popj 17,
dsys_unmount:          uuo 067,0(1)
                       popj 17,
dsys_flock:            uuo 070,0(1)
                       popj 17,
dsys_dup:              uuo 071,0(1)
                       popj 17,
dsys_symlink:          uuo 072,0(1)
                       popj 17,
dsys_nice:             uuo 073,0(1)
                       popj 17,
dsys_run:              uuo 074,0(1)
                       popj 17,
dsys_wait:             uuo 075,0(1)
                       popj 17,
dsys_getpid:           uuo 076,0
                       popj 17,
dsys_procctl:          uuo 077,0(1)
                       popj 17,
dsys_pipe:             movei 1,020
                       uuo 077,0(1)
                       popj 17,
dsys_exec:             move 2,1
                       movei 1,022
                       uuo 077,0(1)
                       popj 17,
dsys_gettime:          movei 1,023
                       uuo 077,0(1)
                       popj 17,
dsys_rtctl:            move 2,1
                       movei 1,043
                       uuo 077,0(1)
                       popj 17,
dsys_logctl:          move 4,3
                       move 3,2
                       move 2,1
                       movei 1,044
                       uuo 077,0(1)
                       popj 17,
dsys_dup2:             move 3,2
                       move 2,1
                       movei 1,024
                       uuo 077,0(1)
                       popj 17,
dsys_seek:             move 4,3
                       move 3,2
                       move 2,1
                       movei 1,032
                       uuo 077,0(1)
                       popj 17,
dsys_chown:            move 4,3
                       move 3,2
                       move 2,1
                       movei 1,033
                       uuo 077,0(1)
                       popj 17,
dsys_rmdir:            move 2,1
                       movei 1,034
                       uuo 077,0(1)
                       popj 17,
dsys_utime:            move 3,2
                       move 2,1
                       movei 1,035
                       uuo 077,0(1)
                       popj 17,
        .globl dsys_mkfifo
dsys_mkfifo:           move 3,2
                       move 2,1
                       movei 1,021
                       uuo 077,0(1)
                       popj 17,

; Read one 128-word DECtape block into a userspace buffer.
; AC1=unit, AC2=block, AC3=buffer; UUO 077 extension uses AC1 as selector.
dsys_dtc_read_block:    move 4,3
                        move 3,2
                        move 2,1
                        movei 1,040
                        uuo 077,0(1)
                        popj 17,

; Mount userspace-validated TSFS runtime state (three words).
; C: AC1=state, AC2=target, AC3=flags.
dsys_tsfs_mount:        move 4,3
                        move 3,2
                        move 2,1
                        movei 1,041
                        uuo 077,0(1)
                        popj 17,

; Mount a userspace-validated D6FS handoff.
; C: AC1=handoff, AC2=target, AC3=flags.
dsys_d6fs_mount:        move 4,3
                        move 3,2
                        move 2,1
                        movei 1,042
                        uuo 077,0(1)
                        popj 17,
