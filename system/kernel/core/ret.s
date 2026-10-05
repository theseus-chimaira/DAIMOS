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
        .globl  kret_zero
        .globl  kret_one
        .globl  kret_neg1
        .globl  kret_ok
        .globl  kret_arg
        .globl  kret_busy
        .globl  kret_neg2
        .globl  kret_neg3
        .globl  kret_neg4
        .globl  kret_neg5
        .globl  kconst_1_1
        .globl  kconst_2_2
        .globl  kconst_3_3
        .globl  kconst_4_4
        .globl  kconst_5_5
        .globl  kconst_6_6
        .globl  kconst_7_7

/** @brief Return 0 in AC1; also the generic success (`ok`) tail. */
kret_zero:
kret_ok:
        movei   1,0
        popj    017,


/** @brief Return 1 in AC1. */
kret_one:
        movei   1,1
        popj    017,

/** @brief Return -1 in AC1; also the generic invalid-argument tail. */
kret_neg1:
kret_arg:
        seto    1,
        popj    017,

/** @brief Return -3 in AC1; also the generic busy/device-I/O tail. */
kret_busy:
kret_neg3:
        hrroi   1,0777775
        popj    017,

/** @brief Return -2 in AC1. */
kret_neg2:
        hrroi   1,0777776
        popj    017,

/** @brief Return -4 in AC1. */
kret_neg4:
        hrroi   1,0777774
        popj    017,

/** @brief Return -5 in AC1. */
kret_neg5:
        hrroi   1,0777773
        popj    017,

/*
 * Shared stack-adjust literals.  DAS coalesces repeated literals within one
 * object, but independently linked KCORE/MRES objects otherwise each retain
 * their own copy.  These immutable words are therefore cheaper as one KCORE
 * constant set referenced by every resident object.
 */
kconst_1_1: .word 1,,1
kconst_2_2: .word 2,,2
kconst_3_3: .word 3,,3
kconst_4_4: .word 4,,4
kconst_5_5: .word 5,,5
kconst_6_6: .word 6,,6
kconst_7_7: .word 7,,7
