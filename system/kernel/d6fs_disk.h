#ifndef DAIMON_D6FS_DISK_H
#define DAIMON_D6FS_DISK_H

#include "d6fs.h"
#include "vfs.h"

#define D6FS_DSK_BOOT_MEMBERS       4U
#define D6FS_DSK_LAYOUT_MAGIC        0442654636222UL /* SIXBIT /D6FSR2/ */
#define D6FS_DSK_LAYOUT_MAGIC_WORD   006U
#define D6FS_DSK_LAYOUT_RANGE_WORD   007U
#define D6FS_DSK_LAYOUT_SUPER_A      010U
#define D6FS_DSK_LAYOUT_SUPER_B      011U
#define D6FS_DSK_LAYOUT_SWAP_TAIL    012U
#define D6FS_DSK_LAYOUT_BOOTSTREAM    013U
#define D6FS_DSK_LAYOUT_LOGSTORE_START 014U
#define D6FS_DSK_LAYOUT_LOGSTORE_BLOCKS 015U
#define D6FS_DSK_LAYOUT_BADMAP_START  016U
#define D6FS_DSK_LAYOUT_BADMAP_BLOCKS 017U

struct d6fs_dsk {
        struct d6fs_diskset set;
        unsigned int unit[D6FS_MAX_MEMBERS];
        kword_t base[D6FS_MAX_MEMBERS];
        kword_t swap_tail_blocks;
        kword_t logstore_start;
        kword_t logstore_blocks;
};

struct d6fs_dsk_layout {
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

/*
 * Build the current PDP-6 diskset from the Stage1 040/041 handoff and the
 * D6FS root-layout extension in each member's DBOOT descriptor.
 */

/* Initialize the D6FS view of an already assembled diskset. */

int d6fs_dsk_read_block(void *opaque, kword_t logical,
    kword_t block[D6FS_BLOCK_WORDS]);
int d6fs_dsk_write_block(void *opaque, kword_t logical,
    const kword_t block[D6FS_BLOCK_WORDS]);


/* Return 1 when the bootset has no D6FS root-layout extension. */
struct d6fs_dsk *d6fs_dsk_boot_disk_get(void);

kword_t d6fs_dsk_swap_blocks(const struct d6fs_dsk *disk);
int d6fs_dsk_swap_read(struct d6fs_dsk *disk, kword_t logical,
    kword_t block[D6FS_BLOCK_WORDS]);
int d6fs_dsk_swap_write(struct d6fs_dsk *disk, kword_t logical,
    const kword_t block[D6FS_BLOCK_WORDS]);
int d6fs_dsk_log_read(struct d6fs_dsk *disk, kword_t blockno,
    kword_t block[D6FS_BLOCK_WORDS]);
int d6fs_dsk_log_write(struct d6fs_dsk *disk, kword_t blockno,
    const kword_t block[D6FS_BLOCK_WORDS]);


#endif
