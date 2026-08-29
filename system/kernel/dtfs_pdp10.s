; dtfs_pdp10.s -- resident indirect bridge to block-addressed DTC MRES.
        .text
        .globl dtfs_v1_dtc_call

; int dtfs_v1_dtc_call(address, unit, block, buffer)
; C args arrive in AC1..AC4; DTC service expects unit/block/buffer in AC1..AC3.
dtfs_v1_dtc_call:
        move    5,1
        move    1,2
        move    2,3
        move    3,4
        andi    5,0777777
        jumpe   5,dtfs_dtc_no_service
        pushj   17,(5)
        popj    17,
dtfs_dtc_no_service:
        hrroi   1,1
        popj    17,
