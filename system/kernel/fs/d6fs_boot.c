#include "d6fs_boot.h"
#include "d6fs_provider.h"
#include "kinit.h"
#include "blockset_boot.h"
#include "logstore.h"

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
    unsigned int flags, kword_t super_a, kword_t super_b, unsigned int copy)
{
        struct d6fs_reader *reader;
        vnode_t root;
        unsigned int id;
        unsigned int writable;
        kword_t *scratch;
        kword_t dirty_block;

        if (super == 0 || copy > 1U || super_a == super_b ||
            super->total_blocks == 0UL || super->fcb_count == 0UL ||
            super->root_fcb >= super->fcb_count ||
            d6fs_provider_reader_addr == 0U ||
            d6fs_block_read_addr == 0U)
                return -1;

        writable = (flags & VFS_MOUNT_RDONLY) == 0U &&
            d6fs_block_write_addr != 0U && blockset_boot_writable() > 0;
        if (!writable)
                flags |= VFS_MOUNT_RDONLY;

        /* Disk boot owns initial namespace creation.  There is no
         * temporary root to replace or rebind. */
        if (vfs_namespace_root != VFS_NODE_NONE)
                return -1;
        if (vfs_mount(VFS_NODE_NONE, D6FS_PROVIDER, D6FS_KIND_NODE,
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
                if (blockset_boot_read(dirty_block, scratch) != 0)
                        goto fail;
                scratch[D6FS_SB_SEQUENCE] = super->sequence + 1UL;
                scratch[D6FS_SB_STATE] = D6FS_STATE_DIRTY;
                if (blockset_boot_write(dirty_block, scratch) != 0)
                        goto fail;
                reader->super.sequence = super->sequence + 1UL;
                reader->opaque = (void *)(unsigned long)(id |
                    D6FS_PROVIDER_MOUNT_WRITABLE |
                    ((copy ^ 1U) ? D6FS_PROVIDER_MOUNT_COPY : 0U));
        }

        return 0;

fail:
        reader->opaque = 0;
        D6FS_READER_CACHE_BLOCK(reader) = D6FS_CACHE_INVALID;
        (void)vfs_unmount(root);
        return -1;
}

int
d6fs_boot_mount_root(unsigned int flags)
{
        kword_t a[D6FS_SUPER_WORDS];
        kword_t b[D6FS_SUPER_WORDS];
        struct d6fs_super_info super;
        const kword_t *selected;
        kword_t high;
        kword_t log_start;
        kword_t log_blocks;
        kword_t *scratch;
        kword_t super_a;
        kword_t super_b;
        kword_t total;
        unsigned int copy;
        unsigned int i;
        int rc;
        struct vfs_stat st;

        scratch = d6fs_boot_block_buffer();
        rc = blockset_boot_discover(&super_a, &super_b);
        if (rc != 0)
                return rc;
        if (d6fs_direct_map_addr != 0U) {
                unsigned int direct_unit;
                kword_t direct_base;
                kword_t direct_blocks;
                kword_t direct_tail;

                if (blockset_boot_direct(&direct_unit, &direct_base,
                    &direct_blocks, &direct_tail) != 0)
                        *(kword_t *)(unsigned long)d6fs_direct_map_addr =
                            ((kword_t)direct_unit << 18U) | direct_base;
        }
        total = blockset_boot_blocks();
        if (total == 0UL || super_a >= total || super_b >= total ||
            super_a == super_b || blockset_boot_read(super_a, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_SUPER_WORDS; ++i)
                a[i] = scratch[i];
        if (blockset_boot_read(super_b, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_SUPER_WORDS; ++i)
                b[i] = scratch[i];
        if (d6fs_super_select(a, b, total, &super, &copy) != 0)
                return -1;
        selected = copy == 0U ? a : b;
        high = selected[D6FS_SB_RESERVATION_HIGH];
        log_start = selected[D6FS_SB_LOG_RESERVATION] >>
            D6FS_RESERVATION_START_SHIFT;
        log_blocks = (((high >> D6FS_RESERVATION_LOG_HIGH_SHIFT) &
            D6FS_RESERVATION_LEN_HIGH_MASK) << 12U) |
            (selected[D6FS_SB_LOG_RESERVATION] &
            D6FS_RESERVATION_LEN_LOW_MASK);
        if (log_blocks != 0UL &&
            ((super_a >= log_start && super_a - log_start < log_blocks) ||
            (super_b >= log_start && super_b - log_start < log_blocks)))
                return -1;
        logstore_boot_configure(log_start, log_blocks);
        rc = d6fs_boot_runtime_init(&super, flags, super_a, super_b, copy);
        if (rc != 0)
                return rc;
        if (vfs_namespace_root == VFS_NODE_NONE ||
            vfs_stat(vfs_namespace_root, &st) != 0 ||
            st.type != VFS_TYPE_DIR) {
                if (vfs_namespace_root != VFS_NODE_NONE)
                        (void)vfs_unmount(vfs_namespace_root);
                return -1;
        }
        return 0;
}
