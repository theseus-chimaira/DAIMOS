; fs_mres_pdp10.s -- fixed resident bridge to an optional filesystem MRES.
        .text
        .globl fs_mres_call

; int fs_mres_call(address, request)
; C args arrive in AC1,AC2.  Dispatcher expects request pointer in AC1.
fs_mres_call:
        move    3,1
        move    1,2
        andi    3,0777777
        jumpe   3,fs_mres_no_service
        pushj   17,(3)
        popj    17,
fs_mres_no_service:
        hrroi   1,1
        popj    17,
