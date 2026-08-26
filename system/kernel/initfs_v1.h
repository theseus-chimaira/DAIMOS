#ifndef DAIMON_INITFS_V1_H
#define DAIMON_INITFS_V1_H

#include "memfs_v1.h"

#define INITFS_V1_MAGIC         0051646060UL
#define INITFS_V1_VERSION       1U
#define INITFS_V1_HDR_WORDS     8U
#define INITFS_V1_ENT_WORDS     8U

#define INITFS_V1_REG           1U
#define INITFS_V1_DIR           4U

int initfs_v1_mount(struct memfs_v1 *fs, struct memfs_v1_node *nodes,
    unsigned int node_count, const kword_t *image, unsigned int image_words);

#endif
