/**
 * @file swap_store.c
 * @brief Compact shared allocator for raw process/MEMFS swap backing.
 *
 * D6FS exposes one raw swap-tail reservation.  This allocator owns allocation
 * within that reservation; process swap and demand-backed MEMFS keep their
 * mapping records separately.  One bit per block is smaller than another
 * extent table and permits arbitrary contiguous runs to be returned cheaply.
 */
#include "swap_store.h"

#define SWAP_BITMAP_BITS 36U
#define SWAP_PROCESS_RESERVE 0200UL

kword_t *swap_store_bitmap;
kword_t swap_store_blocks;
kword_t swap_store_blocks_used;
unsigned int swap_store_enabled;

unsigned int
swap_store_bitmap_words(kword_t blocks)
{
        return (unsigned int)((blocks + SWAP_BITMAP_BITS - 1U) /
            SWAP_BITMAP_BITS);
}

void
swap_store_init(kword_t *bitmap, kword_t blocks)
{
        swap_store_bitmap = bitmap;
        swap_store_blocks = blocks;
        swap_store_blocks_used = 0UL;
        swap_store_enabled = 0U;
}

static int
swap_store_test(kword_t block)
{
        unsigned int bit;
        unsigned int word;

        word = (unsigned int)(block / SWAP_BITMAP_BITS);
        bit = (unsigned int)(block % SWAP_BITMAP_BITS);
        return (swap_store_bitmap[word] & ((kword_t)1U << bit)) != 0UL;
}

static void
swap_store_mark(kword_t first, kword_t blocks, int used)
{
        kword_t block;

        for (block = first; block < first + blocks; ++block) {
                unsigned int bit;
                unsigned int word;
                kword_t mask;

                word = (unsigned int)(block / SWAP_BITMAP_BITS);
                bit = (unsigned int)(block % SWAP_BITMAP_BITS);
                mask = (kword_t)1U << bit;
                if (used)
                        swap_store_bitmap[word] |= mask;
                else
                        swap_store_bitmap[word] &= ~mask;
        }
}

int
swap_store_alloc(kword_t blocks, kword_t *firstp)
{
        kword_t first;
        kword_t i;

        if (!swap_store_enabled || blocks == 0UL || firstp == 0 ||
            swap_store_bitmap == 0 ||
            blocks > swap_store_blocks - swap_store_blocks_used)
                return -1;
        for (first = 0UL; first <= swap_store_blocks - blocks; ++first) {
                for (i = 0UL; i < blocks; ++i) {
                        if (swap_store_test(first + i))
                                break;
                }
                if (i == blocks) {
                        swap_store_mark(first, blocks, 1);
                        swap_store_blocks_used += blocks;
                        *firstp = first;
                        return 0;
                }
                first += i;
        }
        return -1;
}

/** Allocate MEMFS backing while retaining the original process-swap capacity. */
int
swap_store_alloc_memfs(kword_t blocks, kword_t *firstp)
{
        kword_t free_blocks;
        kword_t reserve;

        reserve = swap_store_blocks < SWAP_PROCESS_RESERVE ?
            swap_store_blocks : SWAP_PROCESS_RESERVE;
        free_blocks = swap_store_blocks - swap_store_blocks_used;
        if (free_blocks <= reserve || blocks > free_blocks - reserve)
                return -1;
        return swap_store_alloc(blocks, firstp);
}

void
swap_store_free(kword_t first, kword_t blocks)
{
        if (blocks == 0UL || first >= swap_store_blocks ||
            blocks > swap_store_blocks - first)
                return;
        swap_store_mark(first, blocks, 0);
        swap_store_blocks_used -= blocks;
}
