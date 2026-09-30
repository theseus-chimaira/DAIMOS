#ifndef DAIMON_SWAP_STORE_H
#define DAIMON_SWAP_STORE_H

#include "kcore.h"

/* One shared raw backing pool.  Callers retain their own mapping metadata;
 * the allocator therefore needs only one bit per block. */
extern kword_t *swap_store_bitmap;
extern kword_t swap_store_blocks;
extern kword_t swap_store_blocks_used;
extern unsigned int swap_store_enabled;

unsigned int swap_store_bitmap_words(kword_t blocks);
void swap_store_init(kword_t *bitmap, kword_t blocks);
int swap_store_alloc(kword_t blocks, kword_t *firstp);
int swap_store_alloc_memfs(kword_t blocks, kword_t *firstp);
void swap_store_free(kword_t first, kword_t blocks);

#endif
