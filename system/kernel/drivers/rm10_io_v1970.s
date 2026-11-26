/**
 * @file rm10_io_v1970.s
 * @brief KA10 RC10/RM10 provider using the first reserved DF10 pair.
 *
 * The existing drum service ABI passes unit<<16 | block<<3 in AC1,
 * and a physical 128-word buffer in AC2. Convert to BCD track/segment
 * only here: 30 blocks per track, 90 tracks. No RD10 compatibility.
 * One active stack descriptor, shared DRM clock hook and PI2. Other
 * callers wait on its event and retry; no new KCORE or per-process
 * storage. DF10 owns 0150/0151 until completion or verified STOP.
 */
        .text
        .globl drm236_read_block_service
        .globl drm236_write_block_service
        .globl drm236_pi_handler
        .globl drm236_watchdog_tick_v1079
        .globl drm236_active_request
        .globl ka10_df10_cia
        .globl ka10_df10_completion
        .globl storage_request_init
        .globl proc_table
        .globl proc_wait_event
        .globl proc_wakeup_event
        .globl pdp10_pi_dispatch_done
        .globl mfsdev_drm_reads
        .globl mfsdev_drm_writes
        .globl mfsdev_storage_errors
        .globl kconst_3_3
        .globl kret_zero
        .globl kret_neg1
        .globl rm10_start_v1970
        .globl rm10_abort_v1970

        .equ RM10_BUSY,0000020
        .equ RM10_DONE,0000010
        .equ RM10_ERRORS,0377740
        .equ RM10_ACK,0177710
        .equ RM10_STOP,0177730
        .equ RM10_WRITE,0001000
        .equ RM10_PI,2
        .equ RM10_BLOCKS,05214
        .equ RM10_TICKS,0454

drm236_read_block_service:
        setz 4,
        jrst rm10_request_v1970
drm236_write_block_service:
        movei 4,RM10_WRITE
rm10_request_v1970:
        ; DF10 reserves CDA zero for a zero-fill/discard transfer.
        ; Reject wrapped counts and invalid hardware group addresses.
        tlne 1,0777777
        jrst kret_neg1
        trne 1,7
        jrst kret_neg1
        move 5,1
        andi 5,0177777
        lsh 5,-3
        cail 5,RM10_BLOCKS
        jrst kret_neg1
        tlne 2,0777777
        jrst kret_neg1
        caige 2,2
        jrst kret_neg1
        caile 2,0777600
        jrst kret_neg1
        add 017,kconst_3_3
        pushj 017,storage_request_init
rm10_claim_v1970:
        hrrz 1,drm236_active_request
        jumpe 1,rm10_claim_idle_v1970
        addi 1,2
        pushj 017,proc_wait_event
        jrst rm10_claim_v1970
rm10_claim_idle_v1970:
        coni 0170,1
        trne 1,RM10_BUSY
        jrst rm10_unowned_busy_v1970
        movei 1,-2(017)
        pushj 017,rm10_start_v1970
        movei 1,(017)
        pushj 017,proc_wait_event
        move 1,(017)
        sub 017,kconst_3_3
        sojn 1,kret_neg1
        jrst kret_zero
rm10_unowned_busy_v1970:
        sub 017,kconst_3_3
        jrst kret_neg1

/** @brief Form a DF10 control word and native RC10 command. */
rm10_start_v1970:
        move 7,1
        hlrz 1,(7)
        hrrz 2,(7)
        subi 2,1
        hrli 2,0777600                ; -128,,buffer-1
        movem 2,ka10_df10_cia
        setzm ka10_df10_completion
        move 5,1
        andi 5,0177777
        lsh 5,-3
        idivi 5,036                   ; track, block within track
        move 3,6
        idivi 5,012                   ; track tens, ones
        lsh 5,4
        ior 5,6
        lsh 5,031                    ; BCD track at bit 25
        andi 1,0600000
        lsh 1,022                    ; unit at bit 34
        ior 1,5
        move 5,3
        lsh 5,1
        idivi 5,012
        lsh 5,4
        ior 5,6
        lsh 5,022                    ; BCD segment at bit 18
        ior 1,5
        ior 1,1(7)                   ; direction
        iori 1,ka10_df10_cia
        cono 0170,RM10_ACK+RM10_PI
        hrli 7,RM10_TICKS
        movem 7,drm236_active_request
        datao 0170,1
        popj 017,

/** @brief Acknowledge and validate one genuine PI2 DMA completion. */
drm236_pi_handler:
        coni 0170,2
        trnn 2,RM10_DONE
        jrst pdp10_pi_dispatch_done
        trne 2,RM10_BUSY
        jrst pdp10_pi_dispatch_done
        hrrz 1,drm236_active_request
        jumpe 1,rm10_pi_ack_v1970
        trne 2,RM10_ERRORS
        jrst rm10_pi_error_v1970
        hrrz 2,(1)
        addi 2,0177
        hrli 2,ka10_df10_completion
        came 2,ka10_df10_completion
        jrst rm10_pi_error_v1970
        movei 2,1
        jrst rm10_pi_finish_v1970
rm10_pi_error_v1970:
        seto 2,
rm10_pi_finish_v1970:
        cono 0170,RM10_ACK
        pushj 017,rm10_finish_v1970
        jrst pdp10_pi_dispatch_done
rm10_pi_ack_v1970:
        cono 0170,RM10_ACK
        jrst pdp10_pi_dispatch_done

; AC2 status, AC1 descriptor. Only AC1/2 are scratch. The owner
; cannot reuse its stack until the controller is quiescent.
rm10_finish_v1970:
        movem 2,2(1)
        jumpg 2,rm10_count_success_v1970
        aos mfsdev_storage_errors+5
        jrst rm10_finished_v1970
rm10_count_success_v1970:
        skipe 1(1)
        jrst rm10_count_write_v1970
        aos mfsdev_drm_reads
        jrst rm10_finished_v1970
rm10_count_write_v1970:
        aos mfsdev_drm_writes
rm10_finished_v1970:
        setzm drm236_active_request
        addi 1,2
        skipn proc_table
        popj 017,
        jrst proc_wakeup_event

/** @brief PI6 deadline: atomically claim, stop DMA, verify quiescence. */
drm236_watchdog_tick_v1079:
        skipn drm236_active_request
        popj 017,
        coni 0004,2
        cono 0004,000400
        push 017,2
        skipn drm236_active_request
        jrst rm10_watchdog_restore_v1970
        movsi 1,-1
        addm 1,drm236_active_request
        hlrz 1,drm236_active_request
        jumpn 1,rm10_watchdog_restore_v1970
rm10_abort_v1970:
        cono 0170,RM10_STOP
        coni 0170,2
        trne 2,RM10_BUSY
        halt .                       ; never release a live DMA buffer
        cono 0170,RM10_ACK
        hrrz 1,drm236_active_request
        seto 2,
        pushj 017,rm10_finish_v1970
rm10_watchdog_restore_v1970:
        pop 017,2
        trne 2,000200
        cono 0004,000200
        popj 017,
