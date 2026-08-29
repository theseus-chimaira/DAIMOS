; d6fs_disk_pdp10.s -- indirect bridge to the DSK270 sector service.
        .text
        .globl d6fs_dsk_v2_call

; int d6fs_dsk_v2_call(address, raw_address, block)
; C args arrive in AC1..AC3.  DSK service expects raw address/buffer in AC1..AC2.
d6fs_dsk_v2_call:
        move    4,1
        move    1,2
        move    2,3
        andi    4,0777777
        jumpe   4,d6fs_dsk_v2_no_service
        pushj   17,(4)
        popj    17,
d6fs_dsk_v2_no_service:
        hrroi   1,1
        popj    17,
