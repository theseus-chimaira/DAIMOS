#include "fs_mres.h"
#include "d6fs_provider.h"
#include "d6fs_disk.h"
#include "d6fs.h"
#include "dsk270.h"


static int
d6fs_mres_disk_init(struct d6fs_dsk_v2 *disk, unsigned int members,
    const unsigned int *units, const kword_t *blocks)
{
        unsigned int i;

        if (disk == 0 || units == 0 || blocks == 0 || members == 0U ||
            members > D6FS_V2_MAX_MEMBERS)
                return -1;
        disk->set.members = members;
        disk->swap_tail_blocks = 0UL;
        disk->logstore_start = 0UL;
        disk->logstore_blocks = 0UL;
        for (i = 0U; i < D6FS_V2_MAX_MEMBERS; ++i) {
                disk->unit[i] = 0U;
                disk->base[i] = 0UL;
                disk->set.blocks[i] = 0UL;
        }
        for (i = 0U; i < members; ++i) {
                if (units[i] >= DSK270_UNITS || blocks[i] == 0UL)
                        return -1;
                disk->unit[i] = units[i];
                disk->set.blocks[i] = blocks[i];
        }
        return d6fs_diskset_valid(&disk->set) ? 0 : -1;
}

static int
d6fs_mres_mount_root(struct d6fs_dsk_v2 *disk, kword_t super_a,
    kword_t super_b, unsigned int flags, kword_t *scratch, vnode_t *rootp)
{
        kword_t a[D6FS_V2_SUPER_WORDS];
        kword_t b[D6FS_V2_SUPER_WORDS];
        struct d6fs_super_info super;
        unsigned int copy;
        unsigned int i;
        kword_t total;
        vnode_t target;
        vnode_t root;
        int rc;

        if (disk == 0 || scratch == 0 || rootp == 0)
                return -1;
        total = d6fs_diskset_blocks(&disk->set);
        if (total == 0UL || super_a >= total || super_b >= total ||
            super_a == super_b ||
            d6fs_dsk_v2_read_block(disk, super_a, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_V2_SUPER_WORDS; ++i)
                a[i] = scratch[i];
        if (d6fs_dsk_v2_read_block(disk, super_b, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_V2_SUPER_WORDS; ++i)
                b[i] = scratch[i];
        if (d6fs_super_select(a, b, total, &super, &copy) != 0)
                return -1;
        target = vfs_root();
        if ((flags & VFS_V1_MOUNT_RDONLY) == 0U &&
            dsk270_write_addr_v1 != 0U) {
                rc = d6fs_provider_mount_rw(target,
                    d6fs_dsk_v2_read_block, d6fs_dsk_v2_write_block, disk,
                    &super, flags, &root);
                if (rc != 0)
                        return rc;
                if (d6fs_provider_enable_state(root, super_a, super_b,
                    copy) != 0) {
                        (void)vfs_unmount(root);
                        return -1;
                }
        } else {
                rc = d6fs_provider_mount(target, d6fs_dsk_v2_read_block,
                    disk, &super, flags | VFS_V1_MOUNT_RDONLY, &root);
                if (rc != 0)
                        return rc;
        }
        if (vfs_set_root(root) != 0) {
                (void)vfs_unmount(root);
                return -1;
        }
        *rootp = root;
        return 0;
}

int
d6fs_mres_dispatch(struct fs_mres_request *r)
{
        struct d6fs_dsk_v2 *disk;

        if (r == 0)
                return -1;
        switch ((unsigned int)r->op) {
        case FS_MRES_OP_LOOKUP:
                return d6fs_provider_lookup(r->a,
                    (const struct vfs_name *)(unsigned long)r->b,
                    (vnode_t *)(unsigned long)r->c);
        case FS_MRES_OP_READDIR:
                return d6fs_provider_readdir(r->a, (unsigned int)r->b,
                    (struct vfs_dirent *)(unsigned long)r->c);
        case FS_MRES_OP_STAT:
                return d6fs_provider_stat(r->a,
                    (struct vfs_stat *)(unsigned long)r->b);
        case FS_MRES_OP_PARENT:
                return d6fs_provider_parent(r->a,
                    (vnode_t *)(unsigned long)r->b);
        case FS_MRES_OP_PARENT_NAME:
                return d6fs_provider_parent_name(r->a,
                    (vnode_t *)(unsigned long)r->b,
                    (struct vfs_name *)(unsigned long)r->c);
        case FS_MRES_OP_CREATE:
                return d6fs_provider_create(r->a,
                    (const struct vfs_name *)(unsigned long)r->b,
                    (unsigned int)r->c,
                    (vnode_t *)(unsigned long)r->d);
        case FS_MRES_OP_MKDIR:
                return d6fs_provider_mkdir(r->a,
                    (const struct vfs_name *)(unsigned long)r->b,
                    (unsigned int)r->c,
                    (vnode_t *)(unsigned long)r->d);
        case FS_MRES_OP_SYMLINK:
                return d6fs_provider_symlink(r->a,
                    (const struct vfs_name *)(unsigned long)r->b,
                    (const kword_t *)(unsigned long)r->c,
                    (unsigned int)r->d,
                    (vnode_t *)(unsigned long)r->e);
        case FS_MRES_OP_UNLINK:
                return d6fs_provider_unlink(r->a,
                    (const struct vfs_name *)(unsigned long)r->b);
        case FS_MRES_OP_RENAME:
                return d6fs_provider_rename(r->a,
                    (const struct vfs_name *)(unsigned long)r->b,
                    r->c, (const struct vfs_name *)(unsigned long)r->d);
        case FS_MRES_OP_TRUNCATE:
                return d6fs_provider_truncate(r->a,
                    (unsigned int)r->b, r->c);
        case FS_MRES_OP_CHMOD:
                return d6fs_provider_chmod(r->a, (unsigned int)r->b);
        case FS_MRES_OP_READ_WORDS:
                return d6fs_provider_read_words(r->a,
                    (unsigned int)r->b, (kword_t *)(unsigned long)r->c,
                    (unsigned int)r->d);
        case FS_MRES_OP_WRITE_WORDS:
                return d6fs_provider_write_words(r->a,
                    (unsigned int)r->b,
                    (const kword_t *)(unsigned long)r->c,
                    (unsigned int)r->d, r->e);
        case FS_MRES_OP_SYNC:
                return d6fs_provider_sync(r->a);
        case FS_MRES_OP_PREPARE_UNMOUNT:
                return d6fs_provider_prepare_unmount(r->a);
        case FS_MRES_OP_D6FS_BOOT_DISK:
                return (int)(unsigned long)d6fs_dsk_v2_boot_disk_get();
        case FS_MRES_OP_D6FS_BLOCK_BUFFER:
                return (int)(unsigned long)d6fs_provider_block_buffer();
        case FS_MRES_OP_D6FS_CACHE_INVALID:
                d6fs_provider_cache_invalidate();
                return 0;
        case FS_MRES_OP_D6FS_DISK_INIT:
                return d6fs_mres_disk_init(
                    (struct d6fs_dsk_v2 *)(unsigned long)r->a,
                    (unsigned int)r->b,
                    (const unsigned int *)(unsigned long)r->c,
                    (const kword_t *)(unsigned long)r->d);
        case FS_MRES_OP_D6FS_DISK_BLOCKS:
                disk = (struct d6fs_dsk_v2 *)(unsigned long)r->a;
                return (int)d6fs_diskset_blocks(&disk->set);
        case FS_MRES_OP_D6FS_READ_BLOCK:
                return d6fs_dsk_v2_read_block(
                    (void *)(unsigned long)r->a, r->b,
                    (kword_t *)(unsigned long)r->c);
        case FS_MRES_OP_D6FS_WRITE_BLOCK:
                return d6fs_dsk_v2_write_block(
                    (void *)(unsigned long)r->a, r->b,
                    (const kword_t *)(unsigned long)r->c);
        case FS_MRES_OP_D6FS_MOUNT_ROOT:
                return d6fs_mres_mount_root(
                    (struct d6fs_dsk_v2 *)(unsigned long)r->a,
                    r->b, r->c, (unsigned int)r->d,
                    (kword_t *)(unsigned long)r->e,
                    (vnode_t *)(unsigned long)r->f);
        case FS_MRES_OP_D6FS_LOG_READ:
                return d6fs_dsk_v2_log_read(
                    (struct d6fs_dsk_v2 *)(unsigned long)r->a, r->b,
                    (kword_t *)(unsigned long)r->c);
        case FS_MRES_OP_D6FS_LOG_WRITE:
                return d6fs_dsk_v2_log_write(
                    (struct d6fs_dsk_v2 *)(unsigned long)r->a, r->b,
                    (const kword_t *)(unsigned long)r->c);
        default:
                return -1;
        }
}
