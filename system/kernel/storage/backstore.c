/**
 * @file backstore.c
 * @brief Compact allocator and I/O boundary for authoritative raw backing.
 *
 * Process swap and MEMFS share one contiguous-run allocator.  AUXSTORE may
 * supply a direct DRM/DSK region; otherwise I/O falls back to the legacy
 * D6FS/BLOCKSET raw tail.  Callers never need to know which backend is active.
 */
#include "bstore.h"
#include "blockset_mres.h"
#include "fs_mres.h"
#define BACKSTORE_BITMAP_BITS 36U

kword_t *backstore_bitmap;
kword_t backstore_blocks;
kword_t backstore_blocks_used;
unsigned int backstore_enabled;

unsigned int
backstore_bitmap_words(kword_t blocks)
{
        return (unsigned int)((blocks + BACKSTORE_BITMAP_BITS - 1U) /
            BACKSTORE_BITMAP_BITS);
}

void
backstore_init(kword_t *bitmap, kword_t blocks)
{
        backstore_bitmap = bitmap;
        backstore_blocks = blocks;
        backstore_blocks_used = 0UL;
        backstore_enabled = 0U;
}

static int
backstore_test(kword_t block)
{
        unsigned int bit;
        unsigned int word;

        word = (unsigned int)(block / BACKSTORE_BITMAP_BITS);
        bit = (unsigned int)(block % BACKSTORE_BITMAP_BITS);
        return (backstore_bitmap[word] & ((kword_t)1U << bit)) != 0UL;
}

static void
backstore_mark(kword_t first, kword_t blocks, int used)
{
        kword_t block;

        for (block = first; block < first + blocks; ++block) {
                unsigned int bit;
                unsigned int word;
                kword_t mask;

                word = (unsigned int)(block / BACKSTORE_BITMAP_BITS);
                bit = (unsigned int)(block % BACKSTORE_BITMAP_BITS);
                mask = (kword_t)1U << bit;
                if (used)
                        backstore_bitmap[word] |= mask;
                else
                        backstore_bitmap[word] &= ~mask;
        }
}

int
backstore_alloc(kword_t blocks, kword_t reserve, kword_t *firstp)
{
        kword_t first;
        kword_t free_blocks;
        kword_t i;

        if (!backstore_enabled || blocks == 0UL || firstp == 0 ||
            backstore_bitmap == 0 || blocks > backstore_blocks)
                return -1;
        free_blocks = backstore_blocks - backstore_blocks_used;
        if (free_blocks <= reserve || blocks > free_blocks - reserve)
                return -1;
        for (first = 0UL; first <= backstore_blocks - blocks; ++first) {
                for (i = 0UL; i < blocks; ++i) {
                        if (backstore_test(first + i))
                                break;
                }
                if (i == blocks) {
                        backstore_mark(first, blocks, 1);
                        backstore_blocks_used += blocks;
                        *firstp = first;
                        return 0;
                }
                first += i;
        }
        return -1;
}

void
backstore_free(kword_t first, kword_t blocks)
{
        if (blocks == 0UL || first >= backstore_blocks ||
            blocks > backstore_blocks - first)
                return;
        backstore_mark(first, blocks, 0);
        backstore_blocks_used -= blocks;
}
