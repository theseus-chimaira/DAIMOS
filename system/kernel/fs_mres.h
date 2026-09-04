#ifndef DAIMON_FS_MRES_H
#define DAIMON_FS_MRES_H

#include "vfs.h"

/* One exported entry per optional filesystem MRES. */
#define FS_MRES_OP_LOOKUP              1U
#define FS_MRES_OP_READDIR             2U
#define FS_MRES_OP_STAT                3U
#define FS_MRES_OP_PARENT              4U
#define FS_MRES_OP_PARENT_NAME         5U
#define FS_MRES_OP_CREATE              6U
#define FS_MRES_OP_MKDIR               7U
#define FS_MRES_OP_SYMLINK             8U
#define FS_MRES_OP_UNLINK              9U
#define FS_MRES_OP_RENAME             10U
#define FS_MRES_OP_TRUNCATE            11U
#define FS_MRES_OP_CHMOD               12U
#define FS_MRES_OP_READ_WORDS          13U
#define FS_MRES_OP_WRITE_WORDS         14U
#define FS_MRES_OP_SYNC                15U
#define FS_MRES_OP_PREPARE_UNMOUNT     16U
#define FS_MRES_OP_FORMAT_UNIT         17U
#define FS_MRES_OP_MOUNT_UNIT          18U
#define FS_MRES_OP_MEMFS_INIT          19U
/* Provider-private operation 20 is intentionally reused.  Calls are already
 * directed to a specific provider MRES, so global sparse numbering only wastes
 * resident vector words. */
#define FS_MRES_OP_D6FS_MOUNT_ROOT     20U
#define FS_MRES_OP_DTFS_BIND            20U
#define FS_MRES_OP_MEMFS_USAGE          20U

struct fs_mres_request {
        kword_t op;
        kword_t a;
        kword_t b;
        kword_t c;
        kword_t d;
        kword_t e;
        kword_t f;
};

extern unsigned int fs_memfs_service_addr;
extern unsigned int fs_dtfs_service_addr;
extern unsigned int fs_d6fs_service_addr;

int fs_mres_call(unsigned int address, struct fs_mres_request *req);
int fs_provider_call(unsigned int provider, struct fs_mres_request *req);
void fs_copy_words(const kword_t *src, kword_t *dst, unsigned int count);
void fs_zero_block_workspace(void);
int fs_dtfs_format_unit(unsigned int unit);
int fs_dtfs_mount_unit(unsigned int unit, vnode_t target,
    unsigned int flags, vnode_t *rootp);

#endif
