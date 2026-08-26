; ge_io.s -- compact interrupt-driven PDP-6 GE/GTY driver.
;
; GE/GTY owns PI4 independently.  DCS uses PI2, so this handler contains only
; the two GE terminal devices and needs no cross-driver dispatch glue.

        .text
        .globl devicefs_v1_io_in
        .globl devicefs_v1_io_out
        .globl ge_pi_handler
        .globl ge_getchar
        .globl ge_putchar
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_ok_v34
        .globl pdp10_ret_arg_v34
        .globl pdp10_ret_busy_v34

; ge_rx_word: zero idle, -1 waiting, otherwise 4,,raw-GTYI-word (ready).
; ge_tx_state: bit 0 owns one complete GE frame; bit 1 awaits GTYO DONE.
ge_pi_handler:
        conso 0070,00010
        jrst pdp10_pi_handler_return
        skipl ge_rx_word
        jrst ge_pi_gtyi_disable
        datai 0070,1
        aos devicefs_v1_io_in+7
        tlo 1,4
        movem 1,ge_rx_word
ge_pi_gtyi_disable:
        cono 0070,0
        jrst pdp10_pi_handler_return

; Return GE_PACK(console, character), or GE_E_BUSY if another read is waiting.
ge_getchar:
        move 1,ge_rx_word
        jumpg 1,ge_get_ready
        jumpl 1,pdp10_ret_busy_v34

        ; Consume a character which arrived while input PI was disabled.
        consz 0070,00010
        jrst ge_get_hardware

        setom ge_rx_word
        cono 0070,000004
ge_get_wait:
        skipg 1,ge_rx_word
        jrst ge_get_wait

ge_get_hardware:
        datai 0070,1
        aos devicefs_v1_io_in+7
        jrst ge_get_unpack_raw

ge_get_ready:
        setzm ge_rx_word
        tlz 1,4
ge_get_unpack_raw:
        hlrz 2,1
        andi 2,3
        lsh 2,010
        andi 1,0177
        ior 1,2
        popj 017,

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
        aos devicefs_v1_io_out+7
ge_put_decoded_wait:
        conso 0750,00100
        jrst ge_put_decoded_wait
        popj 017,

; AC1 = GE_PACK(console, byte).  Return 0 or GE_E_*.
; Each character is a complete GE message: SOH, address, status, STX, byte,
; ETX, longitudinal parity.  The parity byte simplifies to address XOR byte
; XOR 1 because STX XOR ETX is 1 and status is zero.
ge_putchar:
        skipe ge_tx_state
        jrst pdp10_ret_busy_v34
ge_putchar_idle:
        move 4,1
        move 5,1
        lsh 5,-010
        andi 5,077
        caile 5,3
        jrst pdp10_ret_arg_v34
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
        jrst pdp10_ret_ok_v34
        .bss
ge_rx_word:
        .block 1
ge_tx_state:
        .block 1
