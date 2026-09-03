; ptr_io.s -- resident PDP-6 paper-tape reader driver.
; The PI7 leaf is local so a PTR-only machine does not load the other IO7
; devices.  Interrupt handlers preserve AC2/AC3 as required by PI fanout.
        .text
        .globl ptr_pi_handler
        .globl ptr_getchar
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_arg
        .globl pdp10_ret_busy
        .globl pdp10_ret_ok

ptr_pi_handler:
        conso 0104,0010
        jrst pdp10_pi_handler_return
        skipn ptr_state
        jrst ptr_pi_prefetch
        datai 0104,ptr_state
        aos ptr_io_in
        aos ptr_state
        cono 0104,0
        jrst pdp10_pi_handler_return
ptr_pi_prefetch:
        cono 0104,0010
        jrst pdp10_pi_handler_return

; AC1 = int *destination.  Return 0 or PT_E_ARG/BUSY/TIMEOUT.
ptr_getchar:
        jumpe 1,pdp10_ret_arg
        move 4,1
        move 2,ptr_state
        jumpg 2,ptr_get_software
        jumpl 2,pdp10_ret_busy
        consz 0104,0010
        jrst ptr_get_hardware
        setom ptr_state
        cono 0104,0027
        movei 5,0200000
ptr_get_wait:
        move 2,ptr_state
        jumpg 2,ptr_get_software
        sojg 5,ptr_get_wait
        setzm ptr_state
        cono 0104,0
        jrst ptr_ret_timeout
ptr_get_hardware:
        datai 0104,3
        aos ptr_io_in
        cono 0104,0
        andi 3,0377
        movem 3,(4)
        jrst ptr_get_ok
ptr_get_software:
        subi 2,1
        andi 2,0377
        movem 2,(4)
        setzm ptr_state
ptr_get_ok:
        jrst pdp10_ret_ok
ptr_ret_timeout:
        hrroi 1,0777776
        popj 017,

        .bss
ptr_state:
        .block 1

; Device-local accounting state; absent devices consume no fixed KCORE.
        .bss
ptr_io_in: .block 1
