; kfmt_pdp10.s -- compact resident unsigned 18-bit decimal formatter.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1

; PROCFS formats only process slots and process sizes.  Both values fit in
; an 18-bit PDP-10 halfword, so a six-decade table is sufficient here.
kfmt_u18_pow10:
        .long   0303240                 ; 100000 decimal
        .long   023420                  ; 10000 decimal
        .long   01750                   ; 1000 decimal
        .long   0144                    ; 100 decimal
        .long   012                     ; 10 decimal
        .long   1

        .globl  kfmt_u18_decimal_readchar
kfmt_u18_decimal_readchar:
        jumpe   3,pdp10_ret_neg1
        movei   4,0
kfmt_u18_first:
        move    6,kfmt_u18_pow10(4)
        caml    1,6
        jrst    kfmt_u18_found
        addi    4,1
        caie    4,5
        jrst    kfmt_u18_first
kfmt_u18_found:
        movei   6,6
        sub     6,4
        jumpl   2,pdp10_ret_zero
        camge   2,6
        jrst    kfmt_u18_digit
        came    2,6
        jrst    kfmt_u18_lf
        movei   6,015
        jrst    kfmt_u18_store
kfmt_u18_lf:
        addi    6,1
        came    2,6
        jrst    pdp10_ret_zero
        movei   6,012
        jrst    kfmt_u18_store
kfmt_u18_digit:
        add     4,2
        move    7,1
        setz    6,
        jumpe   4,kfmt_u18_top_digit
        move    5,4
        subi    5,1
        div     6,kfmt_u18_pow10(5)
        setz    6,
        div     6,kfmt_u18_pow10(4)
        jrst    kfmt_u18_digit_ready
kfmt_u18_top_digit:
        div     6,kfmt_u18_pow10(4)
kfmt_u18_digit_ready:
        addi    6,060
kfmt_u18_store:
        movem   6,(3)
        movei   1,1
        popj    17,
