#include "fs_mres.h"
#include "memfs.h"
#include "dtfs.h"
#include "d6fs_provider.h"


int
fs_dtfs_format_unit(unsigned int unit)
{
        struct fs_mres_request req;

        req.op = FS_MRES_OP_FORMAT_UNIT;
        req.a = (kword_t)unit;
        return fs_mres_call(fs_dtfs_service_addr, &req);
}

int
fs_dtfs_mount_unit(unsigned int unit, vnode_t target,
    unsigned int flags, vnode_t *rootp)
{
        struct fs_mres_request req;

        req.op = FS_MRES_OP_MOUNT_UNIT;
        req.a = (kword_t)unit;
        req.b = target;
        req.c = (kword_t)flags;
        req.d = (kword_t)(unsigned long)rootp;
        return fs_mres_call(fs_dtfs_service_addr, &req);
}
