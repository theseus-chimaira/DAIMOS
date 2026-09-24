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
        .equ    PROC_WAIT_CHILD_LH,040000
        .equ    PROC_WAIT_INTR_LH,060000
        .equ    PROC_CPU_SLEEP_LH_MASK,017700
        .equ    PROC_SCHED_QUANTUM_TICKS,4
        .equ    PROC_TRANSITION_RH,0200000
        .equ    PROC_FILE_TABLE_OFFSET,047
        .equ    PROC_CRED_OFFSET,0107
        .equ    PROC_UMASK_OFFSET,0110
        .equ    PROC_USTACK_BASE,0111
        .equ    PROC_KSTACK_WORDS,0306
        .equ    KERNEL_IDLE_STACK_WORDS,0100

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
        .globl  proc_sched_deferred_ticks
        .globl  proc_runq_head
        .globl  mach_kernel_stack_base
        .globl  proc_wait_event
        .globl  proc_wait_event_intr
        .globl  proc_wait_child
        .globl  proc_wakeup_event
        .globl  proc_sched_pi_tick
        .globl  proc_sched_tick_select
        .globl  proc_sched_resched_select
        .globl  proc_sched_pi_resched
        .globl  proc_sched_kick
        .globl  proc_rt_owner
        .globl  proc_sched_resched_current
        .globl  proc_swap_service_one
        .globl  proc_record_kernel_sp
        .globl  proc_exit_current
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_one
        .globl  pdp10_ret_neg1
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

; AC1 = slot.  Return AC1 = address of its three-word struct proc, AC2 clobbered.
proc_slot_ptr:
        move    2,1
        lsh     1,1
        add     1,2                    ; 3 * slot
        add     1,proc_table
        popj    17,

; Intrusive runnable queue.  PROC_SRUN owns sched RH, so the link costs no
; per-process storage.  Add is O(1); remove is O(number of runnable jobs), but
; removal occurs only on state transitions rather than on ordinary quanta.
; AC1 = slot.  Preserve AC1..AC4 so the event/wait assembly can call these
; helpers without enlarging its save frames.
proc_runq_add:
        move    5,1
        imuli   5,3
        add     5,proc_table
        hlrz    0,2(5)
        andi    0,PROC_STATE_LH_MASK
        caie    0,PROC_STATE_RUN
        popj    17,
        move    6,proc_runq_head
        hrrm    6,2(5)
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

; Initial user entry must leave the next syscall using the process-private
; kernel stack rather than KCORE's bootstrap/idle stack.  Preserve the
; initial-user argument ACs while publishing that stack pointer.
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



; int proc_rt_control(unsigned int command)
; One RT owner exists system-wide.  ENABLE is idempotent for the owner;
; DISABLE and YIELD both release ownership and immediately enter the normal
; scheduler.  AC1 carries the command/result; AC2..AC4 are caller-scratch.
        .globl  proc_rt_control
proc_rt_control:
        move    2,proc_current_slot
        jumpe   2,pdp10_ret_neg1
        cain    1,1                    ; SYS_RTCTL_ENABLE
        jrst    proc_rt_enable
        caie    1,0                    ; SYS_RTCTL_DISABLE
        cain    1,2                    ; SYS_RTCTL_YIELD
        jrst    proc_rt_release
        jrst    pdp10_ret_neg1
proc_rt_enable:
        skipn   3,proc_rt_owner
        jrst    proc_rt_claim
        came    3,2
        jrst    pdp10_ret_neg1
        jrst    pdp10_ret_zero
proc_rt_claim:
        movem   2,proc_rt_owner
        jrst    pdp10_ret_zero
proc_rt_release:
        camn    2,proc_rt_owner
        jrst    proc_rt_release_owner
        jrst    pdp10_ret_neg1
proc_rt_release_owner:
        setzm   proc_rt_owner
        pushj   17,proc_sched_resched_current
        jrst    pdp10_ret_zero

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
        jrst    pdp10_ret_zero
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
        move    4,045(4)                ; packed control word
        lsh     4,-3
        andi    4,0377                  ; session id
        camn    4,5
        jrst    pdp10_ret_one
proc_tty_session_next:
        addi    3,PROC_WORDS
        addi    2,1
        jrst    proc_tty_session_scan
; int proc_session_teardown(unsigned int leader_slot, kword_t leader_ctl)
; Session-leader exit path.  A controlling TTY is encoded directly in the
; leader control word, so teardown needs no permanent session table or TTY
; census.  V0.9 HUP is unconditionally fatal, so clearing the session TTY
; record and delivering HUP also guarantees that stopped jobs cannot survive.
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
        setzm   proc_tty_records(4)    ; ID lifetime keeps this authoritative

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
        ldb     5,[POINT 8,045(4),32]
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

; int proc_event_send(unsigned int target, unsigned int event, int group)
; Validate one PID or scan one process group, then hand actual state changes to
; proc_event_apply.  The group scan packs its two boolean results into the LH
; of AC14 while the RH remains the slot index, avoiding GCC's large C frame.
        .globl  proc_event_send
        .globl  proc_event_apply
proc_event_send:
        add     17,[5,,5]
        movei   0,-4(17)
        hrli    0,010
        blt     0,(17)                 ; preserve AC10..AC14
        move    010,1                  ; target
        move    011,2                  ; event
        move    012,proc_current_slot  ; caller slot
        jumpe   010,proc_event_send_fail
        cail    011,6                   ; CHLD and above are not user-sendable
        jrst    proc_event_send_fail

        move    4,012
        lsh     4,1
        add     4,012
        add     4,proc_table
        hlrz    4,(4)
        move    013,045(4)             ; caller session/domain control word
        jumpn   3,proc_event_send_group

; PID delivery: target must be live, u-area resident, and in caller's domain.
        caml    010,proc_slots
        jrst    proc_event_send_fail
        move    3,010
        lsh     3,1
        add     3,010
        add     3,proc_table
        move    4,2(3)
        and     4,[0300000000000]
        jumpe   4,proc_event_send_fail
        hrrz    4,(3)
        trnn    4,0400000               ; resident u-area flag
        jrst    proc_event_send_fail
        hlrz    4,(3)
        move    5,045(4)
        xor     5,013
        tdne    5,[01774000]            ; domain differs
        jrst    proc_event_send_fail
        move    1,010
        move    2,011
        jrst    proc_event_send_apply_tail

; Group delivery requires both session and domain to match.  AC14 LH bit 0
; records any match and bit 1 records a deferred self-delivery; RH is slot.
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
        move    5,045(4)
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
        sub     17,[5,,5]
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
        sub     17,[5,,5]
        popj    17,

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
        jrst    pdp10_ret_zero
        move    4,2(3)                 ; packed scheduler word
        and     4,[0300000000000]      ; FREE/ZOMB have low state bits clear
        jumpn   4,pdp10_ret_one
        addi    3,PROC_WORDS
        addi    2,1
        jrst    proc_has_live_user_scan
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
        iorm    3,045(4)
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
        jrst    pdp10_ret_zero
        hlrz    2,(1)
        move    1,045(2)
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
        move    4,proc_current_slot    ; parent slot * 3
        lsh     4,1
        add     4,proc_current_slot
        add     4,proc_table           ; parent descriptor
        hlrz    3,(4)
        move    3,045(3)               ; parent control word
        ldb     5,[POINT 8,3,32]       ; parent session

        jumpe   2,proc_child_inherit
        caie    2,1
        jrst    proc_child_join
; NEW pgrp.
        jumpn   6,pdp10_ret_neg1
        move    1,7
        jrst    proc_child_set
proc_child_inherit:
        jumpn   6,pdp10_ret_neg1
        hrrz    1,(4)
        andi    1,0377
        jumpe   1,pdp10_ret_neg1
        jrst    proc_child_set
proc_child_join:
        caie    2,2
        jrst    pdp10_ret_neg1
        jumpe   6,pdp10_ret_neg1
        caile   6,0377
        jrst    pdp10_ret_neg1
        movei   1,1                    ; scan slot
        move    2,proc_table
        addi    2,3
proc_child_join_loop:
        caml    1,proc_high_slot
        jrst    pdp10_ret_neg1
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
        ldb     4,[POINT 8,045(4),32]
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
        move    6,045(4)
        and     6,[007776000007]       ; clear session/domain/TTY
        move    5,3
        and     5,[770001777770]
        ior     6,5
        movem   6,045(4)
        jrst    pdp10_ret_zero

; int proc_control(unsigned int op, unsigned int arg)
; Compact native PROCCTL dispatcher.  The syscall dispatcher guarantees a live
; current process with a resident u-area.  AC1=op, AC2=arg.  Only AC1..AC7 are
; used except around explicit C calls, so the common query/update cases need no
; GCC save frame.
        .globl  proc_control
        .globl  proc_tty_records
        .globl  proc_tty_release_session
proc_control:
        jumpl   1,pdp10_ret_neg1
        caile   1,031
        jrst    pdp10_ret_neg1
        ; AC3 is the current slot/session identity used by NEWSESSION,
        ; NEWDOMAIN, and the TTY ownership operations below.  Do not depend
        ; on an arbitrary user AC3 value surviving the syscall trap.
        move    3,proc_current_slot
        move    4,3
        lsh     4,1
        add     4,3
        add     4,proc_table
        jrst    @proc_control_table(1)
proc_control_table:
        .word   proc_control_getpgrp
        .word   proc_control_getsession
        .word   proc_control_getdomain
        .word   proc_control_newsession
        .word   proc_control_newdomain
        .word   proc_control_getevents
        .word   proc_control_event_pid
        .word   proc_control_event_pgrp
        .word   proc_control_gettty
        .word   proc_control_tty_attach
        .word   proc_control_tty_detach
        .word   proc_control_tty_getfg
        .word   proc_control_tty_setfg
        .word   proc_control_getuid
        .word   proc_control_getgid
        .word   proc_control_setuid
        .word   pdp10_ret_neg1         ; 020 reserved by UUO-077 extension bank
        .word   pdp10_ret_neg1         ; 021 reserved by UUO-077 extension bank
        .word   pdp10_ret_neg1         ; 022 reserved by UUO-077 extension bank
        .word   pdp10_ret_neg1         ; 023 reserved by UUO-077 extension bank
        .word   pdp10_ret_neg1         ; 024 reserved by UUO-077 extension bank
        .word   proc_control_setgid    ; 025
        .word   proc_control_tty_getmode ; 026
        .word   proc_control_tty_setmode ; 027
        .word   proc_control_isatty    ; 030
        .word   proc_control_umask     ; 031

proc_control_getpgrp:
        jumpn   2,pdp10_ret_neg1
        hrrz    1,(4)
        andi    1,0377
        popj    17,
proc_control_getsession:
        jumpn   2,pdp10_ret_neg1
        hlrz    5,(4)
        ldb     1,[POINT 8,045(5),32]
        popj    17,
proc_control_getdomain:
        jumpn   2,pdp10_ret_neg1
        hlrz    5,(4)
        ldb     1,[POINT 8,045(5),24]
        popj    17,

proc_control_newsession:
        jumpn   2,pdp10_ret_neg1
        hlrz    5,(4)
        ldb     1,[POINT 8,045(5),32]   ; old session for TTY release
        move    6,(4)
        andcmi  6,0377                 ; preserve LH, clear pgrp in RH
        move    7,3
        andi    7,0377
        ior     6,7
        movem   6,(4)
        dpb     3,[POINT 8,045(5),32]
        hrloi   6,07777
        andm    6,045(5)               ; TTY state -> NO_TTY
        move    2,3                    ; leaving slot
        pushj   17,proc_tty_release_session
        move    1,proc_current_slot
        popj    17,

proc_control_newdomain:
        jumpn   2,pdp10_ret_neg1
        hlrz    5,(4)
        dpb     3,[POINT 8,045(5),24]
        move    1,3
        popj    17,

proc_control_getevents:
        jumpn   2,pdp10_ret_neg1
        hlrz    5,(4)
        ldb     1,[POINT 7,045(5),16]
        move    6,045(5)
        trne    6,1
        iori    1,0200
        hrloi   6,0777401
        andcmi  6,1
        andm    6,045(5)
        popj    17,

proc_control_event_pid:
        setz    3,
        jrst    proc_control_event
proc_control_event_pgrp:
        movei   3,1
proc_control_event:
        tdne    2,[-04000]
        jrst    pdp10_ret_neg1
        move    1,2
        andi    1,0377
        lsh     2,-010
        andi    2,07
        jrst    proc_event_send

proc_control_gettty:
        jumpn   2,pdp10_ret_neg1
        hlrz    5,(4)
        move    1,045(5)
        lsh     1,-036
        popj    17,

proc_control_tty_attach:
        cail    2,025
        jrst    pdp10_ret_neg1
        hlrz    5,(4)
        move    6,045(5)
        ldb     7,[POINT 8,045(5),32]
        came    7,3
        jrst    pdp10_ret_neg1
        move    7,6
        lsh     7,-036
        andi    7,077
        cail   7,2
        jrst    pdp10_ret_neg1
proc_control_tty_attach_state_ok:
        move    7,proc_tty_records(2)
        move    1,7
        andi    1,0377
        jumpe   1,proc_control_tty_attach_claim
        came    1,3
        jrst    pdp10_ret_neg1
        jrst    proc_control_tty_attach_set
proc_control_tty_attach_claim:
        hrrz    1,(4)
        andi    1,0377
        lsh     1,010
        ior     1,3
        ior     1,[01600000000]         ; canonical + echo + signals
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
        move    6,045(5)
        move    1,6
        lsh     1,-036
        subi    1,2
        jumpl   1,proc_control_tty_owned_bad ; DETACHED becomes -1
        cail    1,025
        jrst    proc_control_tty_owned_bad
        move    7,proc_tty_records(1)
        move    4,7
        andi    4,0377
        ldb     5,[POINT 8,045(5),32]
        camn    4,5
        popj    17,
proc_control_tty_owned_bad:
        seto    1,
        popj    17,

proc_control_tty_detach:
        jumpn   2,pdp10_ret_neg1
        pushj   17,proc_control_tty_owned
        jumpl   1,pdp10_ret_neg1
        came    5,3                    ; only the session leader detaches
        jrst    pdp10_ret_neg1
        push    17,5                   ; line reset is C and may use ACs
        push    17,1
        pushj   17,proc_tty_line_reset
        pop     17,1
        pop     17,5
        setzm   proc_tty_records(1)
        move    1,5                    ; session
        movei   2,1                    ; DETACHED
        pushj   17,proc_tty_set_session_state
        jrst    pdp10_ret_zero

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
        ldb     1,[POINT 8,045(5),32]
        came    1,7
        jrst    proc_tty_set_session_state_next
        move    1,045(5)
        tlz     1,0770000
        ior     1,6
        movem   1,045(5)
proc_tty_set_session_state_next:
        addi    4,PROC_WORDS
        aoja    3,proc_tty_set_session_state_loop

proc_control_tty_getfg:
        jumpn   2,pdp10_ret_neg1
        pushj   17,proc_control_tty_owned
        jumpl   1,pdp10_ret_neg1
        ldb     1,[POINT 8,7,27]
        popj    17,


proc_control_tty_getmode:
        jumpn   2,pdp10_ret_neg1
        pushj   17,proc_control_tty_owned
        jumpl   1,pdp10_ret_neg1
        move    1,7
        lsh     1,-031
        andi    1,07
        popj    17,

proc_control_tty_setmode:
        tdne    2,[-010]
        jrst    pdp10_ret_neg1
        push    17,2                   ; requested mode
        pushj   17,proc_control_tty_owned
        jumpl   1,proc_control_tty_setmode_bad
        move    6,proc_current_slot
        lsh     6,1
        add     6,proc_current_slot
        add     6,proc_table
        hrrz    6,(6)
        andi    6,0377                 ; caller pgrp
        ldb     5,[POINT 8,proc_tty_records(1),27]
        came    5,6                    ; only foreground owner changes mode
        jrst    proc_control_tty_setmode_bad
        pop     17,2
        jrst    proc_tty_mode_set
proc_control_tty_setmode_bad:
        sub     17,[1,,1]
        jrst    pdp10_ret_neg1

proc_control_getuid:
proc_control_getgid:
        jumpn   2,pdp10_ret_neg1
        hlrz    5,(4)
        hlrz    6,PROC_CRED_OFFSET(5)
        caie    1,015                  ; GETUID keeps LH, GETGID selects RH
        hrrz    6,PROC_CRED_OFFSET(5)
        move    1,6
        popj    17,

; UID 0 may install login credentials.  An ordinary process may only request
; its current UID/GID, so it cannot acquire another identity.  SETUID (017)
; and SETGID (025) share the path; AC1 still contains the dispatch opcode.
proc_control_setuid:
proc_control_setgid:
proc_control_setcred:
        caile   2,0777777
        jrst    pdp10_ret_neg1
        hlrz    5,(4)
        move    6,PROC_CRED_OFFSET(5)
        hlrz    7,6                    ; current UID controls privilege
        jumpe   7,proc_control_setcred_store
        caie    1,017                  ; SETUID compares UID, SETGID compares GID
        hrrz    7,6
        came    2,7
        jrst    pdp10_ret_neg1
proc_control_setcred_store:
        cain    1,017                  ; SETUID executes only the LH store
        hrlm    2,PROC_CRED_OFFSET(5)
        caie    1,017                  ; SETGID executes only the RH store
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
        jumpe   1,pdp10_ret_neg1
        move    1,(1)
        tlz     1,707070               ; strip packed FILE metadata
        camn    1,[020002000000]       ; MonitorFS device view CTY0 IO endpoint
        jrst    proc_control_tty_owned
        jrst    pdp10_ret_neg1

; Classic umask semantics: install ARG low nine bits and return the old mask.
; The word is process-private, inherited by RUN and retained by EXEC.
proc_control_umask:
        andi    2,0777
        hlrz    5,(4)
        move    1,PROC_UMASK_OFFSET(5)
        movem   2,PROC_UMASK_OFFSET(5)
        popj    17,

proc_control_tty_setfg:
        jumpe   2,pdp10_ret_neg1
        caile   2,0377
        jrst    pdp10_ret_neg1
        pushj   17,proc_control_tty_owned
        jumpl   1,pdp10_ret_neg1
        push    17,1                   ; tty id
        push    17,2                   ; requested pgrp
        move    1,5                    ; session
        movei   3,0
        pushj   17,proc_tty_session_has
        pop     17,2
        pop     17,4                   ; tty id
        jumpe   1,pdp10_ret_neg1
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
; The C versions remain available to host tests.  Re-deriving the current
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
        .globl  pdp10_ret_neg2
        .globl  pdp10_ret_neg3

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

; Packed canonical-line byte helpers.  KCC expands the divide/remainder
; and variable shifts substantially; keep the target versions compact.
        .globl  proc_tty_line_put
proc_tty_line_put:
        move    6,3                    ; preserve character
        idivi   2,5                    ; AC2=word index, AC3=byte index
        addi    2,1                    ; word zero is the line header
        add     2,1                    ; AC2=&line[word]
        movei   4,4
        sub     4,3
        imuli   4,7                    ; shift=(4-byte)*7
        movei   5,0177
        lsh     5,0(4)                 ; mask
        move    7,0(2)
        setcm   5,5
        and     7,5
        andi    6,0177
        lsh     6,0(4)
        ior     7,6
        movem   7,0(2)
        popj    17,

        .globl  proc_tty_line_get
proc_tty_line_get:
        idivi   2,5                    ; AC2=word index, AC3=byte index
        addi    2,1
        add     2,1
        movei   4,4
        sub     4,3
        imuli   4,7
        move    5,0(2)
        movn    4,4
        lsh     5,0(4)
        andi    5,0177
        move    1,5
        popj    17,

; int proc_tty_line_take(unsigned int tty)
; Drain one cooked byte, allocate the bounded line block before a hardware
; read, or return -3 when the caller must obtain another device byte.
proc_tty_line_take:
        cail    1,025
        jrst    pdp10_ret_neg1
        move    2,proc_tty_records(1)
        move    3,2
        lsh     3,-031
        trnn    3,01                   ; RAW mode never allocates a line
        jrst    pdp10_ret_neg3
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
        move    4,3
        addi    4,1                    ; new drain index
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
        sub     17,[1,,1]              ; discard saved tty
        popj    17,
proc_tty_line_take_nl:
        move    1,0(17)
        pushj   17,proc_tty_line_reset
        sub     17,[1,,1]
        movei   1,012
        popj    17,
proc_tty_line_take_eof:
        move    1,0(17)
        pushj   17,proc_tty_line_reset
        sub     17,[1,,1]
        jrst    pdp10_ret_neg2
proc_tty_line_take_more:
        sub     17,[1,,1]
        jrst    pdp10_ret_neg3
proc_tty_line_take_bad:
        sub     17,[1,,1]
        jrst    pdp10_ret_neg1

; int proc_tty_canon_input(unsigned int tty, unsigned int ch)
; Target canonical editor.  AC10-AC13 are saved because the echo/MM helpers
; may use all caller-scratch ACs; this also preserves the KCC ABI.
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
        sub     17,[1,,1]
proc_tty_canon_kill_store:
        and     6,[-0400]              ; length = 0
        movem   6,0(12)
        jrst    proc_tty_canon_repeat
proc_tty_canon_not_kill:
        caie    11,004                 ; ^D
        jrst    proc_tty_canon_not_eof
        skipn   7
        iori    6,01200000             ; READY|EOF for an empty line
        skipe   7
        iori    6,0200000              ; submit nonempty partial line
        movem   6,0(12)
        jrst    proc_tty_canon_take
proc_tty_canon_not_eof:
        cail    7,0170                 ; 120-byte bounded canonical line
        jrst    proc_tty_canon_full
        move    4,6
        and     4,[-0400]
        move    5,7
        addi    5,1
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
        move    1,10
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        jrst    proc_tty_line_take
proc_tty_canon_return:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int proc_tty_read_enter(void)
proc_tty_read_enter:
proc_tty_read_enter_retry:
        move    5,proc_current_slot
        lsh     5,1
        add     5,proc_current_slot
        add     5,proc_table
        hlrz    6,(5)
        hlrz    1,045(6)
        lsh     1,-014                 ; packed TTY state
        jumpe   1,pdp10_ret_zero       ; NO_TTY -> historical CTY
        subi    1,2                    ; attached state -> tty id
        jumpl   1,pdp10_ret_neg1        ; DETACHED becomes -1
        cail    1,025
        jrst    pdp10_ret_neg1
        move    2,proc_tty_records(1)
        move    3,2
        andi    3,0377                 ; record session
        ldb     4,[POINT 8,045(6),32]
        came    3,4
        jrst    pdp10_ret_neg1
        ldb     3,[POINT 8,proc_tty_records(1),27] ; foreground pgrp
        hrrz    4,(5)
        andi    4,0377                 ; current pgrp
        camn    3,4
        popj    17,                    ; AC1 still tty id
        move    1,3
        movei   2,3                    ; SYS_EVENT_TSTP
        movei   3,1                    ; group delivery
        pushj   17,proc_event_send
        jumpn   1,pdp10_ret_neg1
        jrst    proc_tty_read_enter_retry

; int proc_tty_input(unsigned int tty, unsigned int ch)
proc_tty_input:
        move    4,1                    ; tty
        move    5,2                    ; character
        cail    4,025
        jrst    pdp10_ret_neg1
        move    7,proc_current_slot
        lsh     7,1
        add     7,proc_current_slot
        add     7,proc_table
        hlrz    2,(7)                  ; u-area; keep callee-saved AC10+ intact
        hlrz    3,045(2)
        lsh     3,-014                 ; TTY state
        jumpn   3,proc_tty_input_attached
        jumpn   4,pdp10_ret_neg1       ; NO_TTY accepts CTY only
        move    1,5
        andi    1,0177
        popj    17,
proc_tty_input_attached:
        subi    3,2
        came    3,4
        jrst    pdp10_ret_neg1
        move    1,proc_tty_records(4)  ; retain record until mode extraction
        move    3,1
        andi    3,0377
        ldb     6,[POINT 8,045(2),32]
        came    3,6
        jrst    pdp10_ret_neg1
        ldb     6,[POINT 8,proc_tty_records(4),27] ; foreground pgrp
        jumpe   6,pdp10_ret_neg1
        hrrz    3,(7)
        andi    3,0377
        came    6,3
        jrst    pdp10_ret_neg1
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
        jrst    pdp10_ret_neg3         ; consumed: caller retries
proc_tty_input_tstp:
        caie    5,032                  ; ^Z
        jrst    proc_tty_input_mode
        move    1,6
        movei   2,3                    ; SYS_EVENT_TSTP
        movei   3,1
        pushj   17,proc_event_send
        jumpn   1,pdp10_ret_neg1
        jrst    pdp10_ret_neg3         ; consumed: caller retries
proc_tty_input_mode:
        trnn    3,01                   ; RAW: return byte directly
        jrst    proc_tty_input_char
        move    1,4                    ; canonical helper(tty, ch)
        move    2,5
        jrst    proc_tty_canon_input
proc_tty_input_char:
        move    1,5
        popj    17,

; int proc_tty_output(unsigned int ch)
proc_tty_output:
        move    4,1                    ; character
        move    6,proc_current_slot
        lsh     6,1
        add     6,proc_current_slot
        add     6,proc_table
        hlrz    7,(6)
        hlrz    1,045(7)
        lsh     1,-014
        jumpe   1,proc_tty_output_pack_cty
        subi    1,2                    ; tty id
        jumpl   1,pdp10_ret_neg1        ; DETACHED becomes -1
        cail    1,025
        jrst    pdp10_ret_neg1
        move    3,proc_tty_records(1)
        andi    3,0377
        ldb     5,[POINT 8,045(7),32]
        came    3,5
        jrst    pdp10_ret_neg1
        lsh     1,010
        andi    4,0377
        ior     1,4
        popj    17,
proc_tty_output_pack_cty:
        move    1,4
        andi    1,0377
        popj    17,

; int proc_tty_pending_take(unsigned int tty)
proc_tty_pending_take:
        cail    1,025
        jrst    pdp10_ret_neg1
        move    2,proc_tty_records(1)
        move    3,2
        lsh     3,-020
        andi    3,0777
        jumpe   3,pdp10_ret_neg1
        and     2,[777600177777]       ; clear pending byte field
        movem   2,proc_tty_records(1)
        move    1,3
        subi    1,1
        popj    17,

; int proc_tty_pending_store(unsigned int tty, unsigned int ch)
proc_tty_pending_store:
        cail    1,025
        jrst    pdp10_ret_neg1
        caile   2,0377
        jrst    pdp10_ret_neg1
        move    4,proc_tty_records(1)
        move    3,4
        lsh     3,-020
        andi    3,0777
        jumpn   3,pdp10_ret_neg1
        addi    2,1                    ; validated byte 0..0377 -> marker 1..0400
        lsh     2,020
        ior     4,2
        movem   4,proc_tty_records(1)
        jrst    pdp10_ret_zero

; int proc_wait_event(volatile kword_t *eventp)
; Internal event waits are noninterruptible.  User-visible waits use
; proc_wait_event_intr and return -1 when ALRM is already pending or wakes them.
proc_wait_event_intr:
        skipe   (1)
        jrst    pdp10_ret_zero
        skipn   proc_current_slot
        jrst    proc_wait_event
        move    2,file_table
        move    2,-2(2)                 ; packed control word at u-area 045
        tlne    2,0100                  ; pending ALRM (event 5)
        jrst    pdp10_ret_neg1
        movsi   4,PROC_WAIT_INTR_LH
        jrst    proc_wait_event_common

proc_wait_event:
        movsi   4,PROC_WAIT_EVENT_LH
proc_wait_event_common:
        skipe   (1)
        jrst    pdp10_ret_zero
        skipn   proc_current_slot
        jrst    proc_wait_boot
        push    17,1                    ; preserve event pointer
        move    1,proc_current_slot
        pushj   17,proc_runq_remove
        pop     17,1
        move    2,proc_current_slot
        lsh     2,1
        add     2,proc_current_slot
        add     2,proc_table
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
        move    1,proc_current_slot
        pushj   17,proc_runq_add
        jrst    pdp10_ret_zero

proc_wait_intr_return:
        move    2,file_table
        move    2,-2(2)
        tlne    2,0100                  ; ALRM remained pending across sleep
        jrst    pdp10_ret_neg1
        jrst    pdp10_ret_zero


; Request an immediate software PI6 reschedule after the caller has changed
; the current process state (for example native job-control TSTP).  The PI
; saves the executive continuation and will resume it after the process is
; made runnable again.
proc_sched_resched_current:
        setom   proc_sched_kick
        cono    0004,004002
        popj    17,

; int proc_wait_child(void)
; WAIT scans while executive code is non-preemptible.  If live children exist
; but none is reportable, arm a child wait and request PI6 immediately.
proc_wait_child:
        skipn   proc_current_slot
        jrst    pdp10_ret_neg1
        move    2,file_table
        move    2,-2(2)
        tlne    2,0100                  ; ALRM interrupts user WAIT
        jrst    pdp10_ret_neg1
        move    1,proc_current_slot
        pushj   17,proc_runq_remove
        move    2,proc_current_slot
        lsh     2,1
        add     2,proc_current_slot
        add     2,proc_table
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

; void proc_wakeup_event(volatile kword_t *eventp)
; PI-safe.  Wake every event sleeper, including a swapped sleeper whose
; logical state remains resident in the compact process descriptor.
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
        addi    3,1
        jrst    proc_wakeup_scan
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
        pushj   17,vm_activate_current
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
        pushj   17,vm_activate_current
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
        jumpn   1,proc_sched_resched_save
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
.if PROC_STACK_WATERMARK
        skipn   proc_current_slot
        pushj   17,kernel_idle_stack_watermark_scan
.endif
        skipn   proc_current_slot
        jrst    proc_sched_tick_idle
        jrst    proc_sched_tick_ready
proc_sched_tick_idle:
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
        move    2,proc_sched_deferred_ticks
        caige   2,PROC_SCHED_QUANTUM_TICKS
        jrst    proc_sched_tick_fast_return
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
        pushj   17,proc_sched_tick_select
proc_sched_restore_selected:
        movem   1,proc_current_slot
        jumpe   1,proc_restore_idle
        pushj   17,proc_uarea_slot
        skipn   CTX_K_PC(1)
        jrst    proc_restore_user
        jrst    proc_restore_kernel

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

        .bss
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
