/**
 * @file dpy_io.s
 * @brief Resident interrupt-driven PDP-6 Type 340 display driver.
 *
 * DPY device 0130 uses PI7, the PDP-6's lowest interrupt priority.  The APR
 * line clock remains on PI6.  When both devices are present MINIT patches the
 * CLK handler's post-service JRST to dpy_clock_handler.  The hook is stackless
 * and starts one display refresh every second real 60 Hz tick.  DPY DONE
 * interrupts are independently handled on PI7.
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
        .globl dpy_write_words
        .globl dpy_putchar
        .globl dpy_banner_init
        .globl dpy_clk_tick_load
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
        .globl dpy_text_rows_used

        .equ DPY_TEXT_ROWS,052
        .equ DPY_TEXT_BLOCKS,016
        .equ DPY_TEXT_ROW_WORDS,034
        .equ DPY_TEXT_ROW_END,017

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
        cono 0130,000007
        ; dpy_pending denotes ownership of the Type-344 DATAO stream, not
        ; merely one outstanding word.  Keep it asserted throughout a
        ; retained refresh frame so a higher-priority PI6 clock interrupt
        ; cannot observe a false idle window between DONE and the next DATAO.
        ; A zero refresh cursor identifies the standalone dpy_putword path.
        skipn dpy_refresh_iowd
        jrst dpy_pi_refresh_complete
        move 1,dpy_refresh_iowd
        aobjn 1,dpy_pi_refresh_send
        skipn dpy_text_active
        jrst dpy_pi_refresh_complete

        ; A banner frame keeps row=DPY_TEXT_ROWS as a non-text sentinel.
        ; Text setup uses row=-1.  Normal row/block spans use row >= 0 and the
        ; packed dpy_refresh_block word (LH=end block, RH=next block).
        move  1,dpy_refresh_row
        cain  1,DPY_TEXT_ROWS
        jrst  dpy_pi_refresh_complete
        jumpl 1,dpy_pi_text_next_row
        hrrz  2,dpy_refresh_block
        caie  2,DPY_TEXT_ROW_END
        jrst  dpy_pi_text_next_block

dpy_pi_text_next_row:
        aos   1,dpy_refresh_row
        caml  1,dpy_text_rows_used
        jrst  dpy_pi_refresh_complete

        ; Locate the final nonblank six-cell block.  Zero word 0 defines a
        ; blank block regardless of stale word 1, so clearing a row costs only
        ; fourteen stores.  Store last+1 in LH and start block zero in RH.
        move  2,1
        add   2,dpy_text_top
        cail  2,DPY_TEXT_ROWS
        subi  2,DPY_TEXT_ROWS
        imuli 2,DPY_TEXT_ROW_WORDS
        add   2,dpy_text_base
        addi  2,032                    ; first word of block 13
        movei 3,DPY_TEXT_BLOCKS
dpy_pi_text_find_last:
        jumpe 3,dpy_pi_text_blank_row
        skipe (2)
        jrst  dpy_pi_text_found_last
        subi  2,2
        soja  3,dpy_pi_text_find_last
dpy_pi_text_blank_row:
        setzm dpy_refresh_block        ; completely blank row: CR/LF only
        jrst  dpy_pi_text_next_block
dpy_pi_text_found_last:
        setzm dpy_refresh_block
        hrlm  3,dpy_refresh_block

dpy_pi_text_next_block:
        hrrz  2,dpy_refresh_block
        hlrz  3,dpy_refresh_block
        caml  2,3
        jrst  dpy_pi_text_row_end

        ; Recompute the physical ring row at block boundaries.  AC2/AC3 are
        ; the PI dispatcher's private scratch and are restored before generic
        ; PI7 dispatch; no interrupted-context AC is borrowed here.
        move  1,dpy_refresh_row
        add   1,dpy_text_top
        cail  1,DPY_TEXT_ROWS
        subi  1,DPY_TEXT_ROWS
        imuli 1,DPY_TEXT_ROW_WORDS
        add   1,dpy_text_base
        move  3,2
        lsh   3,1
        add   1,3                     ; native block address
        skipn (1)
        jrst  dpy_pi_text_blank_block
        skipn 1(1)
        jrst  dpy_pi_text_simple_block

        ; Fixed-pair complex blocks explicitly select SI/SO before every glyph.
        ; Remember only the final cell's state so a following simple/blank block
        ; can be prefixed by one non-printing SI word when necessary.
        move  3,1(1)
        lsh   3,-6
        andi  3,077
        caie  3,036                   ; DPY_T342_SO
        jrst  dpy_pi_text_complex_primary
        setom dpy_refresh_shifted
        jrst  dpy_pi_text_complex_send
dpy_pi_text_complex_primary:
        setzm dpy_refresh_shifted
dpy_pi_text_complex_send:
        addi  2,1
        hrrm  2,dpy_refresh_block
        subi  1,1
        hrli  1,-3                    ; two words: initial AOBJN count -(2+1)
        jrst  dpy_pi_refresh_prime

dpy_pi_text_simple_block:
        skipn dpy_refresh_shifted
        jrst  dpy_pi_text_simple_send
        setzm dpy_refresh_shifted
        move  1,[-2,,dpy_text_si_word-1]
        jrst  dpy_pi_refresh_prime    ; retry same block after shift reset
dpy_pi_text_simple_send:
        addi  2,1
        hrrm  2,dpy_refresh_block
        subi  1,1
        hrli  1,-2
        jrst  dpy_pi_refresh_prime

dpy_pi_text_blank_block:
        skipn dpy_refresh_shifted
        jrst  dpy_pi_text_blank_send
        setzm dpy_refresh_shifted
        move  1,[-2,,dpy_text_si_word-1]
        jrst  dpy_pi_refresh_prime    ; retry same block after shift reset
dpy_pi_text_blank_send:
        addi  2,1
        hrrm  2,dpy_refresh_block
        move  1,[-2,,dpy_text_blank_word-1]
        jrst  dpy_pi_refresh_prime

dpy_pi_text_row_end:
        setzm dpy_refresh_shifted      ; row-end word begins with SI padding
        movei 2,DPY_TEXT_ROW_END
        hrrm  2,dpy_refresh_block
        move  1,[-2,,dpy_text_row_end_word-1]

dpy_pi_refresh_prime:
        aobjn 1,dpy_pi_refresh_send
        jrst  dpy_pi_refresh_complete
dpy_pi_refresh_send:
        movem 1,dpy_refresh_iowd
        hrrz 1,1
        move 1,(1)
        datao 0130,1
        jrst dpy_pi_dispatch_return
dpy_pi_refresh_complete:
        setzm dpy_refresh_iowd
        setzm dpy_pending
dpy_pi_dispatch_return:
        move 2,pdp10_pi_level_span+6
        movei 3,pdp10_pi_return_level7
        jrst pdp10_pi_dispatch

/**
 * @brief Stackless post-CLK hook installed only while a Type 340 is present.
 *
 * The ordinary CLK PI6 handler has already acknowledged/service the event.
 * Compare its monotonic tick counter with the last value observed by DPY:
 * software PI6 scheduler kicks leave the counter unchanged and return
 * immediately, while each real line-clock tick advances it.  No PUSHJ is
 * permitted here: KINIT may be interrupted with AC17 pointing into low AC
 * storage, where PUSHJ could overwrite dispatcher AC2/AC3.
 */
dpy_clock_handler:
 dpy_clk_tick_load:
        move 1,0
        camn 1,dpy_clk_last_tick
        jrst pdp10_pi_handler_return
        movem 1,dpy_clk_last_tick
        ; A userspace raw-program writer refreshes this short lease on every
        ; frame.  While it is nonzero the normal retained-text refresh stays
        ; out of the way.  If the writer exits, the lease expires by itself
        ; and text refresh resumes without a close/ioctl cleanup path.
        skipn dpy_raw_hold
        jrst dpy_clock_text
        sos dpy_raw_hold
        jrst pdp10_pi_handler_return
dpy_clock_text:
        sosle dpy_refresh_divider
        jrst pdp10_pi_handler_return
        movei 1,2
        movem 1,dpy_refresh_divider
        jrst dpy_refresh_start

/** Start one asynchronous replay and tail-return through the PI6 dispatcher. */
dpy_refresh_start:
        skipe dpy_pending
        jrst pdp10_pi_handler_return
        ; The generated banner is a complete frame relative to Type-340 reset
        ; state.  Restore that state before every replay so character/mode and
        ; beam position left by the previous frame cannot accumulate.
        cono 0130,000107              ; INIT + retain low-priority data PIA 7
        skipn dpy_text_active
        jrst dpy_refresh_banner
        seto 1,
        movem 1,dpy_refresh_row
        setzm dpy_refresh_block
        setzm dpy_refresh_shifted
        move 1,[-3,,dpy_text_setup_words-1]
        aobjn 1,dpy_refresh_start_send
        jrst pdp10_pi_handler_return
dpy_refresh_banner:
        ; Mark this span as a banner frame, not as the text setup span.
        ; dpy_text_active may become nonzero while the banner is still in
        ; flight.  Without this sentinel the final banner DONE would then
        ; fall into the text-row continuation path and feed character words
        ; without first executing dpy_text_setup_words.  Depending on the
        ; banner's ending mode those words can STOP the Type 340 and leave
        ; dpy_pending set forever.
        movei 1,DPY_TEXT_ROWS
        movem 1,dpy_refresh_row
        move 1,[-6,,dpy_banner_words-1]
        aobjn 1,dpy_refresh_start_send
        jrst pdp10_pi_handler_return
dpy_refresh_start_send:
        movem 1,dpy_refresh_iowd
        hrrz 1,1
        move 1,(1)
        setom dpy_pending
        datao 0130,1
        jrst pdp10_pi_handler_return

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
        setzm dpy_refresh_iowd
        setom dpy_pending
        datao 0130,1
dpy_put_wait:
        skipe dpy_pending
        jrst dpy_put_wait
dpy_put_ok:
        jrst kret_ok

/**
 * @brief Execute one complete userspace native Type-340 program.
 * @param AC1 Program words.
 * @param AC2 Number of 36-bit words.
 * @return AC1 = words executed, or zero for an empty program.
 *
 * This is the raw /DEV/DPY0 transport used by DPYVIEW.  It owns no retained
 * graphics buffer: userspace replays the program at refresh rate.  A short
 * raw lease suppresses the ordinary text refresher while frames keep arriving.
 * The first word is always relative to Type-340 reset state.
 *
 * AC4..AC7 are caller-scratch.  The routine is synchronous so the mapped
 * userspace source buffer remains valid until every word has reached DONE.
 */
dpy_write_words:
        jumpe 2,dpy_write_words_zero
        move 4,1                     ; current source
        move 5,2                     ; remaining
        move 6,2                     ; original count / return value
        movei 7,4                    ; four 60-Hz ticks ~= 67 ms
        movem 7,dpy_raw_hold
dpy_write_wait_idle:
        skipe dpy_pending
        jrst dpy_write_wait_idle
        ; Start each submission from a deterministic display state.  DPYVIEW
        ; submits a complete frame in one WRITE_WORDS call.
        cono 0130,000107
        setzm dpy_refresh_iowd
dpy_write_loop:
        move 1,(4)
        setom dpy_pending
        datao 0130,1
dpy_write_wait_done:
        skipe dpy_pending
        jrst dpy_write_wait_done
        addi 4,1
        sojg 5,dpy_write_loop
        move 1,6
        popj 17,
dpy_write_words_zero:
        setz 1,
        popj 17,

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
; Native text blocks thereafter contain only Type-342 character-mode words.
dpy_text_setup_words:
        .word 0020134020000
        .word 0201716060000
; One blank six-cell block in primary character mode.
dpy_text_blank_word:
        .word 0404040404040
; Non-printing primary-set reset between a shifted complex block and a simple.
dpy_text_si_word:
        .word 0353535353535
; End one logical row while staying in the primary set.
dpy_text_row_end_word:
        .word 0353535353433

        .bss
/** Nonzero while one DATAO word is awaiting the Type 340 DONE interrupt. */
dpy_pending:
        .block 1
/** Two-to-one line-clock divider: 60 Hz clock -> 30 Hz display refresh. */
dpy_refresh_divider:
        .block 1
/** Last resident CLK tick observed by the stackless PI6 display hook. */
dpy_clk_last_tick:
        .block 1
/** Remaining real line-clock ticks for userspace raw-display ownership. */
dpy_raw_hold:
        .block 1
/** AOBJN state: negative remaining count in LH, current banner address in RH. */
dpy_refresh_iowd:
        .block 1
/** Logical text row currently being streamed; -1 denotes setup span. */
dpy_refresh_row:
        .block 1
/** Packed native-block cursor: LH=last block + 1, RH=next block/sentinel. */
dpy_refresh_block:
        .block 1
/** Nonzero when the preceding complex block's final cell selected SO. */
dpy_refresh_shifted:
        .block 1
/** Compact KINIT banner; mkbootbanner currently emits exactly five words. */
dpy_banner_words:
        .block 5
