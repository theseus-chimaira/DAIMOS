; kfmt_pdp10.s -- compact resident unsigned 36-bit decimal formatter.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1

kfmt_u36_pow10:
        .long   10000000000
        .long   1000000000
        .long   100000000
        .long   10000000
        .long   1000000
        .long   100000
        .long   10000
        .long   1000
        .long   100
        .long   10
        .long   1

        .globl  kfmt_u36_decimal_readchar
kfmt_u36_decimal_readchar:
        jumpe   3,kfmt_u36_bad
        move    5,1
        tlc     5,0400000
        movei   4,0
kfmt_u36_first:
        move    6,kfmt_u36_pow10(4)
        tlc     6,0400000
        caml    5,6
        jrst    kfmt_u36_found
        addi    4,1
        caie    4,012
        jrst    kfmt_u36_first
kfmt_u36_found:
        movei   6,013
        sub     6,4
        jumpl   2,kfmt_u36_none
        camge   2,6
        jrst    kfmt_u36_digit
        came    2,6
        jrst    kfmt_u36_lf
        movei   6,015
        jrst    kfmt_u36_store
kfmt_u36_lf:
        addi    6,1
        came    2,6
        jrst    kfmt_u36_none
        movei   6,012
        jrst    kfmt_u36_store
kfmt_u36_digit:
        add     4,2
        move    7,1
        setz    6,
        jumpge  7,kfmt_u36_div_ready
        tlz     7,0400000
        movei   6,1
kfmt_u36_div_ready:
        jumpe   4,kfmt_u36_top_digit
        move    5,4
        subi    5,1
        div     6,kfmt_u36_pow10(5)
        setz    6,
        div     6,kfmt_u36_pow10(4)
        jrst    kfmt_u36_digit_ready
kfmt_u36_top_digit:
        div     6,kfmt_u36_pow10(4)
kfmt_u36_digit_ready:
        addi    6,060
kfmt_u36_store:
        movem   6,(3)
        movei   1,1
        popj    17,
kfmt_u36_none:
        jrst    pdp10_ret_zero
kfmt_u36_bad:
        jrst    pdp10_ret_neg1
