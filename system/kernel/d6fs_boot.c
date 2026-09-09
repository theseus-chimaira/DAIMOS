#include "d6fs_boot.h"
#include "d6fs_provider.h"
#include "kinit.h"
#include "diskset_boot.h"

/* KINIT-only scratch used while probing and mounting the root D6FS.
 * It is reclaimed with KINIT and does not consume resident FS storage. */
static kword_t d6fs_boot_scratch[D6FS_BLOCK_WORDS];

kword_t *
d6fs_boot_block_buffer(void)
{
        return d6fs_boot_scratch;
}

static int
d6fs_boot_runtime_init(const struct d6fs_super_info *super,
    unsigned int flags, kword_t super_a, kword_t super_b, unsigned int copy,
    vnode_t *rootp)
{
        struct d6fs_reader *reader;
        vnode_t target;
        vnode_t root;
        unsigned int id;
        unsigned int writable;
        kword_t *scratch;
        kword_t dirty_block;

        if (super == 0 || rootp == 0 || copy > 1U || super_a == super_b ||
            super->total_blocks == 0UL || super->fcb_count == 0UL ||
            super->root_fcb >= super->fcb_count ||
            d6fs_provider_reader_addr == 0U ||
            d6fs_diskset_read_addr == 0U)
                return -1;

        writable = (flags & VFS_MOUNT_RDONLY) == 0U &&
            d6fs_diskset_write_addr != 0U && diskset_boot_writable() > 0;
        if (!writable)
                flags |= VFS_MOUNT_RDONLY;

        target = vfs_namespace_root;
        if (vfs_mount(target, D6FS_PROVIDER, D6FS_KIND_NODE,
            super->root_fcb, flags, &root) != 0)
                return -1;
        id = VFS_MOUNT_ID(root);
        reader = (struct d6fs_reader *)(unsigned long)d6fs_provider_reader_addr;
        if (reader->opaque != 0) {
                (void)vfs_unmount(root);
                return -1;
        }

        reader->alloc_cursor = super->summary_start + super->summary_blocks;
        if (reader->alloc_cursor >= super->total_blocks)
                reader->alloc_cursor = 0UL;

        reader->opaque = (void *)(unsigned long)id;
        reader->super = *super;
        D6FS_RUNTIME_SUPER_BLOCK(reader, 0U) = super_a;
        D6FS_RUNTIME_SUPER_BLOCK(reader, 1U) = super_b;
        D6FS_READER_CACHE_BLOCK(reader) = D6FS_CACHE_INVALID;

        if (writable) {
                if (super->state != D6FS_STATE_CLEAN)
                        goto fail;
                dirty_block = copy == 0U ? super_b : super_a;
                scratch = d6fs_boot_block_buffer();
                if (diskset_boot_read(dirty_block, scratch) != 0)
                        goto fail;
                scratch[D6FS_SB_SEQUENCE] = super->sequence + 1UL;
                scratch[D6FS_SB_STATE] = D6FS_STATE_DIRTY;
                if (diskset_boot_write(dirty_block, scratch) != 0)
                        goto fail;
                reader->super.sequence = super->sequence + 1UL;
                reader->opaque = (void *)(unsigned long)(id |
                    D6FS_PROVIDER_MOUNT_WRITABLE |
                    ((copy ^ 1U) ? D6FS_PROVIDER_MOUNT_COPY : 0U));
        }

        *rootp = root;
        return 0;

fail:
        reader->opaque = 0;
        D6FS_READER_CACHE_BLOCK(reader) = D6FS_CACHE_INVALID;
        (void)vfs_unmount(root);
        return -1;
}

int
d6fs_boot_mount_root(unsigned int flags, vnode_t *rootp)
{
        kword_t a[D6FS_SUPER_WORDS];
        kword_t b[D6FS_SUPER_WORDS];
        struct d6fs_super_info super;
        kword_t *scratch;
        kword_t super_a;
        kword_t super_b;
        kword_t total;
        unsigned int copy;
        unsigned int i;
        int rc;
        struct vfs_stat st;

        if (rootp == 0)
                return -1;
        scratch = d6fs_boot_block_buffer();
        rc = diskset_boot_discover(&super_a, &super_b);
        if (rc != 0)
                return rc;
        total = diskset_boot_blocks();
        if (total == 0UL || super_a >= total || super_b >= total ||
            super_a == super_b || diskset_boot_read(super_a, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_SUPER_WORDS; ++i)
                a[i] = scratch[i];
        if (diskset_boot_read(super_b, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_SUPER_WORDS; ++i)
                b[i] = scratch[i];
        if (d6fs_super_select(a, b, total, &super, &copy) != 0)
                return -1;
        rc = d6fs_boot_runtime_init(&super, flags, super_a, super_b, copy,
            rootp);
        if (rc != 0)
                return rc;
        if (*rootp == VFS_NODE_NONE || vfs_stat(*rootp, &st) != 0 ||
            st.type != VFS_TYPE_DIR) {
                (void)vfs_unmount(*rootp);
                *rootp = VFS_NODE_NONE;
                return -1;
        }
        vfs_namespace_root = *rootp;
        return 0;
}
