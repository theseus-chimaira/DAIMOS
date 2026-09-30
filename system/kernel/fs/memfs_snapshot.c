/** Optional orderly-shutdown persistence for the singleton MEMFS. */
#include "memfs.h"
#include "fs_mres.h"
#include "blockset_mres.h"
#include "swap_store.h"
#include "storage.h"

#define HALF_MASK 0777777UL
#define TYPE_SHIFT 15U
#define TYPE_MASK 07UL
#define SNAP_META_WORDS 01000UL

static int snapshot_enabled;
static kword_t snapshot_first;
static kword_t snapshot_blocks;

static kword_t checksum_words(const kword_t *p, kword_t n)
{
        kword_t sum = 0UL;
        while (n-- != 0UL)
                sum = (sum + *p++) & 0777777777777UL;
        return sum;
}

int memfs_snapshot_mount(struct memfs *fs, unsigned int flags)
{
        kword_t *h = fs_block_workspace;
        kword_t blocks;
        kword_t first;
        unsigned int slot;

        snapshot_enabled = 0;
        if ((flags & MEMFS_MOUNT_PERSIST) == 0U)
                return 0;
        if (!swap_store_enabled || swap_store_blocks == 0UL)
                return -1;
        blocks = 1UL + (SNAP_META_WORDS / DSK_WORDS_PER_SECTOR) +
            ((fs->pool_words + DSK_WORDS_PER_SECTOR - 1UL) / DSK_WORDS_PER_SECTOR);
        if (blocks >= swap_store_blocks)
                return -1;
        first = swap_store_blocks - blocks;
        if (blockset_runtime_reg_call(BLOCKSET_MRES_OP_TAIL_READ,
            swap_store_blocks - 1UL, 1UL, (kword_t)(unsigned long)h) != 0UL)
                return -1;
        if (h[0] == MEMFS_SNAPSHOT_MAGIC && h[1] == MEMFS_SNAPSHOT_VERSION &&
            h[2] == first && h[3] == blocks && h[4] == fs->pool_words) {
                if (blockset_runtime_reg_call(BLOCKSET_MRES_OP_TAIL_READ,
                    first, SNAP_META_WORDS / DSK_WORDS_PER_SECTOR,
                    (kword_t)(unsigned long)fs->nodes) != 0UL)
                        return -1;
                if (checksum_words((kword_t *)fs->nodes, SNAP_META_WORDS) != h[5])
                        return -1;
                fs->used_words = 0U;
                for (slot = 1U; slot < fs->node_count; ++slot) {
                        struct memfs_node *np = &fs->nodes[slot];
                        kword_t words = np->data & HALF_MASK;
                        if ((np->meta & MEMFS_F_USED) != 0UL &&
                            ((np->meta >> TYPE_SHIFT) & TYPE_MASK) == VFS_TYPE_REG &&
                            words != 0UL) {
                                fs->used_words += (unsigned int)words;
                                np->data &= HALF_MASK;
                        }
                }
        } else {
                kword_t b;
                kword_t free_blocks = swap_store_blocks - swap_store_blocks_used;
                kword_t reserve = swap_store_blocks < 0200UL ? swap_store_blocks : 0200UL;
                if (free_blocks <= reserve || blocks > free_blocks - reserve)
                        return -1;
                for (b = first; b < first + blocks; ++b) {
                        unsigned int word = (unsigned int)(b / 36UL);
                        unsigned int bit = (unsigned int)(b % 36UL);
                        kword_t mask = (kword_t)1U << bit;
                        if ((swap_store_bitmap[word] & mask) != 0UL)
                                return -1;
                }
                for (b = first; b < first + blocks; ++b)
                        swap_store_bitmap[b / 36UL] |= (kword_t)1U << (b % 36UL);
                swap_store_blocks_used += blocks;
        }
        snapshot_first = first;
        snapshot_blocks = blocks;
        snapshot_enabled = 1;
        return 0;
}

int memfs_snapshot_shutdown(void)
{
        struct memfs *fs;
        kword_t *h = fs_block_workspace;
        kword_t next;
        unsigned int slot;

        extern struct memfs memfs_mres_fs;
        fs = &memfs_mres_fs;
        if (!snapshot_enabled || fs->nodes == 0)
                return 0;
        h[0] = 0UL;
        if (blockset_runtime_reg_call(BLOCKSET_MRES_OP_TAIL_WRITE,
            swap_store_blocks - 1UL, 1UL, (kword_t)(unsigned long)h) != 0UL)
                return -1;
        next = snapshot_first + SNAP_META_WORDS / DSK_WORDS_PER_SECTOR;
        for (slot = 1U; slot < fs->node_count; ++slot) {
                struct memfs_node *np = &fs->nodes[slot];
                kword_t words = np->data & HALF_MASK;
                kword_t blocks;
                if ((np->meta & MEMFS_F_USED) == 0UL ||
                    ((np->meta >> TYPE_SHIFT) & TYPE_MASK) != VFS_TYPE_REG || words == 0UL)
                        continue;
                if (memfs_data_ensure(fs, slot) != 0)
                        return -1;
                blocks = (words + DSK_WORDS_PER_SECTOR - 1UL) / DSK_WORDS_PER_SECTOR;
                if (next + blocks >= swap_store_blocks)
                        return -1;
                if (blockset_runtime_reg_call(BLOCKSET_MRES_OP_TAIL_WRITE,
                    next, blocks, (np->data >> 18U) & HALF_MASK) != 0UL)
                        return -1;
                fs->pool[slot] = ((next & HALF_MASK) << 18U) | blocks;
                next += blocks;
        }
        if (blockset_runtime_reg_call(BLOCKSET_MRES_OP_TAIL_WRITE,
            snapshot_first, SNAP_META_WORDS / DSK_WORDS_PER_SECTOR,
            (kword_t)(unsigned long)fs->nodes) != 0UL)
                return -1;
        h[0] = MEMFS_SNAPSHOT_MAGIC;
        h[1] = MEMFS_SNAPSHOT_VERSION;
        h[2] = snapshot_first;
        h[3] = snapshot_blocks;
        h[4] = fs->pool_words;
        h[5] = checksum_words((kword_t *)fs->nodes, SNAP_META_WORDS);
        return blockset_runtime_reg_call(BLOCKSET_MRES_OP_TAIL_WRITE,
            swap_store_blocks - 1UL, 1UL, (kword_t)(unsigned long)h) == 0UL ? 0 : -1;
}
