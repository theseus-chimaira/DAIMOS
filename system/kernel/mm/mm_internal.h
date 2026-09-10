#ifndef DAIMON_MM_INTERNAL_H
#define DAIMON_MM_INTERNAL_H

#include "mm.h"

/* Shared descriptor primitives used by permanent MM and disposable KINIT. */
int mm_extent_insert(unsigned int slot, const struct mm_extent *extent);
void mm_extent_coalesce(unsigned int slot);

#endif
