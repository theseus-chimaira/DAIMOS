/* Staged KINIT layout for one MM allocation holding mount-indexed state. */
#ifndef DAIMON_VFS_MOUNT_LAYOUT_V3_H
#define DAIMON_VFS_MOUNT_LAYOUT_V3_H

#include "kcore.h"

#define VFS_LAYOUT_V3_MAX_MOUNTS 16U

/* The common slab is intentionally contiguous. Offset units are PDP-6
 * 36-bit words, not host bytes. Never store host pointers in the layout. */
struct vfs_mount_layout_v3 {
        unsigned int capacity;
        unsigned int target_off;
        unsigned int root_off;
        unsigned int reader_off;
        unsigned int media_off;
        unsigned int shape_off;
        unsigned int extent_off;
        unsigned int total_words;
};

int vfs_mount_layout_v3(unsigned int capacity,
    struct vfs_mount_layout_v3 *layout);

#endif
