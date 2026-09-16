#ifndef DAIMON_DRM236_H
#define DAIMON_DRM236_H

#include "kcore.h"

#define DRM236_UNITS             4U
#define DRM236_WORDS_PER_BLOCK   0200U
#define DRM236_BLOCKS_PER_UNIT   020000U
#define DRM236_GROUP_WORDS       020U
#define DRM236_BLOCK_GROUPS      010U

int drm236_read_block(unsigned int unit, kword_t block, kword_t *buf);
int drm236_write_block(unsigned int unit, kword_t block,
    const kword_t *buf);

#endif
