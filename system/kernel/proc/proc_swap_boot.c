/**
 * @file proc_swap_boot.c
 * @brief Transient boot allocation of the compact process swap-record table.
 *
 * The table size follows the runtime process-slot count selected after memory
 * discovery. Its storage survives KINIT in kernel-dynamic managed core; this
 * compilation unit itself is reclaimable boot code.
 */
#include "proc_swap.h"
#include "fs_mres.h"
#include "mm.h"

#define PROC_SWAP_MM_OWNER 4U

/** Allocate, zero, and publish the per-slot swap-record table. */
int
proc_swap_boot_init(unsigned int slots)
{
        kword_t base;
        kword_t words;
        kword_t *wp;

        if (proc_swap_records != 0 || slots == 0U || slots > PROC_MAX_SLOTS)
                return -1;
        words = (kword_t)slots * (kword_t)PROC_SWAP_RECORD_WORDS;
        if (mm_alloc(words, MM_TYPE_KERNEL_DYNAMIC, PROC_SWAP_MM_OWNER,
            MM_ALLOC_LOW, &base) != MM_OK)
                return -1;
        wp = (kword_t *)(unsigned long)base;
        fs_zero_words(wp, (unsigned int)words);
        proc_swap_records = (struct proc_swap_record *)(unsigned long)base;
        proc_swap_blocks_used = 0UL;
        return 0;
}
