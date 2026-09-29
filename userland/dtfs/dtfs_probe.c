#include "dsys.h"
#include "dtfs_media.h"
#include "dtfs_probe.h"

static kword_t probe_dir[DTFS_BLOCK_WORDS];
static kword_t probe_block[DTFS_BLOCK_WORDS];

static unsigned int
owner_at(const kword_t *dir, unsigned int base, unsigned int index)
{
        unsigned int rem;
        unsigned int shift;

        rem = index % 7U;
        shift = 31U - rem * 5U;
        return (unsigned int)((dir[base + index / 7U] >> shift) & 037UL);
}

static int
native_valid(const kword_t *dir)
{
        unsigned int i;

        if (dir[DTFS_MAGIC_WORD] != DTFS_NATIVE_MAGIC ||
            owner_at(dir, 0U, 0U) != DTFS_OWNER_RESERVED ||
            owner_at(dir, 0U, DTFS_DIR_BLOCK) != DTFS_OWNER_RESERVED)
                return 0;
        for (i = DTFS_BLOCKS; i != DTFS_BLOCKS + 3U; ++i)
                if (owner_at(dir, 0U, i) != DTFS_OWNER_NATIVE_TAG)
                        return 0;
        return 1;
}

static int
tenex_chain_valid(unsigned int unit, const kword_t *dir, unsigned int slot)
{
        unsigned int owner;
        unsigned int block;
        unsigned int blocks;
        unsigned int first;
        unsigned int next;
        unsigned int count;
        unsigned int seen;

        owner = slot + 1U;
        blocks = 0U;
        first = 0U;
        for (block = 1U; block != DTFS_LAST_BLOCK + 1U; ++block) {
                if (owner_at(dir, 0U, block - 1U) != owner)
                        continue;
                ++blocks;
                if (first == 0U &&
                    dsys_dtc_read_block(unit, block, probe_block) == 0 &&
                    (unsigned int)((probe_block[0] >> DTFS_FIRST_SHIFT) &
                    DTFS_BLOCKNO_MASK) == block)
                        first = block;
        }
        if (blocks == 0U || first == 0U)
                return 0;

        block = first;
        seen = 0U;
        for (;;) {
                if (block == 0U || block > DTFS_LAST_BLOCK ||
                    owner_at(dir, 0U, block - 1U) != owner ||
                    dsys_dtc_read_block(unit, block, probe_block) != 0 ||
                    (unsigned int)((probe_block[0] >> DTFS_FIRST_SHIFT) &
                    DTFS_BLOCKNO_MASK) != first)
                        return 0;
                count = (unsigned int)(probe_block[0] & DTFS_COUNT_MASK);
                if (count > DTFS_DATA_WORDS)
                        return 0;
                next = (unsigned int)((probe_block[0] >> DTFS_NEXT_SHIFT) &
                    DTFS_BLOCKNO_MASK);
                ++seen;
                if (next == 0U)
                        return seen == blocks;
                if (count == 0U || seen == blocks)
                        return 0;
                block = next;
        }
}

static int
tenex_valid(unsigned int unit, const kword_t *dir, int deep)
{
        kword_t seen;
        unsigned int index;
        unsigned int owner;
        unsigned int slot;

        if ((unsigned int)(dir[0] >> 26U) != 01736U ||
            owner_at(dir, 0U, DTFS_DIR_BLOCK) != DTFS_TENEX_RESERVED ||
            (dir[0122] & 07777776UL) != 07777776UL)
                return 0;

        seen = 0UL;
        for (index = 0U; index != DTFS_BLOCKS; ++index) {
                owner = owner_at(dir, 0U, index);
                if (owner == 0U)
                        continue;
                if (owner < 027U) {
                        if (dir[DTFS_NAME_BASE + owner - 1U] == 0UL)
                                return 0;
                        seen |= (kword_t)1U << (owner - 1U);
                        continue;
                }
                if (owner < DTFS_TENEX_RESERVED)
                        return 0;
        }

        for (slot = 0U; slot != DTFS_TENEX_MAX_FILE; ++slot) {
                if (dir[DTFS_NAME_BASE + slot] == 0UL) {
                        if (dir[DTFS_TENEX_EXT_BASE + slot] != 0UL)
                                return 0;
                        continue;
                }
                if ((seen & ((kword_t)1U << slot)) == 0UL)
                        return 0;
                if (deep && !tenex_chain_valid(unit, dir, slot))
                        return 0;
        }
        return 1;
}

static int
its_valid(const kword_t *dir)
{
        unsigned int index;
        unsigned int owner;
        unsigned int slot;

        if ((dir[DTFS_ITS_MAP_FIRST] & ~1UL) != DTFS_ITS_MAP_RESERVED ||
            (unsigned int)(dir[DTFS_ITS_MAP_DIR] >> 31U) !=
            DTFS_ITS_DIR_OWNER ||
            (dir[DTFS_ITS_MAP_LAST] & ~1UL) != DTFS_ITS_MAP_END)
                return 0;

        for (index = 7U; index != DTFS_ITS_END_BLOCK; ++index) {
                owner = owner_at(dir, DTFS_ITS_NAME_WORDS, index);
                if (owner == DTFS_OWNER_INVALID)
                        return 0;
                if (owner == 0U || owner > DTFS_ITS_FILE_SLOTS)
                        continue;
                slot = owner - 1U;
                if (dir[slot * 2U] == 0UL && dir[slot * 2U + 1U] == 0UL)
                        return 0;
        }
        return 1;
}

int
dtfs_probe_unit(unsigned int unit, int deep)
{
        if (unit > 7U)
                return -1;
        if (dsys_dtc_read_block(unit, DTFS_DIR_BLOCK, probe_dir) == 0) {
                if (native_valid(probe_dir))
                        return SYS_DTFS_TYPE_NATIVE;
                if (tenex_valid(unit, probe_dir, deep))
                        return SYS_DTFS_TYPE_TENEX;
        }
        if (dsys_dtc_read_block(unit, DTFS_ITS_DIR_BLOCK, probe_dir) == 0 &&
            its_valid(probe_dir))
                return SYS_DTFS_TYPE_ITS;
        return -1;
}
