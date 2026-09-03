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

static int
d6fs_diskset_writable(void)
{
        return d6fs_diskset_call(DISKSET_MRES_OP_WRITABLE, 0UL, 0);
}

int
d6fs_mres_mount_root(const struct d6fs_super_info *super,
    unsigned int flags, kword_t super_a, kword_t super_b, unsigned int copy,
    vnode_t *rootp)
{
        vnode_t target;
        vnode_t root;
        int rc;

        if (super == 0 || rootp == 0 || copy > 1U)
                return -1;
        target = vfs_root();
        if ((flags & VFS_MOUNT_RDONLY) == 0U && d6fs_diskset_writable() > 0) {
                rc = d6fs_provider_mount_rw(target, d6fs_diskset_read,
                    d6fs_diskset_write, 0, super, flags, &root);
                if (rc != 0)
                        return rc;
                if (d6fs_provider_enable_state(root, super_a, super_b,
                    copy) != 0) {
                        (void)vfs_unmount(root);
                        return -1;
                }
        } else {
                rc = d6fs_provider_mount_rw(target, d6fs_diskset_read, 0, 0,
                    super, flags | VFS_MOUNT_RDONLY, &root);
                if (rc != 0)
                        return rc;
        }
        *rootp = root;
        return 0;
}
