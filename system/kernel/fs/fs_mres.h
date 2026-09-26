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
#define FS_MRES_OP_MEMFS_USAGE          20U
#define FS_MRES_OP_D6FS_REMOUNT         20U

struct fs_mres_request {
        kword_t op;
        kword_t a;
        kword_t b;
        kword_t c;
        kword_t d;
        kword_t e;
};

extern kword_t fs_memfs_service_jump;
extern kword_t fs_dtfs_service_jump;
extern kword_t fs_d6fs_service_jump;
extern kword_t fs_tsfs_service_jump;
extern kword_t d6fs_cache_reclaim_jump;
int fs_d6fs_cache_reclaim(kword_t words);
extern kword_t blockset_runtime_service_jump;
int blockset_runtime_reg_call(unsigned int op, kword_t a, kword_t b, kword_t c);
void blockset_direct_configure(unsigned int unit, kword_t tail_base,
    kword_t blocks, kword_t tail_blocks);
extern kword_t blockset_direct_blocks;
extern kword_t blockset_direct_tail;
extern kword_t sys_memfs_usage_call;
extern kword_t sys_dtfs_format_jump;
extern kword_t sys_dtfs_mount_jump;

void fs_copy_words(const kword_t *src, kword_t *dst, unsigned int count);
void fs_move_words(const kword_t *src, kword_t *dst, unsigned int count);
void fs_zero_words(kword_t *dst, unsigned int count);
void fs_zero_block_workspace(void);

#endif
