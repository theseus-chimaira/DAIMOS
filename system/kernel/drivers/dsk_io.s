/**
 * @file dsk_io.s
 * @brief Resident PDP-6 Type 270 disk controller and request scheduler.
 *
 * Type 136 DCT ownership/state lives in fixed KCORE storage_router.s. This
 * optional MRES contains only disk-specific PI leaves, one-sector transfer
 * setup, a bounded two-request-per-unit elevator queue, and a five-second
 * active-transfer watchdog. The shared router is the sole generic PI3/PI5
 * entry point.
 *
 * Runtime requests use three-word descriptors on caller kernel stacks. The
 * active-request word packs a 300-tick watchdog countdown in its left half and
 * the 18-bit descriptor pointer in its right half. Each dsk_queue word packs
 * q0 in the left half and q1 in the right half, avoiding separate queue-node
 * storage. dsk_current_cyl keeps only the raw cylinder field for each unit.
 */

        .globl mfsdev_io_in
        .globl mfsdev_io_out
        .globl mfsdev_storage_errors
        .text
        .globl dsk_pi_handler
        .globl dsk_dct_handler
        .globl dsk_watchdog_tick
        .globl dsk_enqueue
        .globl dsk_queue
        .globl dsk_current_cyl
        .globl dsk_read_sector
        .globl dsk_write_sector
        .globl pdp10_pi_dispatch_done
        .globl storage_state
        .globl storage_iowd
        .globl storage_count
        .globl kret_ok
        .globl kret_zero
        .globl kret_busy
        .globl proc_table
        .globl proc_current_slot
        .globl proc_wait_event
        .globl storage_request_init
        .globl proc_wakeup_event

/**
 * @brief Service the Type 270 PI5 status/idle/error leaf.
 * @return Does not return normally; jumps to pdp10_pi_dispatch_done.
 *
 * AC1 and AC2 are scratch here because the storage router's direct completion
 * path restores interrupted ACs rather than resuming generic PI fanout. Runtime
 * idle completion sets the request event to +1 and accounts the operation;
 * boot completion converts the negative storage_state code to 3/4 for the
 * polling path. Controller errors fail the active request or set boot state 7.
 */
dsk_pi_handler:
        coni 0270,1
        trne 1,001777
        jrst dsk_pi_error
        trne 1,0400000
        jrst dsk_pi_idle
        trnn 1,040000
        jrst pdp10_pi_dispatch_done
        move 2,storage_state
        addi 2,3
        jumpe 2,dsk_pi_start_read
        cono 0200,003403
        cono 0270,002105
        jrst pdp10_pi_dispatch_done
dsk_pi_start_read:
        cono 0200,004003
        cono 0270,001105
        jrst pdp10_pi_dispatch_done

dsk_pi_idle:
        cono 0270,0
        skipn 2,dsk_active_request
        jrst dsk_pi_boot_done
        aos 1,2(2)
        move 1,1(2)
        aos @dsk_account_table(1)       ; operation 0=read, 1=write
        hrrz 1,dsk_active_request
        setzm storage_state
        setzm dsk_active_request
        addi 1,2
        pushj 017,proc_wakeup_event
        jrst pdp10_pi_dispatch_done
dsk_pi_boot_done:
        movns storage_state
        jrst pdp10_pi_dispatch_done

dsk_pi_error:
        cono 0270,030200
        cono 0200,0
        skipn dsk_active_request
        jrst dsk_pi_boot_error
        pushj 017,dsk_fail_runtime
        jrst pdp10_pi_dispatch_done
dsk_pi_boot_error:
        movei 2,7
        movem 2,storage_state
        jrst pdp10_pi_dispatch_done

/**
 * @brief Service the Type 136 PI3 final-word/drain sequence.
 * @return Does not return normally; jumps to pdp10_pi_dispatch_done.
 *
 * Reads end the sector immediately. Writes require two additional DCT drain
 * acknowledgements before END/CLEAR because the Type 270 pipeline still owns
 * buffered words. dsk_dct_select is a one-word self-patched state jump.
 */
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
        jrst pdp10_pi_dispatch_done

dsk_dct_write_ack1:
        movei 1,dsk_dct_write_ack2
        hrrm 1,dsk_dct_select
        jrst pdp10_pi_dispatch_done
dsk_dct_write_ack2:
dsk_dct_read_done:
        cono 0270,030115
        cono 0200,0
        jrst pdp10_pi_dispatch_done

dsk_fail_runtime:
        aos mfsdev_storage_errors+2   ; DSK0
        hrrz 1,dsk_active_request
        setom 2(1)
        setzm storage_state
        setzm dsk_active_request
        addi 1,2
        jrst proc_wakeup_event

/**
 * @brief Age and fail the single active Type 270 request after five seconds.
 *
 * CLK calls this at 60 Hz even while slot-0 swap service suppresses scheduler
 * preemption. The left half of dsk_active_request is the remaining tick count;
 * its right half is the request descriptor pointer and is unchanged by SUB.
 */
dsk_watchdog_tick:
        skipn dsk_active_request
        popj 017,
        sub dsk_active_request,[1,,0]
        hlrz 1,dsk_active_request
        jumpe 1,dsk_watchdog_timeout
        popj 017,
dsk_watchdog_timeout:
        cono 0270,0
        cono 0200,0
        pushj 017,dsk_fail_runtime
        popj 017,

/**
 * @brief Build the direct PI3 transfer instruction and -count,,buffer-1 IOWD.
 * @param AC2 Buffer address; @param AC3 word count (normally 0200).
 * @return storage_iowd and low-memory DCT vector prepared; AC4 clobbered.
 */
dsk_setup_read:
        move 4,dsk_dct_blki
        jrst dsk_setup_common
dsk_setup_write:
        move 4,dsk_dct_blko
dsk_setup_common:
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

dsk_read_sector:
        setz 4,
        jrst dsk_sector_request

dsk_write_sector:
        movei 4,1
dsk_sector_request:
        skipn proc_table
        jrst dsk_boot_request

dsk_runtime_request:
        add 017,[3,,3]
        pushj 017,storage_request_init
        movei 1,-2(017)
dsk_runtime_submit:
        ; Slot 0 performs synchronous swap I/O on the permanent idle stack.
        ; PI6 is suppressed while that service is active, so it must not put
        ; its request into the ordinary queue and then depend on a sleeping
        ; process resuming to dispatch it.  Drain older requests in order,
        ; then start the slot-0 descriptor directly and poll its event word.
        skipn proc_current_slot
        jrst dsk_runtime_slot0
        skipe dsk_active_request
        jrst dsk_runtime_queue
        skipe storage_state
        jrst dsk_runtime_busy
        pushj 017,dsk_start_active
        jrst dsk_runtime_wait
dsk_runtime_queue:
        pushj 017,dsk_enqueue
        jumpl 1,dsk_runtime_submit_fail
        ; Close the completion/enqueue race: if the active request completed
        ; between the test above and enqueue, immediately promote the queued
        ; request before this process sleeps.
        pushj 017,dsk_dispatch
dsk_runtime_wait:
        movei 1,(017)
        pushj 017,proc_wait_event
        pushj 017,dsk_dispatch
        jrst dsk_runtime_finish

dsk_runtime_slot0:
        pushj 017,dsk_dispatch
        skipe dsk_active_request
        jrst dsk_runtime_slot0
        skipe storage_state
        jrst dsk_runtime_slot0
        movei 1,-2(017)
        pushj 017,dsk_start_active
dsk_runtime_slot0_wait:
        skipn (017)
        jrst dsk_runtime_slot0_wait
dsk_runtime_finish:
        move 1,(017)
        sub 017,[3,,3]
        sojn 1,kret_neg5
        popj 017,
dsk_runtime_busy:
        hrroi 1,0777775
dsk_runtime_submit_fail:
        sub 017,[3,,3]
        popj 017,

/**
 * @brief Insert a runtime descriptor into its unit's two-entry elevator queue.
 * @param AC1 Descriptor address; descriptor word 0 contains rawaddr,,buffer.
 * @return AC1 = 0, or STORAGE_E_BUSY when both pending slots are occupied.
 *
 * q0 is the left-half descriptor pointer and is ordered nearest at/above the
 * current cylinder; q1 is the later right-half request. No queue node storage
 * exists beyond the four packed dsk_queue words.
 */
dsk_enqueue:
        hlrz 2,(1)
        move 4,2
        lsh 4,-020
        hlrz 5,dsk_queue(4)
        jumpe 5,dsk_enqueue_first
        hrrz 6,dsk_queue(4)
        jumpn 6,kret_busy
        hrrm 1,dsk_queue(4)
        move 6,dsk_current_cyl(4)
        sub 2,6
        andi 2,0177700
        hlrz 7,(5)
        sub 7,6
        andi 7,0177700
        caml 2,7
        jrst dsk_enqueue_ok
        hrlm 1,dsk_queue(4)
        hrrm 5,dsk_queue(4)
dsk_enqueue_ok:
        jrst kret_zero
dsk_enqueue_first:
        hrlm 1,dsk_queue(4)
        jrst dsk_enqueue_ok

dsk_dispatch:
        skipe dsk_active_request
        popj 017,
        skipe storage_state
        popj 017,
        hrlzi 4,0777774             ; AOBJN count -4, unit index 0
dsk_dispatch_scan:
        hlrz 1,dsk_queue(4)
        jumpn 1,dsk_dispatch_found
        aobjn 4,dsk_dispatch_scan
        popj 017,
dsk_dispatch_found:
        hrlzs 2,dsk_queue(4)          ; promote q1 RH -> q0 LH, clear q1

/** @brief Decode AC1 descriptor, arm watchdog state, and start one sector. */
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
        skipe 1(4)
        jrst dsk_start_write
        pushj 017,dsk_setup_read
        hrroi 3,0777775
        jrst dsk_start_go
dsk_start_write:
        pushj 017,dsk_setup_write
        hrroi 3,0777774
dsk_start_go:
        hrli 6,0454                  ; 300 ticks = 5 seconds at 60 Hz
        movem 6,dsk_active_request
        movem 3,storage_state
        datao 0270,1
        cono 0270,000125
        jrst kret_zero

dsk_boot_request:
        skipe storage_state
        jrst kret_busy
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
        jrst    kret_neg5
dsk_wait_done:
        aos @dsk_account_table-3(1)     ; completed sector request
        setzm storage_state
        jrst kret_ok

dsk_account_table:
        .word mfsdev_io_in+016
        .word mfsdev_io_out+016

        .bss
/** Active request: watchdog ticks in LH, 18-bit descriptor pointer in RH. */
dsk_active_request: .block 1
/** Last serviced raw cylinder field for each of four physical units. */
dsk_current_cyl: .block 4
/** Four packed two-entry queues: q0 descriptor in LH, q1 descriptor in RH. */
dsk_queue: .block 4
