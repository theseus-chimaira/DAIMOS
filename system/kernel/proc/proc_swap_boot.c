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
#define SWAP_PERSIST_MAGIC   055464663UL
#define SWAP_PERSIST_VERSION 1UL

static int
proc_swap_reserve_persistent(kword_t blocks)
{
        kword_t *h;
        kword_t b;
        kword_t mask;
        unsigned int bit;
        unsigned int word;
        kword_t last;

        if (blocks == 0UL)
                return 0;
        h = fs_block_workspace;
        last = blocks - 1UL;
        if (blockset_runtime_reg_call(BLOCKSET_MRES_OP_TAIL_READ,
            last, 1UL, (kword_t)(unsigned long)h) != 0UL)
                return 0;
        if (h[0] != SWAP_PERSIST_MAGIC || h[1] != SWAP_PERSIST_VERSION)
                return 0;
        if (h[2] >= blocks || h[3] == 0UL || h[3] > blocks - h[2] ||
            h[2] + h[3] != blocks)
                return 0;
        for (b = h[2]; b < blocks; ++b) {
                word = (unsigned int)(b / 36UL);
                bit = (unsigned int)(b % 36UL);
                mask = (kword_t)1U << bit;
                swap_store_bitmap[word] |= mask;
        }
        swap_store_blocks_used += h[3];
        return 0;
}

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
        if (proc_swap_reserve_persistent(blocks) != 0)
                return -1;
        proc_swap_blocks_used = 0UL;
        return 0;
}
