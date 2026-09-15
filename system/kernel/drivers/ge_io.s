; ge_io.s -- compact interrupt-driven PDP-6 GE/GTY driver.
;
; GE/GTY owns PI4 independently.  One hardware input path serves four logical
; consoles.  Readers request one console; bytes for another console are
; deferred in that logical TTY's existing process-session record.

        .globl devicefs_io_in
        .globl devicefs_io_out
        .text
        .globl ge_pi_handler
        .globl ge_getchar
        .globl ge_putchar
        .globl proc_tty_pending_take
        .globl proc_tty_pending_store
        .globl proc_wait_event_intr
        .globl proc_wakeup_event
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_ok
        .globl pdp10_ret_arg
        .globl pdp10_ret_busy

; ge_rx_word is zero when empty; a raw GTYI word has LH bit 4 set while ready.
; ge_tx_state is nonzero while one complete GE output frame is owned.
ge_pi_handler:
        conso 0070,00010
        jrst pdp10_pi_handler_return
        skipe ge_rx_word
        jrst ge_pi_gtyi_disable
        datai 0070,1
        aos devicefs_io_in+7
        tlo 1,4
        movem 1,ge_rx_word
        setom ge_rx_event
        movei 1,ge_rx_event
        pushj 17,proc_wakeup_event
ge_pi_gtyi_disable:
        cono 0070,0
        jrst pdp10_pi_handler_return

; AC1 = requested GE console 0..3.  Return one byte from that exact console.
ge_getchar:
        caile 1,3
        jrst pdp10_ret_arg
        push 17,1
ge_getchar_loop:
        move 1,(17)
        addi 1,021                   ; GE N is logical TTY 17+N
        pushj 17,proc_tty_pending_take
        jumpge 1,ge_getchar_done

        move 2,ge_rx_word
        jumpn 2,ge_getchar_ready
        consz 0070,00010
        jrst ge_getchar_hardware
        setzm ge_rx_event
        cono 0070,000004
        skipe ge_rx_word
        jrst ge_getchar_loop
        movei 1,ge_rx_event
        pushj 17,proc_wait_event_intr
        jumpn 1,ge_getchar_error
        jrst ge_getchar_loop

ge_getchar_hardware:
        datai 0070,2
        aos devicefs_io_in+7
        tlo 2,4
ge_getchar_ready:
        setzm ge_rx_word
        tlz 2,4
        ldb 3,[POINT 2,2,17]
        camn 3,(17)
        jrst ge_getchar_ready_ours
        move 1,3
        addi 1,021
        andi 2,0177
        pushj 17,proc_tty_pending_store
        setom ge_rx_event
        movei 1,ge_rx_event
        pushj 17,proc_wakeup_event
        jrst ge_getchar_loop

ge_getchar_ready_ours:
        move 1,2
        andi 1,0177
ge_getchar_done:
        sub 17,[1,,1]
        popj 17,
ge_getchar_error:
        move 2,1
        sub 17,[1,,1]
        move 1,2
        popj 17,

; AC1 = decoded 7-bit GE byte.  Caller owns ge_tx_state bit 0.
ge_put_decoded:
        conso 0750,00100
        jrst ge_put_decoded
        andi 1,0177
        move 3,1
        lsh 1,-1
        trne 3,1
        iori 1,0100
        xori 1,0177
        datao 0750,1
        aos devicefs_io_out+7
ge_put_decoded_wait:
        conso 0750,00100
        jrst ge_put_decoded_wait
        popj 017,

; AC1 = GE_PACK(console, byte).  Return 0 or GE_E_*.
ge_putchar:
        skipe ge_tx_state
        jrst pdp10_ret_busy
        cono 0750,0
ge_putchar_idle:
        move 4,1
        ldb 5,[POINT 6,1,27]
        caile 5,3
        jrst pdp10_ret_arg
        setom ge_tx_state
        movei 1,1
        pushj 017,ge_put_decoded
        lsh 5,3
        addi 5,0140
        move 1,5
        pushj 017,ge_put_decoded
        movei 1,0
        pushj 017,ge_put_decoded
        movei 1,2
        pushj 017,ge_put_decoded
        andi 4,0177
        move 1,4
        pushj 017,ge_put_decoded
        movei 1,3
        pushj 017,ge_put_decoded
        move 1,5
        xor 1,4
        xori 1,1
        pushj 017,ge_put_decoded
        setzm ge_tx_state
        jrst pdp10_ret_ok

        .bss
ge_rx_word:
        .block 1
ge_rx_event:
        .block 1
ge_tx_state:
        .block 1
