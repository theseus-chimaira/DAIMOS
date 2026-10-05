#ifndef DAIMON_BCACHE_H
#define DAIMON_BCACHE_H

#include "kcore.h"

/* One 36-bit cache tag is SOURCE12,,BLOCK24.  Source classes are disjoint;
 * reuse of a source id is safe only after bcache_reclaim() invalidates the
 * two-entry clean cache. */
#define BCACHE_SOURCE_D6FS       01000U
#define BCACHE_SOURCE_DTC        02000U
#define BCACHE_SOURCE_MASK       07777U
#define BCACHE_BLOCK_MASK        077777777UL
#define BCACHE_SOURCE_SHIFT      24U

int bcache_fetch(kword_t key, kword_t *buf);
void bcache_store(kword_t key, const kword_t *buf);
int bcache_reclaim(kword_t words);
void bcache_workspace_invalidate(void);

#endif
