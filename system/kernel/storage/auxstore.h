#ifndef DAIMON_AUXSTORE_H
#define DAIMON_AUXSTORE_H

#include "kcore.h"

/* Hardware-independent raw backing-store layout.  Block zero contains this
 * descriptor.  V1 describes ranges on one raw unit; V2 describes one
 * four-member interleaved logical DRM set, independent of D6FS allocations.
 * In both cases the ranges must be outside any filesystem data area. */
#define AUXSTORE_MAGIC          0416570636421UL /* SIXBIT /AUXST1/ */
#define AUXSTORE_VERSION        1UL
#define AUXSTORE_SET_VERSION    2UL
#define AUXSTORE_DESC_MAGIC     0U
#define AUXSTORE_DESC_VERSION   1U
#define AUXSTORE_DESC_BACKSTORE 2U
#define AUXSTORE_DESC_LOGSTORE  3U
#define AUXSTORE_DESC_CACHE     4U

#define AUXSTORE_KIND_NONE      0U
#define AUXSTORE_KIND_DSK       1U
#define AUXSTORE_KIND_DRM       2U
#define AUXSTORE_KIND_DRMSET    3U

extern unsigned int auxstore_kind;
extern unsigned int auxstore_unit;
extern kword_t auxstore_backstore_start;
extern kword_t auxstore_backstore_blocks;
extern kword_t auxstore_logstore_start;
extern kword_t auxstore_logstore_blocks;
extern kword_t auxstore_cache_start;
extern kword_t auxstore_cache_blocks;

int auxstore_boot_discover(void);
void auxstore_post_minits(void);
int auxstore_boot_log_read(kword_t first, kword_t *buf);
int auxstore_boot_log_write(kword_t first, const kword_t *buf);

#endif
