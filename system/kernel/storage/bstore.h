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
/* Extend an allocated contiguous run into free immediately following blocks.
 * Returns 0 on success, -1 if bounds, occupancy or state prevents growth. */
int backstore_extend(kword_t first, kword_t old_blocks,
    kword_t new_blocks);
/** Read-only check for a fully allocated backing range, -1 if unavailable. */
int backstore_verify_allocated_v795(kword_t first, kword_t blocks);
void backstore_free(kword_t first, kword_t blocks);
int backstore_read(kword_t first, kword_t blocks, kword_t *buf);
int backstore_write(kword_t first, kword_t blocks, const kword_t *buf);

#endif
