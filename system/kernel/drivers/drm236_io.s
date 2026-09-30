/**
 * @file drm236_io.s
 * @brief Resident PDP-6 Type 167 I/O Processor / Type 236 drum driver.
 *
 * One request transfers exactly one 128-word Type 236 physical block. Before
 * the process table exists, boot-time callers use a bounded polled sequence.
 * At runtime the single DMA engine is serialized with one active descriptor
 * and one pending descriptor; both state words are aliased onto otherwise-dead
 * MonitorFS counter slots in device_state.s and consume no extra KCORE words.
 *
 * A normal transfer produces Type 167 DMA completion and Type 236 drum
 * completion on PI2. The first is acknowledged without completing the request;
 * the second publishes status, wakes the owner, and promotes the pending FIFO
 * entry if one exists. Request descriptors live on caller kernel stacks and
 * remain valid until their completion event wakes that caller.
 */

        .text
        .globl  drm236_pi_handler
        .globl  drm236_read_block_service
        .globl  drm236_write_block_service
        .globl  drm236_active_request
        .globl  drm236_pending_request
        .globl  mfsdev_drm_reads
        .globl  mfsdev_drm_writes
        .globl  mfsdev_storage_errors
        .globl  pdp10_pi_dispatch_done
        .globl  kret_ok
        .globl  kret_neg1
        .globl  proc_table
        .globl  proc_wait_event
        .globl  storage_request_init
        .globl  proc_wakeup_event

        .set    DRM_DP_ERROR,0100060   ; Type 167 error summary bits
        .set    DRM_DP_DONE,0000010
        .set    DRM_DR_DONE,0000100
        .set    DRM_DR_ERROR,0001000
        .set    DRM_DR_SELECT,0000260
        .set    DRM_DR_DESELECT,0000270
        .set    DRM_DR_CLEAR_DESELECT,0200270
        .set    DRM_DR_WRITE,0000220
        .set    DRM_DR_READ,0000230
        .set    DRM_PI_LEVEL,2
        .set    DRM_POLL_LIMIT,0777777
        .set    DRM_POLL_PASSES,010

/**
 * @brief Read one raw Type 236 address/block through boot or runtime service.
 * @param AC1 Type 236 hardware group address.
 * @param AC2 Core buffer address.
 * @return AC1 = 0 on success or -1 on hardware/timeout failure.
 */
drm236_read_block_service:
        setz    3,                      ; DP read: device -> core
        movei   4,DRM_DR_READ
        jrst    drm236_request

drm236_write_block_service:
        movei   3,0100                  ; DP write: core -> device
        movei   4,DRM_DR_WRITE

drm236_request:
        skipn   proc_table
        jrst    drm236_poll_request

        ; Runtime request descriptor on the caller's private kernel stack:
        ;   0: Type-236 address,,core buffer
        ;   1: Type-236 operation (READ or WRITE)
        ;   2: completion event (0 pending, 1 success, -1 failure)
        add     017,[3,,3]
        pushj   017,storage_request_init

drm236_runtime_retry:
        movei   1,-2(017)
        skipn   drm236_active_request
        jrst    drm236_runtime_claim

; Minimal assumption-free scheduler: retain exactly one pending request in the
; existing state word.  There is no rotational/timing model.  If the FIFO slot
; is occupied, wait for the active owner to complete and retry.  Slot 0 uses
; the same path: proc_wait_event polls there, while PI completion promotes the
; pending descriptor directly, so no scheduler wakeup is required for handoff.
drm236_runtime_queue:
        skipe   drm236_pending_request
        jrst    drm236_runtime_queue_full
        movem   1,drm236_pending_request
        ; Close the completion/enqueue race.  If the previous owner completed
        ; before seeing this pending pointer, claim the now-idle engine here.
        skipe   drm236_active_request
        jrst    drm236_runtime_wait
        setzm   drm236_pending_request
drm236_runtime_claim:
        movem   1,drm236_active_request
        pushj   017,drm236_start_active
drm236_runtime_wait:
        movei   1,(017)
        pushj   017,proc_wait_event
        jrst    drm236_runtime_finish

drm236_runtime_queue_full:
        skipn   1,drm236_active_request
        jrst    drm236_runtime_retry
        addi    1,2
        pushj   017,proc_wait_event
        jrst    drm236_runtime_retry

drm236_runtime_finish:
        move    1,(017)
        move    4,-1(017)               ; preserve operation for accounting
        sub     017,[3,,3]
        sojn    1,drm236_account_error
        jrst    drm236_account_success

/**
 * @brief Start the runtime descriptor named by AC1 on PI2.
 * @param AC1 Address of three-word request descriptor.
 * @return Returns normally after programming hardware; completion is PI2.
 *
 * Clobbers AC1..AC6 while decoding address, buffer, direction and operation.
 */
drm236_start_active:
        move    6,(1)
        move    4,1(1)                  ; READ/WRITE command
        hrrz    2,6                     ; core buffer
        hlrz    1,6                     ; Type-236 address
        setz    3,                      ; DP read: device -> core
        trnn    4,010                   ; READ 0230 has bit 010; WRITE 0220 does not
        movei   3,0100                  ; DP write: core -> device
        movei   5,DRM_PI_LEVEL
        jrst    drm236_start_hw

/**
 * @brief Program the common Type 167/236 transfer sequence.
 * @param AC1 Type 236 group address.
 * @param AC2 Core buffer address.
 * @param AC3 Type 167 direction bits (0 read, 0100 write).
 * @param AC4 Type 236 READ/WRITE operation.
 * @param AC5 PI bits, or zero for boot-time polling.
 * @return Hardware started; AC6 clobbered with the -0200,,buffer IOWD.
 */
drm236_start_hw:
        cono    0010,0(3)
        move    6,[-0200,,0]
        hrr     6,2
        datao   0010,6
        datao   0400,1
        cono    0400,DRM_DR_SELECT
        add     4,5
        cono    0400,0(4)
        popj    017,

/**
 * @brief Service Type 167 and Type 236 PI2 completion/error conditions.
 * @return Does not return normally; jumps to pdp10_pi_dispatch_done.
 *
 * AC1 and AC2 are clobbered. Type 167 DONE is acknowledged first and leaves
 * the request active. Type 236 DONE stores +1 in the descriptor event; either
 * hardware error stores -1. Completion clears the active pointer, wakes its
 * event, then promotes and starts the one pending descriptor before leaving
 * interrupt context.
 */
drm236_pi_handler:
        coni    0010,1
        trne    1,DRM_DP_ERROR
        jrst    drm236_pi_error
        trne    1,DRM_DP_DONE
        jrst    drm236_pi_dp_done
        coni    0400,1
        trne    1,DRM_DR_ERROR
        jrst    drm236_pi_error
        trnn    1,DRM_DR_DONE
        jrst    pdp10_pi_dispatch_done
        cono    0400,DRM_DR_DESELECT
        movei   2,1
        jrst    drm236_pi_finish

drm236_pi_dp_done:
        cono    0010,0
        jrst    pdp10_pi_dispatch_done

drm236_pi_error:
        cono    0010,0
        cono    0400,DRM_DR_CLEAR_DESELECT
        seto    2,

drm236_pi_finish:
        skipn   1,drm236_active_request
        jrst    pdp10_pi_dispatch_done
        movem   2,2(1)
        setzm   drm236_active_request
        addi    1,2
        pushj   017,proc_wakeup_event
        skipn   1,drm236_pending_request
        jrst    pdp10_pi_dispatch_done
        setzm   drm236_pending_request
        movem   1,drm236_active_request
        pushj   017,drm236_start_active
        jrst    pdp10_pi_dispatch_done

/**
 * @brief Perform the same transfer before process/event services exist.
 * @param AC1 Type 236 address; AC2 buffer; AC3 direction; AC4 drum operation.
 * @return AC1 = 0 on success or -1 on timeout/hardware error.
 *
 * The two bounded poll phases separately wait for Type 167 and Type 236 DONE.
 * The repeated 18-bit-friendly countdown covers nearly a full drum revolution
 * without requiring a larger loop counter representation.
 */
drm236_poll_request:
        setz    5,
        pushj   017,drm236_start_hw
        ; A Type-236 request may wait almost one full 8192-word revolution.
        ; Keep the inner counter 18-bit/PDP-6-friendly and repeat it enough
        ; times for simulator instruction-rate scaling as well as real hardware.
        movei   7,DRM_POLL_PASSES
drm236_poll_dp_outer:
        movei   5,DRM_POLL_LIMIT
drm236_poll_dp:
        coni    0010,6
        trne    6,DRM_DP_ERROR
        jrst    drm236_poll_error
        trne    6,DRM_DP_DONE
        jrst    drm236_poll_dp_done
        sojg    5,drm236_poll_dp
        sojg    7,drm236_poll_dp_outer
        jrst    drm236_poll_error

drm236_poll_dp_done:
        cono    0010,0
        movei   7,DRM_POLL_PASSES
drm236_poll_dr_outer:
        movei   5,DRM_POLL_LIMIT
drm236_poll_dr:
        coni    0400,6
        trne    6,DRM_DR_ERROR
        jrst    drm236_poll_error
        trne    6,DRM_DR_DONE
        jrst    drm236_poll_success
        sojg    5,drm236_poll_dr
        sojg    7,drm236_poll_dr_outer
        jrst    drm236_poll_error

drm236_poll_success:
        cono    0400,DRM_DR_DESELECT
        jrst    drm236_account_success

drm236_poll_error:
        cono    0010,0
        cono    0400,DRM_DR_CLEAR_DESELECT
        jrst    drm236_account_error

; DRM0 occupies MonitorFS device view id 021.  The dense counter slots also host the
; runtime ownership words in otherwise-unused ids 017/020, so extending the
; arrays through DRM0 costs no additional fixed KCORE.
drm236_account_success:
        trne    4,010                   ; READ 0230 has bit 010, WRITE 0220 does not
        aos     mfsdev_drm_reads
        trnn    4,010
        aos     mfsdev_drm_writes
        jrst    kret_ok

drm236_account_error:
        aos     mfsdev_storage_errors+5
        jrst    kret_neg1
