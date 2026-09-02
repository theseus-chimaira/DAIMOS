#include "diskset.h"
#include "dsk270.h"

struct diskset_phys {
        unsigned int member;
        kword_t block;
};

static struct diskset diskset_boot;

static kword_t
diskset_min_above(const struct diskset *set, kword_t floor)
{
        unsigned int i;
        kword_t next;

        next = 0UL;
        for (i = 0U; i < set->members; ++i) {
                if (set->blocks[i] > floor &&
                    (next == 0UL || set->blocks[i] < next))
                        next = set->blocks[i];
        }
        return next;
}

static unsigned int
diskset_width_above(const struct diskset *set, kword_t floor)
{
        unsigned int i;
        unsigned int width;

        width = 0U;
        for (i = 0U; i < set->members; ++i)
                if (set->blocks[i] > floor)
                        ++width;
        return width;
}

static kword_t
diskset_blocks_ready(const struct diskset *set)
{
        unsigned int i;
        kword_t total;

        total = 0UL;
        for (i = 0U; i < set->members; ++i)
                total += set->blocks[i];
        return total;
}

kword_t
diskset_blocks(void)
{
        return diskset_blocks_ready(&diskset_boot);
}

static int
diskset_map_block(kword_t logical, struct diskset_phys *phys)
{
        unsigned int i;
        unsigned int width;
        unsigned int slot;
        kword_t floor;
        kword_t next;
        kword_t zone_blocks;
        kword_t rel;

        if (phys == 0 || logical >= diskset_blocks_ready(&diskset_boot))
                return -1;
        floor = 0UL;
        for (;;) {
                next = diskset_min_above(&diskset_boot, floor);
                width = diskset_width_above(&diskset_boot, floor);
                if (next == 0UL || width == 0U)
                        return -1;
                zone_blocks = (next - floor) * (kword_t)width;
                if (logical < zone_blocks)
                        break;
                logical -= zone_blocks;
                floor = next;
        }

        slot = (unsigned int)(logical % (kword_t)width);
        rel = logical / (kword_t)width;
        for (i = 0U; i < diskset_boot.members; ++i) {
                if (diskset_boot.blocks[i] <= floor)
                        continue;
                if (slot == 0U) {
                        phys->member = i;
                        phys->block = floor + rel;
                        return 0;
                }
                --slot;
        }
        return -1;
}

static int
diskset_block_io(kword_t logical, kword_t *block, int write)
{
        struct diskset_phys phys;

        if (block == 0 || diskset_map_block(logical, &phys) != 0)
                return -1;
        if (write != 0)
                return dsk270_write_sector(diskset_boot.unit[phys.member],
                    diskset_boot.base[phys.member] + phys.block, block);
        return dsk270_read_sector(diskset_boot.unit[phys.member],
            diskset_boot.base[phys.member] + phys.block, block);
}

int
diskset_read_block(kword_t logical, kword_t block[DISKSET_BLOCK_WORDS])
{
        return diskset_block_io(logical, block, 0);
}

int
diskset_write_block(kword_t logical,
    const kword_t block[DISKSET_BLOCK_WORDS])
{
        return diskset_block_io(logical, (kword_t *)block, 1);
}

int
diskset_writable(void)
{
        return diskset_boot.members != 0U && dsk270_write_addr != 0U ? 1 : 0;
}

kword_t
diskset_swap_blocks(void)
{
        return diskset_boot.swap_tail_blocks * (kword_t)diskset_boot.members;
}

static int
diskset_swap_io(kword_t logical, kword_t count, kword_t *block, int write)
{
        unsigned int member;
        kword_t local;
        int rc;

        if (count == 0UL)
                return 0;
        if (block == 0 || logical >= diskset_swap_blocks() ||
            count > diskset_swap_blocks() - logical)
                return -1;
        member = (unsigned int)(logical / diskset_boot.swap_tail_blocks);
        local = logical % diskset_boot.swap_tail_blocks;
        while (count-- != 0UL) {
                if (write != 0)
                        rc = dsk270_write_sector(diskset_boot.unit[member],
                            diskset_boot.base[member] +
                            diskset_boot.blocks[member] + local, block);
                else
                        rc = dsk270_read_sector(diskset_boot.unit[member],
                            diskset_boot.base[member] +
                            diskset_boot.blocks[member] + local, block);
                if (rc != 0)
                        return rc;
                block += DISKSET_BLOCK_WORDS;
                if (++local == diskset_boot.swap_tail_blocks) {
                        local = 0UL;
                        ++member;
                }
        }
        return 0;
}

int
diskset_swap_read(kword_t logical, kword_t count, kword_t *block)
{
        return diskset_swap_io(logical, count, block, 0);
}

int
diskset_swap_write(kword_t logical, kword_t count, const kword_t *block)
{
        return diskset_swap_io(logical, count, (kword_t *)block, 1);
}

kword_t
diskset_log_blocks(void)
{
        return diskset_boot.logstore_blocks;
}

static int
diskset_log_io(kword_t blockno, kword_t *block, int write)
{
        if (block == 0 || blockno >= diskset_boot.logstore_blocks)
                return -1;
        return diskset_block_io(diskset_boot.logstore_start + blockno,
            block, write);
}

int
diskset_log_read(kword_t blockno, kword_t block[DISKSET_BLOCK_WORDS])
{
        return diskset_log_io(blockno, block, 0);
}

int
diskset_log_write(kword_t blockno,
    const kword_t block[DISKSET_BLOCK_WORDS])
{
        return diskset_log_io(blockno, (kword_t *)block, 1);
}

int
diskset_boot_init(const struct diskset *config)
{
        unsigned int i;
        kword_t total;

        if (config == 0 || config->members == 0U ||
            config->members > DISKSET_MAX_MEMBERS)
                return -1;
        diskset_boot.members = 0U;
        total = 0UL;
        for (i = 0U; i < config->members; ++i) {
                if (config->blocks[i] == 0UL ||
                    config->unit[i] >= DSK270_UNITS ||
                    config->base[i] >= DSK270_SECTORS_PER_UNIT ||
                    config->blocks[i] > DSK270_SECTORS_PER_UNIT -
                    config->base[i] ||
                    config->swap_tail_blocks > DSK270_SECTORS_PER_UNIT -
                    config->base[i] - config->blocks[i])
                        return -1;
                diskset_boot.unit[i] = config->unit[i];
                diskset_boot.base[i] = config->base[i];
                diskset_boot.blocks[i] = config->blocks[i];
                total += config->blocks[i];
        }
        if (config->logstore_start > total ||
            config->logstore_blocks > total - config->logstore_start)
                return -1;
        diskset_boot.swap_tail_blocks = config->swap_tail_blocks;
        diskset_boot.logstore_start = config->logstore_start;
        diskset_boot.logstore_blocks = config->logstore_blocks;
        diskset_boot.members = config->members;
        return 0;
}
