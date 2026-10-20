/**
 * @file tape_io.s
 * @brief Resident PDP-6 DECtape and Type 516 magnetic-tape driver.
 *
 * DTC and MTC share one MRES because both transports use the Type 136 data
 * channel and substantial common transfer/event machinery. The disk controller
 * is independent in dsk_io.s. KINIT installs this package once when either tape
 * controller is present, then publishes only the services belonging to the
 * detected controller(s).
 *
 * storage_state is the shared tape-operation discriminator while a transfer is
 * active. The negative data-transfer states are deliberately chosen so MOVNS
 * on completion yields direct indices into tape_account_table:
 *   -1 DTC read, -2 MTC read, -5 MTC write, -6 DTC write.
 * State 7 is the common I/O-error result. Positive/zero storage_count values are
 * transfer word counts; negative values are wall-clock timeout countdowns aged
 * by storage_clock_tick at 60 Hz.
 *
 * DTC keeps a one-word motion estimate per unit so sequential requests can
 * choose direction without first stopping every transport. Any global DTC stop
 * invalidates all eight estimates. Runtime transfer completion uses one event
 * word because DTC and MTC are mutually exclusive Type 136 owners.
 */

        .text
        .globl mfsdev_io_in
        .globl mfsdev_io_out
        .globl mfsdev_storage_errors
        .globl tape_pi_handler
        .globl tape_dct_handler
        .globl dtc_read_block
        .globl dtc_write_block
        .globl mtc_service
        .globl pdp10_pi_dispatch_done
        .globl storage_state
        .globl storage_iowd
        .globl storage_count
        .globl kret_ok
        .globl kret_arg
        .globl kret_busy
        .globl proc_wait_event
        .globl proc_wakeup_event

/**
 * Polled controller/search paths use wall-clock timeouts. storage_count is
 * negative only while one of these timers is armed; storage_clock_tick moves
 * negative counts toward zero at 60 Hz without disturbing positive transfer
 * word counts. MTC spacing/rewind may legitimately span a full reel.
 */
        ; One 18-bit DECtape end-to-end traversal is roughly 32 seconds at
        ; nominal line speed.  A transport may legitimately coast near the
        ; opposite end while another member is being scanned, so allow one
        ; full traversal plus reversal/settling margin before declaring the
        ; search dead.  This only extends the failure bound; successful seeks
        ; still complete as soon as the requested block arrives.
        .set DTC_SEARCH_TIMEOUT_TICKS,05214      ; 45 seconds
        .set MTC_CONTROL_TIMEOUT_TICKS,0151440   ; 15 minutes
        .set DTC_SEARCH_TIMEOUT_NEG_RH,01000000-DTC_SEARCH_TIMEOUT_TICKS
        .set MTC_CONTROL_TIMEOUT_NEG_RH,01000000-MTC_CONTROL_TIMEOUT_TICKS

/**
 * @brief Service DTC/MTC PI5 status for the current Type 136 tape owner.
 * @return Does not return normally; jumps to pdp10_pi_dispatch_done.
 *
 * AC1/AC2 are scratch under the direct storage-router completion ABI. DTC and
 * MTC status/error interpretation is selected entirely from storage_state.
 * Successful controller completion negates the active negative state so the
 * waiter sees 1/2/5/6; errors publish state 7 and increment the device-specific
 * MonitorFS storage-error counter. Cleanup releases all tape-side Type 136
 * ownership and invalidates cached DECtape motion after a global stop.
 */
tape_pi_handler:
        move 2,storage_state
        aoje 2,tape_pi_dtc_status
        aoje 2,tape_pi_mtc_status
        aoje 2,pdp10_pi_dispatch_done
        aoje 2,pdp10_pi_dispatch_done
        aoje 2,tape_pi_mtc_write_status

tape_pi_dtc_write_status:
        seto 2,
tape_pi_dtc_status:
        coni 0214,1
        trne 1,0000034
        jrst tape_pi_dtc_block_error
        jumpe 2,pdp10_pi_dispatch_done
        trnn 1,0000001
        jrst pdp10_pi_dispatch_done
        jrst tape_pi_done

tape_pi_mtc_write_status:
        seto 2,
tape_pi_mtc_status:
        coni 0224,1
        trne 1,0400520
        jrst tape_pi_error
        trnn 1,0000004
        jrst pdp10_pi_dispatch_done
        jumpn 2,tape_pi_mtc_idle_check
        coni 0200,1
        trne 1,002000
        jrst pdp10_pi_dispatch_done
        coni 0224,1
tape_pi_mtc_idle_check:
        trnn 1,0000001
        jrst pdp10_pi_dispatch_done

tape_pi_done:
        movns storage_state
        jrst tape_pi_wake_cleanup

tape_pi_error:
        aos mfsdev_storage_errors+1   ; MTC0
        jrst tape_pi_error_common

tape_pi_dtc_block_error:
        aos mfsdev_storage_errors     ; DTC0
        move 1,dtc_request_unit
        setzm dtc_motion(1)
tape_pi_error_common:
        movei 1,7
        movem 1,storage_state
tape_pi_wake_cleanup:
        pushj 017,tape_transfer_wakeup
tape_pi_cleanup:
        ; CONO DTC,0 stops every selected Type-551 transport, so the common
        ; owner cleanup also covers DTC block errors without a second
        ; unit-select/stop sequence.  A global stop invalidates every cached
        ; motion estimate, not just the unit which happened to own the failed
        ; request: later unit probes must not treat a stopped transport as if
        ; it were still coasting past the last observed block.
        cono 0224,0
        cono 0210,0
        cono 0200,0
        pushj 017,dtc_forget_motion
        jrst pdp10_pi_dispatch_done

/**
 * @brief Service Type 136 PI3 data-channel requests for the active tape owner.
 * @return Does not return normally; jumps to pdp10_pi_dispatch_done.
 *
 * Reverse DECtape transfers are serviced one word at a time because the memory
 * address walks backward; forward DTC and MTC transfers use BLKI/BLKO and reach
 * the common completion selector only when the block instruction falls through.
 * The selector is self-patched for the two extra DCT acknowledgements required
 * by write pipelines.
 */
tape_dct_handler:
        ; A block-number interrupt must claim the narrow RBN -> DATA SYNC
        ; handoff before the lower-priority clock/scheduler PI6 can run.
        ; During search storage_state is >= 0200; transfers use negative
        ; states and completed transfers are < 0200.
        move 2,storage_state
        caige 2,0200
        jrst tape_dct_data_handler
        jrst dtc_search_pi_handler
tape_dct_data_handler:
        skipn dtc_request_reverse
        jrst tape_dct_select
        skipg storage_count
        jrst tape_dct_select
        jrst tape_dct_reverse_word
tape_dct_select:
        jrst tape_dct_count_done

tape_dct_reverse_word:
        skipn dtc_request_write
        jrst tape_dct_reverse_read
        hrrz 2,storage_iowd
        move 1,(2)
        datao 0200,1
        jrst tape_dct_reverse_advance
tape_dct_reverse_read:
        datai 0200,1
        hrrz 2,storage_iowd
        movem 1,(2)
tape_dct_reverse_advance:
        sosle storage_count
        jrst tape_dct_reverse_more
        movei 1,0200
        movem 1,storage_count
        hrrzs storage_iowd
        jrst tape_dct_count_done
tape_dct_reverse_more:
        sos storage_iowd
        jrst pdp10_pi_dispatch_done

tape_dct_count_done:
        move 2,storage_state
        aoje 2,tape_dct_done_keep_dtc
        aoje 2,tape_dct_mtc_read_full
        aoje 2,pdp10_pi_dispatch_done
        aoje 2,pdp10_pi_dispatch_done
        aoje 2,tape_dct_mtc_write_arm

tape_dct_dtc_write_arm:
        movei 1,tape_dct_dtc_write_ack1
        hrrm 1,tape_dct_select
        jrst tape_dct_arm_handler

tape_dct_mtc_write_arm:
        movei 1,tape_dct_mtc_write_drain
        hrrm 1,tape_dct_select

tape_dct_arm_handler:
        move 1,000047
        movem 1,000046
        jrst pdp10_pi_dispatch_done

tape_dct_done_keep_dtc:
        move 1,dtc_request_unit
        move 2,dtc_motion(1)
        lsh 1,3
        iori 1,0220000
        jumpge 2,tape_dct_keep_forward
        iori 1,0010000
tape_dct_keep_forward:
        cono 0210,0(1)
        cono 0200,0
        movns storage_state
        pushj 017,tape_transfer_wakeup
        jrst pdp10_pi_dispatch_done

tape_dct_mtc_read_full:
        coni 0224,1
        trne 1,0400520
        jrst tape_pi_error
        trnn 1,0000004
        jrst tape_pi_error
        coni 0200,1
        trne 1,002000
        jrst tape_pi_error
        jrst pdp10_pi_dispatch_done

tape_dct_dtc_write_ack1:
        setz 1,
        datao 0200,1
        movei 1,tape_dct_dtc_write_ack2
        hrrm 1,tape_dct_select
        jrst pdp10_pi_dispatch_done
tape_dct_dtc_write_ack2:
        move 1,storage_state
        addi 1,6
        jumpe 1,tape_dct_done_keep_dtc
        cono 0200,0
        cono 0210,0
        jrst tape_pi_done

tape_dct_mtc_write_drain:
        coni 0200,1
        andi 1,0777770
        cono 0200,0(1)
        jrst pdp10_pi_dispatch_done

/** @brief Publish the single shared tape transfer event and wake its waiter. */
tape_transfer_wakeup:
        setom tape_transfer_event
        movei 1,tape_transfer_event
        jrst proc_wakeup_event

/**
 * @brief Build the common Type 136 direct-transfer IOWD and PI3 vector.
 * @param AC2 Buffer address; @param AC3 positive transfer word count.
 * @return storage_count/storage_iowd and low-memory DCT vector armed.
 *
 * AC4 is scratch. The event is cleared before the DCT vector becomes live,
 * preventing completion from racing ahead of waiter initialization.
 */
tape_setup_read:
        move 4,tape_dct_blki
        jrst tape_setup_common
tape_setup_write:
        move 4,tape_dct_blko
tape_setup_common:
        movem 3,storage_count
        ; Arm the event before either controller can begin issuing PI requests.
        setzm tape_transfer_event
        movem 4,000046
        movei 4,tape_dct_count_done
        hrrm 4,tape_dct_select
        subi 2,1
        movn 4,3
        hrl 2,4
        movem 2,storage_iowd
        popj 017,
tape_dct_blki:
        blki 0200,storage_iowd
tape_dct_blko:
        blko 0200,storage_iowd

/**
 * @brief Read one 128-word DECtape physical block.
 * @param AC1 Unit 0..7; @param AC2 block 0..01101; @param AC3 destination.
 * @return AC1 = 0, busy/argument status, or storage I/O error.
 */
dtc_read_block:
        setz 4,
        jrst dtc_block_start
/** @brief Write one 128-word DECtape physical block; ABI matches read. */
dtc_write_block:
        movei 4,1
dtc_block_start:
        skipe storage_state
        jrst kret_busy
        caile 1,7
        jrst kret_arg
        hlrz 6,2
        addi 6,0200
        hrrzs 2
        caile 2,01101
        jrst kret_arg
        movem 4,dtc_request_write
        setzm dtc_request_reverse
        movem 6,storage_state
        movem 1,dtc_request_unit
        movem 2,dtc_target_block
        movei 6,7
        movem 6,dtc_search_retries
        ; The Type-551 supplies only a few mark-track lines between the
        ; block-number DATAI and the next DATA SYNC.  Arm the complete DCT
        ; DMA descriptor, PI3 vector, and waiter event *before* searching.
        ; The search DCT command runs with PIA=0, so this cannot trigger a
        ; premature BLKI/BLKO.  Search timers reuse storage_count and the
        ; 0200-word count is restored as soon as the block number matches.
        ; Waiting until after the match sometimes missed DATA SYNC on AAP,
        ; silently reading an adjacent block without an information error.
        move 5,2                       ; preserve desired physical block
        move 6,3                       ; preserve data buffer
        move 2,3
        move 3,storage_state           ; 0200 or 0400-word physical run
        skipn dtc_request_write
        jrst dtc_prearm_read
        pushj 017,tape_setup_write
        jrst dtc_prearm_done
dtc_prearm_read:
        pushj 017,tape_setup_read
dtc_prearm_done:
        ; RBN requests must enter PI3 via its saved JSR vector at 000047.
        ; Installing a direct JRST here bypasses the interrupt return frame
        ; and can return to the interrupted process as ordinary code.
        ; Type-136 is still unselected so this swap cannot race an interrupt.
        move 4,000047
        movem 4,000046
        move 3,6
        move 2,5
        move 5,dtc_motion(1)
        jumpe 5,dtc_search_begin
        move 6,5
        movms 6
        soj 6,
        sub 6,2
        jumple 6,dtc_search_begin
dtc_search_choose_reverse:
        movei 6,0010000
        movem 6,dtc_request_reverse

dtc_search_begin:
        ; The Type-136 reports RBN data on PI3, not by polling DATAI in the
        ; interrupted process.  No user-mode or PI6 scheduling delay may
        ; intervene between recognizing a block mark and arming its data.
        cono 0200,004043
        pushj 017,dtc_search_command
dtc_search_wait:
        move 4,storage_state
        caie 4,7
        jrst dtc_search_state_ok
        jrst tape_ioerr
dtc_search_state_ok:
        caige 4,0200
        jrst tape_transfer_wait
        coni 0214,4
        ; At the physical end zone the Type-551 can raise both TAPE END and
        ; a mark-track INFO ERROR as its serial decoder loses framing.  TAPE
        ; END is the authoritative transport boundary: reverse the search
        ; before generic error retry.  Retrying in the same direction with
        ; GO cleared would strand the controller at the end of the reel.
        trne 4,0000002
        jrst dtc_search_turn
        trne 4,0000034
        jrst dtc_search_retry
        skipn storage_count
        jrst dtc_search_fail
        jrst dtc_search_wait
dtc_search_turn:
        movei 6,0010000
        xorm 6,dtc_request_reverse
dtc_search_retry:
        sosle dtc_search_retries
        jrst dtc_search_continue
        jrst dtc_search_fail
dtc_search_continue:
        pushj 017,dtc_search_command
        jrst dtc_search_wait
dtc_search_command:
        ; Bound each block-search wait by elapsed line-clock time, not by CPU
        ; instruction count.  Long device motion must not expire faster on a
        ; faster processor or simulator.
        hrroi 4,DTC_SEARCH_TIMEOUT_NEG_RH
        movem 4,storage_count
        move 4,1
        lsh 4,3
        iori 4,0220200
        ior 4,dtc_request_reverse
        cono 0210,0(4)
        popj 017,

/**
 * @brief Accept a DECtape block-number word at Type-136 PI3 priority.
 *
 * AAP and the real 551 do not stop physical tape motion while software
 * handles RBN.  The block number becomes available only a few tape lines
 * before data-start synchronization.  Polling in the process context lets
 * a line-clock PI6 scheduler interrupt consume that window.  The 551 then
 * quietly reads the NEXT physical block, producing 128 valid but incorrect
 * words with no 551 information error.  Read the mark, compare the target,
 * and arm the prebuilt data transfer here, where PI6 cannot preempt us.
 *
 * AC1 and AC2 are PI-dispatch scratch.  No process-private stack, AC3..AC7,
 * or dynamically allocated kernel state is used by this interrupt path.
 */
dtc_search_pi_handler:
        conso 0200,0001000
        jrst pdp10_pi_dispatch_done
        datai 0200,1
        andi 1,001777
        movem 1,dtc_last_mark
        move 2,1
        aoj 2,
        skipn dtc_request_reverse
        movns 2
        move 1,dtc_request_unit
        movem 2,dtc_motion(1)
        move 1,dtc_last_mark
        sub 1,dtc_target_block
        jumpe 1,dtc_search_pi_found
        skipn dtc_request_reverse
        jrst dtc_search_pi_forward
        jumpg 1,dtc_search_pi_continue
        jrst dtc_search_pi_turn
dtc_search_pi_forward:
        jumpl 1,dtc_search_pi_continue
dtc_search_pi_turn:
        movei 2,0010000
        xorm 2,dtc_request_reverse
dtc_search_pi_retry:
        sosle dtc_search_retries
        jrst dtc_search_pi_continue
        movei 2,7
        movem 2,storage_state
        jrst pdp10_pi_dispatch_done
dtc_search_pi_continue:
        ; Each completed block mark/reversal restarts the bounded search.
        hrroi 2,DTC_SEARCH_TIMEOUT_NEG_RH
        movem 2,storage_count
        move 1,dtc_request_unit
        lsh 1,3
        iori 1,0220200
        ior 1,dtc_request_reverse
        cono 0210,0(1)
        jrst pdp10_pi_dispatch_done

dtc_search_pi_found:
        ; A reverse read/write is valid only for a single block.  If the
        ; transfer spans two blocks, turn around while retaining the RBN
        ; JSR vector.  Do not arm BLKI/BLKO until the direction is settled.
        skipn dtc_request_reverse
        jrst dtc_search_pi_found_direction_ok
        move 2,storage_state
        caie 2,0200
        jrst dtc_search_pi_turn
dtc_search_pi_found_direction_ok:
        move 2,tape_dct_blki
        skipn dtc_request_write
        jrst dtc_search_pi_vector_ready
        move 2,tape_dct_blko
dtc_search_pi_vector_ready:
        movem 2,000046
        skipn dtc_request_reverse
        jrst dtc_search_pi_setup
        hrrz 2,storage_iowd
        addi 2,0200
        hrrm 2,storage_iowd
        move 2,000047
        movem 2,000046
dtc_search_pi_setup:
        move 2,storage_state           ; restore complete prearmed run length
        movem 2,storage_count
        move 1,dtc_request_unit
        lsh 1,3
        skipn dtc_request_write
        jrst dtc_search_pi_read
        hrroi 2,0777772
        movem 2,storage_state
        iori 1,0220705
        ior 1,dtc_request_reverse
        cono 0210,0(1)
        cono 0200,003443
        jrst pdp10_pi_dispatch_done
dtc_search_pi_read:
        setom storage_state
        iori 1,0220305
        ior 1,dtc_request_reverse
        cono 0200,004043
        cono 0210,0(1)
        jrst pdp10_pi_dispatch_done

dtc_search_fail:
        aos mfsdev_storage_errors     ; DTC0 search failure
        setzm dtc_motion(1)
        ; tape_ioerr performs the authoritative DTC/DCT owner reset.
        jrst tape_ioerr

/**
 * @brief Execute one Type 516 magnetic-tape data or control operation.
 * @param AC1 Unit 0..7.
 * @param AC2 Data buffer for read/write operations.
 * @param AC3 Positive word count for read/write operations.
 * @param AC4 Operation selector: 0 read, negative write, positive control.
 * @return AC1 = 0/status for successful operations or negative kernel error.
 *
 * Control selector 1 is the compact synchronous status query used by LOGCTL;
 * other positive values are passed through as Type 516 control command bits.
 * Data transfers use the shared event/PI path; motion/control commands poll with
 * a long wall-clock timeout because rewind/spacing can traverse a whole reel.
 */
mtc_service:
        skipe storage_state
        jrst kret_busy
        caile 1,7
        jrst kret_arg
        jumpe 4,mtc_read_words
        jumpge 4,mtc_control
mtc_write_words:
        jumple 3,kret_arg
        pushj 017,tape_setup_write
        hrroi 3,0777773
        movei 4,051005
        movei 5,003403
        jrst mtc_rw_start
mtc_read_words:
        jumple 3,kret_arg
        pushj 017,tape_setup_read
        hrroi 3,0777776
        movei 4,052405
        movei 5,004003
        jrst mtc_rw_start
mtc_control:
        caie 4,1
        jrst mtc_control_command
        lsh 1,4
        cono 0220,0(1)
        cono 0224,2
        cono 0224,0
        coni 0224,1
        popj 017,
mtc_control_command:
        lsh 1,4
        ior 1,4
        cono 0224,0
        cono 0220,0(1)
        ; Control commands are polled because they do not use the data-channel
        ; PI path.  Use a long wall-clock timeout: spacing and rewind can span
        ; a full physical reel, so an instruction-count limit is incorrect.
        hrroi 2,MTC_CONTROL_TIMEOUT_NEG_RH
        movem 2,storage_count
mtc_control_wait:
        coni 0224,2
        trnn 2,0000001
        jrst mtc_control_wait_more
        trne 2,0020000
        jrst mtc_control_wait_more
        trnn 2,0400520
        jrst mtc_control_ok
        aos mfsdev_storage_errors+1   ; MTC0 control error
        jrst tape_ioerr
mtc_control_wait_more:
        skipe storage_count
        jrst mtc_control_wait
mtc_control_timeout:
        aos mfsdev_storage_errors+1   ; MTC0 control timeout
        jrst tape_ioerr
mtc_control_ok:
        setzm storage_count
        jrst kret_ok
mtc_rw_start:
        movem 3,storage_state
        lsh 1,4
        ior 1,4
        cono 0220,0(1)
        cono 0224,000005
        cono 0200,0(5)

tape_transfer_wait:
        movei 1,tape_transfer_event
        pushj 017,proc_wait_event
tape_wait:
        move 1,storage_state
        jumpl 1,tape_wait
        caie 1,7
        jrst tape_wait_done
tape_ioerr:
        ; Error exits must release the shared Type-136 channel as completely
        ; as the PI completion path.  Otherwise a failed polled/control path
        ; can leave DCT or a tape controller selected for the next owner.
        cono 0224,0
        cono 0210,0
        cono 0200,0
        pushj 017,dtc_forget_motion
        setzm storage_state
        jrst    kret_neg5

/**
 * @brief Invalidate every DECtape motion estimate after a global controller stop.
 * @return Normal AC17 return; AC1 clobbered by BLT setup.
 */
dtc_forget_motion:
        setzm dtc_motion
        move 1,[dtc_motion,,dtc_motion+1]
        blt 1,dtc_motion+7
        popj 017,
tape_wait_done:
        aos @tape_account_table-1(1)    ; completed READ/WRITE request
        setzm storage_state
        jrst kret_ok

/**
 * Completion accounting indexed by positive completed storage_state.
 * Slots 3/4 correspond to DSK states and are unreachable from this MRES; they
 * keep the state number usable directly as a compact table index.
 */
tape_account_table:
        .word mfsdev_io_in+014
        .word mfsdev_io_in+015
        .word mfsdev_io_in+016
        .word mfsdev_io_out+016
        .word mfsdev_io_out+015
        .word mfsdev_io_out+014

        .bss
/** DECtape unit owning the active request. */
dtc_request_unit: .block 1
/** Nonzero for DECtape write, zero for read. */
dtc_request_write: .block 1
/** Zero forward; 0010000 when the active DECtape request runs in reverse. */
dtc_request_reverse: .block 1
/** Requested physical DECtape block for the PI3 RBN search owner. */
dtc_target_block: .block 1
/** Bound each RBN search to seven direction changes/restarts. */
dtc_search_retries: .block 1
/** Last Type-136 RBN DATAI, retained while PI3 updates the motion hint. */
dtc_last_mark: .block 1
/** Shared DTC/MTC process event for one Type 136 tape data transfer. */
tape_transfer_event: .block 1
/** Signed per-unit last-block+direction motion estimate for eight DTC units. */
dtc_motion: .block 010
