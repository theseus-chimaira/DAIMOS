/**
 * @file ret.s
 * @brief Shared constant-return tails for resident PDP-6/PDP-10 assembly.
 *
 * These tiny entry points centralize common C-ABI integer returns so resident
 * assembly callers can jump to two-word tails instead of repeating immediate
 * loads and POPJ instructions.  Every instruction used here exists on the
 * PDP-6, so the implementation is architecture-family generic rather than a
 * later-PDP-10 specialization.
 *
 * All entries return through AC17 and place the signed result in AC1.  They do
 * not modify any other accumulator.  Multiple public names intentionally
 * alias the same tail where kernel status conventions share a numeric value.
 */
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_one
        .globl  pdp10_ret_neg1
        .globl  pdp10_ret_ok
        .globl  pdp10_ret_arg
        .globl  pdp10_ret_busy
        .globl  pdp10_ret_neg2
        .globl  pdp10_ret_neg3
        .globl  pdp10_ret_neg4
        .globl  pdp10_ret_neg5

/** @brief Return 0 in AC1; also the generic success (`ok`) tail. */
pdp10_ret_zero:
pdp10_ret_ok:
        movei   1,0
        popj    017,


/** @brief Return 1 in AC1. */
pdp10_ret_one:
        movei   1,1
        popj    017,

/** @brief Return -1 in AC1; also the generic invalid-argument tail. */
pdp10_ret_neg1:
pdp10_ret_arg:
        seto    1,
        popj    017,

/** @brief Return -3 in AC1; also the generic busy/device-I/O tail. */
pdp10_ret_busy:
pdp10_ret_neg3:
        hrroi   1,0777775
        popj    017,

/** @brief Return -2 in AC1. */
pdp10_ret_neg2:
        hrroi   1,0777776
        popj    017,

/** @brief Return -4 in AC1. */
pdp10_ret_neg4:
        hrroi   1,0777774
        popj    017,

/** @brief Return -5 in AC1. */
pdp10_ret_neg5:
        hrroi   1,0777773
        popj    017,
