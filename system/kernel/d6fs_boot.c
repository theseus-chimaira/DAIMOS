#include "d6fs_boot.h"
#include "d6fs_provider.h"
#include "kinit.h"
#include "fs_mres.h"
#include "diskset_boot.h"

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
        return (kword_t *)(unsigned long)d6fs_boot_call(
            FS_MRES_OP_D6FS_BLOCK_BUFFER, 0, 0, 0, 0, 0, 0);
}

void
d6fs_boot_cache_invalidate(void)
{
        (void)d6fs_boot_call(FS_MRES_OP_D6FS_CACHE_INVALID,
            0, 0, 0, 0, 0, 0);
}

static int
d6fs_boot_mount_at_root(kword_t super_a, kword_t super_b,
    unsigned int flags, kword_t scratch[D6FS_BLOCK_WORDS],
    vnode_t *rootp)
{
        return d6fs_boot_call(FS_MRES_OP_D6FS_MOUNT_ROOT,
            super_a, super_b, (kword_t)flags,
            (kword_t)(unsigned long)scratch, (kword_t)(unsigned long)rootp,
            0UL);
}

int
d6fs_boot_mount_root(unsigned int flags, vnode_t *rootp)
{
        kword_t *scratch;
        kword_t super_a;
        kword_t super_b;
        int rc;

        if (rootp == 0)
                return -1;
        scratch = d6fs_boot_block_buffer();
        if (scratch == 0)
                return -1;
        rc = diskset_boot_discover(&super_a, &super_b);
        if (rc != 0)
                return rc;
        return d6fs_boot_mount_at_root(super_a, super_b, flags, scratch,
            rootp);
}
