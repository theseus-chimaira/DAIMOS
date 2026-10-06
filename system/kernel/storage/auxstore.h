#ifndef DAIMON_AUXSTORE_H
#define DAIMON_AUXSTORE_H

#include "kcore.h"

/* Hardware-independent raw backing-store layout.  Block zero contains this
 * descriptor; the three exported regions are absolute physical block ranges
 * on the selected unit and never depend on D6FS metadata. */
#define AUXSTORE_MAGIC          0416570636421UL /* SIXBIT /AUXST1/ */
#define AUXSTORE_VERSION        1UL
#define AUXSTORE_DESC_MAGIC     0U
#define AUXSTORE_DESC_VERSION   1U
#define AUXSTORE_DESC_BACKSTORE 2U
#define AUXSTORE_DESC_LOGSTORE  3U
#define AUXSTORE_DESC_CACHE     4U
/* Optional co-resident, non-root D6FS partition metadata.  The range is
 * physical BASE,,BLOCKS.  SUPER stores the two logical superblock numbers
 * within that D6FS partition.  AUXSTORE itself does not mount the filesystem. */
#define AUXSTORE_DESC_D6FS      5U
#define AUXSTORE_DESC_D6SUPER   6U

#define AUXSTORE_KIND_NONE      0U
#define AUXSTORE_KIND_DSK       1U
#define AUXSTORE_KIND_DRM       2U

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
