/**
 * @file syscall_info.s
 * @brief Compact generic PROCINFO and MEMINFO query implementations.
 *
 * These routines inspect packed process/file/MM state without depending on
 * PDP-6 trap mechanics, so the source intentionally has no machine suffix.
 * MEMINFO reuses its destination fields as a temporary filesystem request to
 * avoid a resident scratch buffer.
 */
        .text
        .globl  kret_zero
        .globl  kret_neg1
        .globl  file_table
        .globl  proc_table
        .globl  proc_slots
        .globl  proc_high_slot
        .globl  proc_comm_words
        .globl  sys_memfs_usage_call
        .globl  mm_core_words

        .equ    PROC_WORDS,3
        .equ    PROC_UAREA_WORDS,0430
        .equ    PROC_STATE_LH_MASK,0700000
        .equ    PROC_UAREA_RH,0400000

/**
 * @brief Fill one sys_procinfo record for a live process-table slot.
 * @param AC1 Slot/PID.
 * @param AC2 Validated result pointer.
 * @return Zero or -1 for an invalid/FREE slot.
 */
        .globl  sys_procinfo
sys_procinfo:
        caml    1,proc_slots
        jrst    kret_neg1
        move    4,1                    ; preserve slot
        move    5,1
        lsh     5,1
        add     5,4                    ; 3 * slot
        add     5,proc_table           ; AC5 -> struct proc
        hlrz    3,2(5)
        andi    3,PROC_STATE_LH_MASK
        jumpe   3,kret_neg1       ; FREE slots are not processes
        movem   4,(2)                  ; pid is the process-table slot
        move    3,(5)
        move    6,3
        lsh     6,-010
        andi    6,0377
        movem   6,1(2)                 ; parent slot
        hlrz    6,2(5)
        lsh     6,-017
        andi    6,07
        movem   6,2(2)                 ; logical scheduler state
        hlrz    6,1(5)
        movem   6,3(2)                 ; logical user words
        move    3,proc_comm_words+2    ; default USER
        jumpe   4,sys_procinfo_swapper
        caie    4,1
        jrst    sys_procinfo_comm
        move    3,proc_comm_words+1
        jrst    sys_procinfo_comm
sys_procinfo_swapper:
        move    3,proc_comm_words
sys_procinfo_comm:
        movem   3,4(2)
        jrst    kret_zero

/**
 * @brief Fill runtime memory, MEMFS, process-slot, and file-slot counters.
 * @param AC1 Validated sys_meminfo pointer.
 * @return Zero; unavailable optional MEMFS statistics are reported as zero.
 */
        .globl  sys_meminfo
sys_meminfo:
        move    2,1                    ; validated info pointer
        movei   1,0                    ; count active FILE slots in place
        move    3,file_table
        movei   4,015
sys_meminfo_file_loop:
        skipn   (3)
        jrst    sys_meminfo_file_next
        addi    1,1
sys_meminfo_file_next:
        addi    3,2
        sojg    4,sys_meminfo_file_loop
        movem   1,7(2)
        move    3,mm_core_words
        movem   3,(2)
        .globl  sys_resident_words_immediate
sys_resident_words_immediate:
        movei   3,0
        movem   3,1(2)
; Reuse info[2..8] as the seven-word filesystem request.  These fields are
; filled with their final values after the optional MEMFS call returns.
        setzm   2(2)
        movei   1,3(2)
        hrli    1,2(2)
        blt     1,010(2)
        movei   1,2(2)
        push    17,2                    ; MRES calls may clobber AC2
sys_memfs_usage_call:
        pushj   17,kret_neg1
        pop     17,2                    ; restore struct sys_meminfo pointer
        jumpn   1,sys_meminfo_no_memfs
        move    4,3(2)
        move    5,4(2)
        jrst    sys_meminfo_have_memfs
sys_meminfo_no_memfs:
        movei   4,0
        movei   5,0
sys_meminfo_have_memfs:
        ; Commit MEMFS results before reusing AC4 as an index register.
        movem   4,3(2)
        movem   5,4(2)
        ; Count occupied slots and resident process+u-area words.
        movei   1,0                    ; resident process words
        movei   3,0                    ; occupied logical slots
        movei   6,0                    ; slot index
        move    4,proc_table           ; process pointer; AC4 is caller-scratch
sys_meminfo_proc_loop:
        caml    6,proc_high_slot
        jrst    sys_meminfo_proc_done
        hlrz    7,2(4)
        andi    7,PROC_STATE_LH_MASK
        jumpe   7,sys_meminfo_proc_next
        addi    3,1
        move    7,(4)
        trne    7,PROC_UAREA_RH
        addi    1,PROC_UAREA_WORDS      ; stable u-area remains while swapped
        hrrz    7,1(4)
        jumpe   7,sys_meminfo_proc_next
        hlrz    7,1(4)
        add     1,7
sys_meminfo_proc_next:
        addi    4,PROC_WORDS
        aoja    6,sys_meminfo_proc_loop
sys_meminfo_proc_done:
        movem   1,2(2)
        movem   3,5(2)
        move    3,proc_slots
        movem   3,6(2)
        movei   3,015
        movem   3,010(2)
        jrst    kret_zero
