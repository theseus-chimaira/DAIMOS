#include "d6fs_v2.h"

static kword_t
d6fs_v2_min_above(const struct d6fs_v2_diskset *set, kword_t floor)
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
d6fs_v2_width_above(const struct d6fs_v2_diskset *set, kword_t floor)
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
d6fs_v2_extent_encode(kword_t start, kword_t blocks, kword_t *runp,
    unsigned int *highp)
{
        kword_t count;

        if (runp == 0 || highp == 0 ||
            start > D6FS_V2_LOGICAL_BLOCK_MASK ||
            blocks == 0UL || blocks > D6FS_V2_EXTENT_MAX_BLOCKS)
                return -1;

        count = blocks - 1UL;
        *runp = ((start & D6FS_V2_LOGICAL_BLOCK_MASK) <<
            D6FS_V2_EXTENT_LOW_BITS) |
            (count & D6FS_V2_EXTENT_LOW_MASK);
        *highp = (unsigned int)((count >> D6FS_V2_EXTENT_LOW_BITS) &
            D6FS_V2_EXTENT_HIGH_MASK);
        return 0;
}

int
d6fs_v2_extent_decode(kword_t run, unsigned int high, kword_t *startp,
    kword_t *blocksp)
{
        kword_t count;

        if (startp == 0 || blocksp == 0 ||
            (high & ~((unsigned int)D6FS_V2_EXTENT_HIGH_MASK)) != 0U)
                return -1;

        *startp = (run >> D6FS_V2_EXTENT_LOW_BITS) &
            D6FS_V2_LOGICAL_BLOCK_MASK;
        count = ((kword_t)high << D6FS_V2_EXTENT_LOW_BITS) |
            (run & D6FS_V2_EXTENT_LOW_MASK);
        *blocksp = count + 1UL;
        return 0;
}

int
d6fs_v2_diskset_valid(const struct d6fs_v2_diskset *set)
{
        unsigned int i;

        if (set == 0 || set->members == 0U ||
            set->members > D6FS_V2_MAX_MEMBERS)
                return 0;
        for (i = 0U; i < set->members; ++i)
                if (set->blocks[i] == 0UL)
                        return 0;
        return 1;
}

kword_t
d6fs_v2_diskset_blocks(const struct d6fs_v2_diskset *set)
{
        unsigned int i;
        kword_t total;

        if (!d6fs_v2_diskset_valid(set))
                return 0UL;
        total = 0UL;
        for (i = 0U; i < set->members; ++i)
                total += set->blocks[i];
        return total;
}

int
d6fs_v2_map_block(const struct d6fs_v2_diskset *set, kword_t logical,
    struct d6fs_v2_phys *phys)
{
        unsigned int i;
        unsigned int width;
        unsigned int slot;
        kword_t floor;
        kword_t next;
        kword_t zone_blocks;
        kword_t rel;

        if (!d6fs_v2_diskset_valid(set) || phys == 0 ||
            logical >= d6fs_v2_diskset_blocks(set))
                return -1;

        floor = 0UL;
        for (;;) {
                next = d6fs_v2_min_above(set, floor);
                width = d6fs_v2_width_above(set, floor);
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
