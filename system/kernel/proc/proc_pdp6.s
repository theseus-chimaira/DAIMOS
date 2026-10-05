/**
 * @file proc_pdp6.s
 * @brief PDP-6 process control, event/TTY hot paths, and context switching.
 *
 * proc_table is allocated after memory discovery. Each active process owns a
 * stable executive u-area allocated from kernel-dynamic core. Saved CPU/syscall
 * state ends at 044, followed by cwd/file state and the private kernel pushdown
 * list. User VM extents may move or swap independently; the u-area stays
 * resident so sleeping kernel continuations retain valid stack addresses.
 *
 * This is specifically the PDP-6 baseline implementation: PI level 6, low-core
 * PI vectors, software PI requests, user/executive return state, and context
 * frame layout are machine-specific. Later PDP-10 implementations may replace
 * these paths while preserving the proc.h process/u-area ABI.
 */

        .equ    PROC_WORDS,3
        .equ    PROC_STATE_LH_MASK,0700000
        .equ    PROC_STATE_RUN,0200000
        .equ    PROC_STATE_SLEEP,0300000
        .equ    PROC_STATE_STOP,0600000
        .equ    PROC_NICE_LH_MASK,077
        .equ    PROC_CPU_LH_MASK,017
        .equ    PROC_SLEEP_LH_MASK,07
        .equ    PROC_WAIT_LH_FIELD_MASK,03
        .equ    PROC_NICE_BIAS,024
        .equ    PROC_USER_MAP_BIT,2
        .equ    PROC_WAIT_LH_MASK,060000
        .equ    PROC_WAIT_EVENT_LH,020000
        .equ    PROC_WAIT_CHILD_LH,040000
        .equ    PROC_WAIT_INTR_LH,060000
        .equ    PROC_CPU_SLEEP_LH_MASK,017700
        .equ    PROC_SCHED_QUANTUM_TICKS,4
        .equ    PROC_TRANSITION_RH,0200000
        .equ    PROC_TIMER_ACTIVE_LH,0400000
        .equ    PROC_TIMER_DUE_LH,0200000
        .equ    PROC_TIMER_TAG_RH,0400000
        .equ    PROC_TIMER_CLOCK_MASK,0377777
        .equ    PROC_FDCTL_OFFSET,024
        .equ    PROC_FILE_TABLE_OFFSET,026
        .equ    PROC_CRED_OFFSET,066
        .equ    PROC_UMASK_OFFSET,067
        .equ    PROC_USTACK_BASE,070
        .equ    PROC_KSTACK_WORDS,0316
        .equ    KERNEL_IDLE_STACK_WORDS,0100

        .equ    CTX_PC,020
        .equ    CTX_KSP,021
        .equ    CTX_M_USER_SP,022
        .equ    CTX_M_SYSCALL_SAVE,023

        .text
        .globl  proc_table
        .globl  proc_slots
        .globl  proc_high_slot
        .globl  proc_current_slot
        .globl  proc_current_ptr
        .globl  proc_sched_cursor
        .globl  proc_sched_deferred_ticks
        .globl  proc_runq_head
        .globl  mach_kernel_stack_base
        .globl  proc_wait_event
        .globl  proc_wait_event_intr
        .globl  proc_sleep_ticks
        .globl  proc_wait_child
        .globl  proc_wait_status
        .globl  proc_wakeup_event
        .globl  proc_sched_pi_tick
        .globl  proc_sched_tick_select
        .globl  proc_sched_resched_select
        .globl  proc_sched_pi_resched
        .globl  proc_sched_kick
        .globl  proc_rt_owner
        .globl  proc_timer_clock
        .globl  proc_timer_next
        .globl  proc_sched_resched_current
        .globl  proc_swap_service_one
        .globl  proc_swap_records
        .globl  mm_is_pinned
        .globl  proc_swap_victim
        .globl  proc_record_kernel_sp
        .globl  proc_exit_current
        .globl  proc_finish_slot
        .globl  kret_zero
        .globl  kret_one
        .globl  kret_neg1
        .globl  pdp10_pi_level6
        .globl  pdp10_pi_sp_save
        .globl  mach_pi_disable
        .globl  mach_pi_restore
        .globl  proc_exit_finish
        .globl  mach_kernel_sp
        .globl  mach_user_sp
        .globl  mach_syscall_save
        .globl  file_table
        .globl  file_find
        .globl  vm_activate_current
        .globl  proc_slot_ptr
        .globl  proc_runq_add
        .globl  proc_runq_remove
        .globl  proc_trim_high
        .globl  proc_uarea_release
        .globl  proc_tty_release_session
        .globl  proc_tty_line_reset
        .globl  proc_tty_session_has
        .globl  proc_session_teardown
        .globl  proc_notify_parent
        .globl  vm_space_destroy
        .globl  mm_free

/**
 * @brief Select a runnable process after an explicit reschedule.
 *
 * The C spelling needs a thirteen-word local frame because KCC spills the
 * queue-selection state.  PDP-6 has enough callee-saved ACs to keep the whole
 * selection tuple live in registers.  AC10..AC16 are saved/restored with one
 * BLT, reducing both resident text and scheduler memory traffic.
 */
proc_sched_resched_select:
        move    1,proc_sched_deferred_ticks
        setzm   proc_sched_deferred_ticks
        jrst    proc_select_runnable_pdp6

/** Select after a clock quantum, charging at least one elapsed tick. */
proc_sched_tick_select:
        move    1,proc_sched_deferred_ticks
        setzm   proc_sched_deferred_ticks
        jumpn   1,proc_select_runnable_pdp6
        movei   1,1

; AC10 elapsed ticks
; AC11 best slot, AC12 best priority, AC13 best cyclic rank
; AC14 previous scheduler cursor, AC15 current slot, AC16 current descriptor
proc_select_runnable_pdp6:
        add     17,kconst_7_7
        movei   0,-6(17)
        hrli    0,10
        blt     0,(17)
        move    10,1
        skipn   proc_table
        jrst    proc_select_none
        move    1,proc_high_slot
        caig    1,1
        jrst    proc_select_none
        setz    11,
        setz    12,
        setz    13,
        hrrz    14,proc_sched_cursor
        andi    14,0377

        ; Sleep age advances only on the original 64-tick boundary.
        jumpe   10,proc_select_rt
        move    1,proc_sched_age_phase
        add     1,10
        movem   1,proc_sched_age_phase
        caige   1,0100
        jrst    proc_select_rt
        subi    1,0100
        movem   1,proc_sched_age_phase
        movei   15,1
        move    16,proc_table
        addi    16,PROC_WORDS
proc_select_age_loop:
        caml    15,proc_high_slot
        jrst    proc_select_rt
        move    1,2(16)
        lsh     1,-041
        caie    1,PROC_STATE_SLEEP
        jrst    proc_select_age_next
        move    1,2(16)
        lsh     1,-034
        andi    1,PROC_SLEEP_LH_MASK
        cain    1,PROC_SLEEP_LH_MASK
        jrst    proc_select_age_next
proc_select_age_inc:
        movsi   2,02000
        addb    2,2(16)
proc_select_age_next:
        addi    15,1
        addi    16,PROC_WORDS
        jrst    proc_select_age_loop

        ; Real-time ownership wins immediately when its process is resident
        ; and runnable.  A swapped RT owner requests swap and returns slot 0.
proc_select_rt:
        skipn   15,proc_rt_owner
        jrst    proc_select_runq_start
        move    16,15
        lsh     16,1
        add     16,15
        add     16,proc_table
        move    1,2(16)
        lsh     1,-041
        caie    1,PROC_STATE_RUN
        jrst    proc_select_runq_start
        move    1,(16)
        trne    1,PROC_TRANSITION_RH
        jrst    proc_select_runq_start
        movem   15,proc_sched_cursor
        move    1,1(16)
        trne    1,0777777
        jrst    proc_select_return_slot
        movei   1,0400
        iorm    1,proc_sched_cursor
        jrst    proc_select_none

proc_select_runq_start:
        hrrz    15,proc_runq_head
proc_select_runq:
        jumpe   15,proc_select_done
        move    16,15
        lsh     16,1
        add     16,15
        add     16,proc_table

        ; Update recent CPU exactly once for each runnable-queue member.
        jumpe   10,proc_select_candidate
        move    1,2(16)
        lsh     1,-030
        andi    1,PROC_CPU_LH_MASK
        came    15,proc_current_slot
        jrst    proc_select_cpu_decay
        move    2,(16)
        trne    2,PROC_TRANSITION_RH
        jrst    proc_select_cpu_decay
        jumpn   1,proc_select_cpu_add
        movei   1,1
proc_select_cpu_add:
        add     1,10
        caile   1,PROC_CPU_LH_MASK
        movei   1,PROC_CPU_LH_MASK
        jrst    proc_select_cpu_store
proc_select_cpu_decay:
        camg    1,10
        jrst    proc_select_cpu_zero
        sub     1,10
        jrst    proc_select_cpu_store
proc_select_cpu_zero:
        setz    1,
proc_select_cpu_store:
        move    2,2(16)
        tlz     2,01700
        move    3,1
        lsh     3,030
        ior     2,3
        movem   2,2(16)

proc_select_candidate:
        move    1,(16)
        trne    1,PROC_TRANSITION_RH
        jrst    proc_select_next
        move    1,1(16)
        trne    1,0777777
        jrst    proc_select_score
        move    1,proc_swap_records
        add     1,15
        move    1,(1)
        jumpe   1,proc_select_next

proc_select_score:
        ; priority = biased nice + recent CPU
        hlrz    1,2(16)
        andi    1,PROC_NICE_LH_MASK
        move    2,2(16)
        lsh     2,-030
        andi    2,PROC_CPU_LH_MASK
        add     1,2

        ; Cyclic rank is modulo 256; rank zero sorts after all others.
        move    2,15
        sub     2,14
        andi    2,0377
        jumpn   2,proc_select_compare
        movei   2,0400
proc_select_compare:
        jumpe   11,proc_select_better
        camge   1,12
        jrst    proc_select_better
        came    1,12
        jrst    proc_select_next
        caml    2,13
        jrst    proc_select_next
proc_select_better:
        move    11,15
        move    12,1
        move    13,2
proc_select_next:
        hrrz    15,2(16)
        jrst    proc_select_runq

proc_select_done:
        jumpe   11,proc_select_none
        movem   11,proc_sched_cursor
        move    16,11
        lsh     16,1
        add     16,11
        add     16,proc_table
        move    1,1(16)
        trne    1,0777777
        jrst    proc_select_return_best
        movei   1,0400
        iorm    1,proc_sched_cursor
        jrst    proc_select_none

proc_select_return_best:
        move    1,11
        jrst    proc_select_restore
proc_select_return_slot:
        move    1,15
        jrst    proc_select_restore
proc_select_none:
        setz    1,
proc_select_restore:
        movei   0,10
        hrli    0,-6(17)
        blt     0,16
        sub     17,kconst_7_7
        popj    17,

/**
 * @brief Reparent children of an exiting process.
 *
 * INIT (slot 1) becomes the new parent when live.  Otherwise orphan zombies
 * are reaped immediately and other children become parentless.  The scan is
 * deliberately linear over the compact three-word process table.
 */
proc_adopt_children_pdp6:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        move    10,1                   ; old parent
        setz    11,                    ; new parent
        caie    10,1
        jrst    proc_adopt_check_init
        jrst    proc_adopt_scan_start
proc_adopt_check_init:
        move    1,proc_table
        move    1,5(1)                 ; slot 1 scheduler word
        and     1,[0300000000000]      ; FREE/ZOMB both clear low state bits
        jumpe   1,proc_adopt_scan_start
        movei   11,1
proc_adopt_scan_start:
        movei   12,1
        move    13,proc_table
        addi    13,PROC_WORDS
proc_adopt_scan:
        caml    12,proc_high_slot
        jrst    proc_adopt_done
        camn    12,10
        jrst    proc_adopt_next
        move    1,2(13)
        lsh     1,-041                 ; state
        jumpe   1,proc_adopt_next
        move    2,(13)
        lsh     2,-010
        andi    2,0377                 ; parent
        came    2,10
        jrst    proc_adopt_next
        jumpn   11,proc_adopt_reparent
        caie    1,4                    ; orphan zombie: reap now
        jrst    proc_adopt_reparent
        setzm   (13)
        setom   1(13)
        setzm   2(13)
        jrst    proc_adopt_next
proc_adopt_reparent:
        move    2,(13)
        and     2,[-0177401]
        move    3,11
        andi    3,0377
        lsh     3,010
        ior     2,3
        movem   2,(13)
        jumpe   11,proc_adopt_next
        caie    1,4
        jrst    proc_adopt_next
        move    1,11
        pushj   17,proc_notify_parent
proc_adopt_next:
        addi    13,PROC_WORDS
        aoja    12,proc_adopt_scan
proc_adopt_done:
        pop     17,13
        pop     17,12
        pop     17,11
        pop     17,10
        jrst    proc_trim_high

/**
 * @brief Release heavy process resources and publish FREE or ZOMB state.
 *
 * AC1 is the slot and AC2 the exit status.  Persistent teardown state stays in
 * AC10..AC16 across helper calls instead of KCC's seven-word local frame.
 */
proc_finish_slot:
        add     17,kconst_7_7
        movei   0,-6(17)
        hrli    0,10
        blt     0,(17)
        move    10,1                   ; slot
        move    11,2                   ; status
        move    12,1
        lsh     12,1
        add     12,10
        add     12,proc_table           ; descriptor
        move    1,proc_rt_owner
        came    1,10
        jrst    proc_finish_ids
        setzm   proc_rt_owner
proc_finish_ids:
        move    1,(12)
        lsh     1,-010
        andi    1,0377
        move    13,1                   ; parent
        hrrz    14,(12)
        andi    14,0377                ; pgrp
        hlrz    1,(12)
        move    15,PROC_FDCTL_OFFSET(1)
        move    16,15
        lsh     16,-3
        andi    16,0177777             ; zombie scope

        move    1,10
        move    2,15
        pushj   17,proc_session_teardown
        jumpn   1,proc_finish_fail

        movei   1,PROC_TRANSITION_RH
        iorb    1,(12)
        move    1,12
        move    2,10
        pushj   17,vm_space_destroy
        jumpe   1,proc_finish_vm_done
        movni   1,PROC_TRANSITION_RH+1
        andb    1,(12)                 ; clear transition after VM failure
        jrst    proc_finish_fail
proc_finish_vm_done:
        move    1,16
        andi    1,0377
        move    2,10
        pushj   17,proc_tty_release_session
        move    1,10
        move    2,12
        pushj   17,proc_uarea_release
        jumpn   1,proc_finish_fail
        move    1,10
        pushj   17,proc_runq_remove
        move    1,10
        pushj   17,proc_adopt_children_pdp6

        jumpn   13,proc_finish_zombie
        setzm   (12)
        setom   1(12)
        setzm   2(12)
        pushj   17,proc_trim_high
        setz    1,
        jrst    proc_finish_restore
proc_finish_zombie:
        move    1,14
        move    2,13
        lsh     2,010
        ior     1,2
        hrl     1,11
        movem   1,(12)
        move    1,16
        tlo     1,024                  ; PROC_SCHED_DEFAULT
        tlz     1,0700000
        tlo     1,0400000              ; PROC_ZOMB
        movem   1,2(12)
        move    1,13
        pushj   17,proc_notify_parent
        setz    1,
        jrst    proc_finish_restore
proc_finish_fail:
        seto    1,
proc_finish_restore:
        movei   0,10
        hrli    0,-6(17)
        blt     0,16
        sub     17,kconst_7_7
        popj    17,

/**
 * @brief Release one process's stable executive u-area.
 * @param AC1 Process slot.
 * @param AC2 Process descriptor.
 * @return Zero on success/no u-area, -1 on invalid base or MM failure.
 */
proc_uarea_release:
        hrrz    3,(2)
        trnn    3,0400000              ; PROC_F_UAREA
        jrst    kret_zero
        hlrz    4,(2)
        jumpe   4,kret_neg1
        push    17,10
        move    10,2
        addi    1,01000                ; PROC_UAREA_MM_OWNER_BASE
        move    3,1                    ; owner
        move    1,4                    ; base
        movei   2,3                    ; MM_TYPE_KERNEL_DYNAMIC
        pushj   17,mm_free
        jumpn   1,proc_uarea_release_fail
        hrrz    1,(10)                 ; clear u-area flag and LH base together
        andi    1,0377777              ; clear PROC_F_UAREA in RH
        movem   1,(10)
        pop     17,10
        jrst    kret_zero
proc_uarea_release_fail:
        pop     17,10
        jrst    kret_neg1

/**
 * @brief Release TTY ownership when the final process in a session exits.
 * @param AC1 Session id.
 * @param AC2 Slot which is leaving the session.
 *
 * The existing compact process-table scanner determines whether another
 * member remains.  Only when the session becomes empty do we scan the 21 TTY
 * records, discard any cooked line buffer, and preserve only output routing.
 */
proc_tty_release_session:
        push    17,10
        push    17,11
        move    10,1
        jumpe   10,proc_tty_release_done
        move    3,2
        move    1,10
        setz    2,
        pushj   17,proc_tty_session_has
        jumpn   1,proc_tty_release_done
        setz    11,
proc_tty_release_loop:
        cail    11,025
        jrst    proc_tty_release_done
        move    1,proc_tty_records(11)
        andi    1,0377
        came    1,10
        jrst    proc_tty_release_next
        move    1,11
        pushj   17,proc_tty_line_reset
        movsi   1,0174000
        andb    1,proc_tty_records(11)
proc_tty_release_next:
        aoja    11,proc_tty_release_loop
proc_tty_release_done:
        pop     17,11
        pop     17,10
        popj    17,

; Remove only trailing FREE descriptors; interior holes remain reusable.
proc_trim_high:
        move    1,proc_high_slot
proc_trim_high_loop:
        caig    1,1
        popj    17,
        subi    1,1
        move    2,1
        lsh     2,1
        add     2,1
        add     2,proc_table
        move    3,2(2)
        tlne    3,PROC_STATE_LH_MASK
        popj    17,
        movem   1,proc_high_slot
        jrst    proc_trim_high_loop

/**
 * @brief Convert a process slot to its three-word descriptor address.
 * @param AC1 Process slot.
 * @return AC1 Descriptor address; AC2 clobbered.
 */
proc_slot_ptr:
        move    2,1
        lsh     1,1
        add     1,2                    ; 3 * slot
        add     1,proc_table
        popj    17,

/**
 * @brief Maintain the intrusive runnable queue stored in sched RH.
 *
 * Add is O(1); removal is O(number of runnable jobs) but occurs only on state
 * transitions. AC1 is the slot. AC1..AC4 are preserved for event/wait callers.
 */
proc_runq_add:
        move    5,1
        imuli   5,3
        add     5,proc_table
        hlrz    0,2(5)
        andi    0,PROC_STATE_LH_MASK
        caie    0,PROC_STATE_RUN
        popj    17,
        move    0,proc_runq_head
        hrrm    0,2(5)
        movem   1,proc_runq_head
        popj    17,

proc_runq_remove:
        move    5,proc_runq_head       ; AC5 = current slot
        setz    6,                     ; AC6 = predecessor slot
proc_runq_remove_scan:
        jumpe   5,proc_runq_remove_done
        camn    5,1
        jrst    proc_runq_remove_found
        move    6,5
        imuli   5,3
        add     5,proc_table
        hrrz    5,2(5)                 ; follow current sched RH
        jrst    proc_runq_remove_scan
proc_runq_remove_found:
        move    7,1
        imuli   7,3
        add     7,proc_table
        hrrz    0,2(7)                 ; successor of target
        jumpe   6,proc_runq_remove_head
        imuli   6,3
        add     6,proc_table
        hrrm    0,2(6)
        popj    17,
proc_runq_remove_head:
        movem   0,proc_runq_head
proc_runq_remove_done:
        popj    17,

/**
 * @brief Return the stable physical u-area base for one slot.
 * @param AC1 Process slot.
 * @return AC1 U-area base; AC2 clobbered.
 */
proc_uarea_slot:
        pushj   17,proc_slot_ptr
        hlrz    1,(1)
        popj    17,

/**
 * @brief Reap or report one matching child for WAIT.
 * @param AC1 Selector: low eight bits are slot/pgrp, bit 0400 selects pgrp.
 * @param AC2 Optional status-word pointer.
 * @param AC3 SYS_WAIT_NOHANG or zero.
 * @return AC1 Child slot, zero for NOHANG, or -1 for error/no children.
 *
 * This is the compact PDP-6 spelling of proc_wait_status().  KCC's version
 * materializes the loop state in a large stack frame even though six stable
 * values fit naturally in callee-saved accumulators.  No policy changes live
 * here: selector validation, STOP/CONT reports, zombie status, and retry after
 * proc_wait_child() exactly mirror the C fallback.
 */
proc_wait_status:
        add     17,kconst_7_7
        movei   0,-6(17)
        hrli    0,010
        blt     0,0(17)                ; save AC10..AC16
        move    010,1                  ; selector
        move    011,2                  ; statusp
        move    012,3                  ; flags

        tdne    012,[-2]               ; only SYS_WAIT_NOHANG is valid
        jrst    proc_wait_status_error
        tdne    010,[-01000]           ; selector uses only bits 0..8
        jrst    proc_wait_status_error
        move    014,010
        andi    014,0377               ; requested slot/pgrp id
        trnn    010,0400
        jrst    proc_wait_status_valid
        jumpe   014,proc_wait_status_error
proc_wait_status_valid:
        move    013,proc_current_slot  ; parent slot

proc_wait_status_retry:
        setz    7,                     ; have_child
        movei   015,1                  ; slot 0 is the executive
        move    016,proc_table
        addi    016,PROC_WORDS
proc_wait_status_scan:
        caml    015,proc_high_slot
        jrst    proc_wait_status_scan_done

        move    4,2(016)
        move    5,4
        lsh     5,-041                 ; numeric PROC_STATE
        jumpe   5,proc_wait_status_next
        move    6,(016)
        move    1,6
        lsh     1,-010
        andi    1,0377                 ; parent slot
        came    1,013
        jrst    proc_wait_status_next
        trnn    010,0400               ; slot selector or pgrp selector?
        jrst    proc_wait_status_slot
        hrrz    1,6
        andi    1,0377
        came    1,014
        jrst    proc_wait_status_next
        jrst    proc_wait_status_match
proc_wait_status_slot:
        jumpe   014,proc_wait_status_match
        came    014,015
        jrst    proc_wait_status_next

proc_wait_status_match:
        movei   7,1
        caie    5,4                    ; PROC_ZOMB
        jrst    proc_wait_status_report

        ; Zombie: status is EXITED,,exit-status, then free the descriptor.
        jumpe   011,proc_wait_status_reap
        movsi   1,1                    ; SYS_WAIT_EXITED
        hlrz    2,(016)
        ior     1,2
        movem   1,(011)
proc_wait_status_reap:
        setzm   (016)
        setzm   1(016)
        setzm   2(016)
        pushj   17,proc_trim_high
        move    1,015
        jrst    proc_wait_status_return

proc_wait_status_report:
        ; A non-zombie can carry STOP/CONT reports only while its u-area lives.
        trnn    6,0400000              ; PROC_F_UAREA << PROC_FLAGS_SHIFT
        jrst    proc_wait_status_next
        hlrz    2,6                    ; u-area base
        move    3,PROC_FDCTL_OFFSET(2)
        move    1,3
        lsh     1,-034
        andi    1,3                    ; PROC_WAIT_REPORT
        caie    1,1                    ; STOPPED
        cain    1,2                    ; CONTINUED
        jrst    proc_wait_status_have_report
        jrst    proc_wait_status_next
proc_wait_status_have_report:
        tlz     3,06000                ; clear PROC_REPORT_BITS
        movem   3,PROC_FDCTL_OFFSET(2)
        jumpe   011,proc_wait_status_report_done
        move    3,1
        addi    3,1                    ; report+1 -> wait status kind
        lsh     3,022
        move    4,1
        addi    4,2                    ; report+2 -> status value
        ior     3,4
        movem   3,(011)
proc_wait_status_report_done:
        move    1,015
        jrst    proc_wait_status_return

proc_wait_status_next:
        addi    015,1
        addi    016,PROC_WORDS
        jrst    proc_wait_status_scan

proc_wait_status_scan_done:
        jumpe   7,proc_wait_status_error
        trne    012,1                  ; SYS_WAIT_NOHANG
        jrst    proc_wait_status_nohang
        pushj   17,proc_wait_child
        jumpn   1,proc_wait_status_error
        jrst    proc_wait_status_retry

proc_wait_status_nohang:
        setz    1,
        jrst    proc_wait_status_return
proc_wait_status_error:
        seto    1,
proc_wait_status_return:
        movei   0,010
        hrli    0,-6(17)
        blt     0,016                   ; restore AC10..AC16
        sub     17,kconst_7_7
        popj    17,

/**
 * @brief Terminate the current process after escaping its private kernel stack.
 *
 * EXIT enters by JRST from syscall dispatch. PI is disabled, execution moves to
 * the permanent slot-0 stack, and proc_exit_finish() may then free the u-area.
 * The machine idles for another runnable process or halts after the final user.
 */
proc_exit_current:
        move    3,1                    ; preserve low 18-bit exit status
        pushj   17,mach_pi_disable
        move    2,1                    ; saved global PI on/off state
        move    1,3
        move    17,mach_kernel_stack_base
        setzm   file_table
        push    17,2
        pushj   17,proc_exit_finish
.if PROC_STACK_WATERMARK
        jumple  1,proc_exit_watermark_halt
.else
        jumple  1,proc_exit_halt       ; final process or fatal release error
.endif
        pop     17,1                   ; another process remains: restore PI
        pushj   17,mach_pi_restore
        jrst    proc_idle_loop
.if PROC_STACK_WATERMARK
proc_exit_watermark_halt:
        pushj   17,kernel_idle_stack_watermark_scan
.endif
proc_exit_halt:
        ; Keep PI disabled.  Re-enabling it here lets a final clock interrupt
        ; redirect the no-process case into proc_idle_loop before HALT.
        halt
        jrst    .-1

/**
 * @brief Publish the current process-private kernel stack after first user entry.
 *
 * Initial argument ACs are preserved while CTX_KSP, mach_kernel_sp, and the
 * process-local file-table base are switched away from the bootstrap stack.
 */
proc_record_kernel_sp:
        push    17,1
        push    17,2
        move    1,proc_current_slot
        pushj   17,proc_uarea_slot
        movei   2,PROC_USTACK_BASE(1)
        movem   2,CTX_KSP(1)
        movem   2,mach_kernel_sp
        movei   2,PROC_FILE_TABLE_OFFSET(1)
        movem   2,file_table
        pop     17,2
        pop     17,1
        popj    17,



/**
 * @brief Control the singleton real-time scheduler ownership slot.
 * @param AC1 SYS_RTCTL_DISABLE, ENABLE, or YIELD.
 * @return AC1 zero on success or -1 on invalid/non-owner requests.
 *
 * ENABLE is idempotent for the owner; DISABLE and YIELD release ownership and
 * immediately re-enter normal scheduling. AC2..AC4 are caller-scratch.
 */
        .globl  proc_rt_control
proc_rt_control:
        move    2,proc_current_slot
        jumpe   2,kret_neg1
        cain    1,1                    ; SYS_RTCTL_ENABLE
        jrst    proc_rt_enable
        caie    1,0                    ; SYS_RTCTL_DISABLE
        cain    1,2                    ; SYS_RTCTL_YIELD
        jrst    proc_rt_release
        jrst    kret_neg1
proc_rt_enable:
        skipn   3,proc_rt_owner
        jrst    proc_rt_claim
        came    3,2
        jrst    kret_neg1
        jrst    kret_zero
proc_rt_claim:
        movem   2,proc_rt_owner
        jrst    kret_zero
proc_rt_release:
        camn    2,proc_rt_owner
        jrst    proc_rt_release_owner
        jrst    kret_neg1
proc_rt_release_owner:
        setzm   proc_rt_owner
        pushj   17,proc_sched_resched_current
        jrst    kret_zero

/**
 * @brief Set the current process nice value with the native privilege rule.
 * @param AC1 Requested signed nice value.
 * @return AC1 effective value, or -1 when a non-root process raises priority.
 *
 * KCC's small C version needs a save frame and several spills.  This leaf
 * keeps the exact -20..19 clamp and UID-0 rule in caller-scratch AC2..AC4.
 */
        .globl  proc_nice_current
proc_nice_current:
        camge   1,[-024]
        move    1,[-024]
        camle   1,[023]
        movei   1,023
        move    2,proc_current_ptr
        hlrz    3,2(2)
        andi    3,077
        subi    3,024
        caml    1,3                    ; only a numerically lower nice needs root
        jrst    proc_nice_store
        hlrz    4,(2)
        hlrz    4,PROC_CRED_OFFSET(4)
        jumpn   4,kret_neg1
proc_nice_store:
        move    3,1
        addi    3,024
        lsh     3,022
        move    4,2(2)
        tlz     4,077
        ior     4,3
        movem   4,2(2)
        popj    17,

; int proc_tty_session_has(unsigned int session, unsigned int pgrp,
;     unsigned int skip_slot)
; Scan the compact process table without a C save frame.  AC1..AC3 carry
; arguments; AC1 is the boolean result.  AC2..AC7 are caller-scratch.
        .globl  proc_tty_session_has
proc_tty_session_has:
        move    5,1                    ; session
        move    6,2                    ; pgrp, zero means wildcard
        move    7,3                    ; slot to exclude
        movei   2,1                    ; first user slot
        move    3,proc_table
        addi    3,PROC_WORDS
proc_tty_session_scan:
        caml    2,proc_high_slot
        jrst    kret_zero
        camn    2,7
        jrst    proc_tty_session_next
        move    4,2(3)                 ; packed scheduler word
        and     4,[0300000000000]      ; FREE/ZOMB have low state bits clear
        jumpe   4,proc_tty_session_next
        jumpe   6,proc_tty_session_scope
        hrrz    4,(3)
        andi    4,0377                  ; process group
        came    4,6
        jrst    proc_tty_session_next
proc_tty_session_scope:
        hrrz    4,(3)
        trnn    4,0400000               ; resident u-area flag
        jrst    proc_tty_session_next
        hlrz    4,(3)                   ; stable u-area base
        move    4,PROC_FDCTL_OFFSET(4)                ; packed control word
        lsh     4,-3
        andi    4,0377                  ; session id
        camn    4,5
        jrst    kret_one
proc_tty_session_next:
        addi    3,PROC_WORDS
        aoja    2,proc_tty_session_scan
/**
 * @brief Tear down a session leader's controlling TTY and HUP its members.
 *
 * AC1 is leader slot and AC2 its packed control word. TTY ownership is encoded
 * directly in process/TTY records, so no permanent session table is required.
 */
        .globl  proc_session_teardown
        .globl  proc_event_apply
        .globl  proc_tty_records
proc_session_teardown:
        push    17,010                  ; preserve callee-saved scan state
        push    17,011
        push    17,012
        move    010,1                  ; leader slot, then session id
        move    3,2                    ; leader control word
        ldb     5,[POINT 8,3,32]       ; session
        came    010,5                  ; only the session leader tears down
        jrst    proc_session_teardown_ok
        move    4,3
        lsh     4,-036                 ; packed TTY state
        subi    4,2                    ; attached state -> TTY id
        jumpl   4,proc_session_teardown_ok
        cail    4,025
        jrst    proc_session_teardown_fail
        move    6,proc_tty_records(4)
        and     6,[174000000000]       ; preserve output route
        movem   6,proc_tty_records(4)

        movei   011,1
        move    012,proc_table
        addi    012,PROC_WORDS
proc_session_teardown_loop:
        caml    011,proc_high_slot
        jrst    proc_session_teardown_ok
        camn    011,010
        jrst    proc_session_teardown_next
        hrrz    3,(012)
        trnn    3,0400000              ; no stable u-area: FREE/ZOMB
        jrst    proc_session_teardown_next
        hlrz    4,(012)
        ldb     5,[POINT 8,PROC_FDCTL_OFFSET(4),32]
        came    5,010
        jrst    proc_session_teardown_next
        move    1,011
        movei   2,2                    ; default-fatal HUP
        pushj   17,proc_event_apply
        jumpn   1,proc_session_teardown_fail
proc_session_teardown_next:
        addi    012,PROC_WORDS
        aoja    011,proc_session_teardown_loop
proc_session_teardown_fail:
        seto    1,
        jrst    proc_session_teardown_return
proc_session_teardown_ok:
        setz    1,
proc_session_teardown_return:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

/**
 * @brief Validate and deliver an event to one PID or process group.
 *
 * AC1=target, AC2=event, AC3=group flag. Group delivery scans the compact table
 * and delegates actual state transitions to proc_event_apply().
 */
        .globl  proc_event_send
        .globl  proc_event_apply
; int proc_event_apply(unsigned int slot, unsigned int event)
; Hot validated event-delivery path.  AC10=slot, AC11=event, AC12=descriptor
; survive calls into the existing resource/runq/parent-notify helpers.
proc_event_apply:
        push    17,010
        push    17,011
        push    17,012
        move    010,1
        move    011,2
        move    012,1
        lsh     012,1
        add     012,1
        add     012,proc_table

        ; INT/TERM/HUP (0..2) and PIPE (7) are fatal.
        caile   011,2
        jrst    proc_event_check_pipe
        jrst    proc_event_fatal
proc_event_check_pipe:
        caie    011,7
        jrst    proc_event_nonfatal
proc_event_fatal:
        came    010,proc_current_slot
        jrst    proc_event_fatal_other
        pushj   17,file_close_all
        move    1,011
        iori    1,0400000               ; SYS_WAIT_EVENT_FLAG
        pop     17,012
        pop     17,011
        pop     17,010
        jrst    proc_exit_current

proc_event_fatal_other:
        hlrz    1,(012)
        addi    1,PROC_FILE_TABLE_OFFSET ; target private file table
        exch    1,file_table            ; AC1 = caller file table
        push    17,1
        pushj   17,file_close_all
        pop     17,1
        movem   1,file_table
        move    1,010
        move    2,011
        iori    2,0400000
        pop     17,012
        pop     17,011
        pop     17,010
        jrst    proc_finish_slot

proc_event_nonfatal:
        ; Events 3..6 map directly to the packed pending-event bitmap.
        hlrz    6,(012)
        move    4,PROC_FDCTL_OFFSET(6)
        movei   5,1
        lsh     5,0(011)
        lsh     5,023                   ; PROC_EVENT_SHIFT = 19 decimal
        ior     4,5
        movem   4,PROC_FDCTL_OFFSET(6)

        caie    011,3                   ; TSTP
        jrst    proc_event_cont
        tlne    4,0400                  ; already job-control stopped
        jrst    proc_event_ok
        tlo     4,0400                  ; PROC_STOP_JOB_BIT
        tlz     4,06000                 ; replace wait report
        tlo     4,02000                 ; PROC_REPORT_STOPPED
        movem   4,PROC_FDCTL_OFFSET(6)
        move    1,010
        pushj   17,proc_runq_remove
        move    5,2(012)
        tlz     5,PROC_STATE_LH_MASK
        tlo     5,0600000               ; PROC_STOP
        movem   5,2(012)
        pushj   17,proc_event_notify_parent
        came    010,proc_current_slot
        jrst    proc_event_ok
        pushj   17,proc_sched_resched_current
        jrst    proc_event_ok

proc_event_cont:
        caie    011,4                   ; CONT
        jrst    proc_event_alrm
        tlnn    4,0400
        jrst    proc_event_ok
        tlz     4,0400
        tlz     4,06000
        tlo     4,04000                 ; PROC_REPORT_CONTINUED
        movem   4,PROC_FDCTL_OFFSET(6)
        tlne    4,01400                 ; another stop reason remains
        jrst    proc_event_cont_report
        hlrz    5,2(012)
        andi    5,PROC_STATE_LH_MASK
        caie    5,0600000               ; PROC_STOP
        jrst    proc_event_cont_report
        move    5,2(012)
        move    3,5
        lsh     3,-037
        andi    3,3                     ; saved wait class
        jumpn   3,proc_event_cont_sleep
        and     5,[-017700000001]        ; clear recent-CPU/sleep-age fields
        tlz     5,PROC_STATE_LH_MASK
        tlo     5,PROC_STATE_RUN
        movem   5,2(012)
        move    1,010
        pushj   17,proc_runq_add
        jrst    proc_event_cont_report
proc_event_cont_sleep:
        tlz     5,PROC_STATE_LH_MASK
        tlo     5,PROC_STATE_SLEEP
        movem   5,2(012)
proc_event_cont_report:
        pushj   17,proc_event_notify_parent
        jrst    proc_event_ok

proc_event_alrm:
        caie    011,5                   ; CHLD only sets its pending bit
        jrst    proc_event_ok
        move    5,2(012)
        move    3,5
        lsh     3,-037
        andi    3,3
        cain    3,2                    ; PROC_WAIT_CHILD
        jrst    proc_event_alrm_wake
        caie    3,3                    ; PROC_WAIT_INTR
        jrst    proc_event_ok
proc_event_alrm_wake:
        hlrz    3,5
        andi    3,PROC_STATE_LH_MASK
        movsi   5,0600077              ; clear wait/RH/CPU age; SLEEP -> RUN
        andb    5,2(012)
        caie    3,PROC_STATE_SLEEP
        jrst    proc_event_ok
        move    1,010
        pushj   17,proc_runq_add
        jrst    proc_event_ok

; STOP/CONT publish one parent report after the child state/control word is set.
proc_event_notify_parent:
        hrrz    1,(012)
        lsh     1,-010
        andi    1,0377
        jrst    proc_notify_parent

proc_event_ok:
        setz    1,
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

proc_event_send:
        add     17,kconst_5_5
        movei   0,-4(17)
        hrli    0,010
        blt     0,(17)                 ; preserve AC10..AC14
        move    010,1                  ; target
        move    011,2                  ; event
        move    012,proc_current_slot  ; caller slot
        jumpe   010,proc_event_send_fail
        cail    011,6                   ; CHLD and above are not user-sendable
        jrst    proc_event_send_fail

        move    1,012
        pushj   17,proc_slot_ptr
        hlrz    4,(1)
        move    013,PROC_FDCTL_OFFSET(4)             ; caller session/domain control word
        jumpn   3,proc_event_send_group

; PID delivery: target must be live, u-area resident, in caller's domain, and
; owned by the caller unless the caller is UID 0.
        caml    010,proc_slots
        jrst    proc_event_send_fail
        move    1,010
        pushj   17,proc_slot_ptr
        move    3,1
        move    4,2(3)
        and     4,[0300000000000]
        jumpe   4,proc_event_send_fail
        hrrz    4,(3)
        trnn    4,0400000               ; resident u-area flag
        jrst    proc_event_send_fail
        hlrz    4,(3)
        pushj   17,proc_event_uid_check
        jumpn   1,proc_event_send_fail
        move    5,PROC_FDCTL_OFFSET(4)
        xor     5,013
        tdne    5,[01774000]            ; domain differs
        jrst    proc_event_send_fail
        move    1,010
        move    2,011
        jrst    proc_event_send_apply_tail

; Group delivery requires session/domain scope and the same UID/root rule as
; PID delivery.  AC14 LH bit 0 records any match and bit 1 records a deferred
; self-delivery; RH is slot.
proc_event_send_group:
        movei   014,1
proc_event_send_group_loop:
        hrrz    6,014
        caml    6,proc_high_slot
        jrst    proc_event_send_group_done
        move    3,6
        lsh     3,1
        add     3,6
        add     3,proc_table
        move    4,2(3)
        and     4,[0300000000000]
        jumpe   4,proc_event_send_group_next
        hrrz    4,(3)
        trnn    4,0400000
        jrst    proc_event_send_group_next
        move    4,(3)
        andi    4,0377
        came    4,010
        jrst    proc_event_send_group_next
        hlrz    4,(3)
        pushj   17,proc_event_uid_check
        jumpn   1,proc_event_send_group_next
        move    5,PROC_FDCTL_OFFSET(4)
        xor     5,013
        tdne    5,[01777770]
        jrst    proc_event_send_group_next
        tlo     014,1                   ; at least one matching member
        came    6,012
        jrst    proc_event_send_group_apply
        cail    011,4                   ; defer self for INT..TSTP
        jrst    proc_event_send_group_apply
        tlo     014,2
        jrst    proc_event_send_group_next
proc_event_send_group_apply:
        move    1,6
        move    2,011
        pushj   17,proc_event_apply
        jumpn   1,proc_event_send_fail
proc_event_send_group_next:
        aoja    014,proc_event_send_group_loop

proc_event_send_group_done:
        tlnn    014,1
        jrst    proc_event_send_fail
        tlnn    014,2
        jrst    proc_event_send_ok
        move    1,012
        move    2,011
proc_event_send_apply_tail:
        movei   0,010
        hrli    0,-4(17)
        blt     0,014
        sub     17,kconst_5_5
        jrst    proc_event_apply

proc_event_send_fail:
        seto    1,
        jrst    proc_event_send_return
proc_event_send_ok:
        movei   1,0
proc_event_send_return:
        movei   0,010
        hrli    0,-4(17)
        blt     0,014
        sub     17,kconst_5_5
        popj    17,

; AC4 is the target u-area base.  Permit event delivery when the caller is
; root or when caller and target effective UIDs match.  Preserve AC4 because
; the caller immediately uses it for target session/domain state.
proc_event_uid_check:
        move    1,proc_current_ptr
        hlrz    3,(1)
        hlrz    2,PROC_CRED_OFFSET(3)              ; caller UID
        jumpe   2,kret_zero            ; UID 0 may administer all users
        hlrz    5,PROC_CRED_OFFSET(4)              ; target UID
        camn    2,5
        jrst    kret_zero
        jrst    kret_neg1

; int proc_has_live_user(void)
; Return true as soon as a non-FREE/non-ZOMB user descriptor is found.  The
; packed state encoding makes this a single scheduler-word mask test per slot.
        .globl  proc_has_live_user
proc_has_live_user:
        movei   2,1                    ; first user slot
        move    3,proc_table
        addi    3,PROC_WORDS
proc_has_live_user_scan:
        caml    2,proc_high_slot
        jrst    kret_zero
        move    4,2(3)                 ; packed scheduler word
        and     4,[0300000000000]      ; FREE/ZOMB have low state bits clear
        jumpn   4,kret_one
        addi    3,PROC_WORDS
        aoja    2,proc_has_live_user_scan
; void proc_notify_parent(unsigned int parent)
; Queue CHLD and wake a parent blocked in WAIT.  The descriptor is decoded
; once; unlike the former C helper chain, the wake path does not revalidate
; proc_table and the slot after notification has already validated them.
        .globl  proc_notify_parent
proc_notify_parent:
        jumpe   1,proc_notify_parent_done
        skipn   3,proc_table
        popj    17,
        caml    1,proc_slots
        popj    17,
        move    2,1
        lsh     2,1
        add     2,1
        add     2,3
        move    4,2(2)
        and     4,[0300000000000]      ; FREE/ZOMB have low state bits clear
        jumpe   4,proc_notify_parent_done
        hlrz    4,(2)
        movsi   3,0200                 ; pending CHLD in control-word LH
        iorm    3,PROC_FDCTL_OFFSET(4)
        move    4,2(2)
        xor     4,[0340000000000]      ; SLEEP + WAIT_CHILD
        and     4,[0760000000000]
        jumpe   4,proc_notify_parent_wake
        move    4,2(2)
        xor     4,[0640000000000]      ; STOP + WAIT_CHILD
        and     4,[0760000000000]
        jumpn   4,proc_notify_parent_done
proc_notify_parent_wake:
        move    4,2(2)
        hllz    4,4                    ; clear wait channel
        tlz     4,PROC_WAIT_LH_MASK
        tlz     4,0100000              ; SLEEP->RUN, STOP remains STOP
        tlz     4,PROC_CPU_SLEEP_LH_MASK
        movem   4,2(2)
        pushj   17,proc_runq_add
proc_notify_parent_done:
        popj    17,

; kword_t proc_scope_id(const struct proc *p)
; Return packed session/domain.  Zombie scope already occupies sched RH;
; live scope is the same 16-bit pair shifted down from the u-area control word.
        .globl  proc_scope_id
proc_scope_id:
        hlrz    2,2(1)
        andi    2,0700000
        caie    2,0400000              ; ZOMB
        jrst    proc_scope_live
        hrrz    1,2(1)
        andi    1,0177777
        popj    17,
proc_scope_live:
        hrrz    2,(1)
        trnn    2,0400000              ; u-area present
        jrst    kret_zero
        hlrz    2,(1)
        move    1,PROC_FDCTL_OFFSET(2)
        lsh     1,-3
        andi    1,0177777
        popj    17,

; int proc_child_hierarchy(unsigned int child_slot, unsigned int mode,
;     unsigned int requested_pgrp)
; Leaf implementation: AC1 child slot, AC2 mode, AC3 requested pgrp.
; Session/domain/TTY inheritance is one masked control-word transfer.
        .globl  proc_child_hierarchy
proc_child_hierarchy:
        move    7,1                    ; child slot
        move    6,3                    ; requested pgrp
        move    4,proc_current_ptr     ; parent descriptor
        hlrz    3,(4)
        move    3,PROC_FDCTL_OFFSET(3)               ; parent control word
        ldb     5,[POINT 8,3,32]       ; parent session

        jumpe   2,proc_child_inherit
        caie    2,1
        jrst    proc_child_join
; NEW pgrp.
        jumpn   6,kret_neg1
        move    1,7
        jrst    proc_child_set
proc_child_inherit:
        jumpn   6,kret_neg1
        hrrz    1,(4)
        andi    1,0377
        jumpe   1,kret_neg1
        jrst    proc_child_set
proc_child_join:
        caie    2,2
        jrst    kret_neg1
        jumpe   6,kret_neg1
        caile   6,0377
        jrst    kret_neg1
        movei   1,1                    ; scan slot
        move    2,proc_table
        addi    2,3
proc_child_join_loop:
        caml    1,proc_high_slot
        jrst    kret_neg1
        ; FREE has all state bits clear.  Preserve the original sched word in
        ; AC4 only long enough to classify zombie/live scope below.
        hlrz    4,2(2)
        andi    4,0700000
        jumpe   4,proc_child_join_next
        hrrz    4,(2)
        andi    4,0377
        came    4,6
        jrst    proc_child_join_next
        hlrz    4,2(2)
        andi    4,0700000
        caie    4,0400000              ; zombie scope lives in sched RH
        jrst    proc_child_join_live
        hrrz    4,2(2)
        andi    4,0377
        jrst    proc_child_join_scope
proc_child_join_live:
        hrrz    4,(2)
        trnn    4,0400000
        jrst    proc_child_join_next
        hlrz    4,(2)
        ldb     4,[POINT 8,PROC_FDCTL_OFFSET(4),32]
proc_child_join_scope:
        came    4,5
        jrst    proc_child_join_next
        move    1,6
        jrst    proc_child_set
proc_child_join_next:
        addi    2,3
        aoja    1,proc_child_join_loop

proc_child_set:
        ; AC1 = selected pgrp, AC3 still parent control, AC7 child slot.
        move    2,7
        lsh     2,1
        add     2,7
        add     2,proc_table           ; child descriptor
        move    4,(2)
        andcmi  4,0377
        andi    1,0377
        ior     4,1
        movem   4,(2)
        hlrz    4,(2)
        move    6,PROC_FDCTL_OFFSET(4)
        and     6,[007776000007]       ; clear session/domain/TTY
        move    5,3
        and     5,[770001777770]
        ior     6,5
        movem   6,PROC_FDCTL_OFFSET(4)
        jrst    kret_zero

; int proc_control(unsigned int op, unsigned int arg)
; Compact native PROCCTL dispatcher.  The syscall dispatcher guarantees a live
; current process with a resident u-area.  AC1=op, AC2=arg.  Only AC1..AC7 are
; used except around explicit C calls, so the common query/update cases need no
; GCC save frame.
        .globl  proc_control
        .globl  proc_tty_records
        .globl  proc_tty_release_session
proc_control:
        jumpl   1,kret_neg1
        caile   1,031
        jrst    kret_neg1
        ; AC3 is the current slot/session identity used by NEWSESSION,
        ; NEWDOMAIN, and the TTY ownership operations below.  Do not depend
        ; on an arbitrary user AC3 value surviving the syscall trap.
        move    3,proc_current_slot
        move    4,proc_current_ptr
        move    5,1
        andi    5,1
        lsh     1,-1
        xct     proc_control_half_select(5)
        jrst    (5)
proc_control_half_select:
        hlrz    5,proc_control_table(1)
        hrrz    5,proc_control_table(1)
proc_control_table:
        .word   proc_control_getpgrp,,proc_control_getsession
        .word   proc_control_getdomain,,proc_control_newsession
        .word   proc_control_newdomain,,proc_control_getevents
        .word   proc_control_event_pid,,proc_control_event_pgrp
        .word   proc_control_gettty,,proc_control_tty_attach
        .word   proc_control_tty_detach,,proc_control_tty_getfg
        .word   proc_control_tty_setfg,,proc_control_getuid
        .word   proc_control_getgid,,proc_control_setuid
        .word   kret_neg1,,kret_neg1   ; 020..021 extension bank
        .word   kret_neg1,,kret_neg1   ; 022..023 extension bank
        .word   kret_neg1,,proc_control_setgid ; 024 reserved, 025 SETGID
        .word   proc_control_tty_getmode,,proc_control_tty_setmode
        .word   proc_control_isatty,,proc_control_umask

proc_control_getpgrp:
        jumpn   2,kret_neg1
        hrrz    1,(4)
        andi    1,0377
        popj    17,
proc_control_getsession:
        jumpn   2,kret_neg1
        hlrz    5,(4)
        ldb     1,[POINT 8,PROC_FDCTL_OFFSET(5),32]
        popj    17,
proc_control_getdomain:
        jumpn   2,kret_neg1
        hlrz    5,(4)
        ldb     1,[POINT 8,PROC_FDCTL_OFFSET(5),24]
        popj    17,

proc_control_newsession:
        jumpn   2,kret_neg1
        hlrz    5,(4)
        ldb     1,[POINT 8,PROC_FDCTL_OFFSET(5),32]   ; old session for TTY release
        move    6,(4)
        andcmi  6,0377                 ; preserve LH, clear pgrp in RH
        move    7,3
        andi    7,0377
        ior     6,7
        movem   6,(4)
        dpb     3,[POINT 8,PROC_FDCTL_OFFSET(5),32]
        hrloi   6,07777
        andm    6,PROC_FDCTL_OFFSET(5)               ; TTY state -> NO_TTY
        move    2,3                    ; leaving slot
        pushj   17,proc_tty_release_session
        move    1,proc_current_slot
        popj    17,

proc_control_newdomain:
        jumpn   2,kret_neg1
        hlrz    5,(4)
        dpb     3,[POINT 8,PROC_FDCTL_OFFSET(5),24]
        move    1,3
        popj    17,

proc_control_getevents:
        jumpn   2,kret_neg1
        hlrz    5,(4)
        ldb     1,[POINT 7,PROC_FDCTL_OFFSET(5),16]
        move    6,PROC_FDCTL_OFFSET(5)
        trne    6,1
        iori    1,0200
        hrloi   6,0777401
        andcmi  6,1
        andm    6,PROC_FDCTL_OFFSET(5)
        popj    17,

proc_control_event_pid:
        setz    3,
        jrst    proc_control_event
proc_control_event_pgrp:
        movei   3,1
proc_control_event:
        tdne    2,[-04000]
        jrst    kret_neg1
        move    1,2
        andi    1,0377
        lsh     2,-010
        andi    2,07
        jrst    proc_event_send

proc_control_gettty:
        jumpn   2,kret_neg1
        hlrz    5,(4)
        move    1,PROC_FDCTL_OFFSET(5)
        lsh     1,-036
        popj    17,

proc_control_tty_attach:
        cail    2,025
        jrst    kret_neg1
        hlrz    5,(4)
        move    6,PROC_FDCTL_OFFSET(5)
        ldb     7,[POINT 8,PROC_FDCTL_OFFSET(5),32]
        came    7,3
        jrst    kret_neg1
        move    7,6
        lsh     7,-036
        andi    7,077
        cail   7,2
        jrst    kret_neg1
proc_control_tty_attach_state_ok:
        move    7,proc_tty_records(2)
        move    1,7
        andi    1,0377
        jumpe   1,proc_control_tty_attach_claim
        came    1,3
        jrst    kret_neg1
        jrst    proc_control_tty_attach_set
proc_control_tty_attach_claim:
        hrrz    1,(4)
        andi    1,0377
        lsh     1,010
        ior     1,3
        ior     1,[01600000000]         ; canonical + echo + signals
        move    6,7
        and     6,[174000000000]         ; preserve output route
        ior     1,6
        movem   1,proc_tty_records(2)
proc_control_tty_attach_set:
        addi    2,2                    ; encoded ATTACHED(tty)
        move    1,3                    ; session id == session-leader slot
        pushj   17,proc_tty_set_session_state
        move    1,2
        subi    1,2                    ; return tty id
        popj    17,

; Decode and validate the caller's controlling TTY.  Return AC1 = TTY id,
; AC5 = session, AC6 = control word, AC7 = TTY record; AC2/AC3 preserved.
; This shared cold path replaces three copies in DETACH/GETFG/SETFG.
proc_control_tty_owned:
        hlrz    5,(4)
        move    6,PROC_FDCTL_OFFSET(5)
        move    1,6
        lsh     1,-036
        subi    1,2
        jumpl   1,proc_control_tty_owned_bad ; DETACHED becomes -1
        cail    1,025
        jrst    proc_control_tty_owned_bad
        move    7,proc_tty_records(1)
        move    4,7
        andi    4,0377
        ldb     5,[POINT 8,PROC_FDCTL_OFFSET(5),32]
        camn    4,5
        popj    17,
proc_control_tty_owned_bad:
        seto    1,
        popj    17,

proc_control_tty_detach:
        jumpn   2,kret_neg1
        pushj   17,proc_control_tty_owned
        jumpl   1,kret_neg1
        came    5,3                    ; only the session leader detaches
        jrst    kret_neg1
        push    17,5                   ; line reset is C and may use ACs
        push    17,1
        pushj   17,proc_tty_line_reset
        pop     17,1
        pop     17,5
        move    2,proc_tty_records(1)
        and     2,[174000000000]         ; preserve output route
        movem   2,proc_tty_records(1)
        move    1,5                    ; session
        movei   2,1                    ; DETACHED
        pushj   17,proc_tty_set_session_state
        jrst    kret_zero

; AC1=session, AC2=packed TTY state.  Update every live member.  AC2 is
; preserved so ATTACH can recover and return its TTY id without stack traffic.
proc_tty_set_session_state:
        move    7,1
        move    6,2
        lsh     6,036
        movei   3,1
        move    4,proc_table
        addi    4,PROC_WORDS
proc_tty_set_session_state_loop:
        caml    3,proc_high_slot
        popj    17,
        hrrz    5,(4)
        trnn    5,0400000              ; no stable u-area: FREE/ZOMB
        jrst    proc_tty_set_session_state_next
        hlrz    5,(4)
        ldb     1,[POINT 8,PROC_FDCTL_OFFSET(5),32]
        came    1,7
        jrst    proc_tty_set_session_state_next
        move    1,PROC_FDCTL_OFFSET(5)
        tlz     1,0770000
        ior     1,6
        movem   1,PROC_FDCTL_OFFSET(5)
proc_tty_set_session_state_next:
        addi    4,PROC_WORDS
        aoja    3,proc_tty_set_session_state_loop

proc_control_tty_getfg:
        jumpn   2,kret_neg1
        pushj   17,proc_control_tty_owned
        jumpl   1,kret_neg1
        ldb     1,[POINT 8,7,27]
        popj    17,


proc_control_tty_getmode:
        jumpn   2,kret_neg1
        pushj   17,proc_control_tty_owned
        jumpl   1,kret_neg1
        move    1,7
        lsh     1,-031
        andi    1,07
        popj    17,

proc_control_tty_setmode:
        tdne    2,[-010]
        jrst    kret_neg1
        push    17,2                   ; requested mode
        pushj   17,proc_control_tty_owned
        jumpl   1,proc_control_tty_setmode_bad
        move    6,proc_current_ptr
        hrrz    6,(6)
        andi    6,0377                 ; caller pgrp
        ldb     5,[POINT 8,proc_tty_records(1),27]
        came    5,6                    ; only foreground owner changes mode
        jrst    proc_control_tty_setmode_bad
        pop     17,2
        jrst    proc_tty_mode_set
proc_control_tty_setmode_bad:
        sub     17,kconst_1_1
        jrst    kret_neg1

proc_control_getuid:
proc_control_getgid:
        jumpn   2,kret_neg1
        hlrz    5,(4)
        hlrz    6,PROC_CRED_OFFSET(5)
        ; Packed dispatch leaves AC1 = opcode >> 1.  GETUID 015 therefore
        ; arrives as 06; GETGID 016 arrives as 07.
        caie    1,06                   ; GETUID keeps LH, GETGID selects RH
        hrrz    6,PROC_CRED_OFFSET(5)
        move    1,6
        popj    17,

; UID 0 may install login credentials.  An ordinary process may only request
; its current UID/GID, so it cannot acquire another identity.  SETUID (017)
; and SETGID (025) share the path.  Packed dispatch leaves AC1 = opcode >> 1,
; so SETUID arrives as 07 and SETGID as 012.
proc_control_setuid:
proc_control_setgid:
proc_control_setcred:
        caile   2,0777                  ; UID/GID are 9-bit IDs
        jrst    kret_neg1
        hlrz    5,(4)
        move    6,PROC_CRED_OFFSET(5)
        hlrz    7,6                    ; current UID controls privilege
        jumpe   7,proc_control_setcred_store
        caie    1,07                   ; SETUID compares UID, SETGID compares GID
        hrrz    7,6
        came    2,7
        jrst    kret_neg1
proc_control_setcred_store:
        cain    1,07                   ; SETUID executes only the LH store
        hrlm    2,PROC_CRED_OFFSET(5)
        caie    1,07                   ; SETGID executes only the RH store
        hrrm    2,PROC_CRED_OFFSET(5)
        move    1,2
        popj    17,

; Return the logical controlling TTY id when FD names the CTY0 conduit.
; A redirected/closed/non-terminal descriptor, or a process without an attached
; controlling terminal, returns -1.  The TTY ownership helper also verifies the
; session association instead of trusting process-local state alone.
proc_control_isatty:
        move    1,2
        pushj   17,file_find
        jumpe   1,kret_neg1
        move    1,(1)
        tlz     1,707070               ; strip packed FILE metadata
        camn    1,[020002000000]       ; MonitorFS device view CTY0 IO endpoint
        jrst    proc_control_tty_owned
        camn    1,[020002000023]       ; TTYDPY0 is TTY0 with DPY output
        jrst    proc_control_tty_owned
        jrst    kret_neg1

; Classic umask semantics: install ARG low nine bits and return the old mask.
; The word is process-private, inherited by RUN and retained by EXEC.
proc_control_umask:
        andi    2,0777
        hlrz    5,(4)
        move    1,PROC_UMASK_OFFSET(5)
        movem   2,PROC_UMASK_OFFSET(5)
        popj    17,

proc_control_tty_setfg:
        jumpe   2,kret_neg1
        caile   2,0377
        jrst    kret_neg1
        pushj   17,proc_control_tty_owned
        jumpl   1,kret_neg1
        push    17,1                   ; tty id
        push    17,2                   ; requested pgrp
        move    1,5                    ; session
        movei   3,0
        pushj   17,proc_tty_session_has
        pop     17,2
        pop     17,4                   ; tty id
        jumpe   1,kret_neg1
        move    5,proc_tty_records(4)
        move    6,5
        lsh     6,-010
        andi    6,0377                  ; old foreground pgrp
        xor     6,2                     ; changed pgrp bits only
        lsh     6,010
        xor     5,6                     ; preserve session and deferred input
        movem   5,proc_tty_records(4)
        move    1,2
        popj    17,

; Compact target implementations of the five multi-terminal data-path helpers.
; Re-deriving the current
; process/u-area here is smaller than KCC's call/save frames on each helper.
        .globl  proc_tty_read_enter
        .globl  proc_tty_input
        .globl  proc_tty_output
        .globl  proc_tty_pending_take
        .globl  proc_tty_pending_store
        .globl  proc_tty_line_take
        .globl  proc_tty_canon_input
        .globl  proc_tty_line_reset
        .globl  proc_tty_mode_set
        .globl  kret_neg2
        .globl  kret_neg3

; Packed canonical-line base helpers: two 18-bit MM bases per word.
        .globl  proc_tty_line_bases
        .globl  proc_tty_line_base_get
proc_tty_line_base_get:
        move    3,1
        lsh     3,-1
        move    3,proc_tty_line_bases(3)
        trnn    1,1
        jrst    proc_tty_line_base_get_even
        hlrz    1,3
        popj    17,
proc_tty_line_base_get_even:
        hrrz    1,3
        popj    17,

        .globl  proc_tty_line_base_set
proc_tty_line_base_set:
        move    3,1
        lsh     3,-1
        trne    1,1
        jrst    proc_tty_line_base_set_odd
        hrrm    2,proc_tty_line_bases(3)
        popj    17,
proc_tty_line_base_set_odd:
        hrlz    2,2
        hllm    2,proc_tty_line_bases(3)
        popj    17,

; Packed canonical-line SIXBIT helpers.  Cooked input admits only ASCII
; 040..0137, so storing the canonical line as six native SIXBIT tokens/word
; saves dynamic RAM and makes bulk S6REC reads a direct word copy.  READCHAR
; still sees ASCII because line_get adds 040 on extraction.
        .globl  proc_tty_line_put
proc_tty_line_put:
        move    6,3                    ; preserve character
        subi    6,040                  ; ASCII -> SIXBIT
        idivi   2,6                    ; AC2=word index, AC3=token index
        addi    2,1                    ; word zero is the line header
        add     2,1                    ; AC2=&line[word]
        movei   4,5
        sub     4,3
        imuli   4,6                    ; shift=(5-token)*6
        movei   5,077
        lsh     5,0(4)                 ; mask
        move    7,0(2)
        setcm   5,5
        and     7,5
        lsh     6,0(4)
        ior     7,6
        movem   7,0(2)
        popj    17,

        .globl  proc_tty_line_get
proc_tty_line_get:
        idivi   2,6                    ; AC2=word index, AC3=token index
        addi    2,1
        add     2,1
        movei   4,5
        sub     4,3
        imuli   4,6
        move    5,0(2)
        movn    4,4
        lsh     5,0(4)
        andi    5,077
        addi    5,040                  ; SIXBIT -> ASCII
        move    1,5
        popj    17,

; int proc_tty_line_take(unsigned int tty)
; Drain one cooked byte, allocate the bounded line block before a hardware
; read, or return -3 when the caller must obtain another device byte.
proc_tty_line_take:
        cail    1,025
        jrst    kret_neg1
        move    2,proc_tty_records(1)
        move    3,2
        lsh     3,-031
        trnn    3,01                   ; RAW mode never allocates a line
        jrst    kret_neg3
        push    17,1                   ; tty survives allocator/helper calls
        pushj   17,proc_tty_line_ensure
        jumpe   1,proc_tty_line_take_bad
        move    7,1                    ; line base; byte helper preserves AC7
        move    6,0(7)                 ; line header
        trnn    6,0200000              ; READY
        jrst    proc_tty_line_take_more
        move    2,6
        andi    2,0377                 ; length
        move    3,6
        lsh     3,-010
        andi    3,0377                 ; drain index
        camge   3,2
        jrst    proc_tty_line_take_byte
        trne    6,0400000              ; submitted by CR/LF
        jrst    proc_tty_line_take_nl
        jrst    proc_tty_line_take_eof ; explicit EOF or exhausted partial
proc_tty_line_take_byte:
        movei   4,1(3)                 ; new drain index
        and     6,[-0177401]           ; clear old drain field
        move    5,4
        lsh     5,010
        ior     6,5
        movem   6,0(7)
        move    1,7
        move    2,3
        pushj   17,proc_tty_line_get
        move    2,6
        andi    2,0377                 ; length
        move    3,6
        lsh     3,-010
        andi    3,0377                 ; updated drain index
        came    3,2
        jrst    proc_tty_line_take_ret
        trne    6,0400000              ; newline remains for next read
        jrst    proc_tty_line_take_ret
        push    17,1                   ; final byte of partial ^D line
        move    1,-1(17)               ; tty below saved byte
        pushj   17,proc_tty_line_reset
        pop     17,1
proc_tty_line_take_ret:
        sub     17,kconst_1_1              ; discard saved tty
        popj    17,
proc_tty_line_take_nl:
        move    1,0(17)
        pushj   17,proc_tty_line_reset
        sub     17,kconst_1_1
        movei   1,012
        popj    17,
proc_tty_line_take_eof:
        move    1,0(17)
        pushj   17,proc_tty_line_reset
        sub     17,kconst_1_1
        jrst    kret_neg2
proc_tty_line_take_more:
        sub     17,kconst_1_1
        jrst    kret_neg3
proc_tty_line_take_bad:
        sub     17,kconst_1_1
        jrst    kret_neg1

/**
 * @brief Process one input byte through the resident canonical TTY editor.
 *
 * AC1=TTY, AC2=character. AC10..AC13 preserve editor state across echo/MM
 * helpers and satisfy the KCC callee-save ABI.
 */
proc_tty_canon_input:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    10,1                   ; tty
        move    11,2                   ; input byte
        cail    10,025
        jrst    proc_tty_canon_bad
        move    3,proc_tty_records(10)
        move    13,3
        lsh     13,-031
        andi    13,07                  ; mode bits
        trnn    13,01
        jrst    proc_tty_canon_raw
        andi    11,0177
        move    4,3
        lsh     4,-034                 ; suppress LF immediately after CR
        trnn    4,1
        jrst    proc_tty_canon_no_cr_pending
        move    4,[02000000000]
        andcam  4,proc_tty_records(10)
        caie    11,012
        jrst    proc_tty_canon_no_cr_pending
        jrst    proc_tty_canon_repeat
proc_tty_canon_no_cr_pending:
        move    1,10
        pushj   17,proc_tty_line_base_get
        move    12,1                   ; line block was allocated pre-read
        jumpe   12,proc_tty_canon_bad
        move    6,0(12)                ; header
        trne    6,0200000              ; already submitted
        jrst    proc_tty_canon_take
        move    7,6
        andi    7,0377                 ; current length
        caie    11,015                 ; CR
        jrst    proc_tty_canon_check_lf
        move    4,[02000000000]
        iorm    4,proc_tty_records(10) ; remember CR so a following LF vanishes
        jrst    proc_tty_canon_newline
proc_tty_canon_check_lf:
        cain    11,012                 ; bare LF is also a newline
        jrst    proc_tty_canon_newline
        caie    11,010                 ; BS
        cain    11,0177                ; DEL
        jrst    proc_tty_canon_erase
        caie    11,025                 ; ^U
        jrst    proc_tty_canon_not_kill
        trnn    13,02                  ; echo each erased column when enabled
        jrst    proc_tty_canon_kill_store
        push    17,7
proc_tty_canon_kill_loop:
        skipn   0(17)
        jrst    proc_tty_canon_kill_done
        sos     0(17)
        move    1,10
        pushj   17,proc_tty_echo_erase
        jrst    proc_tty_canon_kill_loop
proc_tty_canon_kill_done:
        sub     17,kconst_1_1
proc_tty_canon_kill_store:
        and     6,[-0400]              ; length = 0
        movem   6,0(12)
        jrst    proc_tty_canon_repeat
proc_tty_canon_not_kill:
        caie    11,004                 ; ^D
        jrst    proc_tty_canon_not_eof
        jumpn   7,proc_tty_canon_partial_eof
        tlo     6,1                    ; EOF is bit 18, low bit of left half
        iori    6,0200000              ; READY for an empty ^D
        jrst    proc_tty_canon_eof_store
proc_tty_canon_partial_eof:
        iori    6,0200000              ; submit nonempty partial line
proc_tty_canon_eof_store:
        movem   6,0(12)
        jrst    proc_tty_canon_take
proc_tty_canon_not_eof:
        cail    7,0170                 ; 120-byte bounded canonical line
        jrst    proc_tty_canon_full
        ; Cooked DAIMOS input is systemwide SIXBIT text.  Fold lowercase
        ; before both storage and echo, then reject printable non-SIXBIT.
        caige   11,0141                ; 'a'
        jrst    proc_tty_canon_sixbit_range
        caile   11,0172                ; 'z'
        jrst    proc_tty_canon_sixbit_range
        subi    11,040                 ; ASCII lowercase -> uppercase
proc_tty_canon_sixbit_range:
        caige   11,040
        jrst    proc_tty_canon_repeat
        caile   11,0137                ; '_' is highest ASCII SIXBIT glyph
        jrst    proc_tty_canon_full    ; bell + retry
        move    4,6
        and     4,[-0400]
        movei   5,1(7)
        ior     4,5
        movem   4,0(12)                ; commit length before helper call
        move    1,12
        move    2,7
        move    3,11
        pushj   17,proc_tty_line_put
        trnn    13,02
        jrst    proc_tty_canon_repeat
        caige   11,040
        jrst    proc_tty_canon_repeat
        caile   11,0176
        jrst    proc_tty_canon_repeat
        move    1,10
        move    2,11
        pushj   17,proc_tty_echo
        jrst    proc_tty_canon_repeat
proc_tty_canon_newline:
        iori    6,0600000              ; READY|NL
        movem   6,0(12)
        trnn    13,02
        jrst    proc_tty_canon_take
        move    1,10
        movei   2,015
        pushj   17,proc_tty_echo
        move    1,10
        movei   2,012
        pushj   17,proc_tty_echo
        jrst    proc_tty_canon_take
proc_tty_canon_erase:
        jumpe   7,proc_tty_canon_repeat
        subi    7,1
        and     6,[-0400]
        ior     6,7
        movem   6,0(12)
        trnn    13,02
        jrst    proc_tty_canon_repeat
        move    1,10
        pushj   17,proc_tty_echo_erase
        jrst    proc_tty_canon_repeat
proc_tty_canon_full:
        trnn    13,02
        jrst    proc_tty_canon_repeat
        move    1,10
        movei   2,007
        pushj   17,proc_tty_echo
proc_tty_canon_repeat:
        movni   1,3
        jrst    proc_tty_canon_return
proc_tty_canon_raw:
        move    1,11
        andi    1,0177
        jrst    proc_tty_canon_return
proc_tty_canon_bad:
        seto    1,
        jrst    proc_tty_canon_return
proc_tty_canon_take:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        ; Submission is non-draining.  READCHAR retries through line_take;
        ; READ_WORDS consumes the complete packed READY line directly.
        jrst    kret_neg3
proc_tty_canon_return:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

/** @brief Enter or resume a blocking read on the current controlling TTY. */
proc_tty_read_enter:
proc_tty_read_enter_retry:
        move    5,proc_current_ptr
        hlrz    6,(5)
        hlrz    1,PROC_FDCTL_OFFSET(6)
        lsh     1,-014                 ; packed TTY state
        jumpe   1,kret_zero       ; NO_TTY -> historical CTY
        subi    1,2                    ; attached state -> tty id
        jumpl   1,kret_neg1        ; DETACHED becomes -1
        cail    1,025
        jrst    kret_neg1
        move    2,proc_tty_records(1)
        move    3,2
        andi    3,0377                 ; record session
        ldb     4,[POINT 8,PROC_FDCTL_OFFSET(6),32]
        came    3,4
        jrst    kret_neg1
        ldb     3,[POINT 8,proc_tty_records(1),27] ; foreground pgrp
        hrrz    4,(5)
        andi    4,0377                 ; current pgrp
        camn    3,4
        popj    17,                    ; AC1 still tty id
        move    1,4                    ; stop the current background pgrp
        movei   2,3                    ; SYS_EVENT_TSTP
        movei   3,1                    ; group delivery
        pushj   17,proc_event_send
        jumpn   1,kret_neg1
        jrst    proc_tty_read_enter_retry

/** @brief Route one hardware input byte to the owning foreground process group. */
proc_tty_input:
        move    4,1                    ; tty
        move    5,2                    ; character
        cail    4,025
        jrst    kret_neg1
        move    7,proc_current_ptr
        hlrz    2,(7)                  ; u-area; keep callee-saved AC10+ intact
        hlrz    3,PROC_FDCTL_OFFSET(2)
        lsh     3,-014                 ; TTY state
        jumpn   3,proc_tty_input_attached
        jumpn   4,kret_neg1       ; NO_TTY accepts CTY only
        move    1,5
        andi    1,0177
        popj    17,
proc_tty_input_attached:
        subi    3,2
        came    3,4
        jrst    kret_neg1
        move    1,proc_tty_records(4)  ; retain record until mode extraction
        move    3,1
        andi    3,0377
        ldb     6,[POINT 8,PROC_FDCTL_OFFSET(2),32]
        came    3,6
        jrst    kret_neg1
        ldb     6,[POINT 8,proc_tty_records(4),27] ; foreground pgrp
        jumpe   6,kret_neg1
        hrrz    3,(7)
        andi    3,0377
        came    6,3
        jrst    kret_neg1
        andi    5,0177
        move    3,1
        lsh     3,-031
        andi    3,07                   ; canonical/echo/signals mode
        trnn    3,04                   ; SIGNALS disabled: do not consume ^C/^Z
        jrst    proc_tty_input_mode
        caie    5,3                    ; ^C
        jrst    proc_tty_input_tstp
        move    1,6
        movei   2,0                    ; SYS_EVENT_INT
        movei   3,1
        pushj   17,proc_event_send
        jrst    kret_neg3         ; consumed: caller retries
proc_tty_input_tstp:
        caie    5,032                  ; ^Z
        jrst    proc_tty_input_mode
        move    1,6
        movei   2,3                    ; SYS_EVENT_TSTP
        movei   3,1
        pushj   17,proc_event_send
        jumpn   1,kret_neg1
        jrst    kret_neg3         ; consumed: caller retries
proc_tty_input_mode:
        trnn    3,01                   ; RAW: return byte directly
        jrst    proc_tty_input_char
        move    1,4                    ; canonical helper(tty, ch)
        move    2,5
        jrst    proc_tty_canon_input
proc_tty_input_char:
        move    1,5
        popj    17,

/** @brief Emit one packed TTY output character after ownership validation. */
proc_tty_output:
        move    4,1                    ; character
        move    6,proc_current_ptr
        hlrz    7,(6)
        hlrz    1,PROC_FDCTL_OFFSET(7)
        lsh     1,-014
        jumpe   1,proc_tty_output_pack_cty
        subi    1,2                    ; tty id
        jumpl   1,kret_neg1        ; DETACHED becomes -1
        cail    1,025
        jrst    kret_neg1
        move    3,proc_tty_records(1)
        andi    3,0377
        ldb     5,[POINT 8,PROC_FDCTL_OFFSET(7),32]
        came    3,5
        jrst    kret_neg1
        lsh     1,010
        andi    4,0377
        ior     1,4
        popj    17,
proc_tty_output_pack_cty:
        move    1,4
        andi    1,0377
        popj    17,

; int proc_tty_output_route_get(unsigned int tty)
; Return the effective output sink.  Zero route bits mean native tty->tty.
        .globl  proc_tty_output_route_get
proc_tty_output_route_get:
        cail    1,025
        jrst    kret_neg1
        ldb     2,[POINT 5,proc_tty_records(1),6]
        jumpe   2,proc_tty_output_route_native
        subi    2,1
        move    1,2
        popj    17,
proc_tty_output_route_native:
        popj    17,

; int proc_tty_output_route_set(unsigned int tty, unsigned int sink)
; sink 077 restores native routing; otherwise 0..025 selects CTY/DCS/GE/DPY.
        .globl  proc_tty_output_route_set
proc_tty_output_route_set:
        cail    1,025
        jrst    kret_neg1
        move    3,2
        cain    3,077
        jrst    proc_tty_output_route_clear
        caile   3,025
        jrst    kret_neg1
        aoja    3,proc_tty_output_route_store
proc_tty_output_route_clear:
        setz    3,
proc_tty_output_route_store:
        dpb     3,[POINT 5,proc_tty_records(1),6]
        move    1,2
        popj    17,

; int proc_tty_pending_take(unsigned int tty)
proc_tty_pending_take:
        cail    1,025
        jrst    kret_neg1
        move    2,proc_tty_records(1)
        move    3,2
        lsh     3,-020
        andi    3,0777
        jumpe   3,kret_neg1
        and     2,[777600177777]       ; clear pending byte field
        movem   2,proc_tty_records(1)
        move    1,3
        subi    1,1
        popj    17,

; int proc_tty_pending_store(unsigned int tty, unsigned int ch)
proc_tty_pending_store:
        cail    1,025
        jrst    kret_neg1
        caile   2,0377
        jrst    kret_neg1
        move    4,proc_tty_records(1)
        move    3,4
        lsh     3,-020
        andi    3,0777
        jumpn   3,kret_neg1
        addi    2,1                    ; validated byte 0..0377 -> marker 1..0400
        lsh     2,020
        ior     4,2
        movem   4,proc_tty_records(1)
        jrst    kret_zero

/**
 * @brief Sleep the current process for AC1 monotonic 60 Hz ticks.
 *
 * The deadline lives in sched RH while the process is asleep. RH bit 17 tags
 * it as a timer rather than a real kernel event pointer; permanent kernel
 * addresses live below that range. proc_timer_next keeps only the nearest
 * deadline, so ordinary clock ticks do not scan the process table.
 */
proc_sleep_ticks:
        hrrz    1,1
        jumpe   1,kret_zero
        move    4,1                    ; runq_remove preserves AC1..AC4
        move    1,proc_current_slot
        jumpe   1,kret_neg1
        pushj   17,proc_runq_remove
        move    7,4                    ; requested ticks = new deadline distance
        pushj   17,proc_slot_ptr       ; AC1 is still current slot
        move    3,1                    ; current descriptor
        hrrz    1,proc_timer_clock
        andi    1,PROC_TIMER_CLOCK_MASK
        add     4,1
        andi    4,PROC_TIMER_CLOCK_MASK ; wrapped 17-bit deadline
        move    5,2(3)
        tlz     5,PROC_WAIT_LH_MASK
        tlo     5,PROC_WAIT_EVENT_LH
        tlz     5,PROC_STATE_LH_MASK
        tlo     5,PROC_STATE_SLEEP
        movem   5,2(3)
        move    6,4
        ori     6,PROC_TIMER_TAG_RH
        hrrm    6,2(3)

        skipn   5,proc_timer_next
        jrst    proc_sleep_set_next
        tlne    5,PROC_TIMER_DUE_LH
        jrst    proc_sleep_resched
        hrrz    6,5                    ; old deadline distance
        sub     6,1
        andi    6,PROC_TIMER_CLOCK_MASK
        camge   7,6                    ; requested ticks are new distance
        jrst    proc_sleep_set_next
        jrst    proc_sleep_resched
proc_sleep_set_next:
        move    5,4
        hrli    5,PROC_TIMER_ACTIVE_LH
        movem   5,proc_timer_next
proc_sleep_resched:
        setom   proc_sched_kick
        cono    0004,004002
        jrst    kret_zero

/**
 * @brief Sleep on a kernel event, optionally allowing ALRM interruption.
 * @param AC1 Event-word address.
 *
 * proc_wait_event() is noninterruptible. proc_wait_event_intr() rejects an
 * already-pending ALRM and returns -1 when ALRM interrupts the user-visible
 * wait. Both paths remove the current process from the run queue before PI6.
 */
proc_wait_event_intr:
        skipe   (1)
        jrst    kret_zero
        skipn   proc_current_slot
        jrst    proc_wait_event
        move    2,file_table
        move    2,-2(2)                 ; packed process control word
        tlne    2,0100                  ; pending ALRM (event 5)
        jrst    kret_neg1
        movsi   4,PROC_WAIT_INTR_LH
        jrst    proc_wait_event_common

proc_wait_event:
        movsi   4,PROC_WAIT_EVENT_LH
proc_wait_event_common:
        skipe   (1)
        jrst    kret_zero
        skipn   proc_current_slot
        jrst    proc_wait_boot
        push    17,1                    ; preserve event pointer
        move    1,proc_current_slot
        pushj   17,proc_runq_remove
        pop     17,1
        move    2,proc_current_ptr
        move    3,2(2)                 ; packed scheduler word
        tlz     3,PROC_WAIT_LH_MASK
        ior     3,4
        tlz     3,PROC_STATE_LH_MASK
        tlo     3,PROC_STATE_SLEEP
        hrr     3,1
        movem   3,2(2)
        skipe   (1)
        jrst    proc_wait_raced
        setom   proc_sched_kick
        cono    0004,004002             ; software request at PI level 6
        tlne    4,040000                ; INTR class, not internal EVENT
        jrst    proc_wait_intr_return
        jrst    kret_zero
proc_wait_boot:
        skipn   (1)
        jrst    proc_wait_boot
        jrst    kret_zero
proc_wait_raced:
        move    3,2(2)
        tlz     3,PROC_WAIT_LH_MASK
        tlz     3,PROC_STATE_LH_MASK
        tlo     3,PROC_STATE_RUN
        hllz    3,3
        movem   3,2(2)
        move    1,proc_current_slot
        pushj   17,proc_runq_add
        jrst    kret_zero

proc_wait_intr_return:
        move    2,file_table
        move    2,-2(2)
        tlne    2,0100                  ; ALRM remained pending across sleep
        jrst    kret_neg1
        jrst    kret_zero


/**
 * @brief Request an immediate software PI6 reschedule.
 *
 * Call after changing current-process state, for example job-control TSTP.
 * PI6 saves the executive continuation and resumes it once runnable again.
 */
proc_sched_resched_current:
        setom   proc_sched_kick
        cono    0004,004002
        popj    17,

; int proc_wait_child(void)
; WAIT scans while executive code is non-preemptible.  If live children exist
; but none is reportable, arm a child wait and request PI6 immediately.
proc_wait_child:
        skipn   proc_current_slot
        jrst    kret_neg1
        move    2,file_table
        move    2,-2(2)
        tlne    2,0100                  ; ALRM interrupts user WAIT
        jrst    kret_neg1
        move    1,proc_current_slot
        pushj   17,proc_runq_remove
        move    1,proc_current_ptr
        move    2,1
        move    3,2(2)
        tlz     3,PROC_WAIT_LH_MASK
        tlo     3,PROC_WAIT_CHILD_LH
        tlz     3,PROC_STATE_LH_MASK
        tlo     3,PROC_STATE_SLEEP
        hllz    3,3
        movem   3,2(2)
        setom   proc_sched_kick
        cono    0004,004002
        jrst    proc_wait_intr_return

/**
 * @brief Wake every process sleeping on one kernel event word.
 * @param AC1 Event-word address.
 *
 * PI-safe. Swapped sleepers are included because logical sleep state remains
 * resident in the compact process descriptor.
 */
proc_wakeup_event:
        push    17,0
        push    17,2
        push    17,3
        push    17,4
        push    17,5
        push    17,6
        movei   3,1
        move    2,proc_table
        addi    2,PROC_WORDS
proc_wakeup_scan:
        caml    3,proc_high_slot
        jrst    proc_wakeup_done
        hrrz    4,2(2)
        came    1,4
        jrst    proc_wakeup_next
        move    4,2(2)
        tlz     4,PROC_WAIT_LH_MASK
        hllz    4,4                    ; clear wait channel
        tlz     4,0100000              ; SLEEP->RUN, STOP remains STOP
        tlz     4,PROC_CPU_SLEEP_LH_MASK
        movem   4,2(2)
        push    17,1                    ; preserve event pointer
        move    1,3
        pushj   17,proc_runq_add
        pop     17,1
proc_wakeup_after_runq:
        ; The RT owner regains preference immediately when its wait completes.
        ; Idle wakeups likewise request PI6 instead of waiting for a clock tick.
        camn    3,proc_rt_owner
        jrst    proc_wakeup_kick
        skipn   proc_current_slot
        jrst    proc_wakeup_kick
        jrst    proc_wakeup_next
proc_wakeup_kick:
        setom   proc_sched_kick
        cono    0004,004002
proc_wakeup_next:
        addi    2,PROC_WORDS
        aoja    3,proc_wakeup_scan
proc_wakeup_done:
        pop     17,6
        pop     17,5
        pop     17,4
        pop     17,3
        pop     17,2
        pop     17,0
        popj    17,

;. Test-only process-private kernel stack watermarking.  The untouched
; canary in each stack word is that word's own physical address.  Because the
; PDP-6 pushdown stack grows upward, the first untouched word terminates the
; scan.  This code and state disappear completely from production builds.
.if PROC_STACK_WATERMARK
        .globl  proc_stack_highwater
        .globl  kernel_idle_stack_highwater
proc_stack_watermark_scan:
        move    2,1
        addi    2,PROC_USTACK_BASE+1
        movei   3,1
proc_stack_watermark_loop:
        camn    2,(2)
        jrst    proc_stack_watermark_done
        addi    2,1
        addi    3,1
        caig   3,PROC_KSTACK_WORDS
        jrst    proc_stack_watermark_loop
proc_stack_watermark_done:
        camle   3,proc_stack_highwater
        movem   3,proc_stack_highwater
        popj    17,

; Test-only watermark for the permanent idle/exit stack.  Late KINIT no
; longer executes on this stack, so this measures runtime scheduler/exit use.
kernel_idle_stack_watermark_scan:
        move    1,mach_kernel_stack_base
        move    2,1
        addi    2,1
        movei   3,1
kernel_idle_stack_watermark_loop:
        camn    2,(2)
        jrst    kernel_idle_stack_watermark_done
        addi    2,1
        addi    3,1
        caig   3,KERNEL_IDLE_STACK_WORDS
        jrst    kernel_idle_stack_watermark_loop
kernel_idle_stack_watermark_done:
        camle   3,kernel_idle_stack_highwater
        movem   3,kernel_idle_stack_highwater
        popj    17,

.endif

; Save a user-origin PI6 context.  AC1..AC3 and AC17 were already preserved by
; the common PI entry path; all other user ACs are still live here.
proc_save_user:
        move    1,proc_current_slot
        pushj   17,proc_uarea_slot
.if PROC_STACK_WATERMARK
        pushj   17,proc_stack_watermark_scan
.endif
        move    2,1                    ; common save area = user AC0..AC17
        movei   3,proc_save_user_tail
        jrst    proc_save_common

; Save a sleeping executive context.  User-preemption and sleeping-executive
; contexts cannot coexist for one process, so both states share AC0..AC17.
; The private kernel stack already carries the syscall's eventual user return.
proc_save_kernel:
        move    1,proc_current_slot
        pushj   17,proc_uarea_slot
.if PROC_STACK_WATERMARK
        pushj   17,proc_stack_watermark_scan
.endif
        move    2,1
        movei   3,proc_save_kernel_tail

; AC2 = destination AC0 base, AC3 = continuation.  PI entry already preserved
; AC1..AC3 and AC17 in low core, so those registers are free as copy scratch.
proc_save_common:
        movem   0,0(2)
        movei   0,4(2)
        hrli    0,4
        blt     0,016(2)              ; save contiguous AC4..AC16
        move    1,000032
        movem   1,1(2)
        move    1,000033
        movem   1,2(2)
        move    1,000055
        movem   1,3(2)
        move    1,pdp10_pi_sp_save+012
        movem   1,017(2)
        jrst    0(3)

proc_save_user_tail:
        move    1,pdp10_pi_level6
        movem   1,CTX_PC(2)
        move    1,mach_kernel_sp
        movem   1,CTX_KSP(2)
        popj    17,

proc_save_kernel_tail:
        move    1,pdp10_pi_level6
        movem   1,CTX_PC(2)
        move    1,mach_user_sp
        movem   1,CTX_M_USER_SP(2)
        move    1,mach_syscall_save
        movem   1,CTX_M_SYSCALL_SAVE(2)
        move    1,mach_kernel_sp
        movem   1,CTX_KSP(2)
        popj    17,

; Restore user ACs and PI return state for proc_current_slot.
proc_restore_user:
        ; proc_sched_restore_selected already supplies the selected u-area.
        move    2,1                    ; user AC-image base
        movei   3,proc_restore_user_tail
        jrst    proc_restore_pi_common

; Restore a previously sleeping executive context and its syscall globals.
proc_restore_kernel:
        move    2,1
        movei   3,proc_restore_kernel_tail

; AC2 = saved AC-image base, AC3 = context-specific continuation.  PI6 saved
; AC1..AC3/AC17 in low core before either path ran, so their image offsets are
; identical for user and sleeping-executive contexts.
proc_restore_pi_common:
        move    1,1(2)
        movem   1,000032
        move    1,2(2)
        movem   1,000033
        move    1,3(2)
        movem   1,000055
        move    1,017(2)
        movem   1,pdp10_pi_sp_save+012
        jrst    0(3)

proc_restore_user_tail:
        move    1,CTX_PC(2)
        movem   1,pdp10_pi_level6
        move    1,CTX_KSP(2)
        movem   1,mach_kernel_sp
        pushj   17,vm_activate_current
        move    1,proc_current_slot
        pushj   17,proc_uarea_slot
        move    2,1
        movei   3,PROC_FILE_TABLE_OFFSET(1)
        movem   3,file_table
        jrst    proc_restore_common

proc_restore_kernel_tail:
        move    1,CTX_PC(2)
        movem   1,pdp10_pi_level6
        setzm   CTX_PC(2)
        move    1,CTX_M_USER_SP(2)
        movem   1,mach_user_sp
        move    1,CTX_M_SYSCALL_SAVE(2)
        movem   1,mach_syscall_save
        move    1,CTX_KSP(2)
        movem   1,mach_kernel_sp
        pushj   17,vm_activate_current
        move    1,proc_current_slot
        pushj   17,proc_uarea_slot
        move    2,1
        movei   3,PROC_FILE_TABLE_OFFSET(1)
        movem   3,file_table

; AC2 points at the saved AC0 image for either a user or sleeping-kernel
; context.  AC1..AC3 are PI-saved scratch and are restored by the PI return
; path, so only the live unsaved set is copied here.
proc_restore_common:
        movei   1,4
        hrli    1,4(2)
        blt     1,016                  ; restore contiguous AC4..AC16
        move    0,0(2)
        popj    17,

; Switch to the slot-0 executive idle loop when no resident user process runs.
proc_restore_idle:
        setzm   proc_current_slot
        setzm   proc_current_ptr
        setzm   file_table
        movei   1,proc_idle_loop
        movem   1,pdp10_pi_level6
        move    1,mach_kernel_stack_base
        movem   1,pdp10_pi_sp_save+012
        movem   1,mach_kernel_sp
        popj    17,

; Software PI6 reschedule requested by proc_wait_event.  This is not a clock
; tick: save the sleeping executive continuation and select a runnable process
; without charging recent CPU or aging sleepers.
proc_sched_pi_resched:
        ; PI6 may preempt the lower-priority PI7 device path.  Never save or
        ; replace an executive continuation while PI7 is held: that
        ; continuation is the nested PI7 frame, not an ordinary process
        ; kernel context.  Abandoning it would resume user code with PI7 still
        ; held and permanently mask subsequent PI7 requests.
        coni    0004,1
        trne    1,000400               ; CONI PI: PIH level 7 = bit 0400
        popj    17,
        move    1,proc_current_slot
        jumpn   1,proc_sched_resched_save
        ; Slot 0 is also used while KINIT still owns the bootstrap executive
        ; stack.  Only the permanent idle loop is a schedulable slot-0
        ; context; switching away from any other executive continuation would
        ; strand clk_pi_service's save frame on the old stack.
        hrrz    1,pdp10_pi_level6
        caige   1,proc_idle_loop
        popj    17,
        cail    1,proc_idle_loop_end
        popj    17,
        move    1,proc_sched_cursor
        trne    1,0400                  ; slot-0 swap service owns idle stack
        popj    17,
        jrst    proc_sched_resched_choose
proc_sched_resched_save:
        pushj   17,proc_save_kernel
proc_sched_resched_choose:
        pushj   17,proc_sched_resched_select
        jrst    proc_sched_restore_selected

; Called once per qualified line-clock PI6 after the hardware clock flag is
; cleared.  User code is preemptible.  Ordinary executive code is not; only a
; process which explicitly sleeps can be switched while in the kernel.
proc_sched_pi_tick:
        ; Maintain the compact 17-bit monotonic epoch used by SYS_EXT_SLEEP.
        ; proc_timer_next carries ACTIVE in LH bit 400000, DUE in LH bit
        ; 200000, and the next deadline in RH.  The process table is scanned
        ; only after the exact frontier tick becomes due.
        aos     1,proc_timer_clock
        skipn   2,proc_timer_next
        jrst    proc_sched_timer_done
        tlne    2,PROC_TIMER_DUE_LH
        jrst    proc_sched_timer_done
        xor     1,2                    ; compare only the low 17 deadline bits
        trne    1,PROC_TIMER_CLOCK_MASK
        jrst    proc_sched_timer_done
        movsi   2,PROC_TIMER_DUE_LH
        iorm    2,proc_timer_next
proc_sched_timer_done:
        ; A real PI6 clock tick may nest inside PI7.  Timekeeping above is
        ; safe, but a process switch is not: proc_save_kernel would save the
        ; nested PI7 continuation and proc_restore_* could return directly to
        ; user mode, leaving PI7 set in the hardware hold register forever.
        ; Defer only the scheduling decision; the next 60 Hz tick retries it
        ; after PI7 has dismissed normally.
        coni    0004,1
        trne    1,000400               ; CONI PI: PIH level 7 = bit 0400
        popj    17,
        skipn   proc_current_slot
        jrst    proc_sched_tick_idle_check
        jrst    proc_sched_tick_ready
proc_sched_tick_idle_check:
        ; proc_current_slot == 0 is ambiguous during bootstrap.  Permit a
        ; slot-0 context switch only when PI6 interrupted the real idle loop.
        ; KINIT and other bootstrap executive code must run to completion on
        ; the stack on which clk_pi_service saved AC2/AC3.
        hrrz    1,pdp10_pi_level6
        caige   1,proc_idle_loop
        popj    17,
        cail    1,proc_idle_loop_end
        popj    17,
.if PROC_STACK_WATERMARK
        pushj   17,kernel_idle_stack_watermark_scan
.endif
proc_sched_tick_idle:
        move    1,proc_timer_next
        tlne    1,PROC_TIMER_DUE_LH
        jrst    proc_sched_select
        move    1,proc_sched_cursor
        trne    1,0400                  ; do not preempt slot-0 swap I/O
        popj    17,
proc_sched_tick_ready:
        skipn   proc_sched_cursor
        popj    17,
        move    1,pdp10_pi_level6
        tlnn    1,010000
        jrst    proc_sched_exec_tick

        ; AC2 is already saved by clk_pi_service, so it is safe to use as the
        ; one-word quantum counter without saving the full user context.
        aos     2,proc_sched_deferred_ticks
        move    2,proc_rt_owner
        camn    2,proc_current_slot
        popj    17,                     ; RT owner: clock runs, no quantum switch
        move    2,proc_timer_next
        tlne    2,PROC_TIMER_DUE_LH
        jrst    proc_sched_timer_quantum
        move    2,proc_sched_deferred_ticks
        caige   2,PROC_SCHED_QUANTUM_TICKS
        jrst    proc_sched_tick_fast_return
        jrst    proc_sched_timer_save_user
proc_sched_timer_quantum:
        movei   2,PROC_SCHED_QUANTUM_TICKS
        movem   2,proc_sched_deferred_ticks
proc_sched_timer_save_user:
        pushj   17,proc_save_user
        jrst    proc_sched_select
proc_sched_tick_fast_return:
        popj    17,
proc_sched_exec_tick:
        move    1,proc_current_slot
        jumpe   1,proc_sched_select
        pushj   17,proc_slot_ptr
        hlrz    2,2(1)
        andi    2,PROC_STATE_LH_MASK
        caie    2,PROC_STATE_RUN
        jrst    proc_sched_exec_switch
        popj    17,
proc_sched_exec_switch:
        ; SLEEP and STOP both require a switch.  In particular, a real clock
        ; tick can coincide with a software reschedule request after TSTP;
        ; clk_pi_service consumes that request before arriving here.
        pushj   17,proc_save_kernel
proc_sched_select:
        pushj   17,proc_timer_service
        pushj   17,proc_sched_tick_select
proc_sched_restore_selected:
        movem   1,proc_current_slot
        jumpe   1,proc_restore_idle
        pushj   17,proc_slot_ptr
        movem   1,proc_current_ptr
        hlrz    1,(1)
        move    2,CTX_PC(1)
        tlne    2,010000               ; saved PI return is user mode
        jrst    proc_restore_user
        jrst    proc_restore_kernel

; Service the due timer frontier after the interrupted process context has
; already been saved (or while slot 0 is idle).  AC0..AC7 are therefore free
; scratch here.  Expired stopped jobs lose only their timer wait; CONT still
; decides when they become runnable.
proc_timer_service:
        move    7,proc_timer_next
        tlne    7,PROC_TIMER_DUE_LH
        jrst    proc_timer_service_active
        popj    17,
proc_timer_service_active:
        hrrz    7,7                    ; frontier deadline
        hrrz    6,proc_timer_clock
        andi    6,PROC_TIMER_CLOCK_MASK
        sub     6,7
        andi    6,PROC_TIMER_CLOCK_MASK ; elapsed ticks since frontier
        setz    4,                     ; best future offset from frontier
        movei   1,1
        move    2,proc_table
        addi    2,PROC_WORDS
proc_timer_service_loop:
        caml    1,proc_high_slot
        jrst    proc_timer_service_done
        hrrz    3,2(2)
        trnn    3,PROC_TIMER_TAG_RH
        jrst    proc_timer_service_next
        andi    3,PROC_TIMER_CLOCK_MASK ; process deadline
        move    0,3
        sub     0,7
        andi    0,PROC_TIMER_CLOCK_MASK ; offset from old frontier
        camle   0,6                    ; deadline passed since frontier?
        jrst    proc_timer_service_future

        ; A timer-tagged process is either sleeping or job-control stopped.
        ; SLEEP(3) becomes RUN(2) by clearing state bit 0100000; STOP(6) keeps
        ; its state and merely loses the expired timer wait.
        move    3,2(2)
        tlz     3,PROC_WAIT_LH_MASK
        hllz    3,3                    ; clear deadline/wait channel
        tlz     3,PROC_CPU_SLEEP_LH_MASK
        tlnn    3,0400000              ; high state bit clear only for SLEEP
        tlz     3,0100000              ; SLEEP -> RUN
        movem   3,2(2)
        tlne    3,0400000              ; STOP remains off the run queue
        jrst    proc_timer_service_next
        pushj   17,proc_runq_add       ; preserves AC6/AC7 frontier state
        jrst    proc_timer_service_next

proc_timer_service_future:
        jumpe   4,proc_timer_service_best
        camge   0,4                    ; choose nearest offset from frontier
        jrst    proc_timer_service_best
        jrst    proc_timer_service_next
proc_timer_service_best:
        move    4,0
proc_timer_service_next:
        addi    2,PROC_WORDS
        aoja    1,proc_timer_service_loop

proc_timer_service_done:
        jumpe   4,proc_timer_service_none
        move    1,7
        add     1,4                    ; next absolute deadline
        andi    1,PROC_TIMER_CLOCK_MASK
        hrli    1,PROC_TIMER_ACTIVE_LH
        movem   1,proc_timer_next
        popj    17,
proc_timer_service_none:
        setzm   proc_timer_next
        popj    17,

; Choose the best noncurrent resident process for swap-out.
; This cold memory-pressure path mirrors proc.c scoring but keeps loop state in
; callee-saved AC10..AC16 rather than KCC's eleven-word stack frame.
; AC10 exclude slot, AC11 scan slot, AC12 descriptor, AC13 best slot,
; AC14 best score, AC15 sched LH, AC16 candidate score.
proc_swap_victim:
        add     17,kconst_7_7
        movei   0,-6(17)
        hrli    0,010
        blt     0,(17)                  ; save AC10..AC16
        move    010,1                   ; excluded owner
        movei   011,1
        move    012,proc_table
        addi    012,PROC_WORDS
        setz    013,                    ; no best slot yet
        setz    014,                    ; best score

proc_swap_victim_loop:
        caml    011,proc_high_slot
        jrst    proc_swap_victim_done
        camn    011,010
        jrst    proc_swap_victim_next
        camn    011,proc_current_slot
        jrst    proc_swap_victim_next

        hlrz    015,2(012)              ; packed scheduling LH
        move    016,015
        andi    016,PROC_STATE_LH_MASK
        caie    016,PROC_STATE_SLEEP
        cain    016,PROC_STATE_STOP
        jrst    proc_swap_victim_state_ok
        caie    016,PROC_STATE_RUN
        jrst    proc_swap_victim_next

proc_swap_victim_state_ok:
        move    1,(012)                 ; meta
        trne    1,PROC_TRANSITION_RH
        jrst    proc_swap_victim_next
        trnn    1,0400000               ; PROC_F_UAREA in meta RH
        jrst    proc_swap_victim_next
        hlrz    1,1                     ; stable u-area base
        move    2,PROC_FDCTL_OFFSET(1)
        trne    2,PROC_USER_MAP_BIT
        jrst    proc_swap_victim_next
        move    1,proc_swap_records
        add     1,011
        skipn   (1)
        jrst    proc_swap_victim_next
        hrrz    1,1(012)                ; resident VM relocation base
        jumpe   1,proc_swap_victim_next
        pushj   17,mm_is_pinned
        jumpn   1,proc_swap_victim_next

        ; Sleeping/stopped jobs strongly outrank runnable jobs.  Preserve the
        ; exact C policy score so victim choice does not change.
        move    1,015
        andi    1,PROC_STATE_LH_MASK
        caie    1,PROC_STATE_SLEEP
        cain    1,PROC_STATE_STOP
        jrst    proc_swap_victim_sleep_score

        move    016,015                 ; recent CPU penalty
        lsh     016,-6
        andi    016,PROC_CPU_LH_MASK
        move    1,015                   ; biased nice
        andi    1,PROC_NICE_LH_MASK
        add     016,1
        addi    016,01000
        caig    1,PROC_NICE_BIAS
        jrst    proc_swap_victim_consider
        subi    1,PROC_NICE_BIAS
        lsh     1,3                     ; *010
        add     016,1
        addi    016,02000
        jrst    proc_swap_victim_consider

proc_swap_victim_sleep_score:
        move    016,015
        lsh     016,-012                ; sleep age is LH bits 10..12
        andi    016,PROC_SLEEP_LH_MASK
        lsh     016,6                   ; *0100
        addi    016,04000
        move    1,015
        lsh     1,-015                  ; wait class is LH bits 13..14
        andi    1,PROC_WAIT_LH_FIELD_MASK
        jumpe   1,proc_swap_victim_consider
        addi    016,040

proc_swap_victim_consider:
        jumpe   013,proc_swap_victim_take
        camg    016,014
        jrst    proc_swap_victim_next
proc_swap_victim_take:
        move    013,011
        move    014,016

proc_swap_victim_next:
        addi    012,PROC_WORDS
        aoja    011,proc_swap_victim_loop

proc_swap_victim_done:
        move    1,013
        jumpn   1,proc_swap_victim_return
        seto    1,
proc_swap_victim_return:
        movei   0,-6(17)
        hrl     0,0
        hrri    0,010
        blt     0,016                   ; restore AC10..AC16
        sub     17,kconst_7_7
        popj    17,

proc_idle_loop:
        ; Disk-backed swap-in must never run in PI context.  The scheduler
        ; marks the exact swapped winner in proc_sched_cursor.  Slot 0 services
        ; only that request, then asks PI6 to perform the normal context switch.
        move    1,proc_sched_cursor
        trnn    1,0400
        jrst    proc_idle_wait
        pushj   17,proc_swap_service_one
        setom   proc_sched_kick
        cono    0004,004002
proc_idle_wait:
        jrst    proc_idle_loop
proc_idle_loop_end:

        .bss
proc_timer_clock:
        .block 1
proc_timer_next:
        .block 1
.if KINIT_STACK_WATERMARK
        .globl  kinit_stack_highwater
.endif
.if PROC_STACK_WATERMARK
proc_stack_highwater:
        .long   0
kernel_idle_stack_highwater:
        .long   0
.endif
.if KINIT_STACK_WATERMARK
kinit_stack_highwater:
        .long   0
.endif

mach_kernel_stack_base:
        .block  1

        .data
        .globl  proc_comm_words
proc_comm_words:
        .word   0636741606045          ; SIXBIT /SWAPPE/
        .word   0515651640000          ; SIXBIT /INIT  /
        .word   0656345620000          ; SIXBIT /USER  /
