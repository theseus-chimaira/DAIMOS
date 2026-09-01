; proc_pdp10.s -- compact process runtime state and wait channel.
;
; V1 has one schedulable user context (INIT).  Process state belongs to the
; unconditional kernel nucleus: RAMFS is an optional filesystem MRES and cannot
; own scheduler state.  A later multiprocess scheduler can widen this state
; without changing driver callers of the event-word API.
        .equ    PROC_V1_RUN_SLEEP_BIT,0100000
        .globl  proc_v1_table
        .globl  proc_v1_wait_channel
        .globl  proc_v1_wait_event
        .globl  proc_v1_wakeup_event
        .globl  pdp10_ret_zero_v1

; int proc_v1_wait_event(volatile kword_t *eventp)
; Kernel-only precondition: eventp is valid and INIT is the current context.
; Zero means pending.  Publish the channel, change SRUN -> SLEEP, then recheck
; the event to close the interrupt-before-sleep race.
proc_v1_wait_event:
        skipe   (1)
        jrst    pdp10_ret_zero_v1
        movem   1,proc_v1_wait_channel
        movei   2,PROC_V1_RUN_SLEEP_BIT
        xorb    2,proc_v1_table+2
        skipe   (1)
        jrst    proc_v1_wait_raced
proc_v1_wait_loop:
        skipe   proc_v1_wait_channel
        jrst    proc_v1_wait_loop
        jrst    pdp10_ret_zero_v1
proc_v1_wait_raced:
        setzm   proc_v1_wait_channel
        xori    2,PROC_V1_RUN_SLEEP_BIT
        movem   2,proc_v1_table+2
        jrst    pdp10_ret_zero_v1

; void proc_v1_wakeup_event(volatile kword_t *eventp)
; PI-safe: clobbers only AC1.  The producer stores a nonzero event value before
; calling this routine.
proc_v1_wakeup_event:
        came    1,proc_v1_wait_channel
        popj    17,
        setzm   proc_v1_wait_channel
        movei   1,PROC_V1_RUN_SLEEP_BIT
        xorb    1,proc_v1_table+2
        popj    17,

        .bss
proc_v1_table:
        .block  4                       ; two two-word struct proc_v1 entries
proc_v1_wait_channel:
        .block  1

        .data
        .globl  proc_v1_comm_words
proc_v1_comm_words:
        .word   0636741606045          ; SIXBIT /SWAPPE/
        .word   0515651640000          ; SIXBIT /INIT  /
