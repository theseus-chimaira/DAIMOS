; logstore_runtime.s -- compact runtime LOGSTORE raw-range MRES.
;
; KINIT performs the full record scan and installs the recovered producer
; cursor.  This MRES deliberately owns no 128-word scratch buffer and no sink
; policy.  It publishes status, bounded raw LOGSTORE I/O, commit-last append, and wait/wake.
;
; Register service ABI:
;   AC5=1 STATUS: returns AC1=next sequence, AC2=capacity,,next slot,
;                AC3=LOGSTORE blocks
;   AC5=2 READ:  AC1=relative LOGSTORE block, AC2=128-word buffer
;   AC5=3 WRITE: AC1=relative LOGSTORE block, AC2=128-word buffer
;   AC5=4 APPEND: AC1=caller-owned 128-word LSREC1 scratch block
;   AC5=5 WAIT: AC1=observed next sequence; sleep until it changes
;
; State words:
;   0 root-logical LOGSTORE start
;   1 LOGSTORE block count
;   2 next producer sequence
;   3 capacity,,next slot
;   4 zero for BLOCKSET mapping; otherwise (unit+1),,physical root base
;   5 append event flag used by WAIT

        .text
        .globl  logstore_mres_dispatch
        .globl  logstore_mres_state
        .globl  logstore_backend_read_jump
        .globl  logstore_backend_write_jump
        .globl  pdp10_ret_neg1
        .globl  proc_wait_event
        .globl  proc_wakeup_event

logstore_mres_dispatch:
        cain    5,1
        jrst    logstore_status
        cain    5,2
        jrst    logstore_read
        cain    5,3
        jrst    logstore_write
        cain    5,4
        jrst    logstore_append
        caie    5,5
        jrst    pdp10_ret_neg1
        ; WAIT is keyed by the producer sequence observed by userspace.  Clear
        ; the event, recheck the sequence, then sleep.  proc_wait_event checks
        ; the flag again after arming the process, closing the append/sleep race.
        camn    1,logstore_mres_state+2
        jrst    logstore_wait_arm
        setz    1,
        popj    17,
logstore_wait_arm:
        setzm   logstore_mres_state+5
        camn    1,logstore_mres_state+2
        jrst    logstore_wait_sleep
        setz    1,
        popj    17,
logstore_wait_sleep:
        movei   1,logstore_mres_state+5
        pushj   17,proc_wait_event
        setz    1,
        popj    17,

; Append uses a caller-owned 128-word scratch block.  The caller supplies
; timestamp/metadata/payload in words 2..126.  LOGSTORE owns word 0 magic,
; word 1 sequence, and word 127 commit trailer.  State advances only after the
; backing write succeeds.
logstore_append:
        jumpe   1,pdp10_ret_neg1
        move    4,1                     ; stable caller scratch pointer
        move    6,logstore_mres_state+2 ; next sequence
        jumpe   6,pdp10_ret_neg1
        move    0,6
        aoje    0,pdp10_ret_neg1        ; reject 36-bit sequence exhaustion
        hrrz    7,3(4)                  ; payload words
        caile   7,0173                  ; 123 payload words maximum
        jrst    pdp10_ret_neg1
        move    0,[0546362454321]       ; SIXBIT /LSREC1/
        movem   0,(4)
        movem   6,1(4)
        move    1,7
        lsh     1,022                   ; payload_words << 18
        add     1,6
        add     1,0
        setcm   1,1
        tro     1,1
        movem   1,0177(4)

        hrrz    1,logstore_mres_state+3 ; producer slot
        addi    1,2                     ; skip state A/B
        move    2,4
        pushj   17,logstore_write
        jumpn   1,logstore_append_done

        hrrz    7,logstore_mres_state+3
        aoj     7,
        hlrz    0,logstore_mres_state+3
        caml    7,0
        setz    7,
        hrrm    7,logstore_mres_state+3
        aos     logstore_mres_state+2
        setom   logstore_mres_state+5
        movei   1,logstore_mres_state+5
        pushj   17,proc_wakeup_event
        setz    1,
logstore_append_done:
        popj    17,

logstore_status:
        move    1,logstore_mres_state+2
        move    2,logstore_mres_state+3
        move    3,logstore_mres_state+1
        popj    17,

logstore_read:
        setz    5,
        jrst    logstore_io
logstore_write:
        movei   5,1

; AC1 relative block, AC2 buffer, AC5 write flag.
logstore_io:
        jumpe   2,pdp10_ret_neg1
        jumpl   1,pdp10_ret_neg1
        caml    1,logstore_mres_state+1
        jrst    pdp10_ret_neg1
        move    4,2
        add     1,logstore_mres_state
        move    6,logstore_mres_state+4
        jumpn   6,logstore_io_direct

        ; Multi-member root: the patched BLOCKSET read/write entry consumes
        ; AC1=root-logical block, AC2=buffer.
        move    2,4
        jumpe   5,logstore_backend_read_jump
logstore_backend_write_jump:
        jrst    0
logstore_backend_read_jump:
        jrst    0

logstore_io_direct:
        ; Singleton root: convert root-logical to physical block and invoke the
        ; patched DSK/DRM KCORE bridge: AC1=unit, AC2=block, AC3=buffer.
        hrrz    2,6
        add     2,1
        hlrz    1,6
        subi    1,1
        move    3,4
        jumpe   5,logstore_backend_read_jump
        jrst    logstore_backend_write_jump

        .bss
logstore_mres_state:
        .block  6
