#ifndef DAIMON_BSTORE_H
#define DAIMON_BSTORE_H

#include "kcore.h"
#include "storage.h"

/* One raw authoritative backing pool shared by process swap and MEMFS.
 * Allocation metadata is deliberately only one bit per 0200-word block. */
extern kword_t *backstore_bitmap;
extern kword_t backstore_blocks;
extern kword_t backstore_blocks_used;
extern unsigned int backstore_enabled;

unsigned int backstore_bitmap_words(kword_t blocks);
void backstore_init(kword_t *bitmap, kword_t blocks);
int backstore_alloc(kword_t blocks, kword_t reserve, kword_t *firstp);
void backstore_free(kword_t first, kword_t blocks);
int backstore_read(kword_t first, kword_t blocks, kword_t *buf);
int backstore_write(kword_t first, kword_t blocks, const kword_t *buf);

#endif
