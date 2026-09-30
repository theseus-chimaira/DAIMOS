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
#include "blockset_mres.h"
#include "swap_store.h"

#define PROC_SWAP_MM_OWNER 4U

/** Allocate, zero, and publish the per-slot swap-record table. */
int
proc_swap_boot_init(unsigned int slots)
{
        kword_t base;
        kword_t blocks;
        kword_t words;
        kword_t *wp;
        unsigned int bitmap_words;

        if (proc_swap_records != 0 || slots == 0U || slots > PROC_MAX_SLOTS)
                return -1;
        blocks = blockset_runtime_reg_call(BLOCKSET_MRES_OP_TAIL_BLOCKS,
            0UL, 0UL, 0UL);
        bitmap_words = swap_store_bitmap_words(blocks);
        words = (kword_t)slots * (kword_t)PROC_SWAP_RECORD_WORDS +
            (kword_t)bitmap_words;
        if (mm_alloc(words, MM_TYPE_KERNEL_DYNAMIC, PROC_SWAP_MM_OWNER,
            MM_ALLOC_LOW, &base) != MM_OK)
                return -1;
        wp = (kword_t *)(unsigned long)base;
        fs_zero_words(wp, (unsigned int)words);
        proc_swap_records = (struct proc_swap_record *)(unsigned long)base;
        swap_store_init(wp + slots * PROC_SWAP_RECORD_WORDS, blocks);
        proc_swap_blocks_used = 0UL;
        return 0;
}
