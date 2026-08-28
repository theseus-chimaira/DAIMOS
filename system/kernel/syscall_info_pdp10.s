; syscall_info_pdp10.s -- compact fixed-layout PROCINFO/MEMINFO syscalls.
        .text
        .globl  pdp10_ret_neg1_v1
        .globl  file_v1_root
        .globl  file_v1_table
        .globl  proc_v1_table
        .globl  proc_v1_comm_words
        .globl  kcore_resident_end_v1

; int sys_v1_procinfo(unsigned int slot, struct sys_v1_procinfo *info)
; Fixed DAIMOS 1.x process slots: 0=SWAPPER, 1=INIT.
        .globl  sys_v1_procinfo
sys_v1_procinfo:
        cail    1,2
        jrst    sys_procinfo_fail
        movem   1,(2)                  ; pid == fixed slot
        setzm   1(2)                   ; ppid
        movei   3,2                    ; RUN
        movem   3,2(2)
        movei   3,0
        jumpe   1,sys_procinfo_words
        hlrz    3,proc_v1_table+3      ; INIT memory words
sys_procinfo_words:
        movem   3,3(2)
        move    3,proc_v1_comm_words(1)
        movem   3,4(2)
        movei   1,0
        popj    17,
sys_procinfo_fail:
        jrst    pdp10_ret_neg1_v1

; int sys_v1_meminfo(struct sys_v1_meminfo *info)
        .globl  sys_v1_meminfo
sys_v1_meminfo:
        move    2,1                    ; validated info pointer
        movei   1,0                    ; count active FILE slots in place
        movei   3,file_v1_table+2
        movei   4,040
sys_meminfo_file_loop:
        move    5,(3)
        trne    5,1
        addi    1,1
        addi    3,3
        sojg    4,sys_meminfo_file_loop
        movem   1,7(2)
        movsi   3,1                    ; 262144 words
        movem   3,(2)
        move    3,kcore_resident_end_v1
        movem   3,1(2)
        hlrz    3,proc_v1_table+3
        movem   3,2(2)
        move    4,file_v1_root
        move    3,4(4)
        movem   3,3(2)
        move    3,3(4)
        movem   3,4(2)
        movei   3,2
        movem   3,5(2)
        movem   3,6(2)
        movei   3,040
        movem   3,010(2)
        movei   1,0
        popj    17,
