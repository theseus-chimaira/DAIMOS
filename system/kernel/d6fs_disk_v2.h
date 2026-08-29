#ifndef DAIMON_D6FS_DISK_V2_H
#define DAIMON_D6FS_DISK_V2_H

#include "d6fs_v2.h"
#include "vfs_v1.h"

#define D6FS_DSK_V2_BOOT_MEMBERS       4U
#define D6FS_DSK_V2_SECTORS_PER_UNIT   0130000UL
#define D6FS_DSK_V2_SECTORS_PER_CYL    054U

struct d6fs_dsk_v2 {
        struct d6fs_v2_diskset set;
        unsigned int unit[D6FS_V2_MAX_MEMBERS];
        unsigned int read_addr;
        unsigned int write_addr;
};

/*
 * Build the current PDP-6 diskset from the Stage1 040/041 handoff.
 * swap_tail_blocks is the equal physical tail reserved on every member.
 */
int d6fs_dsk_v2_from_boot(struct d6fs_dsk_v2 *disk,
    kword_t swap_tail_blocks, unsigned int read_addr);

/* Generic initializer used by tests and later non-DSK270 disk providers. */
int d6fs_dsk_v2_init(struct d6fs_dsk_v2 *disk, unsigned int members,
    const unsigned int *units, const kword_t *usable_blocks,
    unsigned int read_addr);
int d6fs_dsk_v2_set_writer(struct d6fs_dsk_v2 *disk, unsigned int write_addr);

int d6fs_dsk_v2_raw_addr(unsigned int unit, kword_t sector,
    kword_t *addressp);
int d6fs_dsk_v2_read_block(void *opaque, kword_t logical,
    kword_t block[D6FS_V2_BLOCK_WORDS]);
int d6fs_dsk_v2_write_block(void *opaque, kword_t logical,
    const kword_t block[D6FS_V2_BLOCK_WORDS]);
int d6fs_dsk_v2_mount(struct d6fs_dsk_v2 *disk, vnode_v1_t target,
    kword_t super_a, kword_t super_b, unsigned int flags,
    kword_t scratch[D6FS_V2_BLOCK_WORDS], vnode_v1_t *rootp);

int d6fs_dsk_v2_mount_root(struct d6fs_dsk_v2 *disk, kword_t super_a,
    kword_t super_b, unsigned int flags,
    kword_t scratch[D6FS_V2_BLOCK_WORDS], vnode_v1_t *rootp);

int d6fs_dsk_v2_call(unsigned int address, kword_t raw_address,
    kword_t *block);

#endif
