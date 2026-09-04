; proc_pdp10.s -- compact process runtime state and wait channel.
;
; V1 has one schedulable user context (INIT).  Process state belongs to the
; unconditional kernel nucleus: RAMFS is an optional filesystem MRES and cannot
; own scheduler state.  A later multiprocess scheduler can widen this state
; without changing driver callers of the event-word API.
        .equ    PROC_RUN_SLEEP_BIT,0100000
        .globl  proc_table
        .globl  proc_wait_channel
        .globl  proc_wait_event
        .globl  proc_wakeup_event
        .globl  pdp10_ret_zero

; int proc_wait_event(volatile kword_t *eventp)
; Kernel-only precondition: eventp is valid and INIT is the current context.
; Zero means pending.  Publish the channel, change SRUN -> SLEEP, then recheck
; the event to close the interrupt-before-sleep race.
proc_wait_event:
        skipe   (1)
        jrst    pdp10_ret_zero
        movem   1,proc_wait_channel
        movei   2,PROC_RUN_SLEEP_BIT
        xorb    2,proc_table+2
        skipe   (1)
        jrst    proc_wait_raced
proc_wait_loop:
        skipe   proc_wait_channel
        jrst    proc_wait_loop
        jrst    pdp10_ret_zero
proc_wait_raced:
        setzm   proc_wait_channel
        xori    2,PROC_RUN_SLEEP_BIT
        movem   2,proc_table+2
        jrst    pdp10_ret_zero

; void proc_wakeup_event(volatile kword_t *eventp)
; PI-safe: clobbers only AC1.  The producer stores a nonzero event value before
; calling this routine.
proc_wakeup_event:
        came    1,proc_wait_channel
        popj    17,
        setzm   proc_wait_channel
        movei   1,PROC_RUN_SLEEP_BIT
        xorb    1,proc_table+2
        popj    17,

        .bss
proc_table:
        .block  4                       ; two two-word struct proc entries

        .data
        .globl  proc_comm_words
proc_comm_words:
        .word   0636741606045          ; SIXBIT /SWAPPE/
        .word   0515651640000          ; SIXBIT /INIT  /
