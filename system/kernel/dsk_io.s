; dsk_io.s -- resident PDP-6 DSK270 driver, independent of tape support.
;
; Type-136 ownership/state lives in fixed KCORE storage_router.s.  This MRES
; contains only disk controller policy, queueing, watchdog, and transfer leaf
; handlers.  The shared router is the sole generic PI3/PI5 handler.

        .text
        .globl devicefs_io_in
        .globl devicefs_io_out
        .globl dsk_pi_handler
        .globl dsk_dct_handler
        .globl dsk_enqueue
        .globl dsk_queue
        .globl dsk_current_cyl
        .globl dsk_read_sector
        .globl dsk_write_sector
        .globl storage_pi_return
        .globl storage_state
        .globl storage_iowd
        .globl storage_count
        .globl pdp10_ret_ok
        .globl pdp10_ret_busy
        .globl proc_table
        .globl proc_wait_event
        .globl proc_wakeup_event

; PI5 DSK status leaf.  AC2 may be used because storage_pi_return bypasses the
; generic fanout cursor and restores interrupted ACs directly.
dsk_pi_handler:
        coni 0270,1
        trne 1,001777
        jrst dsk_pi_error
        trne 1,0400000
        jrst dsk_pi_idle
        trnn 1,040000
        jrst storage_pi_return
        move 2,storage_state
        addi 2,3
        jumpe 2,dsk_pi_start_read
        cono 0200,003403
        cono 0270,002105
        jrst storage_pi_return
dsk_pi_start_read:
        cono 0200,004003
        cono 0270,001105
        jrst storage_pi_return

dsk_pi_idle:
        cono 0270,0
        skipn 2,dsk_active_request
        jrst dsk_pi_boot_done
        aos 1,1(2)
        movei 1,devicefs_io_in+011
        tlne 2,1
        movei 1,devicefs_io_out+011
        movei 2,0200
        addm 2,(1)
        hrrz 1,dsk_active_request
        setzm storage_state
        setzm dsk_active_request
        aoj 1,
        pushj 017,proc_wakeup_event
        jrst storage_pi_return
dsk_pi_boot_done:
        movns storage_state
        jrst storage_pi_return

dsk_pi_error:
        cono 0270,0
        cono 0200,0
        skipn dsk_active_request
        jrst dsk_pi_boot_error
        pushj 017,dsk_fail_runtime
        jrst storage_pi_return
dsk_pi_boot_error:
        movei 2,7
        movem 2,storage_state
        jrst storage_pi_return

; PI3 final-word leaf.  Reads can end immediately; writes need the two DCT
; drain requests required by the Type-270 pipeline before ending the sector.
dsk_dct_handler:
dsk_dct_select:
        jrst dsk_dct_count_done

dsk_dct_count_done:
        move 2,storage_state
        addi 2,3
        jumpe 2,dsk_dct_read_done
        movei 1,dsk_dct_write_ack1
        hrrm 1,dsk_dct_select
        move 1,000047
        movem 1,000046
        jrst storage_pi_return

dsk_dct_write_ack1:
        movei 1,dsk_dct_write_ack2
        hrrm 1,dsk_dct_select
        jrst storage_pi_return
dsk_dct_write_ack2:
dsk_dct_read_done:
        cono 0270,030115
        cono 0200,0
        jrst storage_pi_return

; Called once per 60 Hz line-clock tick through the word immediately before
; dsk_read_sector.  Runtime requests reuse storage_count as timeout budget.
dsk_watchdog_tick:
        skipn dsk_active_request
        popj 017,
        sosle storage_count
        popj 017,
        cono 0270,0
        cono 0200,0
        jrst dsk_fail_runtime

dsk_fail_runtime:
        hrrz 1,dsk_active_request
        setom 1(1)
        setzm storage_state
        setzm dsk_active_request
        aoj 1,
        jrst proc_wakeup_event

; Build direct PI3 block transfer and its -count,,buffer-1 IOWD.
dsk_setup_read:
        move 4,dsk_dct_blki
        jrst dsk_setup_common
dsk_setup_write:
        move 4,dsk_dct_blko
dsk_setup_common:
        movem 3,storage_count
        movem 4,000046
        movei 4,dsk_dct_count_done
        hrrm 4,dsk_dct_select
        subi 2,1
        movn 4,3
        hrl 2,4
        movem 2,storage_iowd
        popj 017,
dsk_dct_blki:
        blki 0200,storage_iowd
dsk_dct_blko:
        blko 0200,storage_iowd

; The watchdog entry is deliberately the word immediately before the exported
; read service; clk_io.s uses read_addr-1 without another binding word.
dsk_watchdog_entry:
        jrst dsk_watchdog_tick
dsk_read_sector:
        setz 4,
        jrst dsk_sector_request

dsk_write_sector:
        movei 4,1
dsk_sector_request:
        skipn proc_table+2
        jrst dsk_boot_request

dsk_runtime_request:
        hrlz 5,1
        hrr 5,2
        push 017,5
        setz 5,
        push 017,5
        movei 1,-1(017)
        hrl 1,4
dsk_runtime_submit:
        skipe dsk_active_request
        jrst dsk_runtime_queue
        skipe storage_state
        jrst dsk_runtime_busy
        pushj 017,dsk_start_active
        jrst dsk_runtime_wait
dsk_runtime_queue:
        pushj 017,dsk_enqueue
        jumpl 1,dsk_runtime_submit_fail
dsk_runtime_wait:
        movei 1,(017)
        pushj 017,proc_wait_event
        pushj 017,dsk_dispatch
        move 1,(017)
        sub 017,[2,,2]
        sojn 1,dsk_runtime_ioerr
        popj 017,
dsk_runtime_busy:
        hrroi 1,0777775
dsk_runtime_submit_fail:
        sub 017,[2,,2]
        popj 017,
dsk_runtime_ioerr:
        hrroi 1,0777773
        popj 017,

; Two pending descriptors per unit.  q0 is next by one-way elevator distance;
; q1 is the later request.
dsk_enqueue:
        hlrz 2,(1)
        move 4,2
        lsh 4,-020
        move 3,4
        lsh 3,1
        addi 3,dsk_queue
        skipn (3)
        jrst dsk_enqueue_first
        skipe 1(3)
        jrst pdp10_ret_busy
        move 5,(3)
        movem 1,1(3)
        move 6,dsk_current_cyl(4)
        andi 2,0177700
        sub 2,6
        andi 2,0177700
        hlrz 7,(5)
        andi 7,0177700
        sub 7,6
        andi 7,0177700
        caml 2,7
        jrst dsk_enqueue_ok
        movem 1,(3)
        movem 5,1(3)
dsk_enqueue_ok:
        setz 1,
        popj 017,
dsk_enqueue_first:
        movem 1,(3)
        jrst dsk_enqueue_ok

dsk_dispatch:
        skipe dsk_active_request
        popj 017,
        skipe storage_state
        popj 017,
        setz 4,
dsk_dispatch_scan:
        skipn 1,dsk_queue(4)
        jrst dsk_dispatch_next
        move 2,dsk_queue+1(4)
        movem 2,dsk_queue(4)
        setzm dsk_queue+1(4)
        jrst dsk_start_active
dsk_dispatch_next:
        addi 4,2
        caie 4,010
        jrst dsk_dispatch_scan
        popj 017,

dsk_start_active:
        move 4,1
        move 3,(4)
        hlrz 1,3
        hrrz 2,3
        move 5,1
        lsh 5,-020
        move 6,1
        andi 6,0177700
        movem 6,dsk_current_cyl(5)
        move 6,4
        movei 3,0200
        tlne 4,1
        jrst dsk_start_write
        pushj 017,dsk_setup_read
        hrroi 3,0777775
        jrst dsk_start_go
dsk_start_write:
        pushj 017,dsk_setup_write
        hrroi 3,0777774
dsk_start_go:
        movem 6,dsk_active_request
        movem 3,storage_state
        datao 0270,1
        cono 0270,000125
        setz 1,
        popj 017,

dsk_boot_request:
        skipe storage_state
        jrst pdp10_ret_busy
        movei 3,0200
        jumpe 4,dsk_boot_read
        pushj 017,dsk_setup_write
        hrroi 3,0777774
        jrst dsk_boot_start
dsk_boot_read:
        pushj 017,dsk_setup_read
        hrroi 3,0777775
dsk_boot_start:
        movem 3,storage_state
        datao 0270,1
        cono 0270,000125
dsk_wait:
        move 1,storage_state
        jumpl 1,dsk_wait
        caie 1,7
        jrst dsk_wait_done
        setzm storage_state
        hrroi 1,0777773
        popj 017,
dsk_wait_done:
        move 2,storage_iowd
        hlrz 2,2
        add 2,storage_count
        andi 2,0777777
        addm 2,@dsk_account_table-3(1)
        setzm storage_state
        jrst pdp10_ret_ok

dsk_account_table:
        .word devicefs_io_in+011
        .word devicefs_io_out+011

        .bss
dsk_active_request: .block 1
dsk_current_cyl: .block 4
dsk_queue: .block 010
