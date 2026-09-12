#ifndef DAIMON_MM_INTERNAL_H
#define DAIMON_MM_INTERNAL_H

#include "mm.h"

/* Shared descriptor primitives used by permanent MM and disposable KINIT. */
int mm_extent_insert(int slot, const struct mm_extent *extent);
void mm_extent_coalesce(int slot);
int mm_alloc_aligned_noreclaim(kword_t words, kword_t alignment,
    unsigned int type, unsigned int owner, unsigned int preference,
    kword_t *basep);

#endif
