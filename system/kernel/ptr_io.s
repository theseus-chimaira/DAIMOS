; ptr_io.s -- compact resident PDP-6 paper-tape reader driver.
;
; ptr_state: 0 idle, -1 waiting, byte+1 ready.  The +1 representation lets
; the direct PI7 handler store DATAI straight to memory and mark readiness
; with one AOS, including for byte zero.

        .text
        .globl devicefs_io_in
        .globl devicefs_io_out
        .globl ptr_getchar
        .globl ptr_state
        .globl pdp10_ret_arg
        .globl io7_ret_timeout
        .globl pdp10_ret_busy
        .globl pdp10_ret_ok

; AC1 = int *destination.  Return 0 or PT_E_ARG/BUSY/TIMEOUT.
ptr_getchar:
        jumpe 1,pdp10_ret_arg
        move 4,1
        move 2,ptr_state
        jumpg 2,ptr_get_software
        jumpl 2,pdp10_ret_busy

        ; Consume an already-prefetched hardware character without waiting.
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
        jrst io7_ret_timeout

ptr_get_hardware:
        datai 0104,3
        aos devicefs_io_in+1
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

        .bss
ptr_state:
        .block 1
