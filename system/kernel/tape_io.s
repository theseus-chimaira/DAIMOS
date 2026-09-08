; tape_io.s -- resident PDP-6 DECtape and magnetic-tape Type-136 driver.
;
; DTC and MTC share this MRES because both are tape transports using the same
; slow physical motion and substantial common DCT setup/status machinery.  The
; disk controller is entirely separate in dsk_io.s.

        .text
        .globl devicefs_io_in
        .globl devicefs_io_out
        .globl devicefs_storage_errors
        .globl devicefs_mtc_words_read
        .globl devicefs_mtc_words_written
        .globl tape_pi_handler
        .globl tape_dct_handler
        .globl dtc_read_block
        .globl dtc_write_block
        .globl mtc_service
        .globl pdp10_pi_dispatch_done
        .globl storage_state
        .globl storage_iowd
        .globl storage_count
        .globl pdp10_ret_ok
        .globl pdp10_ret_arg
        .globl pdp10_ret_busy

; PI5 status leaf selected by the fixed Type-136 owner router.
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
        jrst tape_pi_cleanup

tape_pi_error:
        aos devicefs_storage_errors+1   ; MTC0
        movei 1,7
        movem 1,storage_state
tape_pi_cleanup:
        cono 0224,0
        cono 0210,0
        cono 0200,0
        jrst pdp10_pi_dispatch_done

tape_pi_dtc_block_error:
        aos devicefs_storage_errors     ; DTC0
        movei 1,7
        movem 1,storage_state
        move 1,dtc_request_unit
        move 2,1
        lsh 2,3
        iori 2,0200000
        cono 0210,0(2)
        cono 0200,0
        setzm dtc_motion(1)
        jrst pdp10_pi_dispatch_done

; PI3 tape leaf.  Reverse DECtape transfers are serviced one word at a time;
; forward transfers reach this entry only when BLKI/BLKO falls through.
tape_dct_handler:
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

; Direct PI3 block setup shared inside the tape package.
tape_setup_read:
        move 4,tape_dct_blki
        jrst tape_setup_common
tape_setup_write:
        move 4,tape_dct_blko
tape_setup_common:
        movem 3,storage_count
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

; AC1 unit, AC2 physical block 0..01101, AC3 destination of 128 words.
dtc_read_block:
        setz 4,
        jrst dtc_block_start
; AC1 unit, AC2 physical block, AC3 source of 128 words.
dtc_write_block:
        movei 4,1
dtc_block_start:
        skipe storage_state
        jrst pdp10_ret_busy
        caile 1,7
        jrst pdp10_ret_arg
        hlrz 6,2
        addi 6,0200
        hrrzs 2
        caile 2,01101
        jrst pdp10_ret_arg
        movem 4,dtc_request_write
        setzm dtc_request_reverse
        movei 7,7
        movem 6,storage_state
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
        move 5,1
        move 6,4
        aoj 6,
        skipe dtc_request_reverse
        movns 6
        movem 6,dtc_motion(5)
        move 6,4
        sub 6,2
        jumpe 6,dtc_search_found
        skipn dtc_request_reverse
        jrst dtc_search_forward
        jumpg 6,dtc_search_continue
        jrst dtc_search_turn
dtc_search_forward:
        jumpl 6,dtc_search_continue
dtc_search_turn:
        movei 6,0010000
        xorm 6,dtc_request_reverse
dtc_search_retry:
        sojle 7,dtc_search_fail
dtc_search_continue:
        pushj 017,dtc_search_command
        jrst dtc_search_wait
dtc_search_command:
        move 4,1
        lsh 4,3
        iori 4,0220200
        ior 4,dtc_request_reverse
        cono 0210,0(4)
        popj 017,

dtc_search_found:
        skipn dtc_request_reverse
        jrst dtc_search_found_forward
        move 4,storage_state
        caie 4,0200
        jrst dtc_search_turn
dtc_search_found_forward:
        cono 0200,0
        move 2,3
        move 3,storage_state
        skipn dtc_request_write
        jrst dtc_block_setup_read
        pushj 017,tape_setup_write
        jrst dtc_block_setup_direction
dtc_block_setup_read:
        pushj 017,tape_setup_read
dtc_block_setup_direction:
        skipn dtc_request_reverse
        jrst dtc_block_setup_state
        move 4,2
        addi 4,0200
        hrrm 4,storage_iowd
        move 4,000047
        movem 4,000046
dtc_block_setup_state:
        movem 1,dtc_request_unit
        lsh 1,3
        skipn dtc_request_write
        jrst dtc_block_start_read
        hrroi 4,0777772
        movem 4,storage_state
        iori 1,0220705
        ior 1,dtc_request_reverse
        cono 0210,0(1)
        cono 0200,003443
        jrst tape_wait
dtc_block_start_read:
        setom storage_state
        iori 1,0220305
        ior 1,dtc_request_reverse
        cono 0200,004043
        cono 0210,0(1)
        jrst tape_wait

dtc_search_fail:
        aos devicefs_storage_errors     ; DTC0 search failure
        setzm dtc_motion(1)
        lsh 1,3
        iori 1,0200000
        cono 0210,0(1)
        cono 0200,0
        jrst tape_ioerr

; Compact Type-516 service.  AC4 selects READ/WRITE/control operation.
mtc_service:
        skipe storage_state
        jrst pdp10_ret_busy
        caile 1,7
        jrst pdp10_ret_arg
        jumpe 4,mtc_read_words
        jumpge 4,mtc_control
mtc_write_words:
        jumple 3,pdp10_ret_arg
        pushj 017,tape_setup_write
        hrroi 3,0777773
        movei 4,051005
        movei 5,003403
        jrst mtc_rw_start
mtc_read_words:
        jumple 3,pdp10_ret_arg
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
mtc_control_wait:
        coni 0224,2
        trnn 2,0000001
        jrst mtc_control_wait
        trne 2,0020000
        jrst mtc_control_wait
        trnn 2,0400520
        jrst pdp10_ret_ok
        aos devicefs_storage_errors+1   ; MTC0 control error
        jrst tape_ioerr
mtc_rw_start:
        movem 3,storage_state
        lsh 1,4
        ior 1,4
        cono 0220,0(1)
        cono 0224,000005
        cono 0200,0(5)

tape_wait:
        move 1,storage_state
        jumpl 1,tape_wait
        caie 1,7
        jrst tape_wait_done
tape_ioerr:
        setzm storage_state
        hrroi 1,0777773
        popj 017,
tape_wait_done:
        aos @tape_account_table-1(1)    ; completed READ/WRITE request
        caie 1,2                        ; MTC read
        cain 1,5                        ; MTC write
        jrst tape_account_mtc_words
        jrst tape_account_done

tape_account_mtc_words:
        move 2,storage_count
        caie 1,2
        jrst tape_account_mtc_write
        addm 2,devicefs_mtc_words_read
        jrst tape_account_done

tape_account_mtc_write:
        addm 2,devicefs_mtc_words_written

tape_account_done:
        setzm storage_state
        jrst pdp10_ret_ok

tape_account_table:
        .word devicefs_io_in+014
        .word devicefs_io_in+015
        .word devicefs_io_in+016
        .word devicefs_io_out+016
        .word devicefs_io_out+015
        .word devicefs_io_out+014

        .bss
dtc_request_unit: .block 1
dtc_request_write: .block 1
dtc_request_reverse: .block 1
dtc_motion: .block 010

; Device-local accounting state; absent devices consume no fixed KCORE.
        .bss
