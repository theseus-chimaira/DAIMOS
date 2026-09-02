; ret_pdp10.s -- shared tiny PDP-10 return tails.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1
        .globl  pdp10_ret_ok
        .globl  pdp10_ret_arg
        .globl  pdp10_ret_busy

pdp10_ret_zero:
pdp10_ret_ok:
        movei   1,0
        popj    017,

pdp10_ret_neg1:
pdp10_ret_arg:
        seto    1,
        popj    017,

pdp10_ret_busy:
        hrroi   1,0777775
        popj    017,
