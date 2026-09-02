#include "d6fs_disk.h"
#include "kcore.h"
#include "dsk270.h"
#include "d6fs_provider.h"

static struct d6fs_dsk d6fs_dsk_boot_disk;

int
d6fs_dsk_read_block(void *opaque, kword_t logical,
    kword_t block[D6FS_BLOCK_WORDS])
{
        struct d6fs_dsk *disk;
        struct d6fs_phys phys;
        disk = (struct d6fs_dsk *)opaque;
        if (disk == 0 || block == 0 ||
            d6fs_map_block(&disk->set, logical, &phys) != 0 ||
            phys.member >= disk->set.members ||
            phys.block >= disk->set.blocks[phys.member])
                return -1;
        return dsk270_read_sector(disk->unit[phys.member],
            disk->base[phys.member] + phys.block, block);
}

int
d6fs_dsk_write_block(void *opaque, kword_t logical,
    const kword_t block[D6FS_BLOCK_WORDS])
{
        struct d6fs_dsk *disk;
        struct d6fs_phys phys;
        disk = (struct d6fs_dsk *)opaque;
        if (disk == 0 || block == 0 ||
            d6fs_map_block(&disk->set, logical, &phys) != 0 ||
            phys.member >= disk->set.members ||
            phys.block >= disk->set.blocks[phys.member])
                return -1;
        return dsk270_write_sector(disk->unit[phys.member],
            disk->base[phys.member] + phys.block, block);
}

kword_t
d6fs_dsk_swap_blocks(const struct d6fs_dsk *disk)
{
        if (disk == 0)
                return 0UL;
        return disk->swap_tail_blocks * (kword_t)disk->set.members;
}

static int
d6fs_dsk_swap_io(struct d6fs_dsk *disk, kword_t logical,
    kword_t block[D6FS_BLOCK_WORDS], int write)
{
        unsigned int member;
        kword_t local;
        if (disk == 0 || block == 0 || disk->set.members == 0U ||
            disk->swap_tail_blocks == 0UL ||
            logical >= d6fs_dsk_swap_blocks(disk))
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
d6fs_dsk_swap_read(struct d6fs_dsk *disk, kword_t logical,
    kword_t block[D6FS_BLOCK_WORDS])
{
        return d6fs_dsk_swap_io(disk, logical, block, 0);
}

int
d6fs_dsk_swap_write(struct d6fs_dsk *disk, kword_t logical,
    const kword_t block[D6FS_BLOCK_WORDS])
{
        return d6fs_dsk_swap_io(disk, logical, (kword_t *)block, 1);
}

int
d6fs_dsk_log_read(struct d6fs_dsk *disk, kword_t blockno,
    kword_t block[D6FS_BLOCK_WORDS])
{
        if (disk == 0 || blockno >= disk->logstore_blocks)
                return -1;
        return d6fs_dsk_read_block(disk, disk->logstore_start + blockno,
            block);
}

int
d6fs_dsk_log_write(struct d6fs_dsk *disk, kword_t blockno,
    const kword_t block[D6FS_BLOCK_WORDS])
{
        if (disk == 0 || blockno >= disk->logstore_blocks)
                return -1;
        return d6fs_dsk_write_block(disk, disk->logstore_start + blockno,
            block);
}

struct d6fs_dsk *
d6fs_dsk_boot_disk_get(void)
{
        return &d6fs_dsk_boot_disk;
}
