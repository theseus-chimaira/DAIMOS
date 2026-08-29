#include "d6fs_disk_v2.h"
#include "kcore.h"
#include "d6fs_provider_v2.h"

#define D6FS_DSK_V2_HALF_MASK          0777777UL
#define D6FS_DSK_V2_UNUSED_HALF        0777777UL
#define D6FS_DSK_V2_UNIT_SHIFT         16U
#define D6FS_DSK_V2_UNIT_MASK          03U
#define D6FS_DSK_V2_HW_UNIT_SHIFT      18U
#define D6FS_DSK_V2_CYL_SHIFT          6U

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
    const unsigned int *units, const kword_t *usable_blocks,
    unsigned int read_addr)
{
        unsigned int i;

        if (disk == 0 || units == 0 || usable_blocks == 0 ||
            read_addr == 0U || members == 0U ||
            members > D6FS_V2_MAX_MEMBERS)
                return -1;
        disk->set.members = members;
        disk->read_addr = read_addr;
        disk->write_addr = 0U;
        for (i = 0U; i < D6FS_V2_MAX_MEMBERS; ++i) {
                disk->unit[i] = 0U;
                disk->set.blocks[i] = 0UL;
        }
        for (i = 0U; i < members; ++i) {
                if (units[i] > 07U || usable_blocks[i] == 0UL)
                        return -1;
                disk->unit[i] = units[i];
                disk->set.blocks[i] = usable_blocks[i];
        }
        return d6fs_v2_diskset_valid(&disk->set) ? 0 : -1;
}

int
d6fs_dsk_v2_from_boot(struct d6fs_dsk_v2 *disk,
    kword_t swap_tail_blocks, unsigned int read_addr)
{
        unsigned int units[D6FS_DSK_V2_BOOT_MEMBERS];
        kword_t blocks[D6FS_DSK_V2_BOOT_MEMBERS];
        unsigned int index;
        unsigned int members;
        kword_t half;

        if (disk == 0 || read_addr == 0U ||
            swap_tail_blocks >= D6FS_DSK_V2_SECTORS_PER_UNIT)
                return -1;
        members = 0U;
        for (index = 0U; index < D6FS_DSK_V2_BOOT_MEMBERS; ++index) {
                half = d6fs_dsk_v2_handoff_half(index);
                if (half == D6FS_DSK_V2_UNUSED_HALF)
                        continue;
                units[members] = (unsigned int)((half >>
                    D6FS_DSK_V2_UNIT_SHIFT) & D6FS_DSK_V2_UNIT_MASK);
                blocks[members] = D6FS_DSK_V2_SECTORS_PER_UNIT -
                    swap_tail_blocks;
                ++members;
        }
        return d6fs_dsk_v2_init(disk, members, units, blocks, read_addr);
}

int
d6fs_dsk_v2_set_writer(struct d6fs_dsk_v2 *disk, unsigned int write_addr)
{
        if (disk == 0)
                return -1;
        disk->write_addr = write_addr;
        return 0;
}

int
d6fs_dsk_v2_raw_addr(unsigned int unit, kword_t sector, kword_t *addressp)
{
        kword_t cylinder;
        kword_t sec;

        if (addressp == 0 || unit > 03U ||
            sector >= D6FS_DSK_V2_SECTORS_PER_UNIT)
                return -1;
        cylinder = sector / (kword_t)D6FS_DSK_V2_SECTORS_PER_CYL;
        sec = sector % (kword_t)D6FS_DSK_V2_SECTORS_PER_CYL;
        *addressp = ((kword_t)unit << D6FS_DSK_V2_HW_UNIT_SHIFT) |
            (cylinder << D6FS_DSK_V2_CYL_SHIFT) | sec;
        return 0;
}

int
d6fs_dsk_v2_read_block(void *opaque, kword_t logical,
    kword_t block[D6FS_V2_BLOCK_WORDS])
{
        struct d6fs_dsk_v2 *disk;
        struct d6fs_v2_phys phys;
        kword_t raw;

        disk = (struct d6fs_dsk_v2 *)opaque;
        if (disk == 0 || block == 0 || disk->read_addr == 0U ||
            d6fs_v2_map_block(&disk->set, logical, &phys) != 0 ||
            phys.member >= disk->set.members ||
            d6fs_dsk_v2_raw_addr(disk->unit[phys.member], phys.block,
            &raw) != 0)
                return -1;
        return d6fs_dsk_v2_call(disk->read_addr, raw, block);
}

int
d6fs_dsk_v2_write_block(void *opaque, kword_t logical,
    const kword_t block[D6FS_V2_BLOCK_WORDS])
{
        struct d6fs_dsk_v2 *disk;
        struct d6fs_v2_phys phys;
        kword_t raw;

        disk = (struct d6fs_dsk_v2 *)opaque;
        if (disk == 0 || block == 0 || disk->write_addr == 0U ||
            d6fs_v2_map_block(&disk->set, logical, &phys) != 0 ||
            phys.member >= disk->set.members ||
            d6fs_dsk_v2_raw_addr(disk->unit[phys.member], phys.block,
            &raw) != 0)
                return -1;
        return d6fs_dsk_v2_call(disk->write_addr, raw, (kword_t *)block);
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
        if ((flags & VFS_V1_MOUNT_RDONLY) == 0U && disk->write_addr != 0U) {
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
