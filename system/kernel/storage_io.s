; storage_io.s -- shared compact PDP-6 DCT storage transfer engine.
;
; DTC, MTC, and DSK270 all feed data through the Type 136 data control.
; They therefore share one resident pointer, count, and PI5 service path.
; storage_state encodes ownership as -1 DTC, -2 MTC read, -3 DSK read,
; -4 DSK write, -5 MTC write, -6 DTC write; no owner word is required.  Positive values
; are completion/error states.
;
; dtc_read_words / mtc_read_words:
;   AC1 = unit, AC2 = destination, AC3 = exact/max word count.
; dsk_read_sector:
;   AC1 = raw DSK270 hardware address, AC2 = 128-word destination.

        .text
        .globl storage_pi_handler
        .globl dtc_read_words
        .globl dtc_write_block
        .globl mtc_read_words
        .globl mtc_write_words
        .globl dsk_read_sector
        .globl dsk_write_sector
        .globl pdp10_pi_handler_return

storage_pi_handler:
        coni 0200,1
        trne 1,001000
        jrst storage_pi_word
        jrst storage_pi_status

storage_pi_word:
        skipge storage_state
storage_pi_select:
        jrst storage_pi_read
        jrst pdp10_pi_handler_return
storage_pi_read:
        datai 0200,1
        movem 1,@storage_ptr
storage_pi_advance:
        aos storage_ptr
        sosle storage_count
        jrst pdp10_pi_handler_return

        ; Count exhausted.  DTC and DSK have fixed transfer sizes; MTC uses
        ; EOR as the real terminator and treats a full caller buffer as an
        ; error unless EOR is already visible.
        move 2,storage_state
        aoje 2,storage_pi_done
        aoje 2,storage_pi_mtc_read_full
        aoje 2,storage_pi_dsk_read_done
        aoje 2,storage_pi_dsk_write_full

        ; MTC write (-5): install its one-request drain.  DTC write (-6)
        ; is continuous across block boundaries and needs a two-request drain
        ; so the final real word is consumed without writing into the next
        ; block.
        aoje 2,storage_pi_mtc_write_arm
        movei 1,storage_pi_dtc_write_ack1
        hrrm 1,storage_pi_select
        jrst pdp10_pi_handler_return
storage_pi_mtc_write_arm:
        movei 1,storage_pi_mtc_write_drain
        hrrm 1,storage_pi_select
        jrst pdp10_pi_handler_return

storage_pi_dsk_write_full:
        ; DSK write (-4): the next request acknowledges the final DATAO.
        movei 1,storage_pi_write_finish
        hrrm 1,storage_pi_select
        jrst pdp10_pi_handler_return

storage_pi_dsk_read_done:
        movei 1,030105
        cono 0270,0(1)
        jrst storage_pi_done

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

storage_pi_status:
        skipge storage_state
        jrst storage_pi_active
        jrst pdp10_pi_handler_return
storage_pi_active:
        move 2,storage_state
        aoje 2,storage_pi_dtc_status
        aoje 2,storage_pi_mtc_status
        aoje 2,storage_pi_dsk_status
        aoje 2,storage_pi_dsk_status
        aoje 2,storage_pi_mtc_write_status
        jrst storage_pi_dtc_write_status

storage_pi_dtc_status:
        coni 0214,1
        trne 1,0000034
        jrst storage_pi_error
        jrst pdp10_pi_handler_return

storage_pi_mtc_status:
        coni 0224,1
        trne 1,0400520
        jrst storage_pi_error
        trnn 1,0000004
        jrst pdp10_pi_handler_return
        ; EOR can precede delivery of the final DCT word.
        coni 0200,1
        trne 1,002000
        jrst pdp10_pi_handler_return
        coni 0224,1
        trnn 1,0000001
        jrst pdp10_pi_handler_return
        jrst storage_pi_done

storage_pi_mtc_write_status:
        coni 0224,1
        trne 1,0400520
        jrst storage_pi_error
        trnn 1,0000004
        jrst pdp10_pi_handler_return
        trnn 1,0000001
        jrst pdp10_pi_handler_return
        jrst storage_pi_done

storage_pi_dtc_write_status:
        coni 0214,1
        trne 1,0000034
        jrst storage_pi_error
        trnn 1,0000001
        jrst pdp10_pi_handler_return
        jrst storage_pi_done

storage_pi_dsk_status:
        coni 0270,1
        trne 1,001777
        jrst storage_pi_error
        jrst pdp10_pi_handler_return

storage_pi_done:
        ; Preserve the completed owner without adding a word: -1/-2/-3
        ; becomes +1/+2/+3.  The waiter can then special-case DSK IDS.
        move 1,storage_state
        movn 1,1
        movem 1,storage_state
        cono 0224,0
        cono 0210,0
        cono 0200,0
        jrst pdp10_pi_handler_return

storage_pi_error:
        movei 1,4
        movem 1,storage_state
        cono 0224,0
        cono 0210,0
        cono 0200,0
        jrst pdp10_pi_handler_return

storage_busy:
        hrroi 1,0777775
        popj 017,
storage_arg:
        seto 1,
        popj 017,
storage_ioerr:
        setzm storage_state
        hrroi 1,0777773
        popj 017,
storage_ok:
        setzm storage_state
        movei 1,0
        popj 017,

; AC1 unit, AC2 destination, AC3 exact word count.
dtc_read_words:
        skipn storage_state
        jrst dtc_read_idle
        jrst storage_busy
dtc_read_idle:
        caile 1,7
        jrst storage_arg
        jumpg 3,dtc_read_start
        jrst storage_arg
dtc_read_start:
        movem 2,storage_ptr
        movem 3,storage_count
        setom storage_state
        lsh 1,3
        iori 1,0220305
        movei 2,004045
        cono 0200,0(2)
        cono 0210,0(1)
        jrst storage_wait

; AC1 unit, AC2 source of exactly one 128-word DECtape block.
; Type 551 block writes are deliberately fixed-size: the hardware block
; cycle, not a software short count, defines transfer completion.
dtc_write_block:
        skipn storage_state
        jrst dtc_write_idle
        jrst storage_busy
dtc_write_idle:
        caile 1,7
        jrst storage_arg
        movei 4,storage_pi_write
        hrrm 4,storage_pi_select
        movem 2,storage_ptr
        movei 3,0200
        movem 3,storage_count
        hrroi 3,0777772
        movem 3,storage_state
        lsh 1,3
        iori 1,0220705
        cono 0210,0(1)
        movei 2,003445
        cono 0200,0(2)
        jrst storage_wait

; AC1 unit, AC2 destination, AC3 maximum words in one tape record.
mtc_read_words:
        skipn storage_state
        jrst mtc_read_idle
        jrst storage_busy
mtc_read_idle:
        caile 1,7
        jrst storage_arg
        jumpg 3,mtc_read_start
        jrst storage_arg
mtc_read_start:
        movei 4,storage_pi_read
        hrrm 4,storage_pi_select
        movem 2,storage_ptr
        movem 3,storage_count
        hrroi 3,0777776
        movem 3,storage_state
        lsh 1,4
        iori 1,052405
        cono 0220,0(1)
        ; Starting the new command clears stale EOR/status from the previous
        ; record.  Only then enable status PI and arm the input DCT.
        movei 2,5
        cono 0224,0(2)
        movei 2,004005
        cono 0200,0(2)
        jrst storage_wait

; AC1 unit, AC2 source, AC3 exact word count for one magnetic-tape record.
; Direction and count-exhaustion behavior are patched once at start, leaving
; the per-word PI path identical to the disk write path.
mtc_write_words:
        skipn storage_state
        jrst mtc_write_idle
        jrst storage_busy
mtc_write_idle:
        caile 1,7
        jrst storage_arg
        jumpg 3,mtc_write_start
        jrst storage_arg
mtc_write_start:
        movei 4,storage_pi_write
        hrrm 4,storage_pi_select
        movem 2,storage_ptr
        movem 3,storage_count
        hrroi 3,0777773
        movem 3,storage_state
        lsh 1,4
        iori 1,051005
        cono 0220,0(1)
        ; Start MTC before enabling status PI: MTS may still contain EOR from
        ; the preceding record, and enabling it first can complete this new
        ; request before the command clears those flags.  Arm output DCT last
        ; because it asserts its first data request immediately.
        movei 2,5
        cono 0224,0(2)
        movei 2,003405
        cono 0200,0(2)
        jrst storage_wait

; AC1 raw hardware address, AC2 destination of one 128-word sector.
dsk_read_sector:
        skipn storage_state
        jrst dsk_read_idle
        jrst storage_busy
dsk_read_idle:
        movei 3,storage_pi_read
        hrrm 3,storage_pi_select
        movem 2,storage_ptr
        movei 3,0200
        movem 3,storage_count
        hrroi 3,0777775
        movem 3,storage_state
        move 3,1
        datao 0270,3

dsk_wait_dfr:
        coni 0270,3
        trne 3,001777
        jrst storage_ioerr
        trnn 3,040000
        jrst dsk_wait_dfr
        movei 3,004005
        cono 0200,0(3)
        movei 3,001105
        cono 0270,0(3)
        jrst storage_wait

; AC1 raw hardware address, AC2 source of one 128-word sector.
; Direction is encoded in storage_pi_select, not another resident state word.
dsk_write_sector:
        skipn storage_state
        jrst dsk_write_idle
        jrst storage_busy
dsk_write_idle:
        movei 3,storage_pi_write
        hrrm 3,storage_pi_select
        movem 2,storage_ptr
        movei 3,0200
        movem 3,storage_count
        hrroi 3,0777774
        movem 3,storage_state
        move 3,1
        datao 0270,3

dsk_write_wait_dfr:
        coni 0270,3
        trne 3,001777
        jrst storage_ioerr
        trnn 3,040000
        jrst dsk_write_wait_dfr
        movei 3,003405
        cono 0200,0(3)
        movei 3,002105
        cono 0270,0(3)
        jrst storage_wait

storage_wait:
        move 1,storage_state
        jumpl 1,storage_wait
        caie 1,4
        jrst storage_wait_done
        jrst storage_ioerr
storage_wait_done:
        ; Completion code 3 is DSK; tape operations can return immediately.
        caie 1,3
        jrst storage_ok
        coni 0270,2
        jrst dsk_wait_ids
dsk_wait_ids:
        trne 2,001777
        jrst storage_ioerr
        trne 2,0400000
        jrst storage_ok
        coni 0270,2
        jrst dsk_wait_ids

; Write direction is selected once per operation by patching storage_pi_select.
storage_pi_write:
        move 1,@storage_ptr
        datao 0200,1
        jrst storage_pi_advance

; Arrive on the request following the 128th DATAO.  Normalize owner -4 to
; DSK -3 so the common completion code remains +3 and needs no new state.
; DTC write drains in two DCT requests.  The first request means the
; 128th real word has reached the DCT accumulator; queue a dummy only to let
; the controller consume that real word.  The second request proves it did.
; Disconnect and stop before the dummy can be consumed into the next block.
storage_pi_dtc_write_ack1:
        setz 1,
        datao 0200,1
        movei 1,storage_pi_dtc_write_ack2
        hrrm 1,storage_pi_select
        jrst pdp10_pi_handler_return
storage_pi_dtc_write_ack2:
        cono 0200,0
        cono 0210,0
        jrst storage_pi_done

; MTC only needs one drain request because EOR terminates its record.
storage_pi_mtc_write_drain:
        coni 0200,1
        andi 1,0777770
        cono 0200,0(1)
        movei 1,storage_pi_mtc_write_status
        hrrm 1,storage_pi_select
        jrst pdp10_pi_handler_return

storage_pi_write_finish:
        aos storage_state
        movei 1,030105
        cono 0270,0(1)
        jrst storage_pi_done

        .bss
storage_state: .block 1
storage_ptr:   .block 1
storage_count: .block 1
