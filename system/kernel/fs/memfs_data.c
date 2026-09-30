/**
 * @file memfs_data.c
 * @brief Demand allocator for mutable MEMFS file contents.
 *
 * MEMFS owns at most four MM chunks and suballocates file extents inside them.
 * Free-list headers live in free storage, so an empty filesystem carries no
 * data-allocation metadata outside this small MRES state.  A file occupies one
 * contiguous extent; growth may relocate that file but never compacts other
 * files.  Completely free chunks are returned immediately to MM.
 */
#include "memfs.h"
#include "mm.h"
#include "fs_mres.h"
#include "storage.h"
#include "blockset_mres.h"
#include "swap_store.h"

#define MEMFS_DATA_CHUNKS       4U
#define MEMFS_DATA_CHUNK_WORDS  02000UL
#define MEMFS_DATA_MM_OWNER     011U
#define MEMFS_HALF_MASK         0777777UL

struct memfs_data_chunk {
        kword_t base;
        kword_t words;
        kword_t free;
};

static struct memfs_data_chunk memfs_data_chunks[MEMFS_DATA_CHUNKS];
static struct memfs *memfs_data_fs;
static kword_t memfs_data_limit;
static kword_t memfs_data_capacity;
static int memfs_data_allocating;

static kword_t
memfs_alloc_words(kword_t words)
{
        if (words < DSK_WORDS_PER_SECTOR)
                return DSK_WORDS_PER_SECTOR;
        return ((words + DSK_WORDS_PER_SECTOR - 1UL) /
            DSK_WORDS_PER_SECTOR) * DSK_WORDS_PER_SECTOR;
}

void
memfs_data_init(struct memfs *fs, kword_t limit)
{
        unsigned int i;

        for (i = 0U; i < MEMFS_DATA_CHUNKS; ++i) {
                memfs_data_chunks[i].base = 0UL;
                memfs_data_chunks[i].words = 0UL;
                memfs_data_chunks[i].free = 0UL;
        }
        memfs_data_fs = fs;
        memfs_data_limit = limit;
        memfs_data_capacity = 0UL;
}

static int
memfs_chunk_alloc(struct memfs_data_chunk *cp, kword_t need, kword_t *basep)
{
        kword_t cur;
        kword_t prev;

        prev = 0UL;
        for (cur = cp->free; cur != 0UL; cur = ((kword_t *)(unsigned long)cur)[1]) {
                kword_t *hp;
                kword_t size;

                hp = (kword_t *)(unsigned long)cur;
                size = hp[0];
                if (size >= need) {
                        if (size == need) {
                                if (prev == 0UL)
                                        cp->free = hp[1];
                                else
                                        ((kword_t *)(unsigned long)prev)[1] = hp[1];
                                *basep = cur;
                        } else {
                                hp[0] = size - need;
                                *basep = cur + size - need;
                        }
                        return 0;
                }
                prev = cur;
        }
        return -1;
}

static int
memfs_data_alloc(kword_t words, kword_t *basep)
{
        struct memfs_data_chunk *cp;
        kword_t base;
        kword_t chunk_words;
        kword_t need;
        kword_t remaining;
        int rc;
        unsigned int i;

        need = memfs_alloc_words(words);
        for (i = 0U; i < MEMFS_DATA_CHUNKS; ++i) {
                cp = &memfs_data_chunks[i];
                if (cp->base != 0UL && memfs_chunk_alloc(cp, need, basep) == 0)
                        return 0;
        }
        if (need > memfs_data_limit - memfs_data_capacity)
                return -1;
        for (i = 0U; i < MEMFS_DATA_CHUNKS; ++i) {
                if (memfs_data_chunks[i].base == 0UL)
                        break;
        }
        if (i == MEMFS_DATA_CHUNKS)
                return -1;
        remaining = memfs_data_limit - memfs_data_capacity;
        chunk_words = need > MEMFS_DATA_CHUNK_WORDS ? need : MEMFS_DATA_CHUNK_WORDS;
        if (chunk_words > remaining)
                chunk_words = (remaining / DSK_WORDS_PER_SECTOR) * DSK_WORDS_PER_SECTOR;
        if (chunk_words < need)
                return -1;
        memfs_data_allocating = 1;
        rc = mm_alloc(chunk_words, MM_TYPE_KERNEL_DYNAMIC,
            MEMFS_DATA_MM_OWNER, MM_ALLOC_LOW, &base);
        memfs_data_allocating = 0;
        if (rc != MM_OK)
                return -1;
        cp = &memfs_data_chunks[i];
        cp->base = base;
        cp->words = chunk_words;
        cp->free = base;
        ((kword_t *)(unsigned long)base)[0] = chunk_words;
        ((kword_t *)(unsigned long)base)[1] = 0UL;
        memfs_data_capacity += chunk_words;
        return memfs_chunk_alloc(cp, need, basep);
}

static void
memfs_data_free(kword_t base, kword_t words)
{
        struct memfs_data_chunk *cp;
        kword_t cur;
        kword_t next;
        kword_t prev;
        kword_t size;
        kword_t *hp;
        unsigned int i;

        size = memfs_alloc_words(words);
        for (i = 0U; i < MEMFS_DATA_CHUNKS; ++i) {
                cp = &memfs_data_chunks[i];
                if (cp->base != 0UL && base >= cp->base &&
                    base + size <= cp->base + cp->words)
                        break;
        }
        if (i == MEMFS_DATA_CHUNKS)
                return;
        prev = 0UL;
        cur = cp->free;
        while (cur != 0UL && cur < base) {
                prev = cur;
                cur = ((kword_t *)(unsigned long)cur)[1];
        }
        hp = (kword_t *)(unsigned long)base;
        hp[0] = size;
        hp[1] = cur;
        if (prev == 0UL)
                cp->free = base;
        else
                ((kword_t *)(unsigned long)prev)[1] = base;

        next = hp[1];
        if (next != 0UL && base + hp[0] == next) {
                hp[0] += ((kword_t *)(unsigned long)next)[0];
                hp[1] = ((kword_t *)(unsigned long)next)[1];
        }
        if (prev != 0UL) {
                kword_t *pp;

                pp = (kword_t *)(unsigned long)prev;
                if (prev + pp[0] == base) {
                        pp[0] += hp[0];
                        pp[1] = hp[1];
                        hp = pp;
                }
        }
        if (cp->free == cp->base && hp == (kword_t *)(unsigned long)cp->base &&
            hp[0] == cp->words && hp[1] == 0UL) {
                kword_t chunk_base;
                kword_t chunk_words;

                chunk_base = cp->base;
                chunk_words = cp->words;
                cp->base = 0UL;
                cp->words = 0UL;
                cp->free = 0UL;
                memfs_data_capacity -= chunk_words;
                (void)mm_free(chunk_base, MM_TYPE_KERNEL_DYNAMIC,
                    MEMFS_DATA_MM_OWNER);
        }
}

static void
memfs_backing_drop(unsigned int slot)
{
        kword_t span;

        if (memfs_data_fs == 0 || memfs_data_fs->pool == 0)
                return;
        span = memfs_data_fs->pool[slot];
        if (span != 0UL) {
                swap_store_free((span >> 18U) & MEMFS_HALF_MASK,
                    span & MEMFS_HALF_MASK);
                memfs_data_fs->pool[slot] = 0UL;
        }
}

int
memfs_data_ensure(struct memfs *fs, unsigned int slot)
{
        struct memfs_node *np;
        kword_t base;
        kword_t span;
        kword_t words;

        if (fs == 0 || fs != memfs_data_fs || slot >= fs->node_count)
                return -1;
        np = &fs->nodes[slot];
        words = np->data & MEMFS_HALF_MASK;
        if (words == 0UL || ((np->data >> 18U) & MEMFS_HALF_MASK) != 0UL)
                return 0;
        span = fs->pool[slot];
        if (span == 0UL || memfs_data_alloc(words, &base) != 0)
                return -1;
        if (blockset_runtime_reg_call(BLOCKSET_MRES_OP_TAIL_READ,
            (span >> 18U) & MEMFS_HALF_MASK, span & MEMFS_HALF_MASK, base) != 0UL) {
                memfs_data_free(base, words);
                return -1;
        }
        np->data = ((base & MEMFS_HALF_MASK) << 18U) | words;
        return 0;
}

void
memfs_data_dirty(unsigned int slot)
{
        if (memfs_data_fs != 0 && slot < memfs_data_fs->node_count)
                memfs_backing_drop(slot);
}

int
memfs_resize(struct memfs *fs, unsigned int slot, unsigned int words)
{
        struct memfs_node *np;
        kword_t base;
        kword_t oldbase;
        unsigned int copy;
        unsigned int oldwords;
        unsigned int i;

        if (fs == 0 || slot >= fs->node_count || words > MEMFS_HALF_MASK)
                return -1;
        np = &fs->nodes[slot];
        if ((np->meta & 06UL) != MEMFS_F_WRITABLE)
                return -1;
        oldwords = (unsigned int)(np->data & MEMFS_HALF_MASK);
        if (oldwords != 0U && memfs_data_ensure(fs, slot) != 0)
                return -1;
        oldbase = (np->data >> 18U) & MEMFS_HALF_MASK;
        if (words == oldwords)
                return 0;
        if (words == 0U) {
                if (oldbase != 0UL)
                        memfs_data_free(oldbase, oldwords);
                memfs_backing_drop(slot);
                np->data = 0UL;
                fs->used_words -= oldwords;
                return 0;
        }
        if (memfs_data_alloc((kword_t)words, &base) != 0)
                return -1;
        copy = oldwords < words ? oldwords : words;
        for (i = 0U; i < copy; ++i)
                ((kword_t *)(unsigned long)base)[i] =
                    ((kword_t *)(unsigned long)oldbase)[i];
        for (; (kword_t)i < memfs_alloc_words((kword_t)words); ++i)
                ((kword_t *)(unsigned long)base)[i] = 0UL;
        if (oldbase != 0UL)
                memfs_data_free(oldbase, oldwords);
        memfs_backing_drop(slot);
        np->data = ((base & MEMFS_HALF_MASK) << 18U) |
            ((kword_t)words & MEMFS_HALF_MASK);
        fs->used_words = fs->used_words - oldwords + words;
        return 0;
}

static kword_t
memfs_evict_chunk(struct memfs_data_chunk *cp)
{
        struct memfs *fs;
        unsigned int slot;

        fs = memfs_data_fs;
        if (fs == 0 || cp->base == 0UL)
                return 0UL;
        for (slot = 1U; slot < fs->node_count; ++slot) {
                struct memfs_node *np;
                kword_t base;
                kword_t blocks;
                kword_t first;
                kword_t words;

                np = &fs->nodes[slot];
                base = (np->data >> 18U) & MEMFS_HALF_MASK;
                words = np->data & MEMFS_HALF_MASK;
                if (words == 0UL || base < cp->base ||
                    base >= cp->base + cp->words || fs->pool[slot] != 0UL)
                        continue;
                blocks = memfs_alloc_words(words) / DSK_WORDS_PER_SECTOR;
                if (swap_store_alloc_memfs(blocks, &first) != 0)
                        return 0UL;
                if (blockset_runtime_reg_call(BLOCKSET_MRES_OP_TAIL_WRITE,
                    first, blocks, base) != 0UL) {
                        swap_store_free(first, blocks);
                        return 0UL;
                }
                fs->pool[slot] = ((first & MEMFS_HALF_MASK) << 18U) | blocks;
        }
        for (slot = 1U; slot < fs->node_count; ++slot) {
                struct memfs_node *np;
                kword_t base;

                np = &fs->nodes[slot];
                base = (np->data >> 18U) & MEMFS_HALF_MASK;
                if (base >= cp->base && base < cp->base + cp->words)
                        np->data &= MEMFS_HALF_MASK;
        }
        {
                kword_t base;
                kword_t words;

                base = cp->base;
                words = cp->words;
                if (mm_free(base, MM_TYPE_KERNEL_DYNAMIC,
                    MEMFS_DATA_MM_OWNER) != MM_OK)
                        return 0UL;
                cp->base = cp->words = cp->free = 0UL;
                memfs_data_capacity -= words;
                return words;
        }
}

kword_t
memfs_data_reclaim(kword_t wanted)
{
        kword_t released;
        unsigned int i;

        if (memfs_data_allocating || swap_store_blocks == 0UL)
                return 0UL;
        released = 0UL;
        for (i = 0U; i < MEMFS_DATA_CHUNKS && released < wanted; ++i)
                released += memfs_evict_chunk(&memfs_data_chunks[i]);
        return released;
}

void
memfs_data_destroy(void)
{
        unsigned int i;

        if (memfs_data_fs != 0)
                for (i = 0U; i < memfs_data_fs->node_count; ++i)
                        memfs_backing_drop(i);
        for (i = 0U; i < MEMFS_DATA_CHUNKS; ++i) {
                if (memfs_data_chunks[i].base != 0UL)
                        (void)mm_free(memfs_data_chunks[i].base,
                            MM_TYPE_KERNEL_DYNAMIC, MEMFS_DATA_MM_OWNER);
        }
        memfs_data_init(0, 0UL);
}
