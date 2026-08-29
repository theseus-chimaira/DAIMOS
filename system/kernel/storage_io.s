; storage_io.s -- shared compact PDP-6 DCT storage transfer engine.
;
; DTC, MTC, and DSK270 all feed data through the Type 136 data control.
; They share one resident BLKI/BLKO pointer.  Type-136 data uses direct PI3
; block I/O; controller completion/error status remains on PI5.
; storage_state encodes ownership as -1 DTC block read, -2 MTC read,
; -3 DSK read, -4 DSK write, -5 MTC write, -6 DTC block write.  Positive
; values are completion/error states.
;
; mtc_read_words:
;   AC1 = unit, AC2 = destination, AC3 = maximum word count.
; dsk_read_sector:
;   AC1 = raw DSK270 hardware address, AC2 = 128-word destination.

        .text
        .globl devicefs_v1_io_in
        .globl devicefs_v1_io_out
        .globl storage_pi_handler
        .globl storage_dct_handler
        .globl dtc_read_block
        .globl dtc_write_block
        .globl mtc_read_words
        .globl mtc_write_words
        .globl dsk_read_sector
        .globl dsk_write_sector
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_ok_v34
        .globl pdp10_ret_arg_v34
        .globl pdp10_ret_busy_v34

storage_pi_handler:
        ; Controller status only.  Type-136 word transfers run directly from
        ; the PI3 vector and reach storage_dct_handler only at block end.
        skipl storage_state
        jrst pdp10_pi_handler_return
storage_pi_active:
        move 2,storage_state
        aoje 2,storage_pi_dtc_status
        aoje 2,storage_pi_mtc_status
        aoje 2,storage_pi_dsk_status
        aoje 2,storage_pi_dsk_status
        aoje 2,storage_pi_mtc_write_status
        jrst storage_pi_dtc_write_status

storage_dct_handler:
        ; Forward transfers use the low-core BLKI/BLKO fast path and enter
        ; here only at completion.  Reverse transfers must decrement memory
        ; addresses, so route their per-word requests through the normal PI3
        ; saved-AC handler.  Tape motion dominates this small CPU overhead.
        skipn dtc_request_reverse
        jrst storage_dct_select
        skipg storage_count
        jrst storage_dct_select
        jrst storage_dct_reverse_word
storage_dct_select:
        jrst storage_dct_count_done

storage_dct_reverse_word:
        skipn dtc_request_write
        jrst storage_dct_reverse_read
        move 2,dtc_request_buffer
        move 1,(2)
        datao 0200,1
        jrst storage_dct_reverse_advance
storage_dct_reverse_read:
        datai 0200,1
        move 2,dtc_request_buffer
        movem 1,(2)
storage_dct_reverse_advance:
        sosle storage_count
        jrst storage_dct_reverse_more
        jrst storage_dct_count_done
storage_dct_reverse_more:
        sos dtc_request_buffer
        jrst pdp10_pi_handler_return

storage_dct_count_done:
        move 2,storage_state
        aoje 2,storage_pi_done_keep_dtc
        aoje 2,storage_pi_mtc_read_full
        aoje 2,storage_pi_dsk_read_done
        aoje 2,storage_pi_dsk_write_full
        aoje 2,storage_pi_mtc_write_arm
        jrst storage_dct_dtc_write_arm

storage_dct_dtc_write_arm:
        ; DTC write (-6): after the final BLKO, service two more DCT requests
        ; without allowing the dummy word to cross into the following block.
        movei 1,storage_dct_dtc_write_ack1
        hrrm 1,storage_dct_select
        jrst storage_dct_arm_handler

storage_pi_mtc_write_arm:
        movei 1,storage_dct_mtc_write_drain
        hrrm 1,storage_dct_select
        jrst storage_dct_arm_handler

storage_pi_dsk_write_full:
        ; BLKO falls through before its final DCT word has drained to DSK.
        ; Two following DCT requests distinguish pipeline release from actual
        ; controller consumption of that final word.
        movei 1,storage_dct_dsk_write_ack1
        hrrm 1,storage_dct_select

storage_dct_arm_handler:
        ; Replace PI3 vector word 046 with its final-word JSR.  The next DCT
        ; request then enters storage_dct_handler without transferring data.
        move 1,000047
        movem 1,000046
        jrst pdp10_pi_handler_return

storage_pi_dsk_read_done:
        cono 0270,030105
        jrst storage_pi_done
storage_pi_done_keep_dtc:
        ; Block-addressed DTC I/O leaves the selected unit coasting in MOVE
        ; mode.  This avoids a stop/start cycle between adjacent filesystem
        ; blocks.  dtc_motion keeps the last observed block and direction; a
        ; later search corrects a stale estimate after idle time.
        move 1,dtc_active_unit
        jumpe 1,storage_pi_done
        soj 1,
        move 2,dtc_motion(1)
        lsh 1,3
        iori 1,0220000
        jumpge 2,storage_pi_keep_forward
        iori 1,0010000
storage_pi_keep_forward:
        cono 0210,0(1)
        cono 0200,0
        setzm dtc_active_unit
        movns storage_state
        jrst pdp10_pi_handler_return

storage_pi_done:
        ; Preserve the completed owner without adding a word: negative owner
        ; becomes its positive completion code.
        movns storage_state
        jrst storage_pi_stop

storage_pi_mtc_read_full:
        coni 0224,1
        trne 1,0400520
        jrst storage_pi_error
        trnn 1,0000004
        jrst storage_pi_error
        coni 0200,1
        trne 1,002000
        jrst storage_pi_error
        ; EOR can arrive before the transport is actually idle.  ICE is
        ; enabled, so the later TAPE FREE interrupt completes the request.
        jrst pdp10_pi_handler_return

storage_pi_dtc_write_status:
        seto 2,
storage_pi_dtc_status:
        ; AC2 is zero for read, -1 for write after the owner dispatch chain.
        coni 0214,1
        trne 1,0000034
        jrst storage_pi_error
        jumpe 2,pdp10_pi_handler_return
        trnn 1,0000001
        jrst pdp10_pi_handler_return
        jrst storage_pi_done

storage_pi_mtc_write_status:
        seto 2,
storage_pi_mtc_status:
        ; AC2 is zero for read.  The write dispatch enters through the tiny
        ; flag stub above, then shares the common Type-516 error/EOR tests.
        coni 0224,1
        trne 1,0400520
        jrst storage_pi_error
        trnn 1,0000004
        jrst pdp10_pi_handler_return
        jumpn 2,storage_pi_mtc_idle_check
        ; Read EOR can precede delivery of the final DCT word.
        coni 0200,1
        trne 1,002000
        jrst pdp10_pi_handler_return
        coni 0224,1
storage_pi_mtc_idle_check:
        trnn 1,0000001
        jrst pdp10_pi_handler_return
        jrst storage_pi_done

storage_pi_dsk_status:
        conso 0270,001777
        jrst pdp10_pi_handler_return
storage_pi_error:
        movei 1,4
        movem 1,storage_state
        skipe dtc_active_unit
        jrst storage_pi_dtc_block_error
storage_pi_stop:
        cono 0224,0
        cono 0210,0
        cono 0200,0
        jrst pdp10_pi_handler_return
storage_pi_dtc_block_error:
        ; Stop only the failing DECtape unit; other units may be coasting.
        move 1,dtc_active_unit
        soj 1,
        move 2,1
        lsh 2,3
        iori 2,0200000
        cono 0210,0(2)
        cono 0200,0
        setzm dtc_motion(1)
        setzm dtc_active_unit
        jrst pdp10_pi_handler_return
storage_ok:
        setzm storage_state
        jrst pdp10_ret_ok_v34

; Set direct PI3 block transfer and build its combined -count,,buffer-1 word.
; BLKI/BLKO updates both halves itself and dismisses PI for every non-final
; word, so normal transfer traffic executes no resident dispatcher code.
storage_setup_read:
        move 4,storage_dct_blki
        jrst storage_setup_common
storage_setup_write:
        move 4,storage_dct_blko
storage_setup_common:
        movem 3,storage_count
        movem 4,000046
        movei 4,storage_dct_count_done
        hrrm 4,storage_dct_select
        subi 2,1
        movn 4,3
        hrl 2,4
        movem 2,storage_iowd
        popj 017,

storage_dct_blki:
        blki 0200,storage_iowd
storage_dct_blko:
        blko 0200,storage_iowd


; AC1 unit, AC2 physical block 0..01101, AC3 destination of 128 words.
; The block service searches in whichever direction is shortest from the last
; observed position.  A reverse search transfers the data in reverse as well,
; avoiding the extra turnaround used by a forward-only block interface.
dtc_read_block:
        setz 4,
        jrst dtc_block_start

; AC1 unit, AC2 physical block 0..01101, AC3 source of 128 words.
dtc_write_block:
        movei 4,1
dtc_block_start:
        skipe storage_state
        jrst pdp10_ret_busy_v34
        caile 1,7
        jrst pdp10_ret_arg_v34
        jumpl 2,pdp10_ret_arg_v34
        caile 2,01101
        jrst pdp10_ret_arg_v34
        movem 1,dtc_request_unit
        movem 2,dtc_request_block
        movem 3,dtc_request_buffer
        movem 4,dtc_request_write
        setzm dtc_request_reverse
        movei 5,7
        movem 5,dtc_search_tries
        movei 5,7
        movem 5,storage_state

        ; A signed motion word stores +block+1 while moving forward and
        ; -block-1 while moving backward.  Zero means no reliable estimate.
        move 5,dtc_motion(1)
        jumpe 5,dtc_search_begin
        move 6,5
        movms 6
        soj 6,
        sub 6,dtc_request_block
        jumpg 6,dtc_search_choose_reverse
        ; For an exact cached-position request, keep the current direction
        ; until SEARCH reports its first block number.  Reversing before that
        ; report can miss the target while the controller is still activating.
        jrst dtc_search_begin
dtc_search_choose_reverse:
        movei 6,0010000
        movem 6,dtc_request_reverse

dtc_search_begin:
        ; Search block numbers synchronously through DCT0 without consuming a
        ; PI slot.  Other interrupts remain enabled while physical tape moves.
        cono 0200,004040
        pushj 017,dtc_search_command

dtc_search_wait:
        coni 0214,4
        trne 4,0000034
        jrst dtc_search_retry
        trne 4,0000002
        jrst dtc_search_turn
        conso 0200,0001000
        jrst dtc_search_wait
        datai 0200,4
        andi 4,001777

        move 5,dtc_request_unit
        move 6,4
        aoj 6,
        skipn dtc_request_reverse
        jrst dtc_search_store_motion
        movns 6
dtc_search_store_motion:
        movem 6,dtc_motion(5)

        move 6,4
        sub 6,dtc_request_block
        jumpe 6,dtc_search_found
        skipn dtc_request_reverse
        jrst dtc_search_forward
        jumpg 6,dtc_search_continue
        jrst dtc_search_turn
dtc_search_forward:
        jumpl 6,dtc_search_continue
        jrst dtc_search_turn

; Type-551 SEARCH completes after delivering one block number.  Continue the
; search by immediately reissuing the same command while the transport keeps
; moving; this costs no stop/start or direction change.
dtc_search_continue:
        pushj 017,dtc_search_command
        jrst dtc_search_wait

dtc_search_turn:
        sosg dtc_search_tries
        jrst dtc_search_fail
        movei 6,0010000
        xorm 6,dtc_request_reverse
        pushj 017,dtc_search_command
        jrst dtc_search_wait

dtc_search_retry:
        sosg dtc_search_tries
        jrst dtc_search_fail
        pushj 017,dtc_search_command
        jrst dtc_search_wait

dtc_search_command:
        move 4,dtc_request_unit
        lsh 4,3
        iori 4,0220200
        ior 4,dtc_request_reverse
        cono 0210,0(4)
        popj 017,

dtc_search_found:
        cono 0200,0
        move 2,dtc_request_buffer
        movei 3,0200
        skipn dtc_request_write
        jrst dtc_block_setup_read
        pushj 017,storage_setup_write
        jrst dtc_block_setup_direction
dtc_block_setup_read:
        pushj 017,storage_setup_read
dtc_block_setup_direction:
        skipn dtc_request_reverse
        jrst dtc_block_setup_state
        ; Start at caller word 0177.  The ordinary PI3 JSR in low core saves
        ; AC1..AC3 for the tiny reverse per-word handler above.
        move 4,dtc_request_buffer
        addi 4,0177
        movem 4,dtc_request_buffer
        move 4,000047
        movem 4,000046

dtc_block_setup_state:
        move 1,dtc_request_unit
        move 4,1
        aoj 4,
        movem 4,dtc_active_unit
        lsh 1,3
        skipn dtc_request_write
        jrst dtc_block_start_read
        hrroi 4,0777772
        movem 4,storage_state
        iori 1,0220705
        ior 1,dtc_request_reverse
        cono 0210,0(1)
        cono 0200,003443
        jrst storage_wait
dtc_block_start_read:
        setom storage_state
        iori 1,0220305
        ior 1,dtc_request_reverse
        cono 0200,004043
        cono 0210,0(1)
        jrst storage_wait

dtc_search_fail:
        ; Stop only this unit, invalidate its position estimate, and report EIO.
        move 1,dtc_request_unit
        setzm dtc_motion(1)
        lsh 1,3
        iori 1,0200000
        cono 0210,0(1)
        cono 0200,0
        setzm storage_state
        hrroi 1,0777773
        popj 017,

; AC1 unit, AC2 destination, AC3 maximum words in one tape record.
mtc_read_words:
        skipe storage_state
        jrst pdp10_ret_busy_v34
        caile 1,7
        jrst pdp10_ret_arg_v34
        jumple 3,pdp10_ret_arg_v34
        pushj 017,storage_setup_read
        hrroi 3,0777776
        movei 4,052405
        movei 5,004003
        jrst mtc_rw_start

; AC1 unit, AC2 source, AC3 exact word count for one magnetic-tape record.
; Direction and count-exhaustion behavior are patched once at start, leaving
; the per-word PI path identical to the disk write path.
mtc_write_words:
        skipe storage_state
        jrst pdp10_ret_busy_v34
        caile 1,7
        jrst pdp10_ret_arg_v34
        jumple 3,pdp10_ret_arg_v34
        pushj 017,storage_setup_write
        hrroi 3,0777773
        movei 4,051005
        movei 5,003403

mtc_rw_start:
        movem 3,storage_state
        lsh 1,4
        ior 1,4
        cono 0220,0(1)
        ; Starting the command clears stale EOR/status from the previous
        ; record.  Enable status PI only afterwards, and arm DCT last; output
        ; DCT asserts its first data request immediately.
        cono 0224,000005
        cono 0200,0(5)
        jrst storage_wait

; AC1 raw hardware address, AC2 destination/source of one 128-word sector.
; Read/write share address setup, DFR wait, and controller start; the two tiny
; entry stubs provide only direction-specific DCT/DSK command words.
dsk_read_sector:
        skipe storage_state
        jrst pdp10_ret_busy_v34
        movei 3,0200
        pushj 017,storage_setup_read
        hrroi 3,0777775
        movei 4,004003
        movei 5,001105
        jrst dsk_rw_start

dsk_write_sector:
        skipe storage_state
        jrst pdp10_ret_busy_v34
        movei 3,0200
        pushj 017,storage_setup_write
        hrroi 3,0777774
        movei 4,003403
        movei 5,002105

dsk_rw_start:
        movem 3,storage_state
        datao 0270,1
dsk_rw_wait_dfr:
        coni 0270,3
        trne 3,001777
        jrst storage_ioerr
        trnn 3,040000
        jrst dsk_rw_wait_dfr
        cono 0200,0(4)
        cono 0270,0(5)
storage_wait:
        move 1,storage_state
        jumpl 1,storage_wait
        caie 1,4
        jrst storage_wait_done
storage_ioerr:
        setzm storage_state
        hrroi 1,0777773
        popj 017,
storage_wait_done:
        ; The IOWD left half retains the untransferred count for MTC/DSK.
        ; DTC block I/O is always exactly 128 words; reverse transfers do not
        ; advance the IOWD, so account its fixed block size directly.
        move 2,storage_iowd
        hlrz 2,2
        add 2,storage_count
        andi 2,0777777
        caie 1,1
        jrst storage_account_mtc_read
        movei 2,0200
        addm 2,devicefs_v1_io_in+7
        jrst storage_account_done
storage_account_mtc_read:
        caie 1,2
        jrst storage_account_dsk_read
        addm 2,devicefs_v1_io_in+010
        jrst storage_account_done
storage_account_dsk_read:
        caie 1,3
        jrst storage_account_dsk_write
        addm 2,devicefs_v1_io_in+011
        jrst storage_account_done
storage_account_dsk_write:
        caie 1,4
        jrst storage_account_mtc_write
        addm 2,devicefs_v1_io_out+011
        jrst storage_account_done
storage_account_mtc_write:
        caie 1,5
        jrst storage_account_dtc_write
        addm 2,devicefs_v1_io_out+010
        jrst storage_account_done
storage_account_dtc_write:
        movei 2,0200
        addm 2,devicefs_v1_io_out+7
storage_account_done:
        ; Completion code 3 is DSK; tape operations can return immediately.
        caie 1,3
        jrst storage_ok
        coni 0270,2
dsk_wait_ids:
        trne 2,001777
        jrst storage_ioerr
        trne 2,0400000
        jrst storage_ok
        coni 0270,2
        jrst dsk_wait_ids

; DCT drain/ack handlers run only after a final BLKO has fallen through
; the PI3 vector.  storage_state keeps the controller owner unchanged, so
; asynchronous PI5 status continues to dispatch to the correct controller.
storage_dct_dtc_write_ack1:
        setz 1,
        datao 0200,1
        movei 1,storage_dct_dtc_write_ack2
        hrrm 1,storage_dct_select
        jrst pdp10_pi_handler_return
storage_dct_dtc_write_ack2:
        move 1,storage_state
        addi 1,6
        jumpe 1,storage_pi_done_keep_dtc
        cono 0200,0
        cono 0210,0
        jrst storage_pi_done

storage_dct_mtc_write_drain:
        coni 0200,1
        andi 1,0777770
        cono 0200,0(1)
        jrst pdp10_pi_handler_return

storage_dct_dsk_write_ack1:
        movei 1,storage_dct_dsk_write_ack2
        hrrm 1,storage_dct_select
        jrst pdp10_pi_handler_return
storage_dct_dsk_write_ack2:
        aos storage_state
        cono 0270,030105
        jrst storage_pi_done

        .bss
storage_state: .block 1
storage_iowd:  .block 1
storage_count: .block 1
dtc_active_unit: .block 1
dtc_request_unit: .block 1
dtc_request_block: .block 1
dtc_request_buffer: .block 1
dtc_request_write: .block 1
dtc_request_reverse: .block 1
dtc_search_tries: .block 1
dtc_motion: .block 010
