; proc_pdp10.s -- PDP-6 scheduler context switch and event sleep/wakeup.
;
; proc_table is allocated after memory discovery.  Each active process owns a
; stable executive u-area allocated from kernel-dynamic core.  Saved CPU/syscall
; state ends at 044, followed immediately by cwd/file state and the private
; kernel pushdown list.  User extents may move or swap independently; the u-area
; remains resident so a sleeping executive continuation keeps valid stack
; addresses.

        .equ    PROC_WORDS,3
        .equ    PROC_STATE_LH_MASK,0700000
        .equ    PROC_STATE_RUN,0200000
        .equ    PROC_STATE_SLEEP,0300000
        .equ    PROC_WAIT_LH_MASK,060000
        .equ    PROC_WAIT_EVENT_LH,020000
        .equ    PROC_TRANSITION_RH,0200000
        .equ    PROC_FILE_TABLE_OFFSET,046
        .equ    PROC_USTACK_BASE,0115
        .equ    PROC_KSTACK_WORDS,0320

        .equ    CTX_U_PC,020
        .equ    CTX_U_KSP,021
        .equ    CTX_K_PC,022
        .equ    CTX_K_AC0,023
        .equ    CTX_M_USER_SP,043
        .equ    CTX_M_SYSCALL_SAVE,044

        .text
        .globl  proc_table
        .globl  proc_slots
        .globl  proc_high_slot
        .globl  proc_current_slot
        .globl  proc_sched_cursor
        .globl  mach_kernel_stack_base
        .globl  proc_wait_event
        .globl  proc_wakeup_event
        .globl  proc_sched_pi_tick
        .globl  proc_sched_tick_select
        .globl  proc_sched_resched_select
        .globl  proc_sched_pi_resched
        .globl  proc_sched_kick
        .globl  proc_record_kernel_sp
        .globl  proc_exit_current
        .globl  pdp10_ret_zero
        .globl  pdp10_pi_level6
        .globl  pdp10_pi_sp_save
        .globl  mach_pi_disable
        .globl  mach_pi_restore
        .globl  proc_exit_finish
        .globl  mach_kernel_sp
        .globl  mach_user_sp
        .globl  mach_user_apr
        .globl  mach_syscall_save
        .globl  file_table

; AC1 = slot.  Return AC1 = address of its three-word struct proc, AC2 clobbered.
proc_slot_ptr:
        move    2,1
        lsh     1,1
        add     1,2                    ; 3 * slot
        add     1,proc_table
        popj    17,

; AC1 = slot.  Return AC1 = stable physical u-area base, AC2 clobbered.
; meta LH is repurposed from initial entry PC once the user context is built.
proc_uarea_slot:
        pushj   17,proc_slot_ptr
        hlrz    1,(1)
        popj    17,

; EXIT is entered by JRST from the syscall dispatcher.  The current C stack
; therefore lives inside the process u-area and must not survive mm_free().
; Disable PI, switch to the permanent slot-0 stack, release the process, then
; either idle until another runnable process is selected by the clock PI or
; halt for the normal final-user shutdown.
proc_exit_current:
        pushj   17,mach_pi_disable
        move    2,1                    ; saved global PI on/off state
        move    17,mach_kernel_stack_base
        setzm   file_table
        push    17,2
        pushj   17,proc_exit_finish
        jumple  1,proc_exit_halt       ; final process or fatal release error
        pop     17,1                   ; another process remains: restore PI
        pushj   17,mach_pi_restore
        jrst    proc_idle_loop
proc_exit_halt:
        ; Keep PI disabled.  Re-enabling it here lets a final clock interrupt
        ; redirect the no-process case into proc_idle_loop before HALT.
        halt
        jrst    .-1

; Initial user entry must leave the next syscall using the process-private
; kernel stack rather than KCORE's bootstrap/idle stack.  Preserve the
; mach_enter_user argument ACs while publishing that stack pointer.
proc_record_kernel_sp:
        push    17,1
        push    17,2
        move    1,proc_current_slot
        pushj   17,proc_uarea_slot
        move    2,1
        addi    2,PROC_USTACK_BASE
        movem   2,CTX_U_KSP(1)
        movem   2,mach_kernel_sp
        move    2,1
        addi    2,PROC_FILE_TABLE_OFFSET
        movem   2,file_table
        pop     17,2
        pop     17,1
        popj    17,

; int proc_wait_event(volatile kword_t *eventp)
; Publish an event channel and sleep.  Request software PI6 so the executive
; continuation is saved immediately rather than polling until a timer tick.
; The clock MRES distinguishes this request from a real timer interrupt.
proc_wait_event:
        skipe   (1)
        jrst    pdp10_ret_zero
        skipn   proc_current_slot
        jrst    proc_wait_boot
        move    2,proc_current_slot
        move    3,2
        lsh     2,1
        add     2,3
        add     2,proc_table
        move    3,2(2)                 ; packed scheduler word
        tlz     3,PROC_WAIT_LH_MASK
        tlo     3,PROC_WAIT_EVENT_LH
        tlz     3,PROC_STATE_LH_MASK
        tlo     3,PROC_STATE_SLEEP
        hrr     3,1
        movem   3,2(2)
        skipe   (1)
        jrst    proc_wait_raced
        setom   proc_sched_kick
        cono    0004,004002             ; software request at PI level 6
        jrst    proc_wait_armed
proc_wait_armed:
        ; PI6 is taken between instructions while PI is enabled.  This branch
        ; is the saved continuation; after wakeup it simply returns success.
        jrst    pdp10_ret_zero
proc_wait_boot:
        skipn   (1)
        jrst    proc_wait_boot
        jrst    pdp10_ret_zero
proc_wait_raced:
        move    3,2(2)
        tlz     3,PROC_WAIT_LH_MASK
        tlz     3,PROC_STATE_LH_MASK
        tlo     3,PROC_STATE_RUN
        hllz    3,3
        movem   3,2(2)
        jrst    pdp10_ret_zero

; void proc_wakeup_event(volatile kword_t *eventp)
; PI-safe.  Wake every event sleeper, including a swapped sleeper whose
; logical state remains resident in the compact process descriptor.
proc_wakeup_event:
        push    17,2
        push    17,3
        push    17,4
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
        tlz     4,PROC_STATE_LH_MASK
        tlo     4,PROC_STATE_RUN
        hllz    4,4
        movem   4,2(2)
        ; If the CPU is in the scheduler idle loop, request PI6 now instead
        ; of adding up to one clock tick of wakeup latency.
        skipn   proc_current_slot
        jrst    proc_wakeup_kick
        jrst    proc_wakeup_next
proc_wakeup_kick:
        setom   proc_sched_kick
        cono    0004,004002
proc_wakeup_next:
        addi    2,PROC_WORDS
        addi    3,1
        jrst    proc_wakeup_scan
proc_wakeup_done:
        pop     17,4
        pop     17,3
        pop     17,2
        popj    17,

;. Test-only process-private kernel stack watermarking.  The untouched
; canary in each stack word is that word's own physical address.  Because the
; PDP-6 pushdown stack grows upward, the first untouched word terminates the
; scan.  This code and state disappear completely from production builds.
.if PROC_STACK_WATERMARK
        .globl  proc_stack_highwater
proc_stack_watermark_scan:
        move    2,1
        addi    2,PROC_USTACK_BASE+1
        movei   3,1
proc_stack_watermark_loop:
        camn    2,(2)
        jrst    proc_stack_watermark_done
        addi    2,1
        addi    3,1
        caile   3,PROC_KSTACK_WORDS
        jrst    proc_stack_watermark_done
        jrst    proc_stack_watermark_loop
proc_stack_watermark_done:
        camle   3,proc_stack_highwater
        movem   3,proc_stack_highwater
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
        move    2,1
        movem   0,0(2)
        movem   4,4(2)
        movem   5,5(2)
        movem   6,6(2)
        movem   7,7(2)
        movem   010,010(2)
        movem   011,011(2)
        movem   012,012(2)
        movem   013,013(2)
        movem   014,014(2)
        movem   015,015(2)
        movem   016,016(2)
        move    1,000032
        movem   1,1(2)
        move    1,000033
        movem   1,2(2)
        move    1,000055
        movem   1,3(2)
        move    1,pdp10_pi_sp_save+012
        movem   1,017(2)
        move    1,pdp10_pi_level6
        movem   1,CTX_U_PC(2)
        move    1,mach_kernel_sp
        movem   1,CTX_U_KSP(2)
        popj    17,

; Save a sleeping executive context.  This includes syscall-boundary globals
; that another process may overwrite while this process sleeps.
proc_save_kernel:
        move    1,proc_current_slot
        pushj   17,proc_uarea_slot
.if PROC_STACK_WATERMARK
        pushj   17,proc_stack_watermark_scan
.endif
        move    2,1
        movem   0,CTX_K_AC0+0(2)
        movem   4,CTX_K_AC0+4(2)
        movem   5,CTX_K_AC0+5(2)
        movem   6,CTX_K_AC0+6(2)
        movem   7,CTX_K_AC0+7(2)
        movem   010,CTX_K_AC0+010(2)
        movem   011,CTX_K_AC0+011(2)
        movem   012,CTX_K_AC0+012(2)
        movem   013,CTX_K_AC0+013(2)
        movem   014,CTX_K_AC0+014(2)
        movem   015,CTX_K_AC0+015(2)
        movem   016,CTX_K_AC0+016(2)
        move    1,000032
        movem   1,CTX_K_AC0+1(2)
        move    1,000033
        movem   1,CTX_K_AC0+2(2)
        move    1,000055
        movem   1,CTX_K_AC0+3(2)
        move    1,pdp10_pi_sp_save+012
        movem   1,CTX_K_AC0+017(2)
        move    1,pdp10_pi_level6
        movem   1,CTX_K_PC(2)
        move    1,mach_user_sp
        movem   1,CTX_M_USER_SP(2)
        move    1,mach_syscall_save
        movem   1,CTX_M_SYSCALL_SAVE(2)
        move    1,mach_kernel_sp
        movem   1,CTX_U_KSP(2)
        popj    17,

; Recompute PDP-6 relocation/protection from the selected process extent.
proc_load_apr:
        move    1,proc_current_slot
        pushj   17,proc_slot_ptr
        hlrz    2,1(1)
        subi    2,02000
        hrlz    2,2
        hrrz    3,1(1)
        hrr     2,3
        movem   2,mach_user_apr
        datao   0000,mach_user_apr
        popj    17,

; Restore user ACs and PI return state for proc_current_slot.
proc_restore_user:
        move    1,proc_current_slot
        pushj   17,proc_uarea_slot
        move    2,1
        move    1,CTX_U_PC(2)
        movem   1,pdp10_pi_level6
        move    1,1(2)
        movem   1,000032
        move    1,2(2)
        movem   1,000033
        move    1,3(2)
        movem   1,000055
        move    1,017(2)
        movem   1,pdp10_pi_sp_save+012
        move    1,CTX_U_KSP(2)
        movem   1,mach_kernel_sp
        pushj   17,proc_load_apr
        move    1,proc_current_slot
        pushj   17,proc_uarea_slot
        move    2,1
        move    3,1
        addi    3,PROC_FILE_TABLE_OFFSET
        movem   3,file_table
        move    0,0(2)
        move    4,4(2)
        move    5,5(2)
        move    6,6(2)
        move    7,7(2)
        move    010,010(2)
        move    011,011(2)
        move    012,012(2)
        move    013,013(2)
        move    014,014(2)
        move    015,015(2)
        move    016,016(2)
        popj    17,

; Restore a previously sleeping executive context and its syscall globals.
proc_restore_kernel:
        move    1,proc_current_slot
        pushj   17,proc_uarea_slot
        move    2,1
        move    1,CTX_K_PC(2)
        movem   1,pdp10_pi_level6
        setzm   CTX_K_PC(2)
        move    1,CTX_K_AC0+1(2)
        movem   1,000032
        move    1,CTX_K_AC0+2(2)
        movem   1,000033
        move    1,CTX_K_AC0+3(2)
        movem   1,000055
        move    1,CTX_K_AC0+017(2)
        movem   1,pdp10_pi_sp_save+012
        move    1,CTX_M_USER_SP(2)
        movem   1,mach_user_sp
        move    1,CTX_M_SYSCALL_SAVE(2)
        movem   1,mach_syscall_save
        move    1,CTX_U_KSP(2)
        movem   1,mach_kernel_sp
        pushj   17,proc_load_apr
        move    1,proc_current_slot
        pushj   17,proc_uarea_slot
        move    2,1
        move    3,1
        addi    3,PROC_FILE_TABLE_OFFSET
        movem   3,file_table
        move    0,CTX_K_AC0+0(2)
        move    4,CTX_K_AC0+4(2)
        move    5,CTX_K_AC0+5(2)
        move    6,CTX_K_AC0+6(2)
        move    7,CTX_K_AC0+7(2)
        move    010,CTX_K_AC0+010(2)
        move    011,CTX_K_AC0+011(2)
        move    012,CTX_K_AC0+012(2)
        move    013,CTX_K_AC0+013(2)
        move    014,CTX_K_AC0+014(2)
        move    015,CTX_K_AC0+015(2)
        move    016,CTX_K_AC0+016(2)
        popj    17,

; Switch to the slot-0 executive idle loop when no resident user process runs.
proc_restore_idle:
        setzm   proc_current_slot
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
        move    1,proc_current_slot
        jumpe   1,proc_sched_resched_choose
        pushj   17,proc_save_kernel
proc_sched_resched_choose:
        pushj   17,proc_sched_resched_select
        movem   1,proc_current_slot
        jumpe   1,proc_restore_idle
        pushj   17,proc_uarea_slot
        skipn   CTX_K_PC(1)
        jrst    proc_restore_user
        jrst    proc_restore_kernel

; Called once per qualified line-clock PI6 after the hardware clock flag is
; cleared.  User code is preemptible.  Ordinary executive code is not; only a
; process which explicitly sleeps can be switched while in the kernel.
proc_sched_pi_tick:
        skipn   proc_sched_cursor
        popj    17,
        move    1,pdp10_pi_level6
        tlnn    1,010000
        jrst    proc_sched_exec_tick
        pushj   17,proc_save_user
        jrst    proc_sched_select
proc_sched_exec_tick:
        move    1,proc_current_slot
        jumpe   1,proc_sched_select
        pushj   17,proc_slot_ptr
        hlrz    2,2(1)
        andi    2,PROC_STATE_LH_MASK
        caie    2,PROC_STATE_SLEEP
        popj    17,
        jrst    proc_sched_exec_sleep
proc_sched_exec_sleep:
        pushj   17,proc_save_kernel
proc_sched_select:
        pushj   17,proc_sched_tick_select
        movem   1,proc_current_slot
        jumpe   1,proc_restore_idle
        pushj   17,proc_uarea_slot
        skipn   CTX_K_PC(1)
        jrst    proc_restore_user
        jrst    proc_restore_kernel

proc_idle_loop:
        jrst    proc_idle_loop

        .bss
proc_table:
        .block  1                       ; pointer to boot-sized process table
proc_slots:
        .block  1                       ; configured 64/128/256 slot count
proc_high_slot:
        .block  1                       ; one past highest occupied slot
proc_current_slot:
        .block  1
proc_sched_cursor:
        .block  1
proc_sched_kick:
        .block  1
.if PROC_STACK_WATERMARK
proc_stack_highwater:
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
