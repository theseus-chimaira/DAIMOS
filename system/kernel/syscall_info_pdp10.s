; syscall_info_pdp10.s -- compact fixed-layout PROCINFO/MEMINFO syscalls.
        .text
        .globl  pdp10_ret_neg1_v1
        .globl  file_table
        .globl  proc_v1_table
        .globl  proc_v1_comm_words
        .globl  kcore_resident_end_v1
        .globl  fs_memfs_service_addr
        .globl  fs_mres_call

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
        movei   3,file_table+2
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
; Reuse info[2..8] as the seven-word filesystem request.  These fields are
; filled with their final values after the optional MEMFS call returns.
        movei   3,037                  ; FS_MRES_OP_MEMFS_USAGE
        movem   3,2(2)
        setzm   3(2)
        setzm   4(2)
        setzm   5(2)
        setzm   6(2)
        setzm   7(2)
        setzm   010(2)
        move    1,fs_memfs_service_addr
        movei   2,2(2)
        pushj   17,fs_mres_call
        subi    2,2
        jumpn   1,sys_meminfo_no_ramfs
        move    4,3(2)
        move    5,4(2)
        jrst    sys_meminfo_have_ramfs
sys_meminfo_no_ramfs:
        movei   4,0
        movei   5,0
sys_meminfo_have_ramfs:
        hlrz    3,proc_v1_table+3
        movem   3,2(2)
        movem   4,3(2)
        movem   5,4(2)
        movei   3,2
        movem   3,5(2)
        movem   3,6(2)
        movei   3,040
        movem   3,010(2)
        movei   1,0
        popj    17,
