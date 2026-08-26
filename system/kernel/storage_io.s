; storage_io.s -- shared compact PDP-6 DCT storage transfer engine.
;
; DTC, MTC, and DSK270 all feed data through the Type 136 data control.
; They share one resident BLKI/BLKO pointer.  Type-136 data uses direct PI3
; block I/O; controller completion/error status remains on PI5.
; storage_state encodes ownership as -1 DTC, -2 MTC read, -3 DSK read,
; -4 DSK write, -5 MTC write, -6 DTC write; no owner word is required.  Positive values
; are completion/error states.
;
; dtc_read_words / mtc_read_words:
;   AC1 = unit, AC2 = destination, AC3 = exact/max word count.
; dsk_read_sector:
;   AC1 = raw DSK270 hardware address, AC2 = 128-word destination.

        .text
        .globl devicefs_v1_io_in
        .globl devicefs_v1_io_out
        .globl storage_pi_handler
        .globl storage_dct_handler
        .globl dtc_read_words
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
        jrst storage_pi_dtc_status

storage_dct_handler:
storage_dct_select:
        jrst storage_dct_count_done
storage_dct_count_done:
        move 2,storage_state
        aoje 2,storage_pi_done
        aoje 2,storage_pi_mtc_read_full
        aoje 2,storage_pi_dsk_read_done
        aoje 2,storage_pi_dsk_write_full
        aoje 2,storage_pi_mtc_write_arm

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
storage_pi_stop:
        cono 0224,0
        cono 0210,0
        cono 0200,0
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

; AC1 unit, AC2 destination, AC3 exact word count.
dtc_read_words:
        skipe storage_state
        jrst pdp10_ret_busy_v34
        caile 1,7
        jrst pdp10_ret_arg_v34
        jumple 3,pdp10_ret_arg_v34
        pushj 017,storage_setup_read
        setom storage_state
        lsh 1,3
        iori 1,0220305
        cono 0200,004043
        cono 0210,0(1)
        jrst storage_wait

; AC1 unit, AC2 source of exactly one 128-word DECtape block.
; Type 551 block writes are deliberately fixed-size: the hardware block
; cycle, not a software short count, defines transfer completion.
dtc_write_block:
        skipe storage_state
        jrst pdp10_ret_busy_v34
        caile 1,7
        jrst pdp10_ret_arg_v34
        movei 3,0200
        pushj 017,storage_setup_write
        hrroi 3,0777772
        movem 3,storage_state
        lsh 1,3
        iori 1,0220705
        cono 0210,0(1)
        cono 0200,003443
        jrst storage_wait

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
        ; The IOWD left half retains the untransferred count.  Combine it
        ; with the requested count to account actual words, including short
        ; tape records, without adding a per-word PI instruction.
        move 2,storage_iowd
        hlrz 2,2
        add 2,storage_count
        andi 2,0777777
        caie 1,1
        jrst storage_account_mtc_read
        addm 2,devicefs_v1_io_in+12
        jrst storage_account_done
storage_account_mtc_read:
        caie 1,2
        jrst storage_account_dsk_read
        addm 2,devicefs_v1_io_in+13
        jrst storage_account_done
storage_account_dsk_read:
        caie 1,3
        jrst storage_account_dsk_write
        addm 2,devicefs_v1_io_in+14
        jrst storage_account_done
storage_account_dsk_write:
        caie 1,4
        jrst storage_account_mtc_write
        addm 2,devicefs_v1_io_out+14
        jrst storage_account_done
storage_account_mtc_write:
        caie 1,5
        jrst storage_account_dtc_write
        addm 2,devicefs_v1_io_out+13
        jrst storage_account_done
storage_account_dtc_write:
        addm 2,devicefs_v1_io_out+12
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
