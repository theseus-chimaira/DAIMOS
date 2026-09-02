#include "fs_mres.h"
#include "d6fs_provider.h"
#include "d6fs.h"
#include "diskset.h"
#include "diskset_mres.h"
#include "mres.h"

static int
d6fs_diskset_call(unsigned int op, kword_t logical, const void *buffer)
{
        struct diskset_mres_request req;

        if (diskset_service_addr == 0U)
                return -1;
        req.op = (kword_t)op;
        req.a = logical;
        req.b = (kword_t)(unsigned long)buffer;
        req.c = 0UL;
        return mres_call(diskset_service_addr, &req);
}

static int
d6fs_diskset_read(void *opaque, kword_t logical,
    kword_t block[DISKSET_BLOCK_WORDS])
{
        (void)opaque;
        return d6fs_diskset_call(DISKSET_MRES_OP_READ_BLOCK, logical, block);
}

static int
d6fs_diskset_write(void *opaque, kword_t logical,
    const kword_t block[DISKSET_BLOCK_WORDS])
{
        (void)opaque;
        return d6fs_diskset_call(DISKSET_MRES_OP_WRITE_BLOCK, logical, block);
}

static kword_t
d6fs_diskset_blocks(void)
{
        int rc;

        rc = d6fs_diskset_call(DISKSET_MRES_OP_BLOCKS, 0UL, 0);
        return rc < 0 ? 0UL : (kword_t)rc;
}

static int
d6fs_diskset_writable(void)
{
        return d6fs_diskset_call(DISKSET_MRES_OP_WRITABLE, 0UL, 0);
}

static int
d6fs_mres_mount_root(kword_t super_a, kword_t super_b, unsigned int flags,
    kword_t *scratch, vnode_t *rootp)
{
        kword_t a[D6FS_SUPER_WORDS];
        kword_t b[D6FS_SUPER_WORDS];
        struct d6fs_super_info super;
        unsigned int copy;
        unsigned int i;
        kword_t total;
        vnode_t target;
        vnode_t root;
        int rc;

        if (scratch == 0 || rootp == 0)
                return -1;
        total = d6fs_diskset_blocks();
        if (total == 0UL || super_a >= total || super_b >= total ||
            super_a == super_b ||
            d6fs_diskset_read(0, super_a, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_SUPER_WORDS; ++i)
                a[i] = scratch[i];
        if (d6fs_diskset_read(0, super_b, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_SUPER_WORDS; ++i)
                b[i] = scratch[i];
        if (d6fs_super_select(a, b, total, &super, &copy) != 0)
                return -1;
        target = vfs_root();
        if ((flags & VFS_MOUNT_RDONLY) == 0U &&
            d6fs_diskset_writable() > 0) {
                rc = d6fs_provider_mount_rw(target,
                    d6fs_diskset_read, d6fs_diskset_write, 0,
                    &super, flags, &root);
                if (rc != 0)
                        return rc;
                if (d6fs_provider_enable_state(root, super_a, super_b,
                    copy) != 0) {
                        (void)vfs_unmount(root);
                        return -1;
                }
        } else {
                rc = d6fs_provider_mount(target, d6fs_diskset_read, 0,
                    &super, flags | VFS_MOUNT_RDONLY, &root);
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
        case FS_MRES_OP_D6FS_BLOCK_BUFFER:
                return (int)(unsigned long)d6fs_provider_block_buffer();
        case FS_MRES_OP_D6FS_CACHE_INVALID:
                d6fs_provider_cache_invalidate();
                return 0;
        case FS_MRES_OP_D6FS_MOUNT_ROOT:
                return d6fs_mres_mount_root(r->a, r->b,
                    (unsigned int)r->c,
                    (kword_t *)(unsigned long)r->d,
                    (vnode_t *)(unsigned long)r->e);
        default:
                return -1;
        }
}
