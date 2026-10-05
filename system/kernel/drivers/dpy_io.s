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
 * PI7 is an ITS-style Type-340 BLKO data channel.  Low core feeds ordinary
 * words without entering KCORE; BLKO count overflow enters the saved-AC PI7
 * completion path.  dpy_pending remains set for a whole finite frame.
 *
 * The Type 340 is a refresh display rather than a storage display.  KINIT
 * therefore copies its compact boot banner into this MRES.  Every second real
 * 60 Hz clock tick (30 Hz) starts a replay when the controller is idle, and
 * PI7 BLKO requests stream each span without polling; only span overflow
 * enters KCORE to select the next retained-text span.
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
        .globl dpy_refresh_iowd
        .globl kret_ok
        .globl kret_busy
        .globl mm_alloc
        .globl mm_free
        .globl dpy_text_putchar
        .globl dpy_text_base
        .globl dpy_text_top
        .globl dpy_text_active
        .globl dpy_text_rows_used

        .equ DPY_TEXT_ROWS,0134
        .equ DPY_TEXT_BLOCKS,033
        .equ DPY_TEXT_ROW_WORDS,066
        ; Refresh-only states must never overlap a valid logical text row.
        ; Keep them immediately above the 0..133-octal row-number range.
        .equ DPY_REFRESH_BANNER,0134
        .equ DPY_REFRESH_ONESHOT,0135
        .equ DPY_REFRESH_TRAILER,0136

/**
 * @brief PI7 span-completion handler for the ITS-style Type-340 BLKO channel.
 * @return Does not return normally; restores the interrupted PI7 context.
 *
 * MINIT installs `BLKO 0130,dpy_refresh_iowd` directly in low-core word 056.
 * Ordinary data requests therefore execute only that one hardware instruction.
 * On the final word BLKO count overflow selects low-core word 057, whose JSR
 * enters the normal level-7 saved-AC prologue and then this routine.  PI7 is DPY-exclusive while the display is installed.
 *
 * BLKO completion occurs when the final word has just been issued, not when it
 * has finished on the display.  Intermediate spans simply arm the next IOWD;
 * its first word will be requested after the in-flight final word reaches DONE.
 * A harmless SI trailer closes finite frames so dpy_pending is cleared only
 * after all visible content has completed.
 */
dpy_pi_handler:
        .word 0                         ; JSR saves interrupted flags/PC here
        movem 1,dpy_pi_saved_ac1
        movem 2,dpy_pi_saved_ac2
        movem 3,dpy_pi_saved_ac3
        ; A persistent raw list is one continuous BLKO span.  Reload the
        ; immutable initial IOWD on overflow; the final word remains in flight
        ; and its DONE request starts the next pass.
        skipn dpy_list_base
        jrst dpy_pi_retained_done
        move  1,dpy_list_iowd
        movem 1,dpy_refresh_iowd
        jrst  dpy_pi_return

dpy_pi_retained_done:
        move  1,dpy_refresh_row
        cain  1,DPY_REFRESH_TRAILER
        jrst  dpy_pi_refresh_complete
        cain  1,DPY_REFRESH_BANNER
        jrst  dpy_pi_refresh_trailer
        cain  1,DPY_REFRESH_ONESHOT
        jrst  dpy_pi_refresh_trailer
        skipn dpy_text_active
        jrst  dpy_pi_refresh_trailer

        ; Setup uses row=-1.  Thereafter each row is one contiguous BLKO span
        ; through its final initialized two-word block, followed by one CR/LF
        ; span.  The completed IOWD RH names its final word, so the global
        ; row-end word itself is the zero-cost phase marker between those spans.
        jumpl 1,dpy_pi_text_next_row
        hrrz  2,dpy_refresh_iowd
        cain  2,dpy_text_row_end_word
        jrst  dpy_pi_text_next_row
        jrst  dpy_pi_text_row_end

dpy_pi_text_next_row:
        aos   1,dpy_refresh_row
        caml  1,dpy_text_rows_used
        jrst  dpy_pi_refresh_trailer
        move  2,1
        add   2,dpy_text_top
        cail  2,DPY_TEXT_ROWS
        subi  2,DPY_TEXT_ROWS
        imuli 2,DPY_TEXT_ROW_WORDS
        add   2,dpy_text_base          ; physical row base

        ; Find the last initialized block.  First-word zero is an authoritative
        ; trailing-blank marker; initialized blocks contain explicit spaces.
        move  3,2
        addi  3,064                    ; first word of block 26
        movei 1,DPY_TEXT_BLOCKS
dpy_pi_text_find_last:
        jumpe 1,dpy_pi_text_blank_row
        skipe (3)
        jrst  dpy_pi_text_found_last
        subi  3,2
        soja  1,dpy_pi_text_find_last

dpy_pi_text_blank_row:
        move  1,[-1,,dpy_text_row_end_word-1]
        jrst  dpy_pi_refresh_arm

dpy_pi_text_found_last:
        ; AC1 is the number of used blocks.  Two direct display words per block
        ; occupy one contiguous row prefix, so one hardware BLKO span suffices.
        lsh   1,1                     ; used words
        movn  1,1
        lsh   1,022
        subi  2,1
        hrr   1,2
        jrst  dpy_pi_refresh_arm

dpy_pi_text_row_end:
        move  1,[-1,,dpy_text_row_end_word-1]

dpy_pi_refresh_arm:
        movem 1,dpy_refresh_iowd
        jrst  dpy_pi_return

; Delay finite-frame completion by one nonprinting primary-set SI word.  BLKO
; overflow for this trailer means every visible word in the frame has reached
; DONE, so clearing ownership cannot race a PI6-triggered next frame.
dpy_pi_refresh_trailer:
        movei 1,DPY_REFRESH_TRAILER
        movem 1,dpy_refresh_row
        move  1,[-1,,dpy_text_si_word-1]
        movem 1,dpy_refresh_iowd
        jrst  dpy_pi_return

dpy_pi_refresh_complete:
        setzm dpy_refresh_iowd
        setzm dpy_pending
        cono  0130,0                   ; trailer may finish with PIA disabled
        jrst  dpy_pi_return

dpy_pi_return:
        move 3,dpy_pi_saved_ac3
        move 2,dpy_pi_saved_ac2
        move 1,dpy_pi_saved_ac1
        jrst 012,@dpy_pi_handler

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
        sosle dpy_refresh_divider
        jrst pdp10_pi_handler_return
        movei 1,2
        movem 1,dpy_refresh_divider
        jrst dpy_refresh_start

/** Start one asynchronous replay and tail-return through the PI6 dispatcher. */
dpy_refresh_start:
        skipe dpy_pending
        jrst pdp10_pi_handler_return
        skipn dpy_text_active
        jrst dpy_refresh_banner
        seto 1,
        movem 1,dpy_refresh_row
        move 1,[-2,,dpy_text_setup_words-1]
        jrst dpy_refresh_start_arm
dpy_refresh_banner:
        movei 1,DPY_REFRESH_BANNER
        movem 1,dpy_refresh_row
        move 1,[-5,,dpy_banner_words-1]
dpy_refresh_start_arm:
        movem 1,dpy_refresh_iowd
        setom dpy_pending
        ; INIT makes the Type 340 request its first word.  PI7 low core feeds it
        ; through BLKO; no DATAO is issued from the clock interrupt.
        cono 0130,000107
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
 * AC17 is only the normal return stack.  The word is staged in resident state
 * and submitted as a one-word BLKO span.  dpy_pending remains set through the
 * nonprinting completion trailer, so the spin loop exits only after the word
 * has actually completed on the Type 340.
 */
dpy_putword:
        skipn dpy_list_base
        jrst dpy_putword_idle
        jrst kret_busy
dpy_putword_idle:
        skipe dpy_pending
        jrst kret_busy
        movem 1,dpy_put_word
        move 1,[-1,,dpy_put_word-1]
        movem 1,dpy_refresh_iowd
        movei 1,DPY_REFRESH_ONESHOT
        movem 1,dpy_refresh_row
        setom dpy_pending
        cono 0130,000107
dpy_put_wait:
        skipe dpy_pending
        jrst dpy_put_wait
dpy_put_ok:
        jrst kret_ok

/**
 * @brief Replace or stop the persistent userspace Type-340 display list.
 * @param AC1 Source packed-word address; ignored when AC2 is zero.
 * @param AC2 Packed 36-bit word count; zero stops/releases the active list.
 * @return AC1 = accepted word count, or -1 on allocation/input failure.
 *
 * Replacement is transactional with respect to allocation: the old list keeps
 * refreshing while the new list is copied.  Publication runs with the DPY PIA
 * disabled, so PI7 never observes partially initialized list state.  While a
 * list is active dpy_pending remains asserted and ordinary retained-text refresh
 * naturally stays idle.  Stopping the list releases its dynamic extent and the
 * next 30-Hz clock tick resumes native-block TTY refresh.
 */
dpy_write_words:
        push 17,010
        push 17,011
        move 010,1                    ; mapped source
        move 011,2                    ; requested packed words
        jumpe 011,dpy_list_stop
        jumpe 010,dpy_list_fail

        push 17,[0]                   ; allocation-result local
        movei 5,(17)
        push 17,5                     ; fifth mm_alloc argument
        move 1,011
        movei 2,3                     ; MM_TYPE_KERNEL_DYNAMIC
        movei 3,014                   ; DPY dynamic-list owner
        setz 4,                       ; MM_ALLOC_LOW
        pushj 17,mm_alloc
        sub 17,[1,,1]
        jumpn 1,dpy_list_alloc_fail
        move 6,(17)                   ; new list base
        jumpe 6,dpy_list_alloc_fail

        ; Copy mapped userspace words before taking over the controller.
        setz 1,
        hrl 1,010
        hrr 1,6
        move 2,6
        add 2,011
        subi 2,1
        blt 1,(2)

        ; Build the immutable BLKO descriptor before publishing the list.
        move 1,011
        movn 1,1
        lsh 1,022
        move 5,6
        subi 5,1
        hrr 1,5
        movem 1,dpy_list_iowd

        ; Reset with PIA disabled, publish complete state, then let INIT request
        ; the first hardware-BLKO word.  dpy_pending remains set for list life.
        move 7,dpy_list_base          ; old extent, if any
        setzm dpy_list_base
        cono 0130,000100
        move 1,dpy_list_iowd
        movem 1,dpy_refresh_iowd
        movem 6,dpy_list_base
        setom dpy_pending
        cono 0130,000107

        jumpe 7,dpy_list_installed
        move 1,7
        movei 2,3
        movei 3,014
        pushj 17,mm_free
dpy_list_installed:
        move 1,011
        sub 17,[1,,1]
        jrst dpy_list_done

dpy_list_alloc_fail:
        sub 17,[1,,1]
dpy_list_fail:
        seto 1,
        jrst dpy_list_done

dpy_list_stop:
        move 7,dpy_list_base
        setzm dpy_list_base            ; PI7 must stop recycling first
        cono 0130,000100               ; INIT, PIA disabled
        setzm dpy_list_iowd
        setzm dpy_refresh_iowd
        setzm dpy_pending
        cono 0130,000007
        jumpe 7,dpy_list_stopped
        move 1,7
        movei 2,3
        movei 3,014
        pushj 17,mm_free
dpy_list_stopped:
        setz 1,
dpy_list_done:
        pop 17,011
        pop 17,010
        popj 17,

/**
 * @brief Update the retained Type-342 terminal image.
 * @param AC1 ASCII byte.
 * @return dpy_text_putchar() status.
 *
 * The retained-text assembly allocates the dynamic 4968-word block extent on
 * first use and updates the selected fixed-pair cell in place.  Ordinary
 * terminal output performs no Type-340 I/O and never waits for refresh.
 */
dpy_putchar:
        jrst dpy_text_putchar

        .data
; Complete frame setup.  Word 0 sets scale 1 and intensity 4, enters POINT,
; and loads X=0.  Word 1 loads Y=1016 and returns through PARAM into CHAR.
; Native text blocks thereafter contain only Type-342 character-mode words.
dpy_text_setup_words:
        .word 0020114020000
        .word 0201770060000
; Non-printing primary-set word used as the finite-frame completion trailer.
dpy_text_si_word:
        .word 0353535353535
; End one logical row while staying in the primary set.
dpy_text_row_end_word:
        .word 0353535353433

        .bss
/** Interrupted ACs for the dedicated stackless PI7 BLKO completion path. */
dpy_pi_saved_ac1:
        .block 1
dpy_pi_saved_ac2:
        .block 1
dpy_pi_saved_ac3:
        .block 1
/** Nonzero while DPY owns an active BLKO span/frame. */
dpy_pending:
        .block 1
/** Two-to-one line-clock divider: 60 Hz clock -> 30 Hz display refresh. */
dpy_refresh_divider:
        .block 1
/** Last resident CLK tick observed by the stackless PI6 display hook. */
dpy_clk_last_tick:
        .block 1
/** Base of the active persistent raw display list, or zero. */
dpy_list_base:
        .block 1
/** Immutable initial BLKO descriptor for the active persistent raw list. */
dpy_list_iowd:
        .block 1
/** Current hardware BLKO descriptor; exported to MINIT for low-core PI7. */
dpy_refresh_iowd:
        .block 1
/** One-word staging cell used by synchronous dpy_putword(). */
dpy_put_word:
        .block 1
/** Logical text row currently being streamed; -1 denotes setup span. */
dpy_refresh_row:
        .block 1
/** Compact KINIT banner; mkbootbanner currently emits exactly five words. */
dpy_banner_words:
        .block 5
