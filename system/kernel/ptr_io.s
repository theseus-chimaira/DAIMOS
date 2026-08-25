; ptr_io.s -- compact resident PDP-6 paper-tape reader driver.
;
; DATAI starts the next mechanical read.  After satisfying a request, PIA is
; therefore set to zero so that one hardware-prefetched character may become
; DONE without interrupting or overwriting the software result.

        .text
        .globl ptr_pi_service
        .globl ptr_getchar
        .globl ptr_state
        .globl pdp10_pi_handler_return

; ptr_state: bit 0 pending, bit 1 software-ready, byte in bits 2..9.
ptr_pi_service:
        coni 0104,1
        trnn 1,0010
        popj 017,
        move 1,ptr_state
        trnn 1,0001
        jrst ptr_pi_unrequested
        datai 0104,1
        cono 0104,0
        andi 1,0377
        lsh 1,2
        iori 1,0002
        movem 1,ptr_state
        popj 017,

; Preserve an unsolicited prefetched character in the hardware while
; removing its PI request.  CONO DONE with PIA zero leaves CHR readable.
ptr_pi_unrequested:
        cono 0104,0010
        popj 017,

; AC1 = int *destination.  Return 0 or PT_E_ARG/BUSY/TIMEOUT.
ptr_getchar:
        jumpe 1,ptr_get_arg
        move 4,1
        move 2,ptr_state
        trne 2,0002
        jrst ptr_get_software
        trne 2,0001
        jrst ptr_get_busy

        ; Consume an already prefetched hardware character without waiting.
        coni 0104,3
        trne 3,0010
        jrst ptr_get_hardware

        movei 2,0001
        movem 2,ptr_state
        cono 0104,0027
        movei 5,0200000
ptr_get_wait:
        move 2,ptr_state
        trne 2,0002
        jrst ptr_get_software
        sojg 5,ptr_get_wait
        setzm ptr_state
        cono 0104,0
        hrroi 1,0777776
        popj 017,

ptr_get_hardware:
        datai 0104,3
        cono 0104,0
        andi 3,0377
        movem 3,(4)
        movei 1,0
        popj 017,

ptr_get_software:
        move 3,2
        lsh 3,-2
        andi 3,0377
        movem 3,(4)
        setzm ptr_state
        movei 1,0
        popj 017,
ptr_get_arg:
        seto 1,
        popj 017,
ptr_get_busy:
        hrroi 1,0777775
        popj 017,

        .bss
ptr_state:
        .block 1
