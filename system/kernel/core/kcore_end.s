/**
 * @file kcore_end.s
 * @brief Linker boundary markers for initialized and total resident KCORE.
 *
 * This file must remain the final object in the fixed KCORE link.  It emits no
 * storage of its own: the labels capture the location counter after all
 * initialized KCORE sections and after all KCORE BSS respectively.
 *
 * KINIT copies the embedded KCORE image from KCORE_BASE through
 * __kcore_low_init_end, then clears memory from that point through
 * __kcore_low_end.  MRES installation starts exactly at __kcore_low_end, so
 * neither symbol may acquire padding or storage without intentionally changing
 * the permanent-memory layout.  Build-time resident-size reporting also uses
 * __kcore_low_end as the fixed-core upper boundary.
 */

        .data

/**
 * @brief First word after KCORE initialized data.
 *
 * The symbol has no associated word.  Its address is the exclusive upper
 * bound of the initialized image copied by KINIT and the beginning of the BSS
 * range that KINIT must zero.
 */
        .globl __kcore_low_init_end
__kcore_low_init_end:

        .bss

/**
 * @brief First word after all fixed KCORE storage, including BSS.
 *
 * This zero-size label is the exclusive upper bound of fixed resident KCORE
 * and the first address available for MRES package installation.
 */
        .globl __kcore_low_end
__kcore_low_end:
