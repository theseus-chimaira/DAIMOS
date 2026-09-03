#include "d6fs_boot.h"
#include "d6fs_provider.h"
#include "kinit.h"
#include "fs_mres.h"
#include "diskset_boot.h"

/* KINIT-only scratch.  Root probing and the early LOGSTORE append happen
 * before KINIT is reclaimed, so they need not borrow resident FS storage. */
static kword_t d6fs_boot_scratch[D6FS_BLOCK_WORDS];

static int
d6fs_boot_call(unsigned int op, kword_t a, kword_t b, kword_t c,
    kword_t d, kword_t e, kword_t f)
{
        struct fs_mres_request req;

        req.op = (kword_t)op;
        req.a = a;
        req.b = b;
        req.c = c;
        req.d = d;
        req.e = e;
        req.f = f;
        return fs_mres_call(fs_d6fs_service_addr, &req);
}

kword_t *
d6fs_boot_block_buffer(void)
{
        return d6fs_boot_scratch;
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

        if (rootp == 0)
                return -1;
        scratch = d6fs_boot_block_buffer();
        if (scratch == 0)
                return -1;
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
        rc = d6fs_boot_call(FS_MRES_OP_D6FS_MOUNT_ROOT,
            (kword_t)(unsigned long)&super, (kword_t)flags, super_a, super_b,
            (kword_t)copy, (kword_t)(unsigned long)rootp);
        if (rc != 0)
                return rc;
        if (vfs_set_root(*rootp) != 0) {
                (void)vfs_unmount(*rootp);
                *rootp = VFS_NODE_NONE;
                return -1;
        }
        return 0;
}
