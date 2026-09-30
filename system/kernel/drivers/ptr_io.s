/**
 * @file ptr_io.s
 * @brief Resident PDP-6 paper-tape reader driver for device 0104.
 *
 * PTR owns a local PI7 leaf so a PTR-only machine does not load unrelated PI7
 * peripherals. ptr_state has three meanings: zero is idle/no byte, -1 is an
 * outstanding caller-started read, and a positive value is byte+1 prefetched
 * by PI service. The +1 representation keeps NUL distinct from empty state.
 *
 * When DONE arrives without an active caller, the handler leaves the hardware
 * byte pending and disables PI. A later caller consumes it directly with
 * DATAI. This provides one-byte hardware-assisted prefetch without extra BSS.
 */
        .globl mfsdev_io_in
        .text
        .globl ptr_pi_handler
        .globl ptr_getchar
        .globl pdp10_pi_handler_return
        .globl kret_arg
        .globl kret_busy
        .globl kret_ok

/**
 * @brief Service PTR DONE on PI7.
 * @return Does not return normally; jumps to pdp10_pi_handler_return.
 *
 * AC1 is untouched and AC2/AC3/AC17 are preserved for the generic PI ABI. If
 * ptr_state is -1, DATAI stores the byte and AOS converts it to byte+1 before
 * PI is disabled. If no caller owns the request, DONE is retained as a
 * hardware-prefetched byte with PI disabled.
 */
ptr_pi_handler:
        conso 0104,0010
        jrst pdp10_pi_handler_return
        skipn ptr_state
        jrst ptr_pi_prefetch
        datai 0104,ptr_state
        aos mfsdev_io_in+2
        aos ptr_state
        cono 0104,0
        jrst pdp10_pi_handler_return
ptr_pi_prefetch:
        cono 0104,0010
        jrst pdp10_pi_handler_return

/**
 * @brief Read one eight-bit PTR byte into caller storage.
 * @param AC1 Address of destination integer; must be nonzero.
 * @return AC1 = 0, PT_E_ARG (-1), PT_E_TIMEOUT (-2), or PT_E_BUSY (-3).
 *
 * AC4 preserves the destination pointer; AC2/AC3/AC5 are scratch. If DONE is
 * already set the byte is consumed synchronously. Otherwise ptr_state becomes
 * -1 before CONO starts the reader, closing the completion race. Timeout
 * disables PI and releases software ownership; a delayed hardware completion
 * remains available for the next call.
 */
ptr_getchar:
        jumpe 1,kret_arg
        move 4,1
        move 2,ptr_state
        jumpg 2,ptr_get_software
        jumpl 2,kret_busy
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
        jrst kret_neg2
ptr_get_hardware:
        datai 0104,3
        aos mfsdev_io_in+2
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
        jrst kret_ok

        .bss
/** 0 idle, -1 active request, positive byte+1 completed/prefetched value. */
ptr_state:
        .block 1
