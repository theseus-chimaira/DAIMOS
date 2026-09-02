#ifndef DAIMON_D6FS_BOOT_H
#define DAIMON_D6FS_BOOT_H

#include "vfs.h"
#include "d6fs.h"

int d6fs_boot_mount_root(unsigned int flags, vnode_t *rootp);
struct diskset *d6fs_boot_disk(void);
kword_t *d6fs_boot_block_buffer(void);
void d6fs_boot_cache_invalidate(void);
int d6fs_boot_log_read(struct diskset *disk, kword_t blockno,
    kword_t block[D6FS_BLOCK_WORDS]);
int d6fs_boot_log_write(struct diskset *disk, kword_t blockno,
    const kword_t block[D6FS_BLOCK_WORDS]);

#endif
