; syscall_info_pdp10.s -- compact fixed-layout PROCINFO/MEMINFO syscalls.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1
        .globl  file_table
        .globl  proc_table
        .globl  proc_comm_words
        .globl  sys_memfs_usage_call
        .globl  mm_core_words

; int sys_procinfo(unsigned int slot, struct sys_procinfo *info)
; Fixed DAIMOS 1.x process slots: 0=SWAPPER, 1=INIT.
        .globl  sys_procinfo
sys_procinfo:
        cail    1,2
        jrst    pdp10_ret_neg1
        movem   1,(2)                  ; pid == fixed slot
        setzm   1(2)                   ; ppid
        movei   3,2                    ; RUN
        movem   3,2(2)
        movei   3,0
        jumpe   1,sys_procinfo_words
        hlrz    3,proc_table+3      ; INIT memory words
sys_procinfo_words:
        movem   3,3(2)
        move    3,proc_comm_words(1)
        movem   3,4(2)
        jrst    pdp10_ret_zero

; int sys_meminfo(struct sys_meminfo *info)
        .globl  sys_meminfo
sys_meminfo:
        move    2,1                    ; validated info pointer
        movei   1,0                    ; count active FILE slots in place
        movei   3,file_table+2
        movei   4,040
sys_meminfo_file_loop:
        move    5,(3)
        trne    5,1
        addi    1,1
        addi    3,3
        sojg    4,sys_meminfo_file_loop
        movem   1,7(2)
        move    3,mm_core_words         ; detected physical core
        movem   3,(2)
        .globl  sys_resident_words_immediate
sys_resident_words_immediate:
        movei   3,0
        movem   3,1(2)
; Reuse info[2..8] as the seven-word filesystem request.  These fields are
; filled with their final values after the optional MEMFS call returns.
        setzm   2(2)
        setzm   3(2)
        setzm   4(2)
        setzm   5(2)
        setzm   6(2)
        setzm   7(2)
        setzm   010(2)
        movei   1,2(2)
sys_memfs_usage_call:
        pushj   17,pdp10_ret_neg1
        jumpn   1,sys_meminfo_no_ramfs
        move    4,3(2)
        move    5,4(2)
        jrst    sys_meminfo_have_ramfs
sys_meminfo_no_ramfs:
        movei   4,0
        movei   5,0
sys_meminfo_have_ramfs:
        hlrz    3,proc_table+3
        movem   3,2(2)
        movem   4,3(2)
        movem   5,4(2)
        movei   3,2
        movem   3,5(2)
        movem   3,6(2)
        movei   3,040
        movem   3,010(2)
        jrst    pdp10_ret_zero
