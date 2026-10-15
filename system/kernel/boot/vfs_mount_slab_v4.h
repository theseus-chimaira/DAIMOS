/* Transient KINIT MM slab provisioning for boot-sized VFS arrays. */
#ifndef DAIMON_VFS_MOUNT_SLAB_V4_H
#define DAIMON_VFS_MOUNT_SLAB_V4_H

#include "vfs_mount_layout_v3.h"

/* Allocate exactly one owner-identified extent, zero it, and return the
 * absolute word bases used by native assembly. No allocations during I/O. */
struct vfs_mount_slab_v4 {
        kword_t base;
        kword_t target;
        kword_t root;
        kword_t reader;
        kword_t media;
        kword_t shape;
        kword_t extent;
        unsigned int capacity;
        unsigned int words;
};

int vfs_mount_slab_v4_prepare(unsigned int capacity,
    struct vfs_mount_slab_v4 *slab);

#endif
