#include "d6fs_disk_v2.h"
#include "kcore.h"
#include "dsk270.h"
#include "d6fs_provider_v2.h"

#define D6FS_DSK_V2_HALF_MASK          0777777UL
#define D6FS_DSK_V2_UNUSED_HALF        0777777UL
#define D6FS_DSK_V2_UNIT_SHIFT         16U
#define D6FS_DSK_V2_UNIT_MASK          03U
#define D6FS_DSK_V2_LOCATOR_MASK       0177777UL

static struct d6fs_dsk_v2 d6fs_dsk_v2_boot_disk;

static kword_t
d6fs_dsk_v2_handoff_half(unsigned int index)
{
        kword_t word;

        word = kcore_boot_handoff[index / 2U];
        if ((index & 1U) == 0U)
                return (word >> 18) & D6FS_DSK_V2_HALF_MASK;
        return word & D6FS_DSK_V2_HALF_MASK;
}

int
d6fs_dsk_v2_init(struct d6fs_dsk_v2 *disk, unsigned int members,
    const unsigned int *units, const kword_t *usable_blocks)
{
        unsigned int i;

        if (disk == 0 || units == 0 || usable_blocks == 0 ||
            members == 0U ||
            members > D6FS_V2_MAX_MEMBERS)
                return -1;
        disk->set.members = members;
        disk->swap_tail_blocks = 0UL;
        disk->bootstream_blocks = 0UL;
        disk->logstore_start = 0UL;
        disk->logstore_blocks = 0UL;
        disk->badmap_start = 0UL;
        disk->badmap_blocks = 0UL;
        for (i = 0U; i < D6FS_V2_MAX_MEMBERS; ++i) {
                disk->unit[i] = 0U;
                disk->base[i] = 0UL;
                disk->set.blocks[i] = 0UL;
        }
        for (i = 0U; i < members; ++i) {
                if (units[i] >= DSK270_UNITS || usable_blocks[i] == 0UL)
                        return -1;
                disk->unit[i] = units[i];
                disk->set.blocks[i] = usable_blocks[i];
        }
        return d6fs_v2_diskset_valid(&disk->set) ? 0 : -1;
}

int
d6fs_dsk_v2_layout_decode(const kword_t block[D6FS_V2_BLOCK_WORDS],
    struct d6fs_dsk_v2_layout *layout)
{
        kword_t range;

        if (block == 0 || layout == 0 ||
            block[D6FS_DSK_V2_LAYOUT_MAGIC_WORD] != D6FS_DSK_V2_LAYOUT_MAGIC)
                return -1;
        range = block[D6FS_DSK_V2_LAYOUT_RANGE_WORD];
        layout->base = (range >> 18) & D6FS_DSK_V2_HALF_MASK;
        layout->usable_blocks = range & D6FS_DSK_V2_HALF_MASK;
        layout->super_a = block[D6FS_DSK_V2_LAYOUT_SUPER_A];
        layout->super_b = block[D6FS_DSK_V2_LAYOUT_SUPER_B];
        layout->swap_tail_blocks = block[D6FS_DSK_V2_LAYOUT_SWAP_TAIL];
        layout->bootstream_blocks = block[D6FS_DSK_V2_LAYOUT_BOOTSTREAM];
        layout->logstore_start = block[D6FS_DSK_V2_LAYOUT_LOGSTORE_START];
        layout->logstore_blocks = block[D6FS_DSK_V2_LAYOUT_LOGSTORE_BLOCKS];
        layout->badmap_start = block[D6FS_DSK_V2_LAYOUT_BADMAP_START];
        layout->badmap_blocks = block[D6FS_DSK_V2_LAYOUT_BADMAP_BLOCKS];
        if (layout->usable_blocks == 0UL ||
            layout->swap_tail_blocks >= DSK270_SECTORS_PER_UNIT ||
            layout->base >= DSK270_SECTORS_PER_UNIT ||
            layout->usable_blocks > DSK270_SECTORS_PER_UNIT - layout->base ||
            layout->swap_tail_blocks > DSK270_SECTORS_PER_UNIT -
            layout->base - layout->usable_blocks ||
            layout->super_a == layout->super_b ||
            layout->super_a > D6FS_V2_LOGICAL_BLOCK_MASK ||
            layout->super_b > D6FS_V2_LOGICAL_BLOCK_MASK)
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

int
d6fs_dsk_v2_from_boot(struct d6fs_dsk_v2 *disk, kword_t *super_ap,
    kword_t *super_bp)
{
        unsigned int units[D6FS_DSK_V2_BOOT_MEMBERS];
        kword_t blocks[D6FS_DSK_V2_BOOT_MEMBERS];
        kword_t bases[D6FS_DSK_V2_BOOT_MEMBERS];
        kword_t descriptor[D6FS_V2_BLOCK_WORDS];
        struct d6fs_dsk_v2_layout layout;
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
        for (index = 0U; index < D6FS_DSK_V2_BOOT_MEMBERS; ++index) {
                half = d6fs_dsk_v2_handoff_half(index);
                if (half == D6FS_DSK_V2_UNUSED_HALF)
                        continue;
                units[members] = (unsigned int)((half >>
                    D6FS_DSK_V2_UNIT_SHIFT) & D6FS_DSK_V2_UNIT_MASK);
                locator = half & D6FS_DSK_V2_LOCATOR_MASK;
                if (dsk270_read_sector(units[members], locator,
                    descriptor) != 0)
                        return -1;
                if (descriptor[D6FS_DSK_V2_LAYOUT_MAGIC_WORD] !=
                    D6FS_DSK_V2_LAYOUT_MAGIC)
                        return members == 0U ? 1 : -1;
                if (d6fs_dsk_v2_layout_decode(descriptor, &layout) != 0)
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
        if (d6fs_dsk_v2_init(disk, members, units, blocks) != 0)
                return -1;
        for (index = 0U; index < members; ++index)
                disk->base[index] = bases[index];
        disk->swap_tail_blocks = first_swap_tail;
        disk->bootstream_blocks = first_bootstream;
        disk->logstore_start = first_logstore_start;
        disk->logstore_blocks = first_logstore_blocks;
        disk->badmap_start = first_badmap_start;
        disk->badmap_blocks = first_badmap_blocks;
        if (first_super_a >= d6fs_v2_diskset_blocks(&disk->set) ||
            first_super_b >= d6fs_v2_diskset_blocks(&disk->set))
                return -1;
        *super_ap = first_super_a;
        *super_bp = first_super_b;
        return 0;
}

int
d6fs_dsk_v2_read_block(void *opaque, kword_t logical,
    kword_t block[D6FS_V2_BLOCK_WORDS])
{
        struct d6fs_dsk_v2 *disk;
        struct d6fs_v2_phys phys;
        disk = (struct d6fs_dsk_v2 *)opaque;
        if (disk == 0 || block == 0 ||
            d6fs_v2_map_block(&disk->set, logical, &phys) != 0 ||
            phys.member >= disk->set.members ||
            phys.block >= disk->set.blocks[phys.member])
                return -1;
        return dsk270_read_sector(disk->unit[phys.member],
            disk->base[phys.member] + phys.block, block);
}

int
d6fs_dsk_v2_write_block(void *opaque, kword_t logical,
    const kword_t block[D6FS_V2_BLOCK_WORDS])
{
        struct d6fs_dsk_v2 *disk;
        struct d6fs_v2_phys phys;
        disk = (struct d6fs_dsk_v2 *)opaque;
        if (disk == 0 || block == 0 ||
            d6fs_v2_map_block(&disk->set, logical, &phys) != 0 ||
            phys.member >= disk->set.members ||
            phys.block >= disk->set.blocks[phys.member])
                return -1;
        return dsk270_write_sector(disk->unit[phys.member],
            disk->base[phys.member] + phys.block, block);
}

kword_t
d6fs_dsk_v2_swap_blocks(const struct d6fs_dsk_v2 *disk)
{
        if (disk == 0)
                return 0UL;
        return disk->swap_tail_blocks * (kword_t)disk->set.members;
}

static int
d6fs_dsk_v2_swap_io(struct d6fs_dsk_v2 *disk, kword_t logical,
    kword_t block[D6FS_V2_BLOCK_WORDS], int write)
{
        unsigned int member;
        kword_t local;
        if (disk == 0 || block == 0 || disk->set.members == 0U ||
            disk->swap_tail_blocks == 0UL ||
            logical >= d6fs_dsk_v2_swap_blocks(disk))
                return -1;
        member = (unsigned int)(logical % (kword_t)disk->set.members);
        local = logical / (kword_t)disk->set.members;
        if (local >= disk->swap_tail_blocks)
                return -1;
        if (write != 0)
                return dsk270_write_sector(disk->unit[member],
                    disk->base[member] + disk->set.blocks[member] + local,
                    block);
        return dsk270_read_sector(disk->unit[member],
            disk->base[member] + disk->set.blocks[member] + local, block);
}

int
d6fs_dsk_v2_swap_read(struct d6fs_dsk_v2 *disk, kword_t logical,
    kword_t block[D6FS_V2_BLOCK_WORDS])
{
        return d6fs_dsk_v2_swap_io(disk, logical, block, 0);
}

int
d6fs_dsk_v2_swap_write(struct d6fs_dsk_v2 *disk, kword_t logical,
    const kword_t block[D6FS_V2_BLOCK_WORDS])
{
        return d6fs_dsk_v2_swap_io(disk, logical, (kword_t *)block, 1);
}

int
d6fs_dsk_v2_log_read(struct d6fs_dsk_v2 *disk, kword_t blockno,
    kword_t block[D6FS_V2_BLOCK_WORDS])
{
        if (disk == 0 || blockno >= disk->logstore_blocks)
                return -1;
        return d6fs_dsk_v2_read_block(disk, disk->logstore_start + blockno,
            block);
}

int
d6fs_dsk_v2_log_write(struct d6fs_dsk_v2 *disk, kword_t blockno,
    const kword_t block[D6FS_V2_BLOCK_WORDS])
{
        if (disk == 0 || blockno >= disk->logstore_blocks)
                return -1;
        return d6fs_dsk_v2_write_block(disk, disk->logstore_start + blockno,
            block);
}

int
d6fs_dsk_v2_mount(struct d6fs_dsk_v2 *disk, vnode_v1_t target,
    kword_t super_a, kword_t super_b, unsigned int flags,
    kword_t scratch[D6FS_V2_BLOCK_WORDS], vnode_v1_t *rootp)
{
        kword_t a[D6FS_V2_SUPER_WORDS];
        kword_t b[D6FS_V2_SUPER_WORDS];
        struct d6fs_v2_super_info super;
        unsigned int copy;
        unsigned int i;
        kword_t total;

        if (disk == 0 || scratch == 0 || rootp == 0)
                return -1;
        total = d6fs_v2_diskset_blocks(&disk->set);
        if (total == 0UL || super_a >= total || super_b >= total ||
            super_a == super_b)
                return -1;
        if (d6fs_dsk_v2_read_block(disk, super_a, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_V2_SUPER_WORDS; ++i)
                a[i] = scratch[i];
        if (d6fs_dsk_v2_read_block(disk, super_b, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_V2_SUPER_WORDS; ++i)
                b[i] = scratch[i];
        if (d6fs_v2_super_select(a, b, total, &super, &copy) != 0)
                return -1;
        (void)copy;
        if ((flags & VFS_V1_MOUNT_RDONLY) == 0U &&
            dsk270_write_addr_v1 != 0U) {
                int rc;

                rc = d6fs_provider_v2_mount_rw(target,
                    d6fs_dsk_v2_read_block, d6fs_dsk_v2_write_block, disk,
                    &super, flags, rootp);
                if (rc != 0)
                        return rc;
                if (d6fs_provider_v2_enable_state(*rootp, super_a, super_b,
                    copy) != 0) {
                        (void)vfs_v1_unmount(*rootp);
                        return -1;
                }
                return 0;
        }
        return d6fs_provider_v2_mount(target, d6fs_dsk_v2_read_block, disk,
            &super, flags | VFS_V1_MOUNT_RDONLY, rootp);
}

int
d6fs_dsk_v2_mount_root(struct d6fs_dsk_v2 *disk, kword_t super_a,
    kword_t super_b, unsigned int flags,
    kword_t scratch[D6FS_V2_BLOCK_WORDS], vnode_v1_t *rootp)
{
        vnode_v1_t oldroot;
        vnode_v1_t root;

        if (rootp == 0)
                return -1;
        oldroot = vfs_v1_root();
        if (oldroot == VFS_V1_NODE_NONE ||
            d6fs_dsk_v2_mount(disk, oldroot, super_a, super_b, flags,
            scratch, &root) != 0)
                return -1;
        if (vfs_v1_set_root(root) != 0) {
                (void)vfs_v1_unmount(root);
                return -1;
        }
        *rootp = root;
        return 0;
}

struct d6fs_dsk_v2 *
d6fs_dsk_v2_boot_disk_get(void)
{
        return &d6fs_dsk_v2_boot_disk;
}

int
d6fs_dsk_v2_mount_boot_root(unsigned int flags, vnode_v1_t *rootp)
{
        kword_t *scratch;
        kword_t super_a;
        kword_t super_b;
        int rc;

        if (rootp == 0)
                return -1;
        scratch = d6fs_provider_v2_block_buffer();
        rc = d6fs_dsk_v2_from_boot(&d6fs_dsk_v2_boot_disk, &super_a,
            &super_b);
        if (rc != 0)
                return rc;
        return d6fs_dsk_v2_mount_root(&d6fs_dsk_v2_boot_disk, super_a,
            super_b, flags, scratch, rootp);
}
