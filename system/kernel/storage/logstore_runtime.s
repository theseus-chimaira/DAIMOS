; logstore_runtime.s -- compact runtime LOGSTORE raw-range MRES.
;
; KINIT performs the full record scan and installs the recovered producer
; cursor.  This MRES deliberately owns no 128-word scratch buffer and no sink
; policy.  Its first slice only publishes status and bounded raw LOGSTORE I/O.
;
; Register service ABI:
;   AC5=1 STATUS: returns AC1=next sequence, AC2=capacity,,next slot,
;                AC3=LOGSTORE blocks
;   AC5=2 READ:  AC1=relative LOGSTORE block, AC2=128-word buffer
;   AC5=3 WRITE: AC1=relative LOGSTORE block, AC2=128-word buffer
;
; State words:
;   0 root-logical LOGSTORE start
;   1 LOGSTORE block count
;   2 next producer sequence
;   3 capacity,,next slot
;   4 zero for BLOCKSET mapping; otherwise (unit+1),,physical root base

        .text
        .globl  logstore_mres_dispatch
        .globl  logstore_mres_state
        .globl  logstore_backend_read_jump
        .globl  logstore_backend_write_jump
        .globl  pdp10_ret_neg1

logstore_mres_dispatch:
        cain    5,1
        jrst    logstore_status
        cain    5,2
        jrst    logstore_read
        cain    5,3
        jrst    logstore_write
        jrst    pdp10_ret_neg1

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
        .block  5
