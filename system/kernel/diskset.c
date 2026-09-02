#include "diskset.h"
#include "dsk270.h"

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

int
diskset_valid(const struct diskset *set)
{
        unsigned int i;

        if (set == 0 || set->members == 0U ||
            set->members > DISKSET_MAX_MEMBERS)
                return 0;
        for (i = 0U; i < set->members; ++i)
                if (set->blocks[i] == 0UL || set->unit[i] >= DSK270_UNITS)
                        return 0;
        return 1;
}

kword_t
diskset_blocks(const struct diskset *set)
{
        unsigned int i;
        kword_t total;

        if (!diskset_valid(set))
                return 0UL;
        total = 0UL;
        for (i = 0U; i < set->members; ++i)
                total += set->blocks[i];
        return total;
}

int
diskset_map_block(const struct diskset *set, kword_t logical,
    struct diskset_phys *phys)
{
        unsigned int i;
        unsigned int width;
        unsigned int slot;
        kword_t floor;
        kword_t next;
        kword_t zone_blocks;
        kword_t rel;

        if (!diskset_valid(set) || phys == 0 ||
            logical >= diskset_blocks(set))
                return -1;

        floor = 0UL;
        for (;;) {
                next = diskset_min_above(set, floor);
                width = diskset_width_above(set, floor);
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
        for (i = 0U; i < set->members; ++i) {
                if (set->blocks[i] <= floor)
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

int
diskset_read_block(void *opaque, kword_t logical,
    kword_t block[DISKSET_BLOCK_WORDS])
{
        struct diskset *set;
        struct diskset_phys phys;

        set = (struct diskset *)opaque;
        if (set == 0 || block == 0 ||
            diskset_map_block(set, logical, &phys) != 0)
                return -1;
        return dsk270_read_sector(set->unit[phys.member],
            set->base[phys.member] + phys.block, block);
}

int
diskset_write_block(void *opaque, kword_t logical,
    const kword_t block[DISKSET_BLOCK_WORDS])
{
        struct diskset *set;
        struct diskset_phys phys;

        set = (struct diskset *)opaque;
        if (set == 0 || block == 0 ||
            diskset_map_block(set, logical, &phys) != 0)
                return -1;
        return dsk270_write_sector(set->unit[phys.member],
            set->base[phys.member] + phys.block, block);
}

kword_t
diskset_swap_blocks(const struct diskset *set)
{
        if (set == 0)
                return 0UL;
        return set->swap_tail_blocks * (kword_t)set->members;
}

static int
diskset_swap_io(struct diskset *set, kword_t logical,
    kword_t block[DISKSET_BLOCK_WORDS], int write)
{
        unsigned int member;
        kword_t local;

        if (set == 0 || block == 0 || set->members == 0U ||
            set->swap_tail_blocks == 0UL ||
            logical >= diskset_swap_blocks(set))
                return -1;
        member = (unsigned int)(logical % (kword_t)set->members);
        local = logical / (kword_t)set->members;
        if (local >= set->swap_tail_blocks)
                return -1;
        if (write != 0)
                return dsk270_write_sector(set->unit[member],
                    set->base[member] + set->blocks[member] + local, block);
        return dsk270_read_sector(set->unit[member],
            set->base[member] + set->blocks[member] + local, block);
}

int
diskset_swap_read(struct diskset *set, kword_t logical,
    kword_t block[DISKSET_BLOCK_WORDS])
{
        return diskset_swap_io(set, logical, block, 0);
}

int
diskset_swap_write(struct diskset *set, kword_t logical,
    const kword_t block[DISKSET_BLOCK_WORDS])
{
        return diskset_swap_io(set, logical, (kword_t *)block, 1);
}

int
diskset_log_read(struct diskset *set, kword_t blockno,
    kword_t block[DISKSET_BLOCK_WORDS])
{
        if (set == 0 || blockno >= set->logstore_blocks)
                return -1;
        return diskset_read_block(set, set->logstore_start + blockno, block);
}

int
diskset_log_write(struct diskset *set, kword_t blockno,
    const kword_t block[DISKSET_BLOCK_WORDS])
{
        if (set == 0 || blockno >= set->logstore_blocks)
                return -1;
        return diskset_write_block(set, set->logstore_start + blockno, block);
}

struct diskset *
diskset_boot_get(void)
{
        return &diskset_boot;
}
