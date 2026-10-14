/**
 * @file mm_internal.h
 * @brief Shared internal MM primitives used by permanent KCORE and KINIT.
 */
#ifndef DAIMON_MM_INTERNAL_H
#define DAIMON_MM_INTERNAL_H

#include "mm.h"

/** Insert one allocated descriptor into the sorted permanent extent table. */
int mm_extent_insert(int slot, const struct mm_extent *extent);
/** Convert an edge allocation made during boot into permanently unmanaged core. */
int mm_boot_reserve(kword_t base, unsigned int type, unsigned int owner);
/* Validate and place permanent MM backing immediately above KINIT stack. */
int mm_boot_table_region(kword_t core_words, kword_t image_end,
    kword_t stack_words, unsigned int capacity, kword_t *basep,
    kword_t *high_basep);
/** Allocate without cache reclaim, compaction, or process swapping recursion. */
int mm_alloc_aligned_noreclaim(kword_t words, kword_t alignment,
    unsigned int type, unsigned int owner, unsigned int preference,
    kword_t *basep);

#endif
