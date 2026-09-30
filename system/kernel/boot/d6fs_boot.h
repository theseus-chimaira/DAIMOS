#ifndef DAIMON_D6FS_BOOT_H
#define DAIMON_D6FS_BOOT_H

#include "vfs.h"
#include "d6fs.h"

int d6fs_boot_mount_root(unsigned int flags);
int d6fs_boot_select(unsigned int root_class, unsigned int ordinal);
kword_t *d6fs_boot_block_buffer(void);

#endif
