#ifndef DAIMON_D6FS_DISK_V2_H
#define DAIMON_D6FS_DISK_V2_H

#include "d6fs_v2.h"
#include "vfs_v1.h"

#define D6FS_DSK_V2_BOOT_MEMBERS       4U
#define D6FS_DSK_V2_SECTORS_PER_UNIT   0130000UL
#define D6FS_DSK_V2_SECTORS_PER_CYL    054U

#define D6FS_DSK_V2_LAYOUT_MAGIC        0442654636222UL /* SIXBIT /D6FSR2/ */
#define D6FS_DSK_V2_LAYOUT_MAGIC_WORD   006U
#define D6FS_DSK_V2_LAYOUT_RANGE_WORD   007U
#define D6FS_DSK_V2_LAYOUT_SUPER_A      010U
#define D6FS_DSK_V2_LAYOUT_SUPER_B      011U
#define D6FS_DSK_V2_LAYOUT_SWAP_TAIL    012U
#define D6FS_DSK_V2_LAYOUT_BOOTSTREAM    013U
#define D6FS_DSK_V2_LAYOUT_LOGSTORE_START 014U
#define D6FS_DSK_V2_LAYOUT_LOGSTORE_BLOCKS 015U
#define D6FS_DSK_V2_LAYOUT_BADMAP_START  016U
#define D6FS_DSK_V2_LAYOUT_BADMAP_BLOCKS 017U

struct d6fs_dsk_v2 {
        struct d6fs_v2_diskset set;
        unsigned int unit[D6FS_V2_MAX_MEMBERS];
        kword_t base[D6FS_V2_MAX_MEMBERS];
        unsigned int read_addr;
        unsigned int write_addr;
        kword_t swap_tail_blocks;
        kword_t bootstream_blocks;
        kword_t logstore_start;
        kword_t logstore_blocks;
        kword_t badmap_start;
        kword_t badmap_blocks;
};

struct d6fs_dsk_v2_layout {
        kword_t base;
        kword_t usable_blocks;
        kword_t super_a;
        kword_t super_b;
        kword_t swap_tail_blocks;
        kword_t bootstream_blocks;
        kword_t logstore_start;
        kword_t logstore_blocks;
        kword_t badmap_start;
        kword_t badmap_blocks;
};

/* Decode the D6FS root-layout extension carried by a DBOOT descriptor. */
int d6fs_dsk_v2_layout_decode(const kword_t block[D6FS_V2_BLOCK_WORDS],
    struct d6fs_dsk_v2_layout *layout);

/*
 * Build the current PDP-6 diskset from the Stage1 040/041 handoff and the
 * D6FS root-layout extension in each member's DBOOT descriptor.
 */
int d6fs_dsk_v2_from_boot(struct d6fs_dsk_v2 *disk, unsigned int read_addr,
    kword_t *super_ap, kword_t *super_bp);

/* Initialize the current PDP-6 DSK270 bridge; hardware units are 0..3. */
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

/* Return 1 when the bootset has no D6FS root-layout extension. */
int d6fs_dsk_v2_mount_boot_root(unsigned int read_addr,
    unsigned int write_addr, unsigned int flags, vnode_v1_t *rootp);
struct d6fs_dsk_v2 *d6fs_dsk_v2_boot_disk_get(void);

kword_t d6fs_dsk_v2_swap_blocks(const struct d6fs_dsk_v2 *disk);
int d6fs_dsk_v2_swap_read(struct d6fs_dsk_v2 *disk, kword_t logical,
    kword_t block[D6FS_V2_BLOCK_WORDS]);
int d6fs_dsk_v2_swap_write(struct d6fs_dsk_v2 *disk, kword_t logical,
    const kword_t block[D6FS_V2_BLOCK_WORDS]);
int d6fs_dsk_v2_log_read(struct d6fs_dsk_v2 *disk, kword_t blockno,
    kword_t block[D6FS_V2_BLOCK_WORDS]);
int d6fs_dsk_v2_log_write(struct d6fs_dsk_v2 *disk, kword_t blockno,
    const kword_t block[D6FS_V2_BLOCK_WORDS]);

int d6fs_dsk_v2_call(unsigned int address, kword_t raw_address,
    kword_t *block);

#endif
