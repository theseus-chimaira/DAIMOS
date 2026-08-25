; dtc_io.s -- compact asynchronous PDP-6 DECtape forward-transfer driver.
;
; The shape follows the useful low-level ITS-138 UTAPE ideas: one selected
; operation owns the data channel, state is packed into a few words, and PI5
; advances the transfer.  Positioning/filesystem policy deliberately stays
; outside this resident hot path.
;
; dtc_read_words ABI:
;   AC1 = unit (0..7), AC2 = destination, AC3 = word count
;   returns 0, -1 bad argument, -3 busy, or -5 controller/data error.

        .text
        .globl dtc_pi_handler
        .globl dtc_read_words
        .globl pdp10_pi_handler_return

; dtc_state: 0 idle, -1 transfer active, +1 completed, +2 failed.
dtc_pi_handler:
        coni 0200,1
        trne 1,001000
        jrst dtc_pi_word
        coni 0214,1
        trne 1,000034
        jrst dtc_pi_error
        jrst pdp10_pi_handler_return

dtc_pi_word:
        skipge dtc_state
        jrst dtc_pi_take
        jrst pdp10_pi_handler_return

dtc_pi_take:
        datai 0200,1
        movem 1,@dtc_ptr
        aos dtc_ptr
        sosle dtc_count
        jrst pdp10_pi_handler_return
        setom 1
        movem 1,dtc_state
        aos dtc_state
        cono 0210,0
        cono 0200,0
        jrst pdp10_pi_handler_return

dtc_pi_error:
        movei 1,2
        movem 1,dtc_state
        cono 0210,0
        cono 0200,0
        jrst pdp10_pi_handler_return

dtc_read_words:
        skipn dtc_state
        jrst dtc_read_idle
        hrroi 1,0777775
        popj 017,
dtc_read_idle:
        caile 1,7
        jrst dtc_read_arg
        jumpg 3,dtc_read_start
dtc_read_arg:
        seto 1,
        popj 017,
dtc_read_start:
        movem 2,dtc_ptr
        movem 3,dtc_count
        setom dtc_state
        lsh 1,3
        iori 1,0220305
        movei 2,004045
        cono 0200,0(2)
        cono 0210,0(1)
dtc_read_wait:
        move 1,dtc_state
        jumpl 1,dtc_read_wait
        caie 1,2
        jrst dtc_read_ok
        setzm dtc_state
        hrroi 1,0777773
        popj 017,
dtc_read_ok:
        setzm dtc_state
        movei 1,0
        popj 017,

        .bss
dtc_state: .block 1
dtc_ptr:   .block 1
dtc_count: .block 1
