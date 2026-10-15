/* Transient mount slab planner: six indexed word arrays, no allocations. */
#include "vfs_mount_layout_v3.h"

int
vfs_mount_layout_v3(unsigned int capacity,
    struct vfs_mount_layout_v3 *layout)
{
        if (layout == 0 || capacity == 0U ||
            capacity > VFS_LAYOUT_V3_MAX_MOUNTS)
                return -1;

        layout->capacity = capacity;
        layout->target_off = 0U;
        layout->root_off = capacity;
        layout->reader_off = 2U * capacity;
        layout->media_off = 3U * capacity;
        layout->shape_off = 4U * capacity;
        layout->extent_off = 5U * capacity;
        layout->total_words = 6U * capacity;
        return 0;
}
