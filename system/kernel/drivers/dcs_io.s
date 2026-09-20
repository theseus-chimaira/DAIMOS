; dcs_io.s -- compact PDP-6 Type 630 DCS driver.
;
; The hardware has one scanner for all sixteen lines.  Readers name the line
; they want; a scanner result for another line is deferred in that logical
; TTY's existing process-session record by proc_tty_pending_store().  One MRES
; ready word and event are therefore sufficient for all DCS lines.

        .globl mfsdev_io_in
        .globl mfsdev_io_out
        .text
        .globl dcs_pi_handler
        .globl dcs_getchar
        .globl dcs_putchar
        .globl proc_tty_pending_take
        .globl proc_tty_pending_store
        .globl proc_wait_event_intr
        .globl proc_wakeup_event
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_ok
        .globl pdp10_ret_arg

; dcs_rx_word is zero when empty and packed(line,byte)+1 when ready.  The +1
; keeps line 0 / NUL distinct from the empty marker.
dcs_pi_handler:
        conso 0300,000010
        jrst pdp10_pi_handler_return
        skipe dcs_rx_word
        jrst dcs_pi_disable
        coni 0304,1
        andi 1,077
        lsh 1,010
        movem 1,dcs_rx_word
        datai 0304,1
        aos mfsdev_io_in+6
        andi 1,0377
        iorm 1,dcs_rx_word
        aos dcs_rx_word
        setom dcs_rx_event
        movei 1,dcs_rx_event
        pushj 17,proc_wakeup_event
dcs_pi_disable:
        cono 0300,0
        jrst pdp10_pi_handler_return

; AC1 = requested DCS line 0..15.  Return one byte from that exact line.
dcs_getchar:
        caile 1,017
        jrst pdp10_ret_arg
        push 17,1
dcs_getchar_loop:
        move 1,(17)
        addi 1,1                     ; DCS line N is logical TTY N+1
        pushj 17,proc_tty_pending_take
        jumpge 1,dcs_getchar_done

        move 2,dcs_rx_word
        jumpn 2,dcs_getchar_ready
        setzm dcs_rx_event
        ; Clearing the event precedes arming and a second ready check, so a
        ; PI between these instructions cannot be lost.
        cono 0300,000012
        skipe dcs_rx_word
        jrst dcs_getchar_loop
        movei 1,dcs_rx_event
        pushj 17,proc_wait_event_intr
        jumpn 1,dcs_getchar_done
        jrst dcs_getchar_loop

dcs_getchar_ready:
        setzm dcs_rx_word
        subi 2,1
        ldb 3,[POINT 6,2,27]
        camn 3,(17)
        jrst dcs_getchar_ready_ours
        move 1,3
        addi 1,1
        andi 2,0377
        pushj 17,proc_tty_pending_store
        ; Wake a reader of the line for which this byte was deferred.
        setom dcs_rx_event
        movei 1,dcs_rx_event
        pushj 17,proc_wakeup_event
        jrst dcs_getchar_loop

dcs_getchar_ready_ours:
        move 1,2
        andi 1,0377
dcs_getchar_done:
        sub 17,[1,,1]
        popj 17,

; AC1 = DCS_PACK(line, byte).  Return 0 or DCS_E_ARG (-1).
dcs_putchar:
        ldb 2,[POINT 6,1,27]
        caile 2,017
        jrst pdp10_ret_arg
        cono 0304,0(2)
        andi 1,0377
        datao 0300,1
        aos mfsdev_io_out+6
        jrst pdp10_ret_ok

        .bss
dcs_rx_word:
        .block 1
dcs_rx_event:
        .block 1
