; syscall_dispatch_v1.s -- compact native PDP-6 syscall dispatcher.
;
; The C switch for this path grows a sparse 36-word jump table and emits
; repeated register shuffling around the PDP-10 ABI.  Keep syscall semantics
; in C, but dispatch and user-word validation here where the machine layout is
; explicit and compact.
;
; Called only by mach_user_v1.s after the user AC block has been saved.
; Return value is written directly to the saved user AC1 slot.

        .text
        .globl mach_dispatch_syscall_v1
        .globl mach_syscall_ac1_v1
        .globl mach_syscall_ac2_v1
        .globl mach_syscall_ac3_v1
        .globl mach_syscall_ac4_v1
        .globl mach_syscall_ac5_v1
        .globl proc_v1_current
        .globl proc_v1_table
        .globl proc_v1_set_state
        .globl mach_return_to_kernel_request_v1
        .globl file_v1_open
        .globl file_v1_close
        .globl sys_v1_putchar
        .globl sys_v1_getchar
        .globl file_v1_chdir
        .globl file_v1_getcwd
        .globl file_v1_read_words
        .globl file_v1_write_words
        .globl file_v1_stat_path_owner
        .globl file_v1_readdir
        .globl file_v1_mkdir_owner
        .globl file_v1_unlink_owner
        .globl file_v1_rename
        .globl file_v1_truncate_owner
        .globl sys_v1_procinfo
        .globl sys_v1_meminfo
        .globl sys_v1_readchar
        .globl sys_v1_writechar
        .globl pdp10_halt

mach_dispatch_syscall_v1:
        add 17,[1,,1]
        skipn 12,proc_v1_current
        jrst sd_bad

        ; owner = (proc_v1_current - proc_v1_table) / 2.
        subi 12,proc_v1_table
        ash 12,-1

        ; Syscalls 2..10 are dense except for obsolete number 4.
        hrrz 4,mach_syscall_ac1_v1
        move 5,4
        subi 5,2
        jumpl 5,sd_mid
        caile 5,010
        jrst sd_mid
        jrst @sd_low(5)

sd_low:
        .word sd_exit
        .word sd_open
        .word sd_bad
        .word sd_close
        .word sd_putchar
        .word sd_getchar
        .word sd_chdir
        .word sd_getcwd
        .word sd_stat

sd_mid:
        cain 4,016
        jrst sd_dirread
        cain 4,022
        jrst sd_mkdir
        cain 4,024
        jrst sd_unlink
        cain 4,025
        jrst sd_rename
        cain 4,026
        jrst sd_truncate

        ; Syscalls 31..37 form one dense group.
        subi 4,037
        jumpl 4,sd_bad
        caile 4,6
        jrst sd_bad
        jrst @sd_high(4)

sd_high:
        .word sd_read_words
        .word sd_write_words
        .word sd_procinfo
        .word sd_meminfo
        .word sd_readchar
        .word sd_writechar
        .word sd_halt

sd_exit:
        move 1,proc_v1_current
        movei 2,4
        pushj 17,proc_v1_set_state
        pushj 17,mach_return_to_kernel_request_v1
        hrrz 1,mach_syscall_ac2_v1
        jrst sd_return

sd_open:
        move 1,mach_syscall_ac2_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 2,1
        hrrz 3,mach_syscall_ac3_v1
        move 4,3
        andi 3,034
        andi 4,3
        jumpe 4,sd_open_read
        caie 4,1
        movei 4,3
        cain 4,1
        movei 4,2
        jrst sd_open_flags
sd_open_read:
        movei 4,1
sd_open_flags:
        ior 3,4
        move 1,12
        pushj 17,file_v1_open
        jrst sd_return

sd_close:
        move 1,12
        hrrz 2,mach_syscall_ac2_v1
        pushj 17,file_v1_close
        jrst sd_return

sd_putchar:
        move 1,mach_syscall_ac2_v1
        andi 1,0177
        pushj 17,sys_v1_putchar
        jrst sd_return

sd_getchar:
        pushj 17,sys_v1_getchar
        jrst sd_return

sd_chdir:
        move 1,mach_syscall_ac2_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 2,1
        move 1,12
        pushj 17,file_v1_chdir
        jrst sd_return

sd_getcwd:
        move 1,mach_syscall_ac2_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 2,1
        move 1,12
        hrrz 3,mach_syscall_ac3_v1
        pushj 17,file_v1_getcwd
        jrst sd_return

sd_stat:
        move 1,mach_syscall_ac2_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 10,1
        move 1,mach_syscall_ac3_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 3,1
        move 1,12
        move 2,10
        pushj 17,file_v1_stat_path_owner
        jrst sd_return

sd_dirread:
        move 1,mach_syscall_ac3_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 3,1
        move 1,12
        hrrz 2,mach_syscall_ac2_v1
        pushj 17,file_v1_readdir
        jrst sd_return

sd_mkdir:
        move 1,mach_syscall_ac2_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 2,1
        move 1,12
        hrrz 3,mach_syscall_ac3_v1
        pushj 17,file_v1_mkdir_owner
        jrst sd_return

sd_unlink:
        move 1,mach_syscall_ac2_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 2,1
        move 1,12
        pushj 17,file_v1_unlink_owner
        jrst sd_return

sd_rename:
        move 1,mach_syscall_ac2_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 10,1
        move 1,mach_syscall_ac3_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 3,1
        move 1,12
        move 2,10
        pushj 17,file_v1_rename
        jrst sd_return

sd_truncate:
        move 1,mach_syscall_ac2_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 2,1
        move 1,12
        move 3,mach_syscall_ac3_v1
        pushj 17,file_v1_truncate_owner
        jrst sd_return

sd_read_words:
        move 1,mach_syscall_ac3_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 3,1
        move 1,12
        hrrz 2,mach_syscall_ac2_v1
        hrrz 4,mach_syscall_ac4_v1
        pushj 17,file_v1_read_words
        jrst sd_return

sd_write_words:
        move 1,mach_syscall_ac3_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 3,1
        move 1,12
        hrrz 2,mach_syscall_ac2_v1
        hrrz 4,mach_syscall_ac4_v1
        move 5,mach_syscall_ac5_v1
        movem 5,(17)
        pushj 17,file_v1_write_words
        jrst sd_return

sd_procinfo:
        move 1,mach_syscall_ac3_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        move 2,1
        hrrz 1,mach_syscall_ac2_v1
        pushj 17,sys_v1_procinfo
        jrst sd_return

sd_meminfo:
        move 1,mach_syscall_ac2_v1
        pushj 17,sd_user_word
        jumpe 1,sd_bad
        pushj 17,sys_v1_meminfo
        jrst sd_return

sd_readchar:
        move 1,12
        hrrz 2,mach_syscall_ac2_v1
        pushj 17,sys_v1_readchar
        jrst sd_return

sd_writechar:
        move 1,12
        hrrz 2,mach_syscall_ac2_v1
        move 3,mach_syscall_ac3_v1
        andi 3,0777
        pushj 17,sys_v1_writechar
        jrst sd_return

sd_halt:
        pushj 17,pdp10_halt
        seto 1,
        jrst sd_return

sd_bad:
        seto 1,

sd_return:
        movem 1,mach_syscall_ac1_v1
        sub 17,[1,,1]
        popj 17,

; Validate a user word address against the current process memory interval.
; Input/output AC1.  All user word addresses are 18-bit positive quantities,
; so ordinary PDP-10 comparisons are sufficient here.
sd_user_word:
        hrrz 1,1
        move 2,proc_v1_current
        hrrz 3,1(2)
        camge 1,3
        jrst sd_user_bad
        hlrz 4,1(2)
        add 4,3
        caml 1,4
        jrst sd_user_bad
        popj 17,
sd_user_bad:
        setz 1,
        popj 17,
