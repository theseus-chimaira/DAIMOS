#include "fs_mres.h"
#include "memfs.h"
#include "dtfs.h"
#include "d6fs_provider.h"

unsigned int fs_memfs_service_addr;
unsigned int fs_dtfs_service_addr;
unsigned int fs_d6fs_service_addr;

int
fs_provider_call(unsigned int provider, struct fs_mres_request *req)
{
        unsigned int address;

        if (req == 0)
                return -1;
        if (provider == MEMFS_V1_PROVIDER)
                address = fs_memfs_service_addr;
        else if (provider == DTFS_V1_PROVIDER)
                address = fs_dtfs_service_addr;
        else if (provider == D6FS_V2_PROVIDER)
                address = fs_d6fs_service_addr;
        else
                return -1;
        return fs_mres_call(address, req);
}

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
