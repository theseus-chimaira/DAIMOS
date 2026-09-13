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
        .equ    PROC_TRANSITION_RH,0200000
        .equ    PROC_FILE_TABLE_OFFSET,047
        .equ    PROC_USTACK_BASE,0107
        .equ    PROC_KSTACK_WORDS,0311
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
        .globl  vm_activate_current
        .globl  proc_slot_ptr
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
        came    4,5
        jrst    proc_tty_session_next
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
        movem   4,2(2)
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
        move    5,proc_current_slot    ; parent slot
        move    4,5
        lsh     4,1
        add     4,5
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
        caile   1,014
        jrst    pdp10_ret_neg1
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
        caige   7,2
        jrst    proc_control_tty_attach_state_ok
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
        cail    1,025
        jrst    proc_control_tty_owned_bad
        move    7,proc_tty_records(1)
        move    4,7
        andi    4,0377
        ldb     5,[POINT 8,045(5),32]
        came    4,5
        jrst    proc_control_tty_owned_bad
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
        move    1,7
        lsh     1,-010
        andi    1,0377
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
        andi    5,0377
        move    6,2
        lsh     6,010
        ior     5,6
        movem   5,proc_tty_records(4)
        move    1,2
        popj    17,

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
        move    2,proc_current_slot
        move    3,2
        lsh     2,1
        add     2,3
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
        move    2,proc_current_slot
        move    3,2
        lsh     2,1
        add     2,3
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
        hllz    4,4                    ; clear wait channel
        tlz     4,0100000              ; SLEEP->RUN, STOP remains STOP
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
        caile   3,PROC_KSTACK_WORDS
        jrst    proc_stack_watermark_done
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
        caile   3,KERNEL_IDLE_STACK_WORDS
        jrst    kernel_idle_stack_watermark_done
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
.if PROC_STACK_WATERMARK
        skipn   proc_current_slot
        pushj   17,kernel_idle_stack_watermark_scan
.endif
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
        ; Sleep is the only executive-tick case that needs the kernel
        ; context saved.  Fall through directly instead of jumping to
        ; the immediately following instruction.
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
        ; Disk-backed swap-in must never run in PI context.  Slot 0 owns the
        ; permanent idle/exit stack, so service one deserving swapped SRUN
        ; process here and then re-enter ordinary scheduler selection.
        pushj   17,proc_swap_service_one
        jrst    proc_sched_resched_choose

        .bss
.if PROC_STACK_WATERMARK
proc_stack_highwater:
        .long   0
kernel_idle_stack_highwater:
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
