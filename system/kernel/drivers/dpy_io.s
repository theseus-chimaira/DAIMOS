/**
 * @file dpy_io.s
 * @brief Resident interrupt-driven PDP-6 Type 340 display driver.
 *
 * DPY device 0130 uses PI7, the PDP-6's lowest interrupt priority.  The APR
 * line clock remains on PI6.  When both devices are present MINIT replaces the
 * ordinary CLK PI6 entry with dpy_clock_handler, whose only extra job is to
 * start one display refresh every second real 60 Hz tick.  DPY DONE interrupts
 * are independently handled on PI7.
 *
 * Exactly one display word may be in flight. dpy_pending is set before DATAO
 * and cleared only by a real DONE interrupt, so a caller cannot observe
 * completion before the Type 340 has executed both 18-bit halves.
 *
 * The Type 340 is a refresh display rather than a storage display.  KINIT
 * therefore copies its compact boot banner into this MRES.  Every second real
 * 60 Hz clock tick (30 Hz) starts a replay when the controller is idle, and
 * PI7 DONE interrupts chain the remaining words without polling.
 *
 * None of this code executes when no Type 340 is present: MINIT leaves the
 * ordinary CLK PI6 handler installed unless the DPY probe succeeds and this
 * optional MRES is installed.
 */

        .text
        .globl dpy_pi_handler
        .globl dpy_clock_handler
        .globl dpy_putword
        .globl dpy_putchar
        .globl dpy_banner_init
        .globl dpy_clk_pi_service_call
        .globl pdp10_pi_handler_return
        .globl pdp10_pi_dispatch
        .globl pdp10_pi_level_span
        .globl pdp10_pi_return_level7
        .globl kret_ok
        .globl kret_busy

/**
 * @brief PI7 pre-handler for a possible Type 340 DONE interrupt.
 * @return Does not return normally; continues through the normal PI7 table.
 *
 * KINIT patches the existing PI7 span-load instruction to jump here only when
 * DPY is present.  Recreate the displaced span/cursor setup, service DPY DONE
 * if asserted, then enter the normal table dispatcher.  SLV and any future
 * ordinary PI7 handlers therefore retain the standard table semantics and DPY
 * consumes no pdp10_pi_handlers[] slot.  AC2/AC3 are dispatcher-owned state.
 */
dpy_pi_handler:
        move 2,pdp10_pi_level_span+6
        movei 3,pdp10_pi_return_level7
        conso 0130,000200
        jrst pdp10_pi_dispatch
        setzm dpy_pending
        cono 0130,000007
        move 1,dpy_refresh_iowd
        aobjn 1,dpy_pi_refresh_send
        jrst pdp10_pi_dispatch
dpy_pi_refresh_send:
        movem 1,dpy_refresh_iowd
        hrrz 1,1
        move 1,(1)
        setom dpy_pending
        datao 0130,1
        jrst pdp10_pi_dispatch

/**
 * @brief PI6 clock wrapper installed only while a Type 340 is present.
 *
 * Software PI6 scheduler kicks have no APR clock flag and therefore pass
 * straight to CLK without consuming the display divider.  A real line-clock
 * tick first receives the normal CLK service, then every second tick starts a
 * refresh.  Systems without DPY never install this wrapper and execute the
 * original CLK handler with no display overhead.
 */
dpy_clock_handler:
        conso 0000,01000
        jrst dpy_clock_service
dpy_clk_pi_service_call:
        pushj 017,kret_ok
        sosle dpy_refresh_divider
        jrst pdp10_pi_handler_return
        movei 1,2
        movem 1,dpy_refresh_divider
        pushj 017,dpy_refresh_start
        jrst pdp10_pi_handler_return
dpy_clock_service:
        pushj 017,dpy_clk_pi_service_call
        jrst pdp10_pi_handler_return

/** Start one asynchronous replay of the retained display list when idle. */
dpy_refresh_start:
        skipe dpy_pending
        popj 017,
        ; The generated banner is a complete frame relative to Type-340 reset
        ; state.  Restore that state before every replay so character/mode and
        ; beam position left by the previous frame cannot accumulate.
        cono 0130,000107              ; INIT + retain low-priority data PIA 7
        move 1,[-6,,dpy_banner_words-1]
        aobjn 1,dpy_refresh_start_send
        popj 017,
dpy_refresh_start_send:
        movem 1,dpy_refresh_iowd
        hrrz 1,1
        move 1,(1)
        setom dpy_pending
        datao 0130,1
        popj 017,

/**
 * @brief Retain the transient KINIT boot display list for periodic replay.
 * @param AC1 Source address of the fixed five-word generated banner.
 * @return AC1 = 0.
 */
dpy_banner_init:
        movei 3,dpy_banner_words
        movei 2,5
dpy_banner_copy:
        move 4,(1)
        movem 4,(3)
        addi 1,1
        addi 3,1
        sojg 2,dpy_banner_copy
        jrst kret_ok

/**
 * @brief Submit one Type 340 word and synchronously await interrupt completion.
 * @param AC1 One 36-bit word containing two Type 340 instructions.
 * @return AC1 = DPY_E_OK (0) or DPY_E_BUSY (-3).
 *
 * AC17 is only the normal return stack. No scratch AC is needed. Setting the
 * pending flag before DATAO closes the completion race; the spin loop exits
 * only after dpy_pi_handler observes DONE and clears the flag.
 */
dpy_putword:
        skipe dpy_pending
        jrst kret_busy
dpy_put_start:
        setom dpy_pending
        datao 0130,1
dpy_put_wait:
        skipe dpy_pending
        jrst dpy_put_wait
dpy_put_ok:
        jrst kret_ok

/**
 * @brief Render one terminal byte with the Type 342 character generator.
 * @param AC1 ASCII byte.
 * @return dpy_putword() status.
 *
 * Every call is self contained: PARAM->CHAR, SI/SO + character/control + ESC.
 * Thus no cursor/shift state is retained in resident RAM.  Type 342 CR/LF are
 * native controls; shifted code 072 is the six-unit cursor-left used for BS.
 */
dpy_putchar:
        andi 1,0377
        movei 2,035                  ; SI / primary character set
        caie 1,010
        jrst dpy_putchar_lf
        movei 2,036                  ; SO / shifted set
        movei 1,072                  ; cursor left six units
        jrst dpy_putchar_pack
dpy_putchar_lf:
        caie 1,012
        jrst dpy_putchar_cr
        movei 1,033
        jrst dpy_putchar_pack
dpy_putchar_cr:
        caie 1,015
        jrst dpy_putchar_lower
        movei 1,034
        jrst dpy_putchar_pack
dpy_putchar_lower:
        caige 1,0141
        jrst dpy_putchar_primary
        caile 1,0172
        jrst dpy_putchar_bad
        subi 1,0140
        movei 2,036                  ; SO / lower-case set
        jrst dpy_putchar_pack
dpy_putchar_primary:
        caige 1,040
        jrst dpy_putchar_bad
        caile 1,077
        jrst dpy_putchar_upper
        jrst dpy_putchar_pack
dpy_putchar_upper:
        caige 1,0101
        jrst dpy_putchar_bad
        caile 1,0132
        jrst dpy_putchar_bad
        andi 1,077                   ; A..Z -> Type-342 codes 1..032
        jrst dpy_putchar_pack
dpy_putchar_bad:
        movei 1,077                  ; unsupported byte -> '?'
dpy_putchar_pack:
        lsh 2,014                    ; first Type-342 character
        lsh 1,6
        ior 1,2
        ori 1,037                    ; ESC returns display to parameter mode
        hrli 1,060000                ; left half: PARAM -> CHAR mode
        jrst dpy_putword

        .bss
/** Nonzero while one DATAO word is awaiting the Type 340 DONE interrupt. */
dpy_pending:
        .block 1
/** Two-to-one line-clock divider: 60 Hz clock -> 30 Hz display refresh. */
dpy_refresh_divider:
        .block 1
/** AOBJN state: negative remaining count in LH, current banner address in RH. */
dpy_refresh_iowd:
        .block 1
/** Compact KINIT banner; mkbootbanner currently emits exactly five words. */
dpy_banner_words:
        .block 5
