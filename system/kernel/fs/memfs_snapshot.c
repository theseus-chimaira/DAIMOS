/** Optional orderly-shutdown persistence for the singleton MEMFS. */
#include "memfs.h"
#include "fs_mres.h"
#include "bstore.h"
#include "bcache.h"
#include "storage.h"

#define HALF_MASK 0777777UL
#define TYPE_SHIFT 15U
#define TYPE_MASK 07UL
#define SNAP_META_WORDS MEMFS_METADATA_WORDS

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
        if (!backstore_enabled || backstore_blocks == 0UL)
                return -1;
        bcache_workspace_invalidate();
        blocks = 1UL + (SNAP_META_WORDS / DSK_WORDS_PER_SECTOR) +
            ((fs->pool_words + DSK_WORDS_PER_SECTOR - 1UL) / DSK_WORDS_PER_SECTOR);
        if (blocks >= backstore_blocks)
                return -1;
        first = backstore_blocks - blocks;
        if (backstore_read(backstore_blocks - 1UL, 1UL, h) != 0)
                return -1;
        if (h[0] == MEMFS_SNAPSHOT_MAGIC && h[1] == MEMFS_SNAPSHOT_VERSION &&
            h[2] == first && h[3] == blocks && h[4] == fs->pool_words) {
                if (backstore_read(first,
                    SNAP_META_WORDS / DSK_WORDS_PER_SECTOR,
                    (kword_t *)(unsigned long)fs->nodes) != 0)
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
                kword_t free_blocks = backstore_blocks - backstore_blocks_used;
                kword_t reserve = backstore_blocks < 0200UL ? backstore_blocks : 0200UL;
                if (free_blocks <= reserve || blocks > free_blocks - reserve)
                        return -1;
                for (b = first; b < first + blocks; ++b) {
                        unsigned int word = (unsigned int)(b / 36UL);
                        unsigned int bit = (unsigned int)(b % 36UL);
                        kword_t mask = (kword_t)1U << bit;
                        if ((backstore_bitmap[word] & mask) != 0UL)
                                return -1;
                }
                for (b = first; b < first + blocks; ++b)
                        backstore_bitmap[b / 36UL] |= (kword_t)1U << (b % 36UL);
                backstore_blocks_used += blocks;
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
        bcache_workspace_invalidate();
        h[0] = 0UL;
        if (backstore_write(backstore_blocks - 1UL, 1UL, h) != 0)
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
                if (next + blocks >= backstore_blocks)
                        return -1;
                if (backstore_write(next, blocks,
                    (const kword_t *)(unsigned long)
                    ((np->data >> 18U) & HALF_MASK)) != 0)
                        return -1;
                fs->pool[slot] = ((next & HALF_MASK) << 18U) | blocks;
                next += blocks;
        }
        if (backstore_write(snapshot_first,
            SNAP_META_WORDS / DSK_WORDS_PER_SECTOR,
            (const kword_t *)(unsigned long)fs->nodes) != 0)
                return -1;
        h[0] = MEMFS_SNAPSHOT_MAGIC;
        h[1] = MEMFS_SNAPSHOT_VERSION;
        h[2] = snapshot_first;
        h[3] = snapshot_blocks;
        h[4] = fs->pool_words;
        h[5] = checksum_words((kword_t *)fs->nodes, SNAP_META_WORDS);
        return backstore_write(backstore_blocks - 1UL, 1UL, h);
}
