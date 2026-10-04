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
        .globl dpy_text_putchar
        .globl dpy_text_base
        .globl dpy_text_top
        .globl dpy_text_active

        .equ DPY_TEXT_ROWS,052
        .equ DPY_TEXT_LENGTH_OFF,01114
        .equ DPY_TEXT_PROG_OFF,01166
        .equ DPY_TEXT_PROG_WORDS,035

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
        skipn dpy_refresh_iowd
        jrst pdp10_pi_dispatch

        ; BLKO updates the resident IOWD in place and feeds the next display
        ; word directly.  Non-final transfers skip the following JRST; the
        ; final transfer falls through so the next row can be prepared while
        ; the just-issued word is still being displayed.
        blko 0130,dpy_refresh_iowd
        jrst dpy_pi_refresh_span_done
        setom dpy_pending
        jrst pdp10_pi_dispatch

dpy_pi_refresh_span_done:
        setom dpy_pending
        skipn dpy_text_active
        jrst dpy_pi_refresh_frame_done

        ; The setup span has row = -1.  Thereafter each exhausted row selects
        ; the next physical row through the 42-row scroll ring.
        aos 1,dpy_refresh_row
        cail 1,DPY_TEXT_ROWS
        jrst dpy_pi_refresh_frame_done
        add 1,dpy_text_top
        cail 1,DPY_TEXT_ROWS
        subi 1,DPY_TEXT_ROWS
        movem 1,dpy_refresh_phys

        move 1,dpy_text_base
        addi 1,DPY_TEXT_LENGTH_OFF
        add 1,dpy_refresh_phys
        hrrz 1,(1)                    ; compiled words in this physical row
        jumpe 1,dpy_pi_refresh_frame_done
        movn 1,1
        lsh 1,022
        hllm 1,dpy_refresh_iowd

        move 1,dpy_refresh_phys
        imuli 1,DPY_TEXT_PROG_WORDS
        add 1,dpy_text_base
        addi 1,DPY_TEXT_PROG_OFF
        subi 1,1
        hrrm 1,dpy_refresh_iowd
        jrst pdp10_pi_dispatch
dpy_pi_refresh_frame_done:
        setzm dpy_refresh_iowd
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
        skipn dpy_text_active
        jrst dpy_refresh_banner
        seto 1,
        movem 1,dpy_refresh_row
        move 1,[-2,,dpy_text_setup_words-1]
        movem 1,dpy_refresh_iowd
        setom dpy_pending
        blko 0130,dpy_refresh_iowd
        jrst dpy_refresh_start_done
dpy_refresh_start_done:
        popj 017,
dpy_refresh_banner:
        move 1,[-5,,dpy_banner_words-1]
        movem 1,dpy_refresh_iowd
        setom dpy_pending
        blko 0130,dpy_refresh_iowd
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
 * @brief Update the retained Type-342 terminal image.
 * @param AC1 ASCII byte.
 * @return dpy_text_putchar() status.
 *
 * The C helper allocates the dynamic text/cache extent on first use and
 * updates the appropriate cached row.  Ordinary terminal output performs no
 * Type-340 DATAO and never waits for DONE.
 */
dpy_putchar:
        jrst dpy_text_putchar

        .data
; Complete frame setup.  Word 0 sets scale 2 and intensity 4, enters POINT,
; and loads X=0.  Word 1 loads Y=974 and returns through PARAM into CHAR.
; Cached rows thereafter contain only Type-342 character-mode words.
dpy_text_setup_words:
        .word 0020134020000
        .word 0201716060000

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
/** Logical text row currently being streamed; -1 denotes setup span. */
dpy_refresh_row:
        .block 1
/** Physical ring row selected while crossing one compiled row boundary. */
dpy_refresh_phys:
        .block 1
/** Compact KINIT banner; mkbootbanner currently emits exactly five words. */
dpy_banner_words:
        .block 5
