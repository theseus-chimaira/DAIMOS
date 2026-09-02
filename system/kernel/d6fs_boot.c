#include "d6fs_boot.h"
#include "d6fs_provider.h"
#include "dsk270.h"
#include "kinit.h"
#include "fs_mres.h"
#include "diskset.h"
#include "diskset_layout.h"

#define D6FS_BOOT_HALF_MASK          0777777UL
#define D6FS_BOOT_UNUSED_HALF        0777777UL
#define D6FS_BOOT_UNIT_SHIFT         16U
#define D6FS_BOOT_UNIT_MASK          03U
#define D6FS_BOOT_LOCATOR_MASK       0177777UL

static int
d6fs_boot_call(unsigned int op, kword_t a, kword_t b, kword_t c,
    kword_t d, kword_t e, kword_t f)
{
        struct fs_mres_request req;

        req.op = (kword_t)op;
        req.a = a; req.b = b; req.c = c; req.d = d; req.e = e; req.f = f;
        return fs_mres_call(fs_d6fs_service_addr, &req);
}

struct diskset *
d6fs_boot_disk(void)
{
        return (struct diskset *)(unsigned long)d6fs_boot_call(
            FS_MRES_OP_D6FS_BOOT_DISK, 0, 0, 0, 0, 0, 0);
}

kword_t *
d6fs_boot_block_buffer(void)
{
        return (kword_t *)(unsigned long)d6fs_boot_call(
            FS_MRES_OP_D6FS_BLOCK_BUFFER, 0, 0, 0, 0, 0, 0);
}

void
d6fs_boot_cache_invalidate(void)
{
        (void)d6fs_boot_call(FS_MRES_OP_D6FS_CACHE_INVALID,
            0, 0, 0, 0, 0, 0);
}

int
d6fs_boot_log_read(struct diskset *disk, kword_t blockno,
    kword_t block[D6FS_BLOCK_WORDS])
{
        return d6fs_boot_call(FS_MRES_OP_D6FS_LOG_READ,
            (kword_t)(unsigned long)disk, blockno,
            (kword_t)(unsigned long)block, 0, 0, 0);
}

int
d6fs_boot_log_write(struct diskset *disk, kword_t blockno,
    const kword_t block[D6FS_BLOCK_WORDS])
{
        return d6fs_boot_call(FS_MRES_OP_D6FS_LOG_WRITE,
            (kword_t)(unsigned long)disk, blockno,
            (kword_t)(unsigned long)block, 0, 0, 0);
}

static int
d6fs_boot_disk_init(struct diskset *disk, unsigned int members,
    const unsigned int *units, const kword_t *usable_blocks)
{
        return d6fs_boot_call(FS_MRES_OP_D6FS_DISK_INIT,
            (kword_t)(unsigned long)disk, (kword_t)members,
            (kword_t)(unsigned long)units,
            (kword_t)(unsigned long)usable_blocks, 0, 0);
}

static int
d6fs_boot_layout_decode(const kword_t block[D6FS_BLOCK_WORDS],
    struct diskset_layout *layout)
{
        kword_t range;

        if (block == 0 || layout == 0 ||
            block[DISKSET_LAYOUT_MAGIC_WORD] != DISKSET_LAYOUT_MAGIC_D6FSR2)
                return -1;
        range = block[DISKSET_LAYOUT_RANGE_WORD];
        layout->base = (range >> 18) & D6FS_BOOT_HALF_MASK;
        layout->usable_blocks = range & D6FS_BOOT_HALF_MASK;
        layout->super_a = block[DISKSET_LAYOUT_SUPER_A];
        layout->super_b = block[DISKSET_LAYOUT_SUPER_B];
        layout->swap_tail_blocks = block[DISKSET_LAYOUT_SWAP_TAIL];
        layout->bootstream_blocks = block[DISKSET_LAYOUT_BOOTSTREAM];
        layout->logstore_start = block[DISKSET_LAYOUT_LOGSTORE_START];
        layout->logstore_blocks = block[DISKSET_LAYOUT_LOGSTORE_BLOCKS];
        layout->badmap_start = block[DISKSET_LAYOUT_BADMAP_START];
        layout->badmap_blocks = block[DISKSET_LAYOUT_BADMAP_BLOCKS];
        if (layout->usable_blocks == 0UL ||
            layout->swap_tail_blocks >= DSK270_SECTORS_PER_UNIT ||
            layout->base >= DSK270_SECTORS_PER_UNIT ||
            layout->usable_blocks > DSK270_SECTORS_PER_UNIT - layout->base ||
            layout->swap_tail_blocks > DSK270_SECTORS_PER_UNIT -
            layout->base - layout->usable_blocks ||
            layout->super_a == layout->super_b ||
            layout->super_a > D6FS_LOGICAL_BLOCK_MASK ||
            layout->super_b > D6FS_LOGICAL_BLOCK_MASK)
                return -1;
        if (layout->bootstream_blocks != 0UL || layout->logstore_start != 0UL ||
            layout->logstore_blocks != 0UL || layout->badmap_start != 0UL ||
            layout->badmap_blocks != 0UL) {
                if (layout->logstore_start != layout->bootstream_blocks ||
                    layout->badmap_start != layout->logstore_start +
                    layout->logstore_blocks ||
                    layout->super_a != layout->badmap_start +
                    layout->badmap_blocks ||
                    layout->super_b != layout->super_a + 1UL)
                        return -1;
        }
        return 0;
}

static int
d6fs_boot_mount_at_root(struct diskset *disk, kword_t super_a,
    kword_t super_b, unsigned int flags, kword_t scratch[D6FS_BLOCK_WORDS],
    vnode_t *rootp)
{
        return d6fs_boot_call(FS_MRES_OP_D6FS_MOUNT_ROOT,
            (kword_t)(unsigned long)disk, super_a, super_b, (kword_t)flags,
            (kword_t)(unsigned long)scratch, (kword_t)(unsigned long)rootp);
}

static kword_t
d6fs_boot_handoff_half(unsigned int index)
{
        kword_t word;

        word = kinit_boot_handoff[index / 2U];
        if ((index & 1U) == 0U)
                return (word >> 18) & D6FS_BOOT_HALF_MASK;
        return word & D6FS_BOOT_HALF_MASK;
}

static int
d6fs_boot_discover(struct diskset *disk, kword_t *super_ap,
    kword_t *super_bp)
{
        unsigned int units[DISKSET_BOOT_MEMBERS];
        kword_t blocks[DISKSET_BOOT_MEMBERS];
        kword_t bases[DISKSET_BOOT_MEMBERS];
        kword_t descriptor[D6FS_BLOCK_WORDS];
        struct diskset_layout layout;
        unsigned int index;
        unsigned int members;
        kword_t first_super_a;
        kword_t first_super_b;
        kword_t first_swap_tail;
        kword_t first_bootstream;
        kword_t first_logstore_start;
        kword_t first_logstore_blocks;
        kword_t first_badmap_start;
        kword_t first_badmap_blocks;
        kword_t half;
        kword_t locator;

        if (disk == 0 || super_ap == 0 || super_bp == 0)
                return -1;
        members = 0U;
        first_super_a = 0UL;
        first_super_b = 0UL;
        first_swap_tail = 0UL;
        first_bootstream = 0UL;
        first_logstore_start = 0UL;
        first_logstore_blocks = 0UL;
        first_badmap_start = 0UL;
        first_badmap_blocks = 0UL;
        for (index = 0U; index < DISKSET_BOOT_MEMBERS; ++index) {
                half = d6fs_boot_handoff_half(index);
                if (half == D6FS_BOOT_UNUSED_HALF)
                        continue;
                units[members] = (unsigned int)((half >>
                    D6FS_BOOT_UNIT_SHIFT) & D6FS_BOOT_UNIT_MASK);
                locator = half & D6FS_BOOT_LOCATOR_MASK;
                if (dsk270_read_sector(units[members], locator,
                    descriptor) != 0)
                        return -1;
                if (descriptor[DISKSET_LAYOUT_MAGIC_WORD] !=
                    DISKSET_LAYOUT_MAGIC_D6FSR2)
                        return members == 0U ? 1 : -1;
                if (d6fs_boot_layout_decode(descriptor, &layout) != 0)
                        return -1;
                if (members == 0U) {
                        first_super_a = layout.super_a;
                        first_super_b = layout.super_b;
                        first_swap_tail = layout.swap_tail_blocks;
                        first_bootstream = layout.bootstream_blocks;
                        first_logstore_start = layout.logstore_start;
                        first_logstore_blocks = layout.logstore_blocks;
                        first_badmap_start = layout.badmap_start;
                        first_badmap_blocks = layout.badmap_blocks;
                } else if (layout.super_a != first_super_a ||
                    layout.super_b != first_super_b ||
                    layout.swap_tail_blocks != first_swap_tail ||
                    layout.bootstream_blocks != first_bootstream ||
                    layout.logstore_start != first_logstore_start ||
                    layout.logstore_blocks != first_logstore_blocks ||
                    layout.badmap_start != first_badmap_start ||
                    layout.badmap_blocks != first_badmap_blocks) {
                        return -1;
                }
                bases[members] = layout.base;
                blocks[members] = layout.usable_blocks;
                ++members;
        }
        if (d6fs_boot_disk_init(disk, members, units, blocks) != 0)
                return -1;
        for (index = 0U; index < members; ++index)
                disk->base[index] = bases[index];
        disk->swap_tail_blocks = first_swap_tail;
        disk->logstore_start = first_logstore_start;
        disk->logstore_blocks = first_logstore_blocks;
        {
                int total;

                total = d6fs_boot_call(FS_MRES_OP_D6FS_DISK_BLOCKS,
                    (kword_t)(unsigned long)disk, 0, 0, 0, 0, 0);
                if (total <= 0 || first_super_a >= (kword_t)total ||
                    first_super_b >= (kword_t)total)
                        return -1;
        }
        *super_ap = first_super_a;
        *super_bp = first_super_b;
        return 0;
}

int
d6fs_boot_mount_root(unsigned int flags, vnode_t *rootp)
{
        struct diskset *disk;
        kword_t *scratch;
        kword_t super_a;
        kword_t super_b;
        int rc;

        if (rootp == 0)
                return -1;
        disk = d6fs_boot_disk();
        scratch = d6fs_boot_block_buffer();
        rc = d6fs_boot_discover(disk, &super_a, &super_b);
        if (rc != 0)
                return rc;
        return d6fs_boot_mount_at_root(disk, super_a, super_b, flags,
            scratch, rootp);
}
