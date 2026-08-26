; io7_io.s -- compact shared PI7 dispatcher for paper/card peripherals.
;
; One generic PI handler services PTR, PTP, CR, and CP.  Device tests and the
; small completion actions are inlined here, avoiding four PUSHJ/POPJ pairs.
; CR/CP each use one AOBJN-style pointer/count word instead of state+pointer.

        .text
        .globl devicefs_v1_io_in
        .globl devicefs_v1_io_out
        .globl io7_pi_handler
        .globl ptr_state
        .globl ptp_state
        .globl cr_iowd
        .globl cp_iowd
        .globl io7_ret_timeout
        .globl io7_ret_e4
        .globl pdp10_pi_handler_return
        .globl pdp10_ret_arg_v34
        .globl pdp10_ret_busy_v34

io7_pi_handler:
        ; Paper-tape reader DONE.
        conso 0104,0010
        jrst io7_ptp
        skipn ptr_state
        jrst io7_ptr_prefetch
        datai 0104,ptr_state
        aos devicefs_v1_io_in+2
        aos ptr_state
        cono 0104,0
        jrst io7_ptp
io7_ptr_prefetch:
        ; Preserve one unsolicited prefetched character with PI disabled.
        cono 0104,0010

io7_ptp:
        conso 0100,0010
        jrst io7_cr
        setzm ptp_state
        cono 0100,0007

io7_cr:
        coni 0150,1
        trne 1,0400
        jrst io7_cr_done
        trne 1,0010
        jrst io7_cr_data
        trnn 1,0020
        jrst io7_cp
io7_cr_done:
        cono 0150,0027
        setzm cr_iowd
        jrst io7_cp
io7_cr_data:
        move 1,cr_iowd
        aobjn 1,io7_cr_more
        setom cr_iowd
        jrst io7_cr_xfer
io7_cr_more:
        movem 1,cr_iowd
io7_cr_xfer:
        datai 0150,(1)
        aos devicefs_v1_io_in+4

io7_cp:
        coni 0110,1
        trne 1,05000
        jrst io7_cp_done
        trne 1,0010
        jrst io7_cp_data
        trnn 1,0100
        jrst pdp10_pi_handler_return
io7_cp_done:
        cono 0110,0107
        setzm cp_iowd
        jrst pdp10_pi_handler_return
io7_cp_data:
        move 1,cp_iowd
        aobjn 1,io7_cp_more
        move 1,(1)
        andi 1,07777
        datao 0110,1
        aos devicefs_v1_io_out+5
        setom cp_iowd
        cono 0110,010207
        jrst pdp10_pi_handler_return
io7_cp_more:
        movem 1,cp_iowd
        move 1,(1)
        andi 1,07777
        datao 0110,1
        aos devicefs_v1_io_out+5
        jrst pdp10_pi_handler_return

; Shared slow-path returns for the four PI7 peripherals.  Keep these out of
; successful I/O paths; only errors/timeouts pay the extra JRST.
io7_ret_timeout:
        hrroi 1,0777776
        popj 017,
io7_ret_e4:
        hrroi 1,0777774
        popj 017,
