; proc_pdp10.s -- process-table placement for PDP-6/PDP-10.
        .equ    proc_v1_table,0601142
        .globl  proc_v1_table
        .globl  pdp10_ret_zero_v1

        .data
        .globl  proc_v1_comm_words
proc_v1_comm_words:
        .word   0636741606045          ; SIXBIT /SWAPPE/
        .word   0515651640000          ; SIXBIT /INIT  /
