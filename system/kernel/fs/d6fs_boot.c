#include "d6fs_boot.h"
#include "d6fs_provider.h"
#include "kinit.h"
#include "blockset_boot.h"
#include "fs_mres.h"
#include "logstore.h"
#include "fs_backing.h"
#if KINIT_FULL
#include "root_select.h"
#endif

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
    kword_t alloc_cursor, kword_t summary_start, unsigned int flags,
    kword_t super_a, kword_t super_b, unsigned int copy)
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
            d6fs_active_reader == 0 ||
            d6fs_backing_read_addr == 0U)
                return -1;

        writable = (flags & VFS_MOUNT_RDONLY) == 0U &&
            d6fs_backing_write_addr != 0U && blockset_boot_writable() > 0;
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
        reader = d6fs_active_reader;
        if (reader == 0 || id != 1U || reader->opaque != 0) {
                (void)vfs_unmount(root);
                return -1;
        }

        reader->alloc_cursor = alloc_cursor;
        reader->opaque = (summary_start << D6FS_PROVIDER_SUMMARY_SHIFT) |
            (kword_t)id |
            (kword_t)(copy ? D6FS_PROVIDER_MOUNT_COPY : 0U);
        reader->super = *super;
        reader->backing.blocks = super->total_blocks;
        D6FS_RUNTIME_SUPER_BLOCK(reader, 0U) = super_a;
        D6FS_RUNTIME_SUPER_BLOCK(reader, 1U) = super_b;
        D6FS_READER_CACHE_BLOCK(reader) = D6FS_CACHE_INVALID;

        if (writable) {
                dirty_block = copy == 0U ? super_b : super_a;
                scratch = d6fs_boot_block_buffer();
                if (blockset_boot_read(dirty_block, scratch) != 0)
                        goto fail;
                scratch[D6FS_SB_SEQUENCE] = super->sequence + 1UL;
                scratch[D6FS_SB_STATE] = D6FS_STATE_DIRTY;
                if (blockset_boot_write(dirty_block, scratch) != 0)
                        goto fail;
                reader->super.sequence = super->sequence + 1UL;
                reader->opaque ^= (kword_t)D6FS_PROVIDER_MOUNT_COPY;
                reader->opaque |= (kword_t)D6FS_PROVIDER_MOUNT_WRITABLE;
        }

        return 0;

fail:
        /* Preserve the mount id until VFS/provider teardown has released the
         * dynamic reader.  PREPARE_UNMOUNT owns reader destruction. */
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
        kword_t swap_blocks;
        kword_t log_start;
        kword_t log_blocks;
        kword_t tail_blocks;
        kword_t *scratch;
        kword_t super_a;
        kword_t super_b;
        kword_t total;
        kword_t alloc_cursor;
        kword_t summary_start;
        unsigned int copy;
        unsigned int i;
        int rc;
        struct vfs_stat st;

        scratch = d6fs_boot_block_buffer();
        rc = blockset_boot_discover(&super_a, &super_b);
        if (rc != 0)
                return rc;
        if (d6fs_active_reader != 0) {
                unsigned int direct_unit;
                kword_t direct_base;
                kword_t direct_blocks;
                kword_t direct_tail;
                struct d6fs_reader *reader;

                reader = d6fs_active_reader;
                if (reader == 0)
                        return -1;
                if (blockset_boot_direct(&direct_unit, &direct_base,
                    &direct_blocks, &direct_tail) != 0)
                        reader->backing.opaque = ((kword_t)direct_unit << 18U) |
                            direct_base;
#if KINIT_FULL
                        if (root_select_class() == KINIT_ROOT_DRM)
                                reader->backing.opaque |=
                                    (kword_t)FS_BACKING_DIRECT_DRM_TAG << 18U;
#endif
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
        summary_start = selected[D6FS_SB_SUMMARY_START];
        alloc_cursor = summary_start + selected[D6FS_SB_SUMMARY_BLOCKS];
        if (alloc_cursor >= super.total_blocks)
                alloc_cursor = 0UL;
        high = selected[D6FS_SB_RESERVATION_HIGH];
        swap_blocks = (((high >> D6FS_RES_SWAP_HI_SHIFT) &
            D6FS_RESERVATION_LEN_HIGH_MASK) << 12U) |
            (selected[D6FS_SB_SWAP_RESERVATION] &
            D6FS_RESERVATION_LEN_LOW_MASK);
        tail_blocks = blockset_direct_tail;
        if ((swap_blocks == 0UL && tail_blocks != 0UL) ||
            swap_blocks > tail_blocks)
                return -1;
        blockset_direct_blocks = total;
        blockset_direct_tail = swap_blocks;
        log_start = selected[D6FS_SB_LOG_RESERVATION] >>
            D6FS_RESERVATION_START_SHIFT;
        log_blocks = (((high >> D6FS_RES_LOG_HI_SHIFT) &
            D6FS_RESERVATION_LEN_HIGH_MASK) << 12U) |
            (selected[D6FS_SB_LOG_RESERVATION] &
            D6FS_RESERVATION_LEN_LOW_MASK);
        if (log_blocks != 0UL &&
            ((super_a >= log_start && super_a - log_start < log_blocks) ||
            (super_b >= log_start && super_b - log_start < log_blocks)))
                return -1;
        blockset_direct_tail = swap_blocks;
        logstore_boot_configure(log_start, log_blocks);
        if (swap_blocks != 0UL)
                flags |= VFS_MOUNT_STORAGE_SWAP;
        if (log_blocks != 0UL)
                flags |= VFS_MOUNT_STORAGE_LOGSTORE;
        rc = d6fs_boot_runtime_init(&super, alloc_cursor, summary_start,
            flags, super_a, super_b, copy);
        if (rc != 0) {
                blockset_direct_tail = 0UL;
                logstore_boot_configure(0UL, 0UL);
                return rc;
        }
        if (vfs_namespace_root == VFS_NODE_NONE ||
            vfs_stat(vfs_namespace_root, &st) != 0 ||
            st.type != VFS_TYPE_DIR) {
                if (vfs_namespace_root != VFS_NODE_NONE) {
                        i = VFS_MOUNT_ID(vfs_namespace_root);
                        (void)vfs_storage_release(i,
                            flags & VFS_MOUNT_STORAGE_MASK);
                        blockset_direct_tail = 0UL;
                        logstore_boot_configure(0UL, 0UL);
                        (void)vfs_unmount(vfs_namespace_root);
                }
                return -1;
        }
        return 0;
}
