#ifndef MM_PBM_COMMIT_V1979_H
#define MM_PBM_COMMIT_V1979_H
#include "mm.h"
/* PI-excluded leaf. Caller owns the verified original full image and
 * candidate. Fourth argument packs prefix length,,encoded length. */
int mm_pure_bitmap_commit_v1979(kword_t base, unsigned int slot,
    kword_t words, kword_t lengths);
/* Optional placement leaf: exact lower disjoint 0200-aligned target,
 * or original base. The caller retains PI/lifecycle exclusion and the
 * original verified candidate; no descriptor or scratch allocation. */
int mm_pure_bitmap_place_v1985(kword_t base, unsigned int slot,
    kword_t words, kword_t lengths, kword_t target);
/* Read-only internal MM search. Caller validates table, lengths and
 * alignment and holds PI; false return writes aggregate free words. */
int mm_find_fit(kword_t words, kword_t alignment,
    unsigned int preference, kword_t *basep);
#endif
