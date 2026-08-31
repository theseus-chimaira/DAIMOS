; dsk270_pdp10.s -- indirect bridge to the resident DSK270 sector service.
        .text
        .globl dsk270_call

; int dsk270_call(address, raw_address, block)
; C args arrive in AC1..AC3.  MRES expects raw address/buffer in AC1..AC2.
dsk270_call:
        move    4,1
        move    1,2
        move    2,3
        andi    4,0777777
        jumpe   4,dsk270_no_service
        pushj   17,(4)
        popj    17,
dsk270_no_service:
        hrroi   1,1
        popj    17,
