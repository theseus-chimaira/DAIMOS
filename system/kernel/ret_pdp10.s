; ret_pdp10.s -- shared tiny PDP-10 return tails.
        .text
        .globl  pdp10_ret_zero_v1
        .globl  pdp10_ret_neg1_v1
        .globl  pdp10_ret_ok_v34
        .globl  pdp10_ret_arg_v34
        .globl  pdp10_ret_busy_v34

pdp10_ret_zero_v1:
pdp10_ret_ok_v34:
        movei   1,0
        popj    017,

pdp10_ret_neg1_v1:
pdp10_ret_arg_v34:
        seto    1,
        popj    017,

pdp10_ret_busy_v34:
        hrroi   1,0777775
        popj    017,
