/* Transient, fail-closed provisioning of one KINIT-selected MM extent. */
#include "vfs_mount_slab_v4.h"
#include "mm.h"

/* Allocate a kernel-owned slab only once the MM arenas exist. The owner is
 * deliberately private to the VFS mount slab; KINIT retains its descriptor
 * until native callers have been switched to the new address convention. */
#define VFS_MOUNT_SLAB_V4_OWNER 020U

int
vfs_mount_slab_v4_prepare(unsigned int capacity,
    struct vfs_mount_slab_v4 *slab)
{
        struct vfs_mount_layout_v3 layout;
        kword_t base;
        kword_t *words;
        unsigned int i;

        if (slab == 0 || vfs_mount_layout_v3(capacity, &layout) != 0)
                return -1;
        base = 0UL;
        if (mm_alloc((kword_t)layout.total_words, MM_TYPE_KERNEL_DYNAMIC,
            VFS_MOUNT_SLAB_V4_OWNER, MM_ALLOC_LOW, &base) != MM_OK)
                return -1;
        words = (kword_t *)(unsigned long)base;
        for (i = 0U; i < layout.total_words; ++i)
                words[i] = 0UL;
        slab->base = base;
        slab->target = base + layout.target_off;
        slab->root = base + layout.root_off;
        slab->reader = base + layout.reader_off;
        slab->media = base + layout.media_off;
        slab->shape = base + layout.shape_off;
        slab->extent = base + layout.extent_off;
        slab->capacity = layout.capacity;
        slab->words = layout.total_words;
        return 0;
}
