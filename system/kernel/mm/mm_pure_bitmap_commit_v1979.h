#ifndef MM_PBM_COMMIT_V1979_H
#define MM_PBM_COMMIT_V1979_H
#include "mm.h"
/* PI-excluded leaf. Caller owns the verified original full image and
 * candidate. Fourth argument packs prefix length,,encoded length. */
int mm_pure_bitmap_commit_v1979(kword_t base, unsigned int slot,
    kword_t words, kword_t lengths);
#endif
