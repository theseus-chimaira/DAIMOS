#include "fs_mres.h"
#include "d6fs_provider.h"
#include "d6fs.h"
#include "diskset.h"
#include "diskset_mres.h"

extern int d6fs_diskset_service(struct diskset_mres_request *req);

static int
d6fs_diskset_call(unsigned int op, kword_t logical, const void *buffer)
{
        struct diskset_mres_request req;

        req.op = (kword_t)op;
        req.a = logical;
        req.b = (kword_t)(unsigned long)buffer;
        req.c = 0UL;
        return d6fs_diskset_service(&req);
}

int
d6fs_diskset_read(void *opaque, kword_t logical,
    kword_t block[DISKSET_BLOCK_WORDS])
{
        (void)opaque;
        return d6fs_diskset_call(DISKSET_MRES_OP_READ_BLOCK, logical, block);
}

int
d6fs_diskset_write(void *opaque, kword_t logical,
    const kword_t block[DISKSET_BLOCK_WORDS])
{
        (void)opaque;
        return d6fs_diskset_call(DISKSET_MRES_OP_WRITE_BLOCK, logical, block);
}
